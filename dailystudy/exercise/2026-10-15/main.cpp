// 오늘의 실무 패턴: 기존 구조체가 소유한 평범한 정수 저장소는 그대로 두고,
// std::atomic_ref를 사용하는 짧은 비소유 어댑터만 동시 접근 경계에 둔다.
// 핵심 수명 순서는 owner(LegacyMetrics) 생성 -> atomic_ref view 생성 -> worker join
// -> 모든 view 파괴 -> owner의 plain read 이다. 이 순서를 어기면 미정의 동작이며,
// worker의 접근과 plain 접근이 겹치면 데이터 경쟁도 된다.

// <atomic>은 외부 객체를 원자적으로 바라보는 std::atomic_ref와 메모리 순서를 선언한다.
#include <atomic>
// <cstdint>는 폭이 정확히 64비트인 std::uint64_t 정수 별칭을 선언한다.
#include <cstdint>
// <iostream>은 결과를 표준 출력에 쓰는 std::cout과 스트림 삽입 연산을 선언한다.
#include <iostream>
// <thread>는 실행 스레드의 수명을 RAII로 소유하는 C++20 std::jthread를 선언한다.
#include <thread>
// <utility>는 이름 있는 객체를 xvalue 식으로 바꾸는 std::move를 선언한다.
#include <utility>

// struct는 기본 접근 지정자가 public이다. 이 타입은 필드 타입을 std::atomic으로 바꾸기 어려운
// 외부 소유 저장소를 작게 흉내 내되, 오늘 예제에서는 필요한 정렬을 처음부터 선언할 수 있다고 가정한다.
struct LegacyMetrics {
    // alignas는 completed의 주소가 atomic_ref<uint64_t>가 요구하는 정렬을 만족하게 한다.
    // required_alignment는 구현별 상수일 수 있으므로 alignof(uint64_t)와 같다고 단정하지 않는다.
    // 이미 동결된 C ABI/파일 레이아웃은 alignas 추가로 크기·offset이 달라질 수 있으므로, 그 경우에는
    // 실제 기존 주소가 요구 정렬을 만족하는지 확인해야 하며 부족한 저장소에 atomic_ref를 쓰면 안 된다.
    // std::uint64_t는 기본 정수 타입 별칭이고, 빈 중괄호 {}는 값을 0으로 초기화한다.
    alignas(std::atomic_ref<std::uint64_t>::required_alignment)
        std::uint64_t completed{};
};

// 반환 전용 값 객체다. 참조나 포인터를 담지 않으므로 원본 저장소보다 오래 살아도 안전하다.
struct CounterReport {
    std::uint64_t completed{};
};

// class는 기본 접근이 private이다. public API만 원자 접근을 허용하고,
// 실제 atomic_ref handle은 private으로 숨겨 plain/atomic 접근 혼용을 줄인다.
class AtomicCounterView {
public:
    // using은 새 타입을 만드는 문법이 아니라 긴 표준 타입에 읽기 쉬운 별칭을 붙인다.
    using Value = std::uint64_t;
    // 꺾쇠 안 Value는 atomic_ref 클래스 템플릿의 템플릿 인자다.
    using Reference = std::atomic_ref<Value>;

    // [첫 생성 계약: std::atomic_ref<Value>(Value&)]
    // (1) 목적 객체는 아직 생성되지 않은 completed_이고, storage는 살아 있는 정렬된 Value 객체다.
    // (2) 선택되는 표준 선언은 explicit atomic_ref<T>::atomic_ref(T&), T=Value다.
    //     표준은 Throws: Nothing을 규정하지만 이 생성자 선언 자체에는 noexcept가 붙지 않는다.
    // (3) 유일한 인자 storage는 이름 있는 lvalue, 정확한 타입 Value&, 소유권을 넘기지 않는 참조다.
    // (4) 생성자는 반환값이 없고, 새 atomic_ref handle이 storage를 가리키게 한다.
    // (5) storage의 숫자 값과 소유권은 바뀌지 않으며 completed_만 비소유로 결합된다.
    // (6) T는 trivially copyable이어야 하고 storage는 required_alignment를 만족하며 handle보다
    //     오래 살아야 한다. handle이 하나라도 살아 있는 동안 storage에 대한 모든 접근은
    //     atomic_ref를 통해야 한다. 생성은 새 Value를 소유하거나 사용자의 저장소·참조를
    //     이동/무효화하지 않고 표준상 예외를 던지지 않지만(Throws: Nothing), 함수 타입 자체가
    //     noexcept인 것은 아니다. 구현은 연결된 잠금 같은 내부 공유 자원을 얻을 수 있으므로
    //     무할당이라고 단정할 수 없고, 공통 점근 복잡도나 lock-free도 보장되지 않는다.
    explicit AtomicCounterView(Value& storage) noexcept
        : completed_{storage} {}

    // [첫 생성 계약: atomic_ref 복사 생성]
    // (1) other.completed_는 살아 있는 Reference lvalue이며 유효한 Value 객체에 결합되어 있다.
    // (2) Reference의 복사 생성자 atomic_ref(const atomic_ref&) noexcept가 선택된다.
    // (3) 인자는 const Reference&로 빌려 읽으며 숫자 저장소의 소유권은 복사하지 않는다.
    // (4) 생성자는 반환값이 없고 새 handle 하나를 만든다.
    // (5) 두 handle은 같은 Value 객체를 가리키며 원본 handle과 숫자 값은 그대로다.
    // (6) 새 Value를 소유하거나 사용자 참조를 무효화하지 않는 noexcept 비소유 복사이며 대상 수명을
    //     늘리지 않는다. 다만 표준은 공통 점근 복잡도나 내부 공유 자원 획득 부재를 보장하지 않는다.
    //     두 handle의 원자 연산은 같은 대상에 대해 서로 원자적으로 상호작용하지만 plain 접근은 금지다.
    AtomicCounterView(const AtomicCounterView& other) noexcept
        : completed_{other.completed_} {}

    // 이 wrapper에서 "이동"도 저장소를 옮길 수 없다. atomic_ref는 대상을 소유하지 않고
    // 다시 결합할 수도 없으므로, 이동 생성은 의도적으로 같은 binding을 하나 더 복사한다.
    // other도 여전히 같은 저장소에 결합되어 있지만 이동 뒤에는 사용하지 않는 규칙을 따른다.
    AtomicCounterView(AtomicCounterView&& other) noexcept
        : completed_{other.completed_} {}

    // 반환형 void는 결과 값이 없다는 뜻이고, 뒤의 const는 wrapper의 binding을 바꾸지 않음을 뜻한다.
    void increment() const noexcept {
        // [첫 호출 계약: atomic_ref<uint64_t>::fetch_add]
        // (1) 수신 객체 completed_의 정확한 타입은 Reference이며 살아 있고 정렬된 uint64_t에 결합된다.
        // (2) 정수 특수화의 Value fetch_add(Value, memory_order) const noexcept를 선택한다.
        // (3) 첫 인자 1은 int prvalue에서 Value 1로 변환되는 증가량이고 소유권 의미가 없다.
        //     둘째 인자 memory_order_relaxed는 주변 메모리 공개를 요구하지 않는 순서 열거값이다.
        // (4) 반환형 Value는 증가 전 값이며 static_cast<void>로 의도적으로 버린다.
        // (5) underlying completed는 수정 순서 안에서 정확히 1 증가하고 handle binding은 그대로다.
        // (6) 대상 lifetime/alignment와 모든 동시 접근의 atomic_ref 사용이 전제다. 연산은 하나의
        //     원자 read-modify-write지만 다른 메모리와 happens-before를 만들지 않는다. uint64_t는
        //     최대값 뒤 modulo 순환한다. 새 Value를 소유하거나 사용자 참조를 무효화하지 않지만 구현은
        //     연결된 잠금 같은 내부 공유 자원을 얻을 수 있다. 표준은 무할당·공통 점근 복잡도·
        //     lock-free·대기 상한·정확한 명령을 보장하지 않는다. 같은 대상의 원자 연산끼리는 안전하다.
        static_cast<void>(completed_.fetch_add(1, std::memory_order_relaxed));
    }

    [[nodiscard]] Value load() const noexcept {
        // [첫 호출 계약: atomic_ref<uint64_t>::load]
        // (1) 수신 completed_는 살아 있는 대상에 결합된 const 문맥의 Reference다.
        // (2) Value load(memory_order) const noexcept 오버로드를 선택한다.
        // (3) 유일한 인자 memory_order_relaxed는 데이터가 아니라 읽기 순서이며 허용된 load 순서다.
        // (4) 반환형 Value는 호출이 관찰한 값의 독립 복사본이고 CounterReport 초기화에 사용한다.
        // (5) underlying 값과 binding, 인자는 바뀌지 않는다.
        // (6) 대상 수명/정렬/atomic-only 접근 전제가 같고 사용자 참조 무효화나 예외는 없다.
        //     구현은 내부 공유 자원을 얻을 수 있어 무할당이 보장되지 않는다. relaxed load는 다른
        //     payload를 게시하지 않으며 완료 가시성은 뒤 설명처럼 jthread join이 만든다. 표준은 공통
        //     점근 복잡도·lock-free·공정성을 보장하지 않고 실제 구현은 환경에 따라 달라진다.
        return completed_.load(std::memory_order_relaxed);
    }

private:
    // atomic_ref는 숫자를 소유하지 않는다. LegacyMetrics가 모든 view보다 오래 살아야 한다.
    Reference completed_;
};

// 여러 실행 흐름을 만들고 회수하는 응용 계층이다. 저장소 자체가 아니라 view를 값으로 보관한다.
class CounterWorkers {
public:
    // explicit은 AtomicCounterView 하나가 CounterWorkers로 암시 변환되는 것을 막는다.
    explicit CounterWorkers(AtomicCounterView counter) noexcept
        // [첫 호출 계약: std::move]
        // (1) 자유 함수라 수신 객체는 없고 counter는 살아 있는 AtomicCounterView lvalue다.
        // (2) move<AtomicCounterView&>(AtomicCounterView&)의 반환형은 AtomicCounterView&&다.
        // (3) 인자 counter는 lvalue이고 소유 숫자가 아닌 atomic binding handle을 가진다.
        // (4) 반환 xvalue는 counter_의 이동 생성에 즉시 사용하며 별도 객체를 만들지 않는다.
        // (5) move 자체는 counter를 바꾸지 않는다. 선택된 wrapper 이동 생성자가 같은 binding을 복사한다.
        // (6) constexpr/noexcept, 상수 시간, 무할당이며 수명을 늘리지 않는다. 이 식만으로 저장소나
        //     스레드 안전성이 이전되는 것은 아니고 대상 owner가 계속 살아 있어야 한다.
        : counter_{std::move(counter)} {}

    // iterations는 값 매개변수라 호출자의 정수와 독립이다. const 멤버 함수는 counter_ binding을
    // 재지정하지 않지만, atomic_ref의 const 멤버를 통해 외부 숫자를 원자적으로 바꿀 수 있다.
    [[nodiscard]] CounterReport run(std::uint64_t iterations) const {
        // worker를 안쪽 블록에 두면 return용 load 전에 두 jthread 소멸자가 반드시 실행된다.
        {
            // [첫 생성 계약: std::jthread(callable)]
            // (1) first/second 목적 객체는 아직 없고, this가 가리키는 CounterWorkers와 counter_는
            //     바깥 run 호출이 끝날 때까지 살아 있다. iterations는 lambda가 값으로 소유한다.
            // (2) template<class F,class... Args> explicit jthread(F&&,Args&&...)에서 F는 각 lambda 타입,
            //     Args는 빈 팩이다. lambda가 stop_token을 받지 않아 인자 없는 호출 경로가 선택된다.
            // (3) 각 lambda 식은 prvalue라 jthread 실행 상태로 이동되며 this는 비소유 포인터 복사,
            //     iterations는 uint64_t 값 복사다. 어떤 객체의 소유권도 worker에게 넘기지 않는다.
            // (4) 생성자는 반환값이 없고 성공하면 각 jthread가 joinable 실행 스레드를 소유한다.
            // (5) worker는 즉시 시작될 수 있고 두 실행은 같은 atomic_ref 대상만 원자 갱신한다.
            // (6) OS 자원 부족이면 system_error가 가능하다. 둘째 생성이 실패해도 첫째 jthread의 RAII
            //     소멸이 정리를 맡는다. 블록 끝의 joinable jthread 소멸자는 request_stop 후 join하며
            //     반환값은 없다. lambda는 stop token을 받지 않으므로 유한 loop를 자연 종료한다.
            //     성공한 join은 thread 완료와 이후 load 사이를 동기화한다. 생성·합류의 공통 점근 복잡도와
            //     시간/스케줄/내부 할당·시스템 호출은 구현별이고 worker 예외 탈출은 terminate다.
            std::jthread first{[this, iterations]() noexcept {
                // i{}는 uint64_t 0, ++i는 다음 반복을 위한 정수 증가다.
                // 조건 비교와 분기는 매 반복 일어나지만 최적화된 기계 명령은 컴파일러가 정한다.
                for (std::uint64_t i{}; i < iterations; ++i) {
                    counter_.increment();
                }
            }};
            std::jthread second{[this, iterations]() noexcept {
                for (std::uint64_t i{}; i < iterations; ++i) {
                    counter_.increment();
                }
            }};

            // [첫 숨은 호출 계약: scope-exit std::jthread 소멸자와 join]
            // (1) 수신자는 이 블록의 joinable second와 first이고 두 callable은 유한 loop를 실행 중일 수 있다.
            // (2) 각 ~jthread()는 인자 없는 소멸자이며 내부적으로 request_stop() 뒤 join()을 수행한다.
            // (3) 명시 데이터 인자나 소유권 입력은 없고, 각 객체가 이미 소유한 thread handle을 사용한다.
            // (4) 소멸자/request_stop/join의 반환값은 저장하지 않으며 소멸자는 별도 결과 객체를 만들지 않는다.
            // (5) 역순으로 second, first가 파괴되고 성공한 join 뒤 두 실행은 모두 끝나며 handle도 닫힌다.
            // (6) join은 완료와 이후 실행을 동기화하고 필요하면 막힌다. 표준은 공통 점근 복잡도,
            //     공정성·시간 상한·내부 할당 방식을 보장하지 않는다. 자기 자신 join 같은 오류나 noexcept
            //     소멸 중 join 실패는 terminate로 이어질 수 있다. 이 코드는 소유 worker에서 파괴하므로
            //     self-join이 아니며 atomic_ref와 owner 수명도 유지된다.
        }

        // CounterReport{...}는 prvalue다. C++17 이후 같은 타입 결과 객체를 직접 초기화하므로
        // 중간 CounterReport 복사/이동이 필요 없다(흔히 RVO라 부르지만 정확히 보장된 prvalue 복사 생략).
        return CounterReport{counter_.load()};
    }

private:
    AtomicCounterView counter_;
};

// storage는 lvalue 참조로 빌릴 뿐 소유하지 않는다. 호출자는 이 함수 전체보다 오래 살려야 한다.
[[nodiscard]] CounterReport collect(LegacyMetrics& storage) {
    // AtomicCounterView{...}는 prvalue이고 CounterWorkers 값 매개변수를 직접 초기화할 수 있다.
    // storage.completed는 이름 있는 lvalue이며 view가 그 주소의 수명만 빌린다.
    CounterWorkers workers{AtomicCounterView{storage.completed}};
    // 함수 반환 식은 CounterReport prvalue다. 반환 결과 초기화 뒤 workers가 파괴되므로,
    // 호출자가 다음 문장을 실행할 때는 이 함수가 만들었던 모든 atomic_ref handle이 사라져 있다.
    return workers.run(100'000);
}

int main() {
    // owner를 view보다 먼저 생성한다. 중괄호 값 초기화로 completed는 0이다.
    LegacyMetrics storage{};

    // report는 반환 prvalue로 직접 초기화되는 const 소유 값이다. collect가 반환한 뒤에는 worker,
    // lambda, CounterWorkers와 그 안 atomic_ref가 전부 파괴되었다.
    const CounterReport report{collect(storage)};

    // 이제 동시 실행도 atomic_ref handle도 전혀 없으므로 plain read가 허용된다.
    // 이 읽기를 collect 내부나 worker scope 안으로 옮기면 atomic 접근과 섞여 잘못된 코드가 된다.
    const std::uint64_t plain_value{storage.completed};

    // [첫 호출 계약: std::cout 스트림 삽입 operator<< 연쇄]
    // (1) 수신/왼쪽 객체는 정상 상태의 std::ostream인 std::cout이고 main 한 스레드만 사용한다.
    // (2) 구현의 uint64_t 기저 정수용 정수 삽입 overload, char 삽입 overload가 차례로 선택된다.
    // (3) report.completed와 plain_value는 uint64_t lvalue 값 복사 입력, ' '와 '\n'은 char prvalue다.
    //     출력 스트림 버퍼 소유권이나 숫자 소유권은 이동하지 않는다.
    // (4) 각 호출은 같은 std::ostream&를 반환해 다음 <<의 수신자로 사용하고 마지막 참조는 버린다.
    // (5) 성공하면 문자들이 순서대로 버퍼에 기록되고 값 객체는 그대로다. 실패 시 failbit/badbit가 설 수 있다.
    // (6) 형식화 복잡도는 출력 자릿수에 비례하고 I/O 버퍼/로캘 구현이 관여할 수 있다. exceptions mask가
    //     설정된 오류는 예외가 될 수 있으며 참조 무효화는 없다. 동시 출력 원자성에 기대지 않으며,
    //     실제 store/시스템 호출/버퍼 flush는 표준 라이브러리·OS·최적화에 따라 달라진다.
    std::cout << report.completed << ' ' << plain_value << '\n';

    // 두 worker가 각각 정확히 100000번 증가하므로 스케줄과 관계없이 두 값 모두 200000이어야 한다.
    // &&는 왼쪽이 거짓이면 오른쪽 비교를 생략하는 조건 분기이며, 삼항 연산자가 종료 코드를 고른다.
    return (report.completed == 200'000 && plain_value == report.completed) ? 0 : 1;
}
