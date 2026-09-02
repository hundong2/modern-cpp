#include <algorithm>  // std::max를 선언한다; peak 비교의 일반 함수 예시에 사용한다.
#include <atomic>     // std::atomic과 C++20 wait/notify 멤버를 선언한다.
#include <chrono>     // std::chrono::milliseconds와 시간 literal 기반 대기를 선언한다.
#include <cstddef>    // std::ptrdiff_t 같은 크기 관련 정수 타입을 선언한다.
#include <iostream>   // 검증 결과를 main 스레드에서 출력하는 std::cout을 선언한다.
#include <semaphore>  // C++20 std::counting_semaphore와 binary_semaphore를 선언한다.
#include <stop_token> // C++20 std::stop_token 타입을 선언한다.
#include <thread>     // C++20 std::jthread와 std::this_thread::sleep_for를 선언한다.
#include <vector>     // 여러 jthread 관리 객체를 RAII로 소유할 std::vector를 선언한다.

template <std::ptrdiff_t LeastMaxValue>                    // semaphore의 compile-time 최소 최대값을 template 매개변수로 받는다.
class PermitGuard final {                                  // acquire된 permit 하나를 scope exit에서 반환하는 RAII 클래스다.
public:                                                     // caller가 사용할 생성자와 소멸자 구역을 시작한다.
    explicit PermitGuard(std::counting_semaphore<LeastMaxValue>& semaphore) noexcept // 이미 획득한 semaphore 참조를 받는다.
        : semaphore_{semaphore} {}                          // 비소유 참조 멤버를 초기화하며 여기서는 acquire하지 않는다.

    PermitGuard(const PermitGuard&) = delete;               // 하나의 permit을 두 guard가 release하지 않도록 복사를 금지한다.
    PermitGuard& operator=(const PermitGuard&) = delete;    // 복사 대입도 같은 double-release 위험 때문에 금지한다.

    ~PermitGuard() {                                        // scope 종료·예외에서 자동 호출되며 묵시적으로 noexcept다.
        semaphore_.release();                               // acquire 때 소비한 permit 하나를 counter에 돌려준다.
    }                                                       // 소멸자 본문을 끝낸다.

private:                                                    // 구현 세부 상태를 외부에서 바꾸지 못하게 한다.
    std::counting_semaphore<LeastMaxValue>& semaphore_;     // semaphore보다 짧게 사는 비소유 lvalue reference다.
};                                                          // PermitGuard class 정의를 끝낸다.

void update_peak(std::atomic<int>& peak, int candidate) {  // peak atomic과 새 후보 값을 받아 최대 동시 수를 갱신한다.
    int observed = peak.load(std::memory_order_relaxed);    // 현재 peak를 원자적으로 읽되 다른 payload를 publish하지 않아 relaxed를 쓴다.
    while (observed < candidate &&                         // 후보가 기존 값보다 클 때만 CAS 갱신을 시도한다.
           !peak.compare_exchange_weak(                    // weak CAS는 허위 실패 가능하므로 while 반복에 적합하다.
               observed,                                  // 실패 시 실제 peak 값으로 덮어쓰이는 in/out expected 매개변수다.
               candidate,                                 // 성공할 때 저장할 desired 값이다.
               std::memory_order_relaxed,                  // 성공도 독립 통계 값만 다루므로 relaxed면 충분하다.
               std::memory_order_relaxed)) {               // 실패 load에도 주변 publish 관계가 없어 relaxed를 쓴다.
    }                                                       // CAS 성공, 더 큰 값 관찰, 또는 조건 false가 되면 빈 loop를 끝낸다.
}                                                           // peak update 함수는 반환값 없이 끝난다.

int main() {                                                // C++20 예제 진입점이며 검증 결과를 exit code로 반환한다.
    constexpr int permit_limit = 3;                         // 동시에 임계 자원을 사용할 최대 작업 수를 compile-time 상수로 정한다.
    std::counting_semaphore<permit_limit> permits{permit_limit}; // 초기 permit 세 개를 가진 semaphore를 만든다.
    std::atomic<int> active{0};                              // 현재 permit 구역에 들어온 worker 수를 원자적으로 센다.
    std::atomic<int> peak{0};                                // 실행 중 관찰한 active 최대값을 원자적으로 저장한다.
    std::vector<std::jthread> workers;                       // 소멸 시 stop 요청과 join을 수행할 jthread 객체들을 소유한다.
    workers.reserve(9);                                     // 아홉 worker를 만들 때 vector 재할당을 피한다.

    for (int task_id = 0; task_id < 9; ++task_id) {         // semaphore limit보다 많은 아홉 작업을 생성한다.
        workers.emplace_back([&, task_id](std::stop_token token) { // 공유 동기화 객체는 참조, ID는 값으로 capture한다.
            if (token.stop_requested()) {                   // 시작 전에 owner가 stop을 요청했는지 협력적으로 확인한다.
                return;                                     // 아무 permit도 얻지 않았으므로 바로 종료해도 반환 누락이 없다.
            }                                               // 시작 전 stop 분기를 닫는다.
            permits.acquire();                              // permit을 얻을 때까지 block할 수 있고 성공하면 하나를 소비한다.
            PermitGuard<permit_limit> permit_guard{permits}; // 이후 모든 return/예외 경로에서 permit 하나를 자동 반환한다.
            const int now = active.fetch_add(1, std::memory_order_relaxed) + 1; // 증가 전 반환값에 1을 더해 현재 수를 얻는다.
            update_peak(peak, now);                         // CAS loop로 최대 동시 실행 수를 보존한다.
            std::this_thread::sleep_for(std::chrono::milliseconds{5 + task_id}); // 제한 자원을 쓰는 시간을 task별로 조금 달리한다.
            active.fetch_sub(1, std::memory_order_relaxed); // permit 반환 전 active 통계 수를 원자적으로 감소시킨다.
        });                                                 // lambda를 실행하는 jthread를 vector 끝에 생성한다.
    }                                                       // 아홉 worker 생성 loop를 끝낸다.

    for (std::jthread& worker : workers) {                  // peak를 읽기 전에 모든 worker를 명시적으로 join한다.
        worker.join();                                      // 해당 worker 완료와 main의 후속 읽기 사이에 동기화를 만든다.
    }                                                       // 모든 semaphore 작업이 끝났다.

    int payload = 0;                                        // atomic state가 publish할 비원자 payload이며 관계 증명 아래 안전하다.
    int observed_payload = 0;                               // consumer만 쓰고 join 뒤 main만 읽는 결과 저장소다.
    std::atomic<int> state{0};                               // 0=준비 전, 1=publish 완료 상태를 원자적으로 표현한다.

    std::jthread consumer{[&] {                             // payload/state/result 수명이 join까지 유지되므로 참조 capture한다.
        state.wait(0, std::memory_order_acquire);           // state가 0인 동안 효율적으로 기다리고 변경 값을 acquire로 관찰한다.
        observed_payload = payload;                         // release store 전 payload 쓰기가 happens-before라 비원자 읽기가 안전하다.
    }};                                                     // consumer jthread 생성을 끝내며 즉시 실행될 수 있다.

    payload = 42;                                           // producer인 main이 비원자 payload를 먼저 완성한다.
    state.store(1, std::memory_order_release);               // 앞의 payload 쓰기를 state=1과 함께 publish한다.
    state.notify_one();                                     // state.wait 중인 consumer 하나를 깨우도록 알린다.
    consumer.join();                                        // observed_payload 읽기 전에 consumer 쓰기의 완료를 기다린다.

    std::atomic<int> stop_iterations{0};                     // stop 예제의 독립 통계이므로 relaxed atomic으로 충분하다.
    std::jthread stoppable{[&](std::stop_token token) {     // jthread가 첫 매개변수로 자신의 stop token을 전달한다.
        while (!token.stop_requested()) {                   // 강제 kill이 아닌 협력적 중지 지점을 loop마다 확인한다.
            stop_iterations.fetch_add(1, std::memory_order_relaxed); // 반복 수만 원자적으로 더하며 다른 데이터를 publish하지 않는다.
            std::this_thread::yield();                      // scheduler에 다른 runnable thread 실행 기회를 주는 힌트다.
        }                                                   // stop 요청을 관찰하면 일관된 loop 경계에서 빠져나온다.
    }};                                                     // stoppable worker 생성을 끝낸다.
    stoppable.request_stop();                               // stop state를 설정하고 worker가 관찰하도록 요청한다.
    stoppable.join();                                       // worker가 요청을 보고 종료할 때까지 기다린다.

    const int peak_value = peak.load(std::memory_order_relaxed); // join 뒤 통계 peak를 원자적으로 읽는다.
    std::cout << "permit_limit=" << permit_limit            // 설정한 최대 permit 수를 출력한다.
              << ", observed_peak=" << peak_value          // 실제 최대 동시 실행 수를 출력한다.
              << ", payload=" << observed_payload          // acquire/release로 전달된 payload를 출력한다.
              << ", stop_iterations="                     // stop 요청 전 worker가 수행한 횟수 label을 출력한다.
              << stop_iterations.load(std::memory_order_relaxed) // 독립 원자 카운터의 마지막 값을 읽는다.
              << '\n';                                     // 결과 한 줄을 끝낸다.

    const bool permits_ok = peak_value >= 1 && peak_value <= permit_limit; // semaphore 상한과 실제 실행 여부를 함께 검증한다.
    const bool publish_ok = observed_payload == 42;          // acquire/release publish 결과가 예상 payload인지 검증한다.
    return permits_ok && publish_ok ? 0 : 1;                 // 모든 불변식이 맞으면 0, 아니면 테스트 실패 1을 반환한다.
}                                                           // main 자동 객체를 파괴하며 프로그램 실행을 끝낸다.
