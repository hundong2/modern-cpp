// 직접 연습: 레거시 시퀀스의 평범한 next_id 필드를 바꾸지 않고 std::atomic_ref view로
// 두 작업자에게 겹치지 않는 ID를 나눠 준다. 어느 worker가 어떤 ID를 받는지는 달라도
// 1..8 전체의 합 36과 마지막 next_id 9는 항상 같으므로 결정적으로 검증할 수 있다.

// <atomic>은 std::atomic_ref와 std::memory_order_relaxed를 선언한다.
#include <atomic>
// <cstdint>는 정확히 64비트인 std::uint64_t 정수 별칭을 선언한다.
#include <cstdint>
// <iostream>은 std::cout과 출력 스트림 삽입 연산자를 선언한다.
#include <iostream>
// <thread>는 실행 수명을 범위에 묶는 C++20 std::jthread를 선언한다.
#include <thread>
// <utility>는 lvalue를 xvalue 식으로 바꾸는 std::move를 선언한다.
#include <utility>

// struct의 기본 접근은 public이다. 이 객체가 실제 숫자 저장소를 소유한다.
struct LegacySequence {
    // atomic_ref가 요구하는 정렬은 구현에 따라 기본 정렬보다 클 수 있으므로 명시한다.
    // 중괄호 {1}은 std::uint64_t 기본 정수를 첫 발급 ID인 1로 직접 초기화한다.
    alignas(std::atomic_ref<std::uint64_t>::required_alignment)
        std::uint64_t next_id{1};
};

// 결과는 값만 소유한다. atomic_ref나 원본 참조를 반환하지 않아 수명 경계를 넘겨도 안전하다.
struct AllocationSummary {
    std::uint64_t issued_sum{};
    std::uint64_t next_id{};
};

// class는 기본 private이므로 외부 코드는 atomic handle 대신 next()만 사용한다.
class AtomicIdAllocatorView {
public:
    // using은 새 기본 타입이 아니라 기존 타입에 지역 별칭을 붙인다.
    using Value = std::uint64_t;
    // Value는 클래스 템플릿 atomic_ref의 템플릿 인자다.
    using Reference = std::atomic_ref<Value>;

    // [첫 생성 계약: std::atomic_ref<Value>(Value&)]
    // (1) 목적 멤버 next_는 아직 없고 storage는 살아 있으며 required_alignment를 만족한다.
    // (2) explicit atomic_ref<T>::atomic_ref(T&), T=Value가 선택된다. 표준은 Throws: Nothing을
    //     규정하지만 이 생성자 선언 자체에는 noexcept가 붙지 않는다.
    // (3) 유일한 인자 storage는 Value lvalue reference이며 숫자를 빌릴 뿐 소유권을 이전하지 않는다.
    // (4) 생성자는 반환값이 없고 storage에 결합된 비소유 atomic_ref를 만든다.
    // (5) underlying 숫자와 storage 수명은 바뀌지 않고 next_의 binding만 생긴다.
    // (6) Value는 trivially copyable이어야 하고 정렬·수명 전제를 지켜야 한다. atomic_ref가
    //     하나라도 살아 있는 동안 같은 숫자의 모든 접근은 atomic_ref로 해야 한다. 생성은
    //     새 Value를 소유하거나 사용자의 숫자·참조를 복사/무효화하지 않고 표준상 예외를 던지지
    //     않지만(Throws: Nothing), 함수 타입 자체는 noexcept가 아니다. 구현은 연결된 잠금 같은 내부
    //     공유 자원을 얻을 수 있어 무할당·공통 점근 복잡도·lock-free는 보장되지 않는다.
    // explicit는 Value 한 개가 AtomicIdAllocatorView로 암시적으로 바뀌는 실수를 막는다.
    // 콜론 뒤 멤버 초기화 목록은 생성자 본문보다 먼저 실행되어 next_를 storage에 직접 결합한다.
    explicit AtomicIdAllocatorView(Value& storage) noexcept
        : next_{storage} {}

    // [첫 생성 계약: atomic_ref 복사 생성]
    // (1) other.next_는 유효한 Reference lvalue이고 대상 Value가 살아 있다.
    // (2) atomic_ref(const atomic_ref&) noexcept 복사 생성자가 선택된다.
    // (3) const Reference&를 빌리며 underlying 값이나 그 소유권은 복사하지 않는다.
    // (4) 반환값 없이 같은 숫자를 가리키는 새 handle이 생긴다.
    // (5) 원본 handle과 숫자는 그대로이고 두 atomic_ref가 같은 수정 순서를 공유한다.
    // (6) 새 Value를 소유하거나 사용자 참조를 무효화하지 않는 noexcept 비소유 복사이고 대상 수명을
    //     늘리지 않는다. 표준은 공통 점근 복잡도나 내부 공유 자원 획득 부재를 보장하지 않는다.
    AtomicIdAllocatorView(const AtomicIdAllocatorView& other) noexcept
        : next_{other.next_} {}

    // view 이동은 숫자 소유권 이동이 아니다. source handle을 읽어 같은 binding을 복사하므로
    // other도 파괴 전까지 유효하지만, 일반적인 이동 후 규율에 따라 다시 사용하지 않는다.
    AtomicIdAllocatorView(AtomicIdAllocatorView&& other) noexcept
        : next_{other.next_} {}

    // const는 view의 binding을 바꾸지 않는다는 뜻이다. 외부 숫자에 대한 원자 RMW는 허용된다.
    [[nodiscard]] Value next() const noexcept {
        // [첫 호출 계약: atomic_ref<uint64_t>::fetch_add]
        // (1) 수신 next_의 정확한 타입은 Reference이고 live/aligned next_id에 결합되어 있다.
        // (2) Value fetch_add(Value difference, memory_order order) const noexcept를 선택한다.
        // (3) difference는 int prvalue 1에서 Value로 변환된 증가량, relaxed는 순서 열거값이며
        //     둘 다 값 인자이고 소유권 전달은 없다.
        // (4) 반환형 Value는 증가 전 값, 즉 이번 호출자가 발급받은 ID이며 지역 합산에 사용한다.
        // (5) underlying next_id는 수정 순서에서 1 증가하고 handle과 인자는 바뀌지 않는다.
        // (6) 모든 동시 접근이 atomic_ref여야 한다. 하나의 원자 RMW라 중복 ID는 없지만 relaxed는
        //     주변 payload를 게시하지 않는다. uint64_t 최대값 뒤에는 modulo 순환한다. 새 Value를
        //     소유하거나 사용자 참조를 무효화하지 않고 예외도 없지만, 구현은 연결된 잠금 같은 내부
        //     공유 자원을 얻을 수 있다. 무할당·공통 점근 복잡도·lock-free·공정성은 보장되지 않는다.
        return next_.fetch_add(1, std::memory_order_relaxed);
    }

private:
    // 숫자 owner는 LegacySequence이고 이 멤버는 수명을 연장하지 않는 observer다.
    Reference next_;
};

// storage는 비소유 lvalue reference 매개변수다. 호출자가 함수보다 오래 살려 둔다.
[[nodiscard]] AllocationSummary allocate_ids(LegacySequence& storage) {
    // 각 합은 한 worker만 쓰므로 서로 데이터 경쟁하지 않는다. main 실행 흐름은 join 뒤에만 읽는다.
    std::uint64_t first_sum{};
    std::uint64_t second_sum{};
    constexpr std::uint64_t attempts_per_worker{4};

    // atomic_ref handle이 존재하는 기간을 이 블록으로 눈에 보이게 제한한다.
    {
        AtomicIdAllocatorView first_view{storage.next_id};
        // 이름 있는 first_view는 lvalue이고 이 직접 초기화는 wrapper/atomic_ref binding을 복사한다.
        AtomicIdAllocatorView second_view{first_view};

        // [첫 호출 계약: std::move]
        // (1) 자유 함수라 수신 객체는 없고 first_view/second_view는 살아 있는 lvalue다.
        // (2) 각 호출은 move<AtomicIdAllocatorView&>(AtomicIdAllocatorView&)이며 반환형은 view&&다.
        // (3) 인자는 비소유 atomic handle wrapper lvalue이고 underlying ID의 소유권은 없다.
        // (4) 반환 xvalue는 lambda의 ids 캡처 멤버를 직접 초기화하며 별도 값 반환 저장소는 없다.
        // (5) move 자체는 아무 상태도 바꾸지 않고 wrapper 이동 생성자가 같은 binding을 복사한다.
        // (6) constexpr/noexcept이며 복잡도는 상수 시간이고 무할당이다. owner 수명을 늘리지 않으며
        //     이 클래스 계약상 결합되어 있지만 다시 쓰지 않고 블록 끝까지 살려 둔다.

        // [첫 생성 계약: std::jthread(callable)]
        // (1) first/second 목적 객체는 아직 없고 closure가 빌리는 first_sum/second_sum과 storage는
        //     바깥에 살아 있다. ids 캡처는 atomic view 값을 소유한다.
        // (2) template<class F,class... Args> explicit jthread(F&&,Args&&...)에서 F는 lambda 타입,
        //     Args는 빈 팩이다. lambda가 stop_token을 받지 않아 인자 없는 호출 경로가 선택된다.
        // (3) lambda 식은 prvalue로 thread 실행 상태에 이동된다. ids는 xvalue view로 초기화되고,
        //     attempts는 uint64_t 값 복사, &sum은 원본을 빌리는 reference capture다.
        // (4) 생성자는 반환값이 없으며 성공한 jthread는 joinable thread 하나를 독점 소유한다.
        // (5) 두 thread는 즉시 실행될 수 있고 각자 다른 sum만 비원자 쓰며 같은 next_id는 원자 갱신한다.
        // (6) 생성은 system_error를 던질 수 있고, 둘째 생성 실패 때 첫째는 stack unwinding 중 RAII로
        //     정리된다. 블록 끝 joinable 소멸자는 request_stop 후 join하고 반환값은 없다. lambda가
        //     token을 받지 않아 유한 loop를 끝낸 뒤 합류한다. 성공한 join은 worker 쓰기와 이후 읽기를
        //     동기화한다. 공통 점근 복잡도는 지정되지 않고, worker 예외는 terminate하며 자원은 구현별이다.
        std::jthread first{
            [ids = std::move(first_view), attempts_per_worker, &first_sum]() noexcept {
                // for는 초기화 -> 비교 -> 본문 -> 증가 순서다. i는 값 초기화되어 0부터 시작한다.
                for (std::uint64_t i{}; i < attempts_per_worker; ++i) {
                    first_sum += ids.next();
                }
            }};
        std::jthread second{
            [ids = std::move(second_view), attempts_per_worker, &second_sum]() noexcept {
                for (std::uint64_t i{}; i < attempts_per_worker; ++i) {
                    second_sum += ids.next();
                }
            }};

        // [첫 숨은 호출 계약: scope-exit std::jthread 소멸자와 join]
        // (1) 수신자는 joinable second와 first이며 각 closure가 sum 참조와 atomic view를 가진다.
        // (2) 인자 없는 ~jthread()가 내부 request_stop()과 join()을 순서대로 수행한다.
        // (3) 별도 데이터 인자는 없고 각 jthread가 소유한 실행 handle만 소비한다.
        // (4) 소멸자와 내부 join의 결과는 저장하지 않으며 반환 값 객체가 없다.
        // (5) second, first가 역순 파괴되고 join 뒤 두 sum 쓰기가 완료된 다음 closure의 ids가 파괴된다.
        // (6) join 성공은 worker 완료와 뒤 plain read를 동기화한다. 표준은 공통 점근 복잡도,
        //     대기 시간 상한·공정성·내부 할당 방식을 보장하지 않는다. self-join이나 noexcept 소멸 중
        //     join 오류는 terminate 위험이 있으나 여기서는 owner thread에서 파괴한다. 이어 두 원래
        //     view도 파괴되며 underlying next_id는 파괴·복원되지 않는다.
    }

    // 모든 atomic_ref handle과 worker가 사라진 뒤이므로 여기서 처음 plain read를 한다.
    // 이 문장을 위 블록 안으로 옮기면 atomic 접근과 plain 접근을 섞는 잘못된 코드가 된다.
    const std::uint64_t plain_next_id{storage.next_id};

    // 스케줄에 따라 각 sum은 달라질 수 있지만 두 합은 정확히 발급 ID 1..8 전체의 합이다.
    // AllocationSummary{...} prvalue가 함수 결과를 직접 초기화하므로 중간 복사/이동이 없다.
    // 이는 C++17 보장 prvalue 복사 생략이며 흔히 넓은 뜻의 RVO라고 설명한다.
    return AllocationSummary{first_sum + second_sum, plain_next_id};
}

int main() {
    // owner를 가장 먼저 만들고 값 1을 소유하게 한다.
    LegacySequence sequence{};
    // 반환 prvalue를 const 값 객체로 직접 초기화한다. summary는 원본을 참조하지 않는다.
    const AllocationSummary summary{allocate_ids(sequence)};

    // [첫 호출 계약: std::cout 스트림 삽입 operator<< 연쇄]
    // (1) 왼쪽 수신은 main thread가 단독 사용하는 정상 상태 std::ostream 객체 std::cout이다.
    // (2) 구현의 uint64_t 기저 정수용 정수 삽입과 char 삽입 overload가 순서대로 선택된다.
    // (3) 두 멤버는 uint64_t lvalue 값 입력, ' '와 '\n'은 char prvalue이며 소유권 이동은 없다.
    // (4) 매 호출은 같은 std::ostream&를 반환해 다음 <<에 쓰고 마지막 반환 참조는 버린다.
    // (5) 성공하면 "36 9\n" 문자가 순서대로 기록되고 summary와 sequence는 그대로다.
    // (6) 형식화 복잡도는 출력 자릿수에 비례하며 buffering/locale/OS 비용이 있다. 실패하면 stream state가
    //     바뀌고 exceptions mask에 따라 예외가 가능하다. 참조 무효화나 동시 출력 원자성에 기대지 않는다.
    std::cout << summary.issued_sum << ' ' << summary.next_id << '\n';

    // 여덟 번 발급했으므로 ID 집합은 순서와 무관하게 1..8, 합은 36, 다음 값은 9다.
    // 실제 load/store/RMW/비교/분기/시스템 호출의 명령 모양은 CPU, ABI, 라이브러리,
    // 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 하나로 단정하지 않는다.
    return (summary.issued_sum == 36 && summary.next_id == 9 && sequence.next_id == 9) ? 0 : 1;
}
