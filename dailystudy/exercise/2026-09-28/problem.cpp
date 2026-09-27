// <iostream>은 점검 결과를 출력할 std::cout과 operator<<를 선언한다.
#include <iostream>
// <ranges>는 연속 센서 값을 겹치는 창으로 보는 C++23 std::views::slide를 선언한다.
#include <ranges>
// <utility>는 vector 저장소 소유권 이전을 표시하는 std::move를 선언한다.
#include <utility>
// <vector>는 진동 측정값을 연속 저장하고 소유하는 std::vector를 선언한다.
#include <vector>

// struct의 멤버는 기본 public이다. 두 int의 {} 초기화는 각각 0을 만들며 미초기화 읽기를 막는다.
struct InspectionSummary {
    int unstable_frames{};
    int widest_span_mg{};
};

// class는 기본 private이므로 센서 원본 owner를 감추기 좋다. public:은 읽기 창 API만 공개한다.
class VibrationTrace final {
public:
    // explicit은 vector xvalue에서 trace로의 암시 변환을 금지한다. 멤버 초기화 목록은 생성자 본문보다 먼저
    // amplitudes_mg_를 만들며, && 참조는 호출자가 이동을 허용한 vector에 바인딩된다.
    // [첫 호출 계약: std::move와 std::vector<int> 이동 생성자]
    // (1) 수신 객체 amplitudes_mg_는 아직 생성 중인 std::vector<int> 멤버이고, measurements는 유효한
    //     std::vector<int> owner에 바인딩된 이름 있는 rvalue 참조다.
    // (2) std::move<T>(T&&)에서 T=std::vector<int>&, 반환형은 std::vector<int>&&이며 이어서
    //     std::vector<int>::vector(vector&&) noexcept가 선택된다.
    // (3) measurements 식은 lvalue이고 std::move 결과만 xvalue다. 저장소를 넘겨도 되는 의미이며 값 제한은 없다.
    // (4) std::move 반환 참조는 멤버 생성에 즉시 쓰고, 이동 생성자는 별도 반환값이 없다.
    // (5) 성공 뒤 amplitudes_mg_가 원소를 소유하고 원본 vector는 유효하지만 내용 미지정 상태다.
    // (6) 기본 allocator vector 이동은 O(1)·noexcept다. 기존 원소 관찰자는 새 owner의 원소를 가리키며
    //     trace 수명이 원소 수명이다. 자체 동기화는 제공하지 않는다.
    explicit VibrationTrace(std::vector<int>&& measurements) noexcept
        : amplitudes_mg_{std::move(measurements)} {}

    // trace의 정체성과 주소를 고정해 이미 발급한 비소유 frame view의 owner가 갑자기 이동하지 않게 한다.
    VibrationTrace(const VibrationTrace&) = delete;
    VibrationTrace& operator=(const VibrationTrace&) = delete;
    VibrationTrace(VibrationTrace&&) = delete;
    VibrationTrace& operator=(VibrationTrace&&) = delete;

    // using 별칭은 정확한 표준 템플릿 인자를 한곳에 기록한다. BaseView는 const vector를 빌리고,
    // Frames는 그 위의 slide_view이며 Frame은 반복자 역참조로 얻는 한 구간의 view 값이다.
    using BaseView = std::ranges::ref_view<const std::vector<int>>;
    using Frames = std::ranges::slide_view<BaseView>;
    using Frame = std::ranges::range_reference_t<Frames>;
    using Difference = std::ranges::range_difference_t<const std::vector<int>>;

    // const & 한정은 살아 있는 lvalue trace에서만 읽기 view를 만들 수 있다는 API 계약이다.
    [[nodiscard]] Frames frames() const & {
        // [첫 호출 계약: std::views::slide(range, count)]
        // (1) 논리적 수신 범위는 이 lvalue owner의 정확한 타입 const std::vector<int> 멤버이며 유효하다.
        //     std::views::slide는 상태 없는 range adaptor 함수 객체다.
        // (2) slide.operator()<R>(R&&, range_difference_t<R>)에서 R=const std::vector<int>&가 선택되고,
        //     반환형은 std::ranges::slide_view<std::ranges::ref_view<const std::vector<int>>>, 즉 Frames다.
        // (3) amplitudes_mg_는 소유권을 넘기지 않는 const lvalue, frame_width는 Difference lvalue 값 3을
        //     값으로 복사한다. 표준의 양수 전제조건을 만족한다.
        // (4) Frames prvalue를 반환해 호출부의 view를 직접 초기화하며 결과 원소는 각 Frame view다.
        // (5) trace/vector/인자는 그대로이고 결과는 원본 참조와 폭만 보관한다.
        // (6) O(1)·무할당이다. 폭이 0 이하이면 전제조건 위반이고 trace 파괴 뒤 view 접근은 UB다. 이 구체
        //     ref_view 기반 view는 vector 재할당 뒤 새 begin/end로 다시 순회할 수 있지만, 재할당 전에 얻은
        //     반복자·Frame·원소 참조는 무효다. 동시 쓰기는 데이터 경쟁이며, 실질 연산이 던지지 않더라도
        //     어댑터 호출 전반을 무조건 noexcept라고 확대 해석하지 않는다.
        return std::views::slide(amplitudes_mg_, frame_width);
    }

    // 임시 trace의 vector는 전체 식 뒤 사라지므로 rvalue accessor를 삭제해 dangling frame을 원천 차단한다.
    [[nodiscard]] Frames frames() const && = delete;

private:
    static constexpr Difference frame_width{3};
    std::vector<int> amplitudes_mg_;
};

// 반환형은 값 결과다. const VibrationTrace&는 복사하지 않는 읽기 별칭이며 null이 될 수 없다.
// 포인터(const VibrationTrace*)는 null 검사가 필요하고 ->로 접근하지만 여기에는 그런 선택 상태가 필요 없다.
// allowed_span_mg는 작은 기본 정수 타입 int를 값으로 받아 호출자 값과 독립적이다. 최댓값-최솟값이 int
// 범위에 들어온다는 것이 호출 전제조건이며, 범위를 넘는 signed 뺄셈은 미정의 동작이다.
[[nodiscard]] InspectionSummary inspect_trace(
    const VibrationTrace& trace,
    const int allowed_span_mg) {
    InspectionSummary summary{};

    // [첫 호출 계약: slide_view range-for 숨은 연산과 Frame 값 초기화]
    // (1) 수신 범위는 trace.frames()가 반환한 정확한 타입 VibrationTrace::Frames 임시이고 trace가 살아 있다.
    //     range-for의 숨은 auto&& 범위 변수가 view 수명을 루프 끝까지 연장한다.
    // (2) simple-view 기반 Frames::begin() const/end() const를 고른다. slide iterator는 operator==만 선언하므로
    //     소스의 !=는 rewritten candidate인 !(current==end)로 해석된다. 이어 *와 ++, Frame 초기화를 수행한다.
    // (3) 각 멤버 연산의 명시 인자는 없다. 비교는 반복자 두 값을 빌리고, 끝 전 iterator만 역참조/증가한다.
    //     frame은 세 int 자체가 아니라 그 구간을 보는 경량 view prvalue로 초기화된다.
    // (4) begin/end는 iterator, 비교는 bool, 역참조는 Frame, 전위 증가는 iterator&를 반환해 숨은 루프가 쓴다.
    // (5) 반복 위치와 frame 경계만 이동하고 센서 원본과 trace 상태는 변하지 않는다.
    // (6) 연산당 O(1)이고 동적 할당을 요구하지 않으며 전체 frame 수는 max(N-3+1,0)이다. 끝 반복자나 owner
    //     파괴·구조 변경으로 무효화된 관찰자 사용은 UB다. 예외 명세는 선택된 표준 iterator 연산을 따르고,
    //     slide iterator의 비-const 연산이 예외로 끝나면 그 iterator는 singular 상태가 된다. 같은 원본의
    //     동시 쓰기는 안전하지 않다.
    for (const VibrationTrace::Frame frame : trace.frames()) {
        int lowest_mg{};
        int highest_mg{};
        bool has_measurement{false};

        // [첫 호출 계약: Frame range-for 숨은 begin/end/비교/역참조/증가]
        // (1) 수신 객체는 현재 세 측정치를 빌리는 정확한 타입 const VibrationTrace::Frame이고 owner가 유효하다.
        // (2) Frame::begin() const/end() const, 소스의 !=를 만족하는 동등 비교, operator*() const와 ++를 고른다.
        //     vector iterator가 contiguous_iterator라 창은 표준상 span<const int> 계열이지만, !=가 직접
        //     연산자인지 ==의 rewritten candidate인지는 구현 반복자 타입을 따른다.
        // (3) 명시 인자는 없다. 반복자는 비소유 값이고 끝 전에서만 역참조/증가한다. 역참조 const int lvalue를
        //     amplitude_mg 값에 복사하며 원소 소유권은 이동하지 않는다.
        // (4) 두 경계는 iterator, 비교는 bool, 역참조는 const int&, 증가는 iterator&를 반환해 루프가 소비한다.
        // (5) iterator만 전진하고 frame/원본은 그대로다. amplitude_mg는 반복마다 새 int 값이다.
        // (6) 각 연산은 O(1)이고 동적 할당을 요구하지 않으며 정확히 세 번 돈다. 예외 명세는 선택된 표준
        //     iterator 연산을 따른다. 무효/끝 반복자 접근은 UB고 동시 원본 쓰기와 안전하지 않다.
        for (const int amplitude_mg : frame) {
            // 첫 값은 최솟값과 최댓값 모두의 초기 기준이다. bool은 참/거짓을 담는 기본 타입이다.
            if (!has_measurement) {
                lowest_mg = amplitude_mg;
                highest_mg = amplitude_mg;
                has_measurement = true;
            } else {
                if (amplitude_mg < lowest_mg) {
                    lowest_mg = amplitude_mg;
                }
                if (amplitude_mg > highest_mg) {
                    highest_mg = amplitude_mg;
                }
            }
        }

        const int span_mg{highest_mg - lowest_mg};
        // 실행 관점에서는 연속 원소 load, 정수 비교/조건 분기와 결과 store로 낮아질 수 있다. 컴파일러가
        // 분기를 조건 이동 등으로 바꿀 수도 있으며 CPU·ABI·최적화 옵션 없이는 특정 어셈블리를 단정할 수 없다.
        // virtual 함수가 없는 final 값 타입들이므로 이 코드 자체에는 가상 간접 호출이 필요하지 않다.
        if (span_mg > allowed_span_mg) {
            ++summary.unstable_frames;
        }
        if (span_mg > summary.widest_span_mg) {
            summary.widest_span_mg = span_mg;
        }
    }

    // 이름 있는 summary lvalue는 NRVO(RVO 계열) 후보라 호출자 저장소에 직접 만들어질 수 있다.
    // NRVO가 적용되지 않아도 이 struct의 암시적 복사/이동은 두 int 값만 전달하며 소유권 자원은 없다.
    return summary;
}

int main() {
    // [첫 생성 계약: std::vector<int> initializer_list 생성자]
    // (1) 수신 measurements는 아직 생성되지 않은 std::vector<int, std::allocator<int>> 객체다.
    // (2) vector(std::initializer_list<int>, const allocator_type& = allocator_type())와 생략 인자용
    //     std::allocator<int>::allocator() noexcept를 선택한다.
    // (3) {100,108,115,102,130}의 다섯 int prvalue를 읽어 복사한다. 생략한 두 번째 인자는
    //     std::allocator<int>{} prvalue로 평가되어 const allocator_type&에 바인딩되며 외부 저장소 owner는 없다.
    // (4) 두 생성자는 반환값이 없다. allocator 임시가 vector 생성 인자로 쓰이고 measurements가 완성된다.
    // (5) 성공 뒤 size=5이고 vector가 원소를 단독 소유한다. allocator 임시는 전체 식 끝에 소멸하지만
    //     저장소와 vector는 initializer_list/allocator 객체 수명과 독립이다.
    // (6) 원소 복사·저장소는 O(5)이고 실패 시 std::bad_alloc 등이 전파된다. allocator 임시 생성·소멸은
    //     O(1)·무할당·비투척이다. 아직 무효화할 관찰자는 없고 vector는 자체 동기화를 제공하지 않는다.
    std::vector<int> measurements{100, 108, 115, 102, 130};

    // measurements는 lvalue, std::move(measurements)는 xvalue다. move는 캐스트일 뿐이고 실제 vector 이동은
    // 생성자가 수행한다. trace가 유일한 owner가 되며 원본은 유효하지만 내용 미지정이다.
    VibrationTrace trace{std::move(measurements)};

    // inspect_trace 호출 결과는 prvalue이고 result를 직접 초기화한다. 함수의 NRVO가 적용되면 복사/이동이 없다.
    const InspectionSummary result{inspect_trace(trace, 14)};

    // [첫 호출 계약: std::ostream 삽입 operator<< 연쇄]
    // (1) 수신 std::cout은 사용 가능한 정확한 타입 std::ostream 전역 객체다.
    // (2) Traits=std::char_traits<char>다. 문자열은 비멤버 function template
    //     `template<class Traits> std::basic_ostream<char, Traits>& operator<<(
    //     std::basic_ostream<char, Traits>&, const char*)`, int는 같은 basic_ostream의 멤버
    //     `std::basic_ostream<char, Traits>& operator<<(int)`, '\n'은 같은 Traits의 char 비멤버 template이다.
    // (3) 리터럴은 null 종료 문자를 비소유로 읽고 두 int 멤버는 값으로 읽으며 '\n'은 char prvalue다.
    //     인자 소유권은 이동하지 않는다.
    // (4) 각 호출이 같은 std::ostream&를 반환해 다음 호출의 수신자가 되고 마지막 참조는 버린다.
    // (5) 출력 버퍼·위치·상태 비트만 바뀔 수 있고 result와 trace는 유지된다.
    // (6) locale/형식화/버퍼링 비용이 있으며 단일 복잡도 보장은 없다. 실패는 상태 비트 또는 설정에 따른
    //     ios_base::failure로 보고된다. 기본 동기화 상태의 std::cout 동시 삽입 자체는 data race가 아니지만
    //     문자가 섞일 수 있고 레코드 원자성은 없다. 컨테이너 관찰자 무효화는 없다.
    std::cout << "unstable=" << result.unstable_frames
              << ",widest_span_mg=" << result.widest_span_mg << '\n';

    // [첫 숨은 소멸 계약: 비소유 range 관찰자와 std::vector<int>]
    // (1) 각 range-for 종료 시 Frames/Frame과 구현 iterator·sentinel은 유효한 비소유 관찰자이고, main 종료
    //     시 measurements와 trace의 amplitudes_mg_는 유효한 std::vector<int> 객체다.
    // (2) 각 소멸자는 인자·반환값이 없다. vector에는 `~vector()`, slide_view/ref_view, span 계열 창과
    //     구현 iterator·sentinel에는 각 구현 타입의 소멸자가 선택된다.
    // (3) 데이터 인자나 새 소유권 입력은 없고 비소유 관찰자는 owner를 소유하지 않는다.
    // (4) 소멸자는 값을 반환하지 않으며 호출부가 사용하는 결과도 없다.
    // (5) 관찰자 소멸은 vector를 바꾸지 않는다. vector 소멸은 int 원소 수명을 끝내고 저장소를 반환하며
    //     모든 원소 관찰자를 무효화한다. moved-from measurements도 유효하게 소멸한다.
    // (6) 관찰자 소멸은 O(1)·무할당이고 vector 소멸은 원소 수에 선형이며 저장소를 해제한다. int와 기본
    //     allocator 경로는 비투척이다. 소멸과 같은 객체의 동시 접근은 외부 동기화 없이는 안전하지 않다.

    // main의 끝은 자동으로 성공 코드 0을 반환한다.
}
