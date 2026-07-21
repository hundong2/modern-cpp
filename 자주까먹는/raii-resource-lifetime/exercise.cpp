#include <cstddef> // 활성 자원 수를 표현하는 std::size_t를 제공한다.
#include <iostream> // 스코프 전후의 활성 자원 수를 출력한다.
#include <utility> // 이동 생성자에서 std::exchange로 소유 표시를 이전한다.

class ResourceCounter final { // 실제 OS 자원 대신 획득과 반환 횟수를 검증하는 교육용 저장소다.
public: // Lease만 호출할 획득·반환 연산과 관찰 함수를 공개한다.
    void acquire() noexcept { ++active_; } // 단일 스레드 실습에서 자원 하나가 열렸음을 기록한다.
    void release() noexcept { --active_; } // 소유 Lease가 파괴될 때 자원 하나가 반환됐음을 기록한다.
    [[nodiscard]] std::size_t active() const noexcept { return active_; } // 현재 살아 있는 자원 수를 값으로 반환한다.

private: // 실습자가 횟수를 임의로 덮어쓰지 못하게 상태를 숨긴다.
    std::size_t active_{0U}; // 이 예제는 단일 스레드이므로 원자 타입이나 mutex가 필요 없다.
}; // ResourceCounter 타입 정의를 끝낸다.

class Lease final { // ResourceCounter의 자원 한 개를 유일 소유하는 RAII guard를 정의한다.
public: // 획득, 반환, 이동에 필요한 수명 연산을 공개한다.
    explicit Lease(ResourceCounter& counter) noexcept : counter_{&counter} { // 비소유 포인터로 저장소를 기억하고 소유 상태를 만든다.
        counter_->acquire(); // 생성이 성공한 즉시 자원 한 개를 획득했다고 기록한다.
    } // Lease 객체와 자원 소유 수명이 동시에 시작된다.
    ~Lease() noexcept { // 스코프 종료와 스택 해제에서 자동 실행될 정리 연산을 정의한다.
        if (counter_ != nullptr) { // 이동된 원본은 더 이상 자원을 소유하지 않으므로 중복 반환을 막는다.
            counter_->release(); // 살아 있는 저장소에 자원을 정확히 한 번 반환한다.
        } // 소유 상태 검사 분기를 끝낸다.
    } // 소멸자는 실패를 보고하기보다 자원 반환을 끝내고 예외 없이 종료한다.
    Lease(const Lease&) = delete; // 한 자원을 두 Lease가 반환하는 복사 의미를 컴파일 타임에 금지한다.
    Lease& operator=(const Lease&) = delete; // 복사 대입으로 유일 소유권이 중복되는 것도 막는다.
    Lease(Lease&& other) noexcept : counter_{std::exchange(other.counter_, nullptr)} { } // 포인터를 이전하고 원본을 비소유 상태로 만든다.
    Lease& operator=(Lease&&) = delete; // TODO 1: 기존 소유 자원을 먼저 반환하는 이동 대입을 직접 구현해 본다.

private: // 반환 책임을 외부에서 바꾸지 못하도록 소유 표시를 숨긴다.
    ResourceCounter* counter_; // null이면 비소유, non-null이면 저장소 자원 하나의 반환 책임을 뜻한다.
}; // Lease 타입 정의를 끝낸다.

void leave_early(ResourceCounter& counter) { // 조기 반환에서도 RAII가 동작하는지 확인하는 함수를 정의한다.
    Lease first{counter}; // 첫 자원을 획득하고 이 블록의 자동 객체로 소유한다.
    Lease second{counter}; // 두 번째 자원을 획득하며 first보다 먼저 파괴될 예정이다.
    Lease moved{std::move(second)}; // 두 번째 반환 책임을 moved로 옮기고 second를 빈 상태로 만든다.
    std::cout << "inside active=" << counter.active() << '\n'; // 이동은 소유자만 바꾸므로 활성 자원 수가 2임을 확인한다.
    static_cast<void>(first); // 교육용 변수의 의도적 사용을 표시해 경고를 피한다.
    static_cast<void>(moved); // moved가 현재 두 번째 자원을 소유함을 코드에 드러낸다.
    return; // moved, second, first 역순으로 파괴되지만 실제 release는 moved와 first만 수행한다.
} // 조기 반환으로 이미 블록을 떠났으므로 여기에는 별도 정리 코드가 없다.

int main() { // 실습 guard의 획득·이동·반환 불변식을 검증하는 진입점이다.
    ResourceCounter counter; // Lease보다 오래 살아야 하는 저장소를 먼저 생성한다.
    leave_early(counter); // 함수 내부 조기 반환 뒤에도 모든 자원이 반환되어야 한다.
    std::cout << "after active=" << counter.active() << '\n'; // 기대값 0으로 누수와 이중 반환이 없음을 확인한다.
    // TODO 2: leave_early에서 예외를 던지고 main에서 잡아도 after active=0인지 확인한다.
    // TODO 3: Lease가 ResourceCounter보다 오래 살 수 없다는 계약을 타입으로 더 강하게 표현할 방법을 설명한다.
    return 0; // 성공을 반환하며 counter를 마지막으로 파괴한다.
} // 실습 프로그램 실행을 끝낸다.
