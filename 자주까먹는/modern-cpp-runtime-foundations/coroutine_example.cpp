#include <coroutine> // C++20 코루틴 핸들, suspend_always와 언어 변환 계약을 제공한다.
#include <exception> // 처리하지 못한 예외에서 std::terminate를 호출하기 위해 포함한다.
#include <iostream> // suspend와 resume 순서를 눈으로 확인할 메시지를 출력한다.
#include <utility> // 코루틴 프레임 소유 핸들을 교환하는 std::exchange를 제공한다.

class ManualTask final { // 코루틴 프레임 하나를 유일하게 소유하는 최소 교육용 반환 타입이다.
public: // 컴파일러가 co_await 함수 변환에 사용하는 promise 계약을 공개한다.
    struct promise_type final { // 반환 객체, 중단 지점, 종료 정책을 컴파일러에 알려 주는 타입이다.
        [[nodiscard]] ManualTask get_return_object() noexcept { // 생성된 promise에서 외부 소유 객체를 만든다.
            return ManualTask{std::coroutine_handle<promise_type>::from_promise(*this)}; // 프레임 내부 promise 주소로 타입 안전 핸들을 복원한다.
        } // 반환 객체 생성을 끝낸다.
        [[nodiscard]] std::suspend_always initial_suspend() const noexcept { return {}; } // 호출 즉시 본문을 실행하지 않고 호출자에게 제어권을 돌린다.
        [[nodiscard]] std::suspend_always final_suspend() const noexcept { return {}; } // 완료 프레임을 유지해 소유 객체가 안전하게 destroy하게 한다.
        void return_void() const noexcept { } // co_return 값이 없는 코루틴의 정상 완료를 표현한다.
        [[noreturn]] void unhandled_exception() const noexcept { std::terminate(); } // 교육 예제에서 예외 전달 상태가 없으므로 즉시 종료한다.
    }; // 컴파일러가 요구하는 promise_type 정의를 끝낸다.

    using handle_type = std::coroutine_handle<promise_type>; // 반복되는 구체 핸들 타입에 읽기 쉬운 별칭을 붙인다.

    explicit ManualTask(const handle_type handle) noexcept : handle_{handle} { } // 새 프레임 핸들의 유일한 파괴 책임을 받는다.
    ~ManualTask() { if (handle_) { handle_.destroy(); } } // 소멸 시 프레임과 그 안의 지역 객체를 한 번만 정리한다.
    ManualTask(const ManualTask&) = delete; // 같은 핸들을 두 객체가 destroy하는 이중 해제를 금지한다.
    ManualTask& operator=(const ManualTask&) = delete; // 프레임 소유권 복사 대입도 금지한다.
    ManualTask(ManualTask&& other) noexcept : handle_{std::exchange(other.handle_, {})} { } // 주소만 이전하고 원본 핸들을 null로 만든다.
    ManualTask& operator=(ManualTask&&) = delete; // 예제에 필요 없는 이동 대입을 막아 수명 규칙을 단순하게 유지한다.

    [[nodiscard]] bool resume() { // 외부 이벤트 루프가 호출한다고 가정한 명시적 재개 연산이다.
        if (!handle_ || handle_.done()) { // null 핸들이거나 final_suspend에 도달한 프레임은 재개할 수 없다.
            return false; // 실행하지 않았음을 호출자에게 값으로 알린다.
        } // 안전성 검사를 끝낸다.
        handle_.resume(); // 저장된 명령 위치와 지역 상태를 복원해 다음 중단점까지 현재 스레드에서 실행한다.
        return !handle_.done(); // 실행 뒤 추가 resume가 필요한지를 반환한다.
    } // 재개 연산을 끝낸다.

private: // 프레임 파괴 권한을 ManualTask 구현 안으로 제한한다.
    handle_type handle_{}; // 보통 힙에 놓이는 코루틴 프레임을 가리키는 작고 비소유적인 저수준 핸들이다.
}; // ManualTask 소유 타입 정의를 끝낸다.

struct EventAwaiter final { // 실제 epoll 또는 IOCP 등록을 대신하는 교육용 awaiter다.
    [[nodiscard]] bool await_ready() const noexcept { return false; } // 이벤트가 아직 없다고 알려 항상 중단 경로를 선택한다.
    void await_suspend(std::coroutine_handle<>) const noexcept { } // 실무에서는 여기서 핸들을 이벤트 루프에 등록해야 한다.
    void await_resume() const noexcept { } // 재개 시 전달할 값이나 예외가 없으므로 아무 작업도 하지 않는다.
}; // 중단 프로토콜 세 함수를 가진 awaiter 정의를 끝낸다.

ManualTask handle_client() { // promise_type이 있어 컴파일러가 상태 머신으로 변환할 코루틴을 정의한다.
    std::cout << "client: start and register I/O\n"; // 첫 resume에서 실행되어 I/O 등록 직전 상태를 보여 준다.
    co_await EventAwaiter{}; // 현재 명령 위치를 프레임에 저장하고 호출 스레드에 제어권을 반환한다.
    std::cout << "client: resumed after readiness/completion event\n"; // 두 번째 resume가 이벤트 통지를 흉내 낼 때 실행된다.
} // 암시적 co_return 뒤 final_suspend에 도달해 프레임은 소유 객체의 destroy까지 유지된다.

int main() { // 수동 이벤트 루프의 두 번의 tick을 흉내 내는 진입점이다.
    auto task = handle_client(); // 함수 본문은 initial_suspend 때문에 아직 실행되지 않고 프레임만 생성된다.
    std::cout << "loop: first tick\n"; // 첫 스케줄링 시점을 출력한다.
    static_cast<void>(task.resume()); // 코루틴을 시작해 EventAwaiter 중단점까지 현재 스레드에서 실행한다.
    std::cout << "loop: I/O event arrived\n"; // epoll readiness 또는 IOCP completion 통지를 모의한다.
    static_cast<void>(task.resume()); // 중단 지점 다음부터 실행해 코루틴을 완료시킨다.
    return 0; // task 소멸자가 완료된 프레임을 destroy하고 성공을 반환한다.
} // 코루틴 수명과 프로그램 실행을 끝낸다.
