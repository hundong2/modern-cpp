// <chrono>는 밀리초·초처럼 단위가 타입에 포함된 시간 타입과 변환 함수를 제공한다.
#include <chrono>
// <iostream>은 표준 출력 객체 std::cout과 출력 연산자 <<를 제공한다.
#include <iostream>
// <memory>는 단독 소유권을 표현하는 std::unique_ptr와 std::make_unique를 제공한다.
#include <memory>
// <utility>는 lvalue를 이동 가능한 xvalue로 표현하는 std::move를 제공한다.
#include <utility>

// using은 새 타입을 만드는 것이 아니라 긴 타입 이름에 읽기 쉬운 별칭을 붙인다.
using Duration = std::chrono::milliseconds;
// time_point의 첫 템플릿 인자는 기준 시계, 두 번째는 그 시계에서 사용할 시간 단위이다.
using TimePoint = std::chrono::time_point<std::chrono::steady_clock, Duration>;

// struct는 기본 접근이 public이다. 판단 결과처럼 공개 데이터 묶음에 알맞다.
struct Decision {
    bool retry{};          // bool은 참/거짓을 저장하며 {}는 false로 값 초기화한다.
    Duration wait{};       // 멤버 변수는 객체마다 보관되는 상태이며 {}는 0ms로 초기화한다.
};

// class는 기본 접근이 private이다. 규칙을 불변식과 함께 감추는 데 알맞다.
class RetryPolicy {
public:
    // 생성자는 반환형이 없다. int와 Duration은 생성자 매개변수의 타입이다.
    RetryPolicy(int max_failures, Duration interval)
        // 멤버 초기화 목록은 생성자 본문 전에 멤버를 직접 초기화한다.
        : max_failures_{max_failures}, interval_{interval} {}

    // 반환형 bool, 매개변수 int인 함수다. 뒤의 const는 멤버를 바꾸지 않겠다는 약속이다.
    [[nodiscard]] bool allows(int failures) const {
        // <는 두 기본 정수 값을 비교하는 연산자이며, 결과를 호출자에게 반환한다.
        return failures < max_failures_;
    }

    // const 멤버 함수 호출로 저장된 시간 간격의 값을 복사해 반환한다.
    [[nodiscard]] Duration interval() const { return interval_; }

private:
    int max_failures_{};   // private 멤버는 클래스 외부에서 직접 접근할 수 없다.
    Duration interval_{};  // 시간 단위가 타입에 포함되어 초와 밀리초 혼동을 줄인다.
};

// 현재 시간을 얻는 아키텍처 포트다. struct 기본 public 때문에 public:을 생략할 수 있다.
struct Clock {
    // 가상 소멸자는 기반 포인터로 파생 객체를 삭제할 때 올바른 소멸을 보장한다.
    virtual ~Clock() = default;
    // = 0은 순수 가상 함수다. 구현 어댑터가 now를 반드시 재정의해야 한다.
    [[nodiscard]] virtual TimePoint now() const = 0;
};

// final은 더 이상 상속하지 않을 구체 테스트 어댑터임을 표현한다.
class ManualClock final : public Clock {
public:
    // explicit은 TimePoint 하나가 ManualClock으로 암시적 변환되는 것을 막는다.
    // 올바른 사용은 ManualClock clock{TimePoint{Duration{0}}}; 같은 직접 초기화다.
    explicit ManualClock(TimePoint start) : current_{start} {}

    // override는 기반 클래스의 가상 함수 서명과 정확히 일치하는지 검사하게 한다.
    [[nodiscard]] TimePoint now() const override { return current_; }

    // void는 반환값이 없다는 뜻이고, amount는 복사로 전달되는 함수 매개변수다.
    void advance(Duration amount) {
        // += 연산자는 기존 시간 지점에 기간을 더해 멤버 변수에 다시 저장한다.
        current_ += amount;
    }

private:
    TimePoint current_{}; // 객체가 살아 있는 동안 가짜 현재 시간을 소유한다.
};

class RetryService {
public:
    // unique_ptr<Clock>의 템플릿 인자 Clock은 소유할 객체의 기반 타입이다.
    explicit RetryService(std::unique_ptr<Clock> clock, RetryPolicy policy)
        // std::move의 결과는 xvalue이며 unique_ptr 이동 생성이 단독 소유권을 clock_으로 옮긴다.
        // + 연산자는 시작 시각과 정책 간격을 더해 첫 재시도 마감 시각을 만든다.
        : clock_{std::move(clock)}, policy_{policy}, deadline_{clock_->now() + policy_.interval()} {}

    // 참조는 별칭이며 포인터와 달리 정상적으로 바인딩된 뒤 null일 수 없다.
    [[nodiscard]] const Clock& clock() const { return *clock_; }

    [[nodiscard]] Decision decide(int failures) const {
        // 가상 now 호출은 구체 어댑터를 간접 선택한다. 실제 간접 호출 여부는 최적화에 따라 달라진다.
        const TimePoint current{clock_->now()};
        // >= 비교는 마감 시각 도달 여부를 bool로 만든다. 이름 있는 current와 deadline_은 lvalue 식이다.
        const bool due{current >= deadline_};
        // &&는 왼쪽이 false이면 오른쪽을 평가하지 않는 단락 논리 AND 연산자다.
        if (due && policy_.allows(failures)) {
            // Decision{...}은 prvalue이며 반환 목적지에 직접 구성되어 복사·이동이 생략될 수 있다.
            return Decision{true, policy_.interval()};
        }
        return Decision{false, Duration{0}};
    }

private:
    std::unique_ptr<Clock> clock_; // RetryService가 Clock 객체의 유일한 소유자다.
    RetryPolicy policy_;           // 값으로 소유하므로 서비스와 수명이 같다.
    TimePoint deadline_;           // 생성 시각을 저장한 뒤 판단에서 읽는다.
};

int main() { // main의 반환형 int는 운영체제에 전달할 종료 상태 코드다.
    // chrono 리터럴을 쓰기 위해 이 블록에서 std::chrono_literals 이름을 가져온다.
    using namespace std::chrono_literals;

    // make_unique의 결과는 prvalue unique_ptr이고, 가리키는 ManualClock 객체도 동적으로 생성된다.
    auto manual{std::make_unique<ManualClock>(TimePoint{Duration{0}})};
    // get은 소유권을 옮기지 않는 관찰 포인터를 반환한다. 포인터는 null 가능 별칭이다.
    ManualClock* const observer{manual.get()};
    // RetryPolicy{3, 2s}는 prvalue이고 2s는 seconds 타입의 prvalue이다.
    RetryService service{std::move(manual), RetryPolicy{3, std::chrono::duration_cast<Duration>(2s)}};

    // for는 초기화, 계속 조건, 증가 식으로 3번 반복한다. int는 기본 정수 타입이다.
    for (int step{0}; step < 3; ++step) {
        // ->는 포인터가 가리키는 객체의 멤버 함수를 호출한다.
        observer->advance(1s);
        const Decision decision{service.decide(1)};
        // <<는 값들을 출력 스트림에 차례로 보내는 연산자다. count()는 저장 숫자를 돌려준다.
        std::cout << "step=" << step << ", retry=" << decision.retry
                  << ", wait_ms=" << decision.wait.count() << '\n';
    }

    // const 참조는 객체를 복사하지 않고 빌리며 service가 살아 있는 동안 대상 수명이 유지된다.
    const Clock& clock_ref{service.clock()};
    // dynamic_cast는 다형 기반 참조가 실제 ManualClock인지 안전하게 검사하며 실패하면 null 포인터다.
    const auto* const checked{dynamic_cast<const ManualClock*>(&clock_ref)};
    // !=와 == 비교, && 단락 평가, ?: 조건 연산자로 테스트 성공 여부를 종료 코드로 바꾼다.
    return (checked != nullptr && service.decide(1).retry == true) ? 0 : 1;
}
