// <iostream>은 표준 출력 객체 std::cout과 스트림 삽입 연산자 operator<<를 선언한다.
#include <iostream>
// <ranges>는 C++23의 std::views::slide, slide_view, ref_view와 범위 타입 별칭을 선언한다.
#include <ranges>
// <utility>는 실제 이동을 허용하는 xvalue 변환 함수 std::move를 선언한다.
#include <utility>
// <vector>는 int 원소를 연속 메모리에 소유하는 std::vector를 선언한다.
#include <vector>

// struct는 기본 접근 지정자가 public이므로 계산 결과처럼 단순한 값 묶음에 알맞다.
// int는 여기서 밀리초와 개수를 담는 기본 정수 타입이다. 멤버의 {}는 0으로 값 초기화한다.
struct LatencyReport {
    int budget_breaches{};
    int worst_total_ms{};
};

// class는 기본 접근이 private이다. public:에는 안전한 사용법만 열고 private:에는 실제 owner를 숨긴다.
// 이 클래스가 vector의 수명을 책임지므로, 밖으로 내보내는 slide_view는 history보다 오래 살아서는 안 된다.
class ServiceLatencyHistory final {
public:
    // explicit은 vector xvalue가 ServiceLatencyHistory로 뜻하지 않게 암시 변환되는 것을 막는다.
    // && 매개변수는 임시 또는 std::move로 표시한 xvalue에만 바인딩된다. 이름이 붙은 latency_samples는
    // 함수 안에서는 lvalue이므로, 멤버 초기화 목록에서 다시 std::move를 써야 vector 저장소가 이동한다.
    // [첫 호출 계약: std::move와 std::vector<int> 이동 생성자]
    // (1) 수신 객체 latency_ms_는 아직 생성 중인 std::vector<int> 멤버이고, latency_samples는 호출자가
    //     소유권 이전을 허용한 유효한 std::vector<int> 객체를 가리키는 이름 있는 rvalue 참조다.
    // (2) std::move<T>(T&&)에서 T=std::vector<int>&가 추론되어
    //     std::remove_reference_t<T>&&를 반환하고, vector(vector&&) noexcept 이동 생성자가 선택된다.
    // (3) latency_samples 식은 이름 때문에 lvalue이고 std::move(latency_samples)는 같은 객체를 나타내는
    //     xvalue다. int 원소 저장소 소유권을 넘기는 뜻이며 별도 값 범위 제한은 없다.
    // (4) std::move의 반환형 std::vector<int>&&는 멤버 생성에 즉시 사용한다. 생성자는 반환값이 없다.
    // (5) 성공 후 latency_ms_가 저장소를 소유하고 원본 vector는 유효하지만 내용이 미지정인 이동 후 상태다.
    // (6) 기본 allocator가 같은 vector 이동은 O(1)·noexcept이고, 기존 원소 포인터/참조/반복자는 새 owner의
    //     같은 원소를 계속 가리킨다. 멤버 수명은 history 수명에 묶이고 자체 스레드 동기화는 없다.
    explicit ServiceLatencyHistory(std::vector<int>&& latency_samples) noexcept
        : latency_ms_{std::move(latency_samples)} {}

    // owner의 주소와 수명을 고정하면 이미 만든 비소유 view를 실수로 불안정하게 만들 가능성이 줄어든다.
    // 복사와 이동은 모두 컴파일 단계에서 금지한다. vector 자체의 이동은 위 생성 시 한 번만 일어난다.
    ServiceLatencyHistory(const ServiceLatencyHistory&) = delete;
    ServiceLatencyHistory& operator=(const ServiceLatencyHistory&) = delete;
    ServiceLatencyHistory(ServiceLatencyHistory&&) = delete;
    ServiceLatencyHistory& operator=(ServiceLatencyHistory&&) = delete;

    // using은 새 타입을 만들지 않고 긴 템플릿 특수화에 읽기 쉬운 별칭을 붙인다.
    // 템플릿 인자 const std::vector<int>는 원소 수정이 금지된 vector를 참조한다는 뜻이다.
    using BaseView = std::ranges::ref_view<const std::vector<int>>;
    using Windows = std::ranges::slide_view<BaseView>;
    // range_reference_t<Windows>는 slide 반복자를 역참조할 때 나오는 "한 창을 보는 경량 view 값"이다.
    using Window = std::ranges::range_reference_t<Windows>;
    using Difference = std::ranges::range_difference_t<const std::vector<int>>;

    // 뒤의 const는 owner/원소를 바꾸지 않음을, & 한정자는 *this가 살아 있는 lvalue일 때만 호출됨을 뜻한다.
    [[nodiscard]] Windows windows() const & {
        // [첫 호출 계약: std::views::slide(range, count)]
        // (1) 논리적 수신 범위는 살아 있는 이 객체가 소유한 정확한 타입 const std::vector<int>이고,
        //     호출 전 원소가 유효하며 구조 변경 중이 아니다. std::views::slide는 stateless 함수 객체다.
        // (2) views::slide.operator()<R>(R&&, range_difference_t<R>)에서
        //     R=const std::vector<int>&가 선택되고 결과형은 slide_view<ref_view<const vector<int>>>, 즉 Windows다.
        // (3) latency_ms_는 const lvalue라 빌릴 뿐 소유권을 넘기지 않는다. window_width는 Difference 타입의
        //     const lvalue, 값 3이며 값 매개변수로 복사된다. 표준 전제조건 count > 0을 만족한다.
        // (4) 반환 Windows prvalue는 호출부 결과를 직접 초기화한다. 각 역참조 결과는 원소 복사가 아닌 Window다.
        // (5) vector와 인자는 변하지 않고, 결과 view가 owner를 가리키는 참조와 창 길이만 저장한다.
        // (6) 구성은 O(1)·무할당이다. count<=0이면 전제조건 위반이다. owner 파괴 뒤 view 사용은 UB다. 이 구체
        //     ref_view 기반 view 자체는 vector 객체를 계속 가리키므로 재할당 뒤 새 begin/end를 얻을 수 있지만,
        //     재할당 전에 얻은 반복자·Window·원소 참조는 무효다. 동시 쓰기는 데이터 경쟁이며, 이 타입의 실질
        //     연산은 던질 작업이 없어도 어댑터 호출 전반을 무조건 noexcept라고 확대 해석하지 않는다.
        return std::views::slide(latency_ms_, window_width);
    }

    // 임시 owner는 전체 식 끝에 파괴된다. 이 overload 삭제로 ServiceLatencyHistory{...}.windows()처럼
    // 곧 댕글링할 view를 만드는 식을 컴파일 오류로 막는다.
    [[nodiscard]] Windows windows() const && = delete;

private:
    // 3개 연속 관측치를 한 창으로 본다. constexpr 정수는 컴파일 시간 상수이며 저장을 생략할 수도 있다.
    static constexpr Difference window_width{3};
    // 이 vector가 실제 int 원소의 단독 owner다. slide_view는 이 멤버를 소유하지 않는다.
    std::vector<int> latency_ms_;
};

// 반환형 LatencyReport는 두 계산 결과를 값으로 돌려준다. const ServiceLatencyHistory&는 복사 없이
// 반드시 존재하는 객체에 붙는 읽기 전용 lvalue 참조다. 포인터(const ServiceLatencyHistory*)와 달리
// null 상태를 표현하지 않으며, 함수는 owner의 수명을 연장하지 않는다. budget_ms는 작은 int 값 복사다.
// 호출 전제조건은 각 3개 지연시간의 합이 int 표현 범위 안이라는 것이다. 이를 넘는 signed 덧셈은 UB다.
[[nodiscard]] LatencyReport analyze_latency(
    const ServiceLatencyHistory& history,
    const int budget_ms) {
    LatencyReport report{};
    // bool은 참/거짓 기본 타입이고 {} 값 초기화는 false다. 첫 창 여부를 따로 기억해야 모든 창 합이
    // 음수여도 0이 아니라 실제 최대 합을 기록할 수 있다. 창이 하나도 없을 때만 결과 0을 유지한다.
    bool has_window{};

    // [첫 호출 계약: slide_view range-for의 숨은 begin/end/비교/역참조/증가와 Window 초기화]
    // (1) 숨은 수신 범위는 history.windows()가 만든 정확한 타입 ServiceLatencyHistory::Windows의 임시다.
    //     range-for의 auto&& __range가 루프 끝까지 그 view 수명을 연장하고 history owner도 살아 있다.
    // (2) simple-view인 BaseView에 대해 Windows::begin() const와 end() const가 선택된다. slide iterator는
    //     operator==만 선언하므로 소스의 != 조건은 rewritten candidate인 !(current==end)로 해석된다. 이어
    //     operator*() const/operator++()를 쓰고 역참조 prvalue로 Window를 직접 초기화한다.
    // (3) begin/end/*/++에는 명시 데이터 인자가 없다. 비교는 현재/끝 iterator lvalue를 비소유로 읽고,
    //     증가는 끝이 아닌 현재 iterator에만 허용된다. window는 원소가 아니라 경량 view 값만 받는다.
    // (4) begin/end는 iterator, 비교는 bool, 역참조는 Window, 전위 증가는 iterator&를 반환한다. 숨은 루프가
    //     모두 소비하고 Window 값만 현재 반복의 window에 저장한다.
    // (5) 매 반복마다 현재 iterator와 window 경계만 전진한다. vector 원소/크기/용량과 history는 변하지 않는다.
    // (6) 각 연산은 O(1)이고 동적 할당을 요구하지 않으며 전체 창 수는 max(N-3+1,0)이다. 끝 iterator
    //     역참조/증가, owner 파괴나 구조 변경으로 무효화된 관찰자 사용은 UB다. 예외 명세는 선택된 표준
    //     iterator 연산을 따른다. slide iterator의 비-const 연산이 예외로 끝나면 그 iterator는 singular 상태가
    //     된다. 동시 쓰기와는 안전하지 않으며 관찰자는 유효 구간보다 오래 보관하지 않는다.
    for (const ServiceLatencyHistory::Window window : history.windows()) {
        int total_ms{};

        // [첫 호출 계약: Window range-for의 숨은 begin/end/비교/역참조/증가]
        // (1) 수신 객체는 현재 세 int를 비소유로 가리키는 정확한 별칭 타입
        //     const ServiceLatencyHistory::Window이며 owner와 바깥 slide iterator가 유효하다.
        // (2) Window::begin() const/end() const, 소스의 !=를 만족하는 동등 비교, operator*() const와
        //     operator++()가 선택된다. vector iterator가 contiguous_iterator라 창은 표준상 span<const int>
        //     계열이지만, !=가 직접 연산자인지 ==의 rewritten candidate인지는 구현 반복자 타입을 따른다.
        // (3) 명시 인자는 없고 비교 iterator들은 비소유 lvalue다. 끝이 아닌 iterator만 역참조/증가하며
        //     역참조한 const int lvalue 값이 latency_ms에 복사된다. 소유권 이전은 없다.
        // (4) begin/end는 iterator, 비교는 bool, 역참조는 const int&, 증가는 iterator&를 반환하며 루프가 쓴다.
        // (5) iterator만 전진하고 window/history/원소는 그대로다. latency_ms는 매 반복 독립된 int 값이다.
        // (6) 각 연산은 O(1)이고 동적 할당을 요구하지 않으며 이 창은 정확히 3회 돈다. 예외 명세는 선택된
        //     표준 iterator 연산을 따른다. past-the-end/무효 관찰자 사용은 UB고 동시 원본 쓰기는 데이터 경쟁이다.
        for (const int latency_ms : window) {
            total_ms += latency_ms;
        }

        // 기계 관점에서는 원소 load와 정수 add, budget load/비교, 조건 분기, 결과 store가 될 수 있다.
        // 다만 실제 명령, 벡터화, 분기 제거 여부는 CPU·컴파일러·최적화 옵션에 따라 달라진다.
        // 가상 함수가 없으므로 가상 간접 호출은 필요하지 않지만 구현 세부를 특정 어셈블리로 단정하지 않는다.
        if (total_ms > budget_ms) {
            ++report.budget_breaches;
        }
        // ||는 왼쪽부터 단락 평가한다. 첫 창이면 오른쪽 비교 없이 최대값을 세우고, 이후에는 더 큰 합만 저장한다.
        if (!has_window || total_ms > report.worst_total_ms) {
            report.worst_total_ms = total_ms;
            has_window = true;
        }
    }

    // report 식은 이름 있는 lvalue지만 반환 대상이므로 NRVO(RVO 계열) 후보이다. 적용되면 복사/이동 없이
    // 호출자 결과 객체에 직접 만들어지고, 미적용이어도 이 작은 struct의 암시적 이동/복사는 값 두 개뿐이다.
    return report;
}

int main() {
    // [첫 생성 계약: std::vector<int> initializer_list 생성자]
    // (1) 수신 객체 latency_samples는 아직 수명이 시작되지 않은 std::vector<int, std::allocator<int>>다.
    // (2) vector(std::initializer_list<int>, const allocator_type& = allocator_type())와 생략 인자용
    //     std::allocator<int>::allocator() noexcept가 선택된다.
    // (3) 다섯 int prvalue {42,55,61,38,77}를 담은 initializer_list를 읽어 복사한다. 생략한 두 번째 인자는
    //     std::allocator<int>{} prvalue로 평가되어 const allocator_type&에 바인딩되며 외부 저장소 owner는 없다.
    // (4) 두 생성자는 반환값이 없다. allocator 임시가 vector 생성 인자로 쓰이고 latency_samples가 완성된다.
    // (5) 성공 뒤 size=5이며 vector가 저장소를 단독 소유한다. allocator 임시는 전체 식 끝에 소멸하지만
    //     저장소와 vector는 initializer_list/allocator 객체 수명과 독립이다.
    // (6) 원소 복사·저장소는 O(5)이고 실패 시 std::bad_alloc 등이 전파된다. allocator 임시의 생성·소멸은
    //     O(1)·무할당·비투척이다. 아직 무효화할 관찰자는 없고 vector는 자체 동기화를 제공하지 않는다.
    std::vector<int> latency_samples{42, 55, 61, 38, 77};

    // latency_samples는 lvalue, std::move(latency_samples)는 xvalue다. std::move 자체는 load나 이동을 하지 않고
    // 위 생성자가 vector 이동 생성자를 선택하게 한다. owner는 history로 바뀌며 원본은 내용 미지정 상태다.
    ServiceLatencyHistory history{std::move(latency_samples)};

    // analyze_latency(...) 호출식은 prvalue 결과다. report는 그 결과로 직접 초기화되며, 함수 안의 NRVO가
    // 적용되면 같은 객체가 처음부터 여기 만들어진다. const는 이후 결과 수정을 막는다.
    const LatencyReport report{analyze_latency(history, 150)};

    // [첫 호출 계약: std::ostream 삽입 operator<< 연쇄]
    // (1) 수신 객체 std::cout은 프로그램 시작 시 구성된 정확한 타입 std::ostream 전역 객체이며 사용 가능하다.
    // (2) Traits=std::char_traits<char>다. 문자열은 비멤버 function template
    //     `template<class Traits> std::basic_ostream<char, Traits>& operator<<(
    //     std::basic_ostream<char, Traits>&, const char*)`, int는 같은 basic_ostream의 멤버
    //     `std::basic_ostream<char, Traits>& operator<<(int)`, '\n'은 같은 Traits의 char 비멤버 template을
    //     선택한다. 순서는 문자열/int/문자열/int/char이고 앞 반환 stream이 다음 수신자가 된다.
    // (3) 문자열 리터럴은 null 종료 const char 배열 lvalue에서 비소유 포인터로 변환되고, 두 report 멤버는
    //     int lvalue 값으로 읽으며 '\n'은 char prvalue다. 어느 인자도 소유권을 넘기지 않는다.
    // (4) 각 삽입은 std::ostream&를 반환해 다음 삽입의 수신자로 사용하고 마지막 반환 참조는 버린다.
    // (5) 출력 버퍼와 상태 비트/위치는 바뀔 수 있지만 report와 history는 변하지 않는다.
    // (6) 형식 변환·locale·버퍼링 비용이 있으며 표준은 단순한 한 복잡도식을 보장하지 않는다. 실패는 기본적으로
    //     상태 비트에 기록되고 exceptions mask 설정 시 ios_base::failure가 날 수 있다. 기본 동기화 상태의
    //     std::cout에 여러 스레드가 삽입해도 그 접근 자체는 data race를 만들지 않지만 문자가 섞일 수 있고
    //     레코드 단위 원자성은 없다. 컨테이너 관찰자 무효화는 없다.
    std::cout << "breaches=" << report.budget_breaches
              << ",worst_total_ms=" << report.worst_total_ms << '\n';

    // [첫 숨은 소멸 계약: 비소유 range 관찰자와 std::vector<int>]
    // (1) 각 range-for 종료 시 Windows/Window와 구현 iterator·sentinel은 유효한 비소유 관찰자이고, main
    //     종료 시 latency_samples와 history의 latency_ms_는 유효한 std::vector<int> 객체다.
    // (2) 각 타입의 소멸자는 인자와 반환값이 없다. vector에는 `~vector()`가 선택되고, slide_view/ref_view,
    //     span 계열 창과 구현 iterator·sentinel에는 해당 구현 타입의 소멸자가 선택된다.
    // (3) 소멸에 데이터 인자나 소유권 입력은 없다. 비소유 관찰자는 owner를 소유하지 않는다.
    // (4) 소멸자는 값을 반환하지 않으며 호출부가 소비할 결과도 없다.
    // (5) 관찰자 소멸은 vector를 바꾸지 않는다. vector 소멸은 int 원소 수명을 끝내고 저장소를 반환하며 그
    //     원소의 모든 포인터·참조·반복자를 무효화한다. moved-from latency_samples도 유효하게 소멸한다.
    // (6) 관찰자 소멸은 O(1)·무할당이고, vector 소멸은 원소 수에 선형이며 저장소를 해제한다. int와 기본
    //     allocator 경로는 예외를 던지지 않는다. 소멸과 같은 객체의 동시 접근은 외부 동기화 없이는 안전하지 않다.

    // main 끝에 return을 생략하면 C++가 return 0;으로 간주한다.
}
