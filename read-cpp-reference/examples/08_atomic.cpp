#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    std::atomic<int> counter{0};

    constexpr int thread_count = 4;
    constexpr int increments_per_thread = 10'000;
    std::vector<std::thread> workers;

    for (int i = 0; i < thread_count; ++i) {
        workers.emplace_back([&counter] {
            for (int n = 0; n < increments_per_thread; ++n) {
                // 결과 숫자만 필요하고 다른 메모리와의 순서가 필요 없으므로 relaxed.
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& worker : workers) {
        worker.join(); // main이 최종 값을 읽기 전에 모든 worker 종료
    }

    const int expected = thread_count * increments_per_thread;
    std::cout << "counter = " << counter.load() << '\n';
    std::cout << "expected = " << expected << '\n';

    // 런타임/객체 기준 질의. 플랫폼에 따라 true 또는 false일 수 있다.
    std::cout << std::boolalpha
              << "atomic<int>.is_lock_free() = "
              << counter.is_lock_free() << '\n';

    // C++17 compile-time 상수. 이것도 구현에 따라 값이 다를 수 있다.
    std::cout << "atomic<int>::is_always_lock_free = "
              << std::atomic<int>::is_always_lock_free << '\n';

    // volatile overload가 실제로 존재함을 보여 주는 교육용 예.
    // volatile이 atomicity를 만드는 것이 아니다. 타입이 atomic이라 atomic이다.
    volatile std::atomic<int> device_like_atomic{0};
    std::cout << "volatile overload = "
              << device_like_atomic.is_lock_free() << '\n';

    // 아래처럼 일반 int에 volatile만 붙여 여러 thread에서 ++하면 안전하지 않다.
    // volatile int unsafe_counter = 0; // thread synchronization 용도가 아님

    return counter.load() == expected ? 0 : 1;
}
