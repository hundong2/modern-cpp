// <generator>는 co_yield로 한 원소씩 지연 생산하는 C++23 std::generator를 선언한다.
#include <generator>
// <iostream>은 표준 출력 객체 std::cout과 삽입 연산을 선언한다.
#include <iostream>
// <string>은 문자를 소유하는 std::string을 선언한다.
#include <string>
// <utility>는 식을 xvalue로 바꾸는 std::move를 선언한다.
#include <utility>
// <vector>는 연속 저장소를 소유하는 std::vector와 reserve/push_back/size를 선언한다.
#include <vector>

// struct는 기본 접근이 public이어서 값 묶음에 알맞다. int는 지연 시간을 담는 기본 정수 타입이고,
// latency_ms{}는 빈 중괄호 값 초기화로 0이 된다. string은 서비스 이름 문자를 직접 소유한다.
struct MetricSample {
    std::string service;
    int latency_ms{};
};

// class는 기본 접근이 private이다. public:에 소유권 이전 API만 열고 private:에 실제 저장소를 숨겨
// "배치는 자기 sample을 소유한다"는 아키텍처 경계를 한 타입으로 표현한다.
class MetricBatch final {
public:
    // explicit은 vector 한 개가 MetricBatch로 암시적으로 변환되는 것을 막는다. 값 매개변수 samples는
    // 호출자가 복사 또는 이동 비용을 선택하게 하고, 멤버 초기화 목록은 본문 대입 전에 samples_를 만든다.
    // [첫 호출 계약: std::move(samples)와 vector 이동 생성]
    // (1) 수신 객체는 아직 생성 중인 std::vector<MetricSample> samples_이고, 인자 samples는 유효한 소유 vector다.
    // (2) std::move<T>(T&&)에서 T=std::vector<MetricSample>&가 추론되고,
    //     vector(vector&&) noexcept 이동 생성자가 멤버 초기화에 선택된다.
    // (3) samples는 이름 있는 lvalue이며 std::move(samples)는 같은 객체를 가리키는 xvalue다. 원소 소유권을
    //     이전해도 되는 sink 매개변수이고 null/범위 전제조건은 없다.
    // (4) std::move는 vector&&를 반환해 즉시 소비되고, 이동 생성자는 별도 반환값이 없다.
    // (5) 성공하면 samples_가 저장소를 소유하고 samples는 유효하지만 내용이 미지정인 이동 후 상태가 된다.
    // (6) 표준 allocator가 같은 이 경우 이동은 상수 시간·noexcept이고 기존 원소 참조는 새 owner를 가리킨다.
    //     객체 수명은 batch에 묶이며 동기화를 제공하지 않는다. 이동 후 samples 내용에 의존하면 안 된다.
    explicit MetricBatch(std::vector<MetricSample> samples) noexcept
        : samples_{std::move(samples)} {}

    // 같은 배치를 두 owner가 공유한다고 오해하지 않도록 복사를 막고 이동만 허용한다.
    MetricBatch(const MetricBatch&) = delete;
    MetricBatch& operator=(const MetricBatch&) = delete;
    MetricBatch(MetricBatch&&) noexcept = default;
    MetricBatch& operator=(MetricBatch&&) noexcept = default;

    // 반환형 const vector&는 소유하지 않는 읽기 전용 별칭이다. const는 이 경로로 원소를 바꾸지 못하게 한다.
    [[nodiscard]] const std::vector<MetricSample>& samples() const & noexcept {
        return samples_;
    }

    // 임시 owner에서 참조를 꺼내면 즉시 댕글링할 수 있어 rvalue 호출을 컴파일 단계에서 금지한다.
    [[nodiscard]] const std::vector<MetricSample>& samples() const && = delete;

private:
    // [첫 생성 계약: private vector 기본 멤버 초기화식]
    // (1) 수신 samples_는 이 초기화식을 사용하는 생성자에서 아직 수명이 시작되지 않은 vector 멤버다.
    // (2) std::vector<MetricSample>::vector()와 그 specialization의 기본 allocator_type 구성이 선택된다.
    // (3) 데이터 인자는 없고 allocator는 기본값이며 소유권을 넘길 외부 객체도 없다.
    // (4) 함수 반환값 없이 빈 vector 멤버를 생성한다.
    // (5) 적용되면 size=0이고 원소 수명은 시작하지 않는다. 오늘 명시 생성자는 samples_{std::move(samples)}를
    //     직접 지정하므로 이 기본 멤버 초기화식은 그 생성 경로에서는 평가되지 않는다.
    // (6) 표준 allocator 조합은 상수 시간·noexcept다. 빈 상태에 원소 저장소가 필요하지 않지만 표준 계약만으로
    //     구현 내부 할당을 단정하지 않는다. 관찰자 무효화·공유 원소 수명·동기화 대상은 아직 없다.
    std::vector<MetricSample> samples_{};
};

// using은 새 타입을 만들지 않고 긴 템플릿 타입에 별칭을 붙인다. 템플릿 인자 const MetricSample&는
// generator가 원소를 복사하지 않고 읽기 전용 lvalue 참조로 노출한다는 뜻이다.
using SlowSampleStream = std::generator<const MetricSample&>;

// 반환형은 지연 입력 범위이고, batch는 값 매개변수라 코루틴 프레임 안에서 소유된다.
// threshold_ms는 복사되는 기본 정수이며 호출 뒤 원본과 독립이다.
// [첫 생성/호출 계약: std::generator coroutine protocol 전체]
// (1) 사용자 수신 객체는 없다. 컴파일러가 promise와 두 매개변수를 담는 frame을 만들며 호출 전 인자는
//     유효한 MetricBatch와 int여야 한다.
// (2) 필요하면 promise_type::operator new(size_t)로 frame을 확보하고, get_return_object() noexcept,
//     initial_suspend() const noexcept를 거친다. 정상 종료는 return_void() const noexcept와
//     final_suspend() noexcept, 예외는 unhandled_exception(), owner 파괴는 generator 소멸자와 frame delete를 쓴다.
// (3) 숨은 allocation 크기는 구현이 정하고 호출자가 소유하지 않는다. 명시 매개변수 batch는 이동/복사되어
//     frame이 소유하고 threshold_ms는 값 복사된다.
// (4) get_return_object는 frame handle을 소유한 generator 값을 반환해 호출자가 저장한다. suspend 함수의
//     awaiter는 coroutine 상태 전이에 쓰고 void 함수·소멸/해제 결과는 직접 사용하지 않는다.
// (5) 호출 직후 initial suspend 상태이고, 정상 본문 끝에는 final suspend 상태다. 처리되지 않은 예외는
//     promise 경로를 거쳐 재개 호출자에게 전달되며 generator 파괴가 중단 frame과 그 batch를 파괴한다.
// (6) frame 할당은 상수 크기 요청이지만 bad_alloc 가능성이 있고, 전체 실행은 본문 O(N)이다. generator와
//     frame 참조의 수명은 owner 파괴까지이며 실행 중 동시 resume/파괴는 안전하지 않다.
[[nodiscard]] SlowSampleStream select_slow_samples(MetricBatch batch, const int threshold_ms) {
    // [첫 호출 계약: const vector range-for의 숨은 begin/end/operator*/operator++/operator!=]
    // (1) 수신 범위는 batch.samples()가 돌려준 const std::vector<MetricSample>&이고 frame 속 owner가 살아 있다.
    // (2) const_iterator begin() const noexcept, end() const noexcept, const_reference operator*() const,
    //     const_iterator& operator++(), bool 비교(operator!= 또는 ==에서 합성)가 선택된다.
    // (3) begin/end/*/++에는 명시 데이터 인자가 없다. 숨은 begin!=end 비교는 현재/끝 const_iterator lvalue
    //     두 개를 비소유 피연산자로 쓰고, sample은 각 const MetricSample lvalue에 바인딩된 비소유 참조다.
    // (4) begin/end는 반복자를 값으로, operator*는 const MetricSample&를, 증가는 iterator&를 돌려주며
    //     컴파일러가 만든 루프가 모두 소비한다.
    // (5) 읽기 순회라 vector 크기·용량·원소와 batch 소유권은 바뀌지 않는다.
    // (6) begin/end와 각 iterator 연산은 O(1)·무할당이고 begin/end는 noexcept다. frame/vector가 살아 있고
    //     구조 변경이 없어야 한다. end는 past-the-end라 역참조·증가할 수 없고, 무효 iterator 사용은 미정의
    //     동작이다. 전체 O(N)이며 같은 vector를 다른 스레드가 쓰면 데이터 경쟁이다.
    for (const MetricSample& sample : batch.samples()) {
        // if는 정수 load와 비교 뒤 조건 분기로 구현될 수 있다. 정확한 명령은 최적화와 CPU에 따라 다르다.
        if (sample.latency_ms >= threshold_ms) {
            // [첫 호출 계약: std::generator<const MetricSample&>::promise_type::yield_value]
            // (1) 수신 promise는 현재 실행 중인 generator 코루틴 프레임에 있고 sample 원소도 그 frame의 batch에 있다.
            // (2) 선택 형태는 suspend_always yield_value(const MetricSample& value) noexcept다.
            // (3) sample은 const MetricSample lvalue이며 소유권을 넘기지 않고 주소만 다음 역참조 대상으로 빌려준다.
            // (4) 반환 suspend_always awaiter는 co_yield가 코루틴을 중단하게 하며 호출자가 직접 저장하지 않는다.
            // (5) frame과 batch는 유지되고 현재 원소 참조가 노출된다. 다음 증가 때 코루틴이 다시 실행된다.
            // (6) 이 참조 설정은 상수 시간·무할당·noexcept지만 참조는 generator/frame 수명을 넘길 수 없다.
            //     iterator를 잘못 쓰거나 generator를 동시에 재개하면 전제조건 위반이며 자체 동기화가 없다.
            co_yield sample;
        }
    }
}

int main() {
    // [첫 생성 계약: std::vector<MetricSample> 기본 생성]
    // (1) 수신 객체 samples는 아직 생성 전이다.
    // (2) vector() 기본 생성자가 선택되고 원소 타입 템플릿 인자는 MetricSample이다.
    // (3) 인자는 없으며 allocator는 기본값을 사용한다.
    // (4) 생성자는 반환값이 없고 빈 소유 vector를 만든다.
    // (5) 성공 뒤 size=0, capacity는 구현이 정한 값이며 원소 소유권은 없다.
    // (6) 이 기본 allocator specialization은 상수 시간·noexcept다. 빈 상태는 원소 저장소를 요구하지 않지만
    //     구현 내부 할당 여부를 표준 계약 이상으로 단정하지 않는다. 관찰자·공유 원소는 아직 없다.
    std::vector<MetricSample> samples{};

    // [첫 호출 계약: vector::reserve]
    // (1) 수신자는 빈 std::vector<MetricSample> samples다.
    // (2) void reserve(size_type new_capacity) 오버로드를 선택한다.
    // (3) 4는 허용되는 새 최소 capacity 값인 int prvalue이며 size_type으로 변환되고 소유권 의미는 없다.
    // (4) 반환형 void라 저장하지 않는다.
    // (5) 성공하면 size는 0이고 capacity>=4다. 기존 원소가 없으며 인자 4는 변하지 않는다.
    // (6) 재할당은 원소 수에 선형이고 할당 실패 bad_alloc 또는 한계 초과 length_error가 가능하다.
    //     성공한 재할당은 기존 관찰자를 모두 무효화하며 vector는 자체 동기화를 제공하지 않는다.
    samples.reserve(4);

    // [첫 호출 계약: string 직접 생성과 vector::push_back(T&&)]
    // (1) 수신 samples는 size=0, capacity>=4인 vector다. MetricSample 임시는 아직 없다.
    // (2) string(const char*)로 "gateway"를 깊게 복사하고 void push_back(MetricSample&&)를 선택한다.
    // (3) 문자열 리터럴은 null 종료 const char 배열 lvalue이고 MetricSample{...}은 이동 가능한 prvalue다.
    //     임시가 string을 소유하며 push_back이 완성된 sample 소유권을 vector 원소로 옮긴다.
    // (4) string 생성자는 반환값이 없고 aggregate 초기화가 sample prvalue를 완성하며 push_back은 void다.
    // (5) 성공하면 size=1이고 새 원소가 이름과 135를 소유한다. 임시는 전체 식 끝에 파괴된다.
    // (6) 문자열 길이에 선형이며 문자열 할당이 생길 수 있다. push_back은 상각 O(1); 예약 범위 안이라
    //     vector 재할당은 없고 기존 원소 관찰자는 유지되지만 과거 end 반복자는 무효화된다. string 할당 예외는
    //     전파되며 성공 전 얻은 원소 참조는 아직 없고 동기화도 없다.
    samples.push_back(MetricSample{std::string{"gateway"}, 135});
    // 같은 계약으로 각 prvalue sample을 예약된 저장소에 이동한다.
    samples.push_back(MetricSample{std::string{"search"}, 95});
    samples.push_back(MetricSample{std::string{"billing"}, 220});
    samples.push_back(MetricSample{std::string{"cache"}, 80});

    // 위 첫 std::move/vector 이동 계약을 적용한다. samples는 xvalue로 바뀌고 batch가 저장소를 인수한다.
    MetricBatch batch{std::move(samples)};

    // [첫 호출 계약: select_slow_samples와 generator 결과 구성]
    // (1) 사용자 데이터 수신 객체는 없고 batch는 네 sample을 소유한 MetricBatch lvalue다.
    // (2) SlowSampleStream select_slow_samples(MetricBatch, int)가 선택되며 반환 타입은
    //     std::generator<const MetricSample&>다.
    // (3) std::move(batch)는 xvalue라 frame의 값 매개변수를 이동 구성하고 100은 int prvalue 임계값이다.
    // (4) 반환 generator prvalue는 stream을 직접 초기화한다. 같은 타입 중간 복사는 없고 결과를 순회에 사용한다.
    // (5) batch는 유효하지만 내용 미지정인 이동 후 상태이고, 코루틴 본문은 아직 실행되지 않았으며 frame이 입력을 소유한다.
    // (6) 호출/초기 중단은 원소 수와 무관한 상수 작업이지만 frame 할당 실패가 가능하다. generator는 이동 전용이고
    //     begin을 같은 객체에 두 번 호출하면 미정의 동작이다. frame/반환 참조는 단일 스레드에서 소비한다.
    SlowSampleStream stream{select_slow_samples(std::move(batch), 100)};

    // [첫 생성 계약: std::vector<std::string> 기본 생성]
    // (1) 수신 copied_services는 아직 생성 전이다.
    // (2) std::vector<std::string>::vector()와 기본 allocator 구성이 선택된다.
    // (3) 인자는 없다. (4) 반환값 없이 빈 문자열 owner 목록을 만든다. (5) 성공 뒤 size=0이다.
    // (6) 이 기본 allocator 조합은 상수 시간·noexcept다. 빈 vector는 string 원소 저장소를 요구하지 않지만
    //     구현 내부 할당 여부를 표준 계약 이상으로 단정하지 않으며 관찰자·공유 원소는 없다.
    std::vector<std::string> copied_services{};
    int total_latency_ms{};

    // [첫 호출 계약: generator range-for의 begin/end/operator*/operator++/operator==]
    // (1) 수신 stream은 initial suspend 지점의 유효한 std::generator<const MetricSample&> lvalue다.
    // (2) iterator begin(), default_sentinel_t end() const noexcept,
    //     reference iterator::operator*() const noexcept(is_nothrow_copy_constructible_v<reference>),
    //     iterator& operator++(), friend bool operator==(const iterator&, default_sentinel_t)가 선택된다.
    // (3) begin/end/*/++에는 명시 데이터 인자가 없다. 끝 비교는 현재 iterator const lvalue를 빌리고 end가
    //     만든 default_sentinel_t는 by-value 매개변수로 복사하며, sample은 역참조한 const lvalue를 빌린다.
    // (4) 숨은 반환값은 range-for 상태에 사용되며 밖에 저장하지 않는다.
    // (5) 각 증가가 frame을 전진시키지만 입력 batch는 읽기만 한다. 루프 뒤 coroutine은 final suspend 상태다.
    // (6) 각 재개는 다음 산출까지 선형, 전체 O(N)이다. begin은 initial-suspend 상태에서 정확히 한 번만, ++는
    //     아직 끝나지 않은 유효 iterator에서만 쓴다. 종료 iterator의 역참조/증가, owner 파괴 뒤 iterator·sample
    //     사용은 미정의 동작이다. begin은 active-stack bookkeeping/최초 재개에서 실패할 수 있고 ++는 재개된
    //     본문 예외를 전파할 수 있다. 같은 generator의 동시 재개·파괴는 안전하지 않다.
    for (const MetricSample& sample : stream) {
        // [첫 호출 계약: vector<string>::push_back(const string&)]
        // (1) 수신 copied_services는 지금까지의 독립 이름 복사본을 소유한 vector다.
        // (2) void push_back(const std::string& value) 복사 overload가 선택된다.
        // (3) sample.service는 frame 원소 안의 std::string const lvalue이며 호출 동안 빌리고 문자를 깊게 복사한다.
        // (4) 반환형 void라 무시한다.
        // (5) 성공하면 size가 1 증가하고 새 string은 generator와 독립된 수명을 가진다. 원본은 그대로다.
        // (6) 상각 O(1)+문자 수 선형이며 vector/string 할당이 실패할 수 있다. 재할당이면 기존 vector 관찰자가
        //     전부 무효화되고, 재할당이 없어도 과거 end 반복자는 무효화된다. 원본 sample 참조에는 영향이 없고
        //     이 copy-insertable 타입은 실패 시 효과가 없으며 동기화는 없다.
        copied_services.push_back(sample.service);
        total_latency_ms += sample.latency_ms;
    }

    // [첫 호출 계약: vector::size]
    // (1) 수신 copied_services는 느린 서비스 이름 두 개를 소유한 const가 아닌 vector지만 읽기만 한다.
    // (2) size_type size() const noexcept가 선택되고 인자는 없다.
    // (3) 인자·소유권 이전은 없다.
    // (4) 반환 size_type 값은 삽입된 이름 수이며 출력 피연산자로 즉시 사용한다.
    // (5) vector 크기·용량·원소와 모든 인자 상태는 바뀌지 않는다.
    // (6) 상수 시간·무할당·noexcept이고 참조를 무효화하지 않는다. 동시 구조 변경과 함께 쓰면 데이터 경쟁이다.
    const auto slow_count{copied_services.size()};

    // [첫 호출 계약: std::cout의 operator<< 연쇄]
    // (1) 수신 std::cout은 정상 상태라고 가정하는 전역 std::ostream이고 프로그램은 단일 스레드에서 쓴다.
    // (2) const char*·size_type·int·char에 맞는 ostream 삽입 overload가 왼쪽부터 선택된다.
    // (3) 문자열 리터럴은 비소유 문자 포인터로 읽고 slow_count/total_latency_ms는 값, '\n'은 char prvalue다.
    // (4) 각 호출은 같은 ostream&를 반환해 다음 삽입에 사용하며 최종 참조는 버린다.
    // (5) 출력 버퍼와 상태 비트가 바뀔 수 있지만 피연산자와 vector 소유권은 그대로다.
    // (6) 형식 변환·버퍼링·locale 비용이 들며 표준은 단순한 "출력 문자 수 선형" 복잡도를 별도로 보장하지
    //     않는다. 실패는 상태 비트와 설정된 예외로 나타나며 참조 무효화는 없고, 외부 동기화 없는 다중
    //     스레드 레코드 원자성은 보장하지 않는다.
    std::cout << "slow=" << slow_count << ",total=" << total_latency_ms << '\n';
}
