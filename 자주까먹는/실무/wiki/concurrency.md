# 동시성·mutex·atomic

두 스레드가 같은 메모리 위치에 동시에 접근하고, 그중 하나 이상이 쓰기이며 동기화가 없으면 data race이고 C++에서는 undefined behavior다.

- `std::mutex`: 임계 구역을 한 번에 한 스레드만 실행하게 한다.
- `std::lock_guard`: mutex 잠금/해제를 [RAII](raii.md)로 관리한다.
- `std::condition_variable`: 상태가 바뀔 때까지 잠들어 busy waiting을 피한다. 조건은 spurious wakeup 때문에 predicate로 다시 검사한다.
- `std::atomic<T>`: 특정 객체의 원자 연산과 메모리 순서를 제공한다. 여러 필드 불변식을 자동 보호하지 않는다.

동시 실행 자체와 비동기 API는 다르다. 비동기는 호출자에게 완료를 나중에 통지하는 구조이고, 반드시 여러 스레드를 뜻하지 않는다.
