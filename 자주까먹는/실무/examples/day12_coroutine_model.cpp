#include <coroutine> // C++20 coroutine protocol 타입을 제공한다.
#include <iostream>  // resume 전후 상태를 출력한다.

class OneShot {                                      // 한 번 suspend한 뒤 resume할 coroutine owner다.
public:                                              // promise/handle 수명 API를 공개한다.
    struct promise_type {                            // 컴파일러가 coroutine frame에 넣을 약속 타입이다.
        OneShot get_return_object() {                // 호출자에게 frame owner를 돌려준다.
            return OneShot{std::coroutine_handle<promise_type>::from_promise(*this)}; // promise에서 대응 handle을 얻는다.
        }                                            // owner가 frame 파괴 책임을 가진다.
        std::suspend_always initial_suspend() noexcept { return {}; } // 본문 시작 전에 suspend한다.
        std::suspend_always final_suspend() noexcept { return {}; }   // 완료 뒤 owner가 destroy할 때까지 보존한다.
        void return_void() noexcept {}               // co_return 값이 없는 coroutine 계약이다.
        void unhandled_exception() { std::terminate(); } // 교육용으로 예외를 즉시 종료 처리한다.
    };                                               // promise 계약 정의를 끝낸다.

    using Handle = std::coroutine_handle<promise_type>; // 타입 안전한 frame handle 별칭이다.
    explicit OneShot(Handle handle) : handle_(handle) {} // 새 handle의 단독 소유권을 받는다.
    OneShot(const OneShot&) = delete;                // frame 이중 파괴를 막으려고 복사를 금지한다.
    OneShot(OneShot&& other) noexcept : handle_(other.handle_) { other.handle_ = {}; } // 소유권을 이동한다.
    ~OneShot() { if (handle_) handle_.destroy(); }   // RAII로 coroutine frame을 정확히 한 번 파괴한다.
    void resume() { if (handle_ && !handle_.done()) handle_.resume(); } // suspend 지점에서 실행을 재개한다.

private:                                             // raw handle 직접 조작을 막는다.
    Handle handle_{};                                // coroutine frame을 가리키며 이 객체가 소유한다.
};                                                   // owner 정의를 끝낸다.

OneShot demo() {                                     // 호출하면 frame을 만들고 initial_suspend에서 멈춘다.
    std::cout << "coroutine body\n";                // resume할 때 비로소 실행된다.
    co_return;                                       // final_suspend로 이동해 frame은 아직 남는다.
}                                                    // coroutine 함수 정의를 끝낸다.

int main() {                                         // suspend/resume 수명을 시험한다.
    OneShot task = demo();                           // 본문은 아직 실행되지 않고 handle만 받는다.
    std::cout << "before resume\n";                 // 초기 suspend가 작동했음을 보여 준다.
    task.resume();                                   // 같은 OS thread에서 coroutine 본문을 실행한다.
    return 0;                                        // task 소멸자가 완료된 frame을 파괴한다.
}                                                    // main을 끝낸다.
