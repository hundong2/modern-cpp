#include <atomic>   // data race 없는 독립 카운터를 제공한다.
#include <cstddef>  // std::size_t와 간섭 크기 상수를 제공한다.
#include <cstdint>  // std::uint64_t 고정 폭 정수를 제공한다.
#include <iostream> // 결과를 출력한다.
#include <thread>   // 서로 다른 코어에서 실행될 수 있는 worker를 만든다.

#if defined(__cpp_lib_hardware_interference_size)   // 표준 라이브러리가 cache 간섭 힌트를 제공하는지 본다.
constexpr std::size_t cache_separation = std::hardware_destructive_interference_size; // 구현 힌트 값을 사용한다.
#else                                               // 오래된 표준 라이브러리 fallback 경로다.
constexpr std::size_t cache_separation = 64;        // 흔한 값일 뿐 모든 CPU의 보장은 아니다.
#endif                                              // alignment 상수 선택을 끝낸다.

struct alignas(cache_separation) PaddedCounter {    // 인접 counter와 cache line 공유 가능성을 낮춘다.
    std::atomic<std::uint64_t> value{0};            // 독립적인 원자 카운터를 0으로 초기화한다.
};                                                  // struct 크기에 padding이 생길 수 있다.

int main() {                                        // 두 카운터의 병렬 쓰기를 실행한다.
    PaddedCounter left;                             // 첫 counter를 별도 정렬 경계에 둔다.
    PaddedCounter right;                            // 두 번째 counter도 별도 정렬 경계에 둔다.
    auto increment = [](PaddedCounter& counter) {   // 특정 counter만 갱신하는 worker 함수다.
        for (int i = 0; i < 10000; ++i) {          // 측정이 아닌 빠른 정확성 확인 반복이다.
            counter.value.fetch_add(1, std::memory_order_relaxed); // 다른 데이터와 순서가 필요 없는 원자 증가다.
        }                                           // worker 반복을 끝낸다.
    };                                              // lambda 정의를 끝낸다.
    std::thread a(increment, std::ref(left));       // left 참조를 첫 thread에 안전하게 포장해 전달한다.
    std::thread b(increment, std::ref(right));      // right 참조를 둘째 thread에 전달한다.
    a.join();                                       // left 갱신 완료를 기다린다.
    b.join();                                       // right 갱신 완료를 기다린다.
    const auto sum = left.value.load() + right.value.load(); // join 후 두 원자 값을 읽는다.
    std::cout << sum << '\n';                      // 20000을 출력한다.
    return sum == 20000 ? 0 : 1;                   // 정확성만 검사하며 성능 주장은 하지 않는다.
}                                                   // counter와 thread 객체를 파괴한다.
