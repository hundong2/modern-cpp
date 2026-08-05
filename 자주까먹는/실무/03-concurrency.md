# 3장: 동시성과 비동기 실행

## Day 3: 스레드와 뮤텍스

[data race](wiki/concurrency.md)는 단순히 결과가 가끔 틀리는 문제가 아니라 undefined behavior다. 공유 상태를 최소화하고, 잠금이 보호하는 불변식을 주석으로 적는다. `std::lock_guard`는 중괄호를 나갈 때 unlock하므로 예외 안전하다. 여러 mutex가 필요하면 일관된 순서 또는 `std::scoped_lock`을 사용해 deadlock을 줄인다.

실습: [day03_threaded_counter.cpp](examples/day03_threaded_counter.cpp)

## Day 8: 스레드 풀

스레드 풀은 제한된 worker들이 queue에서 task를 꺼내 실행한다. `condition_variable.wait(lock, predicate)`는 queue가 비었을 때 잠들고 spurious wakeup 후 predicate를 재검사한다. 종료 때는 다음 순서를 지킨다.

1. mutex 아래에서 `stopping = true` 설정.
2. 모든 worker를 깨움.
3. 더 이상 enqueue하지 못하게 함.
4. queue drain 또는 cancel 정책에 따라 worker 종료.
5. join.

무제한 queue는 부하를 해결하지 않고 메모리로 미룬다. 실무 풀에는 bounded queue, 거부/timeout 정책, task 예외 관찰, metric이 필요하다.

실습: [day08_thread_pool.cpp](examples/day08_thread_pool.cpp)

## Day 9: `future`

`std::packaged_task<R()>`는 함수 실행 결과/예외를 shared state에 저장하고 `future<R>`가 이를 한 번 가져간다. `future.get()`은 준비될 때까지 block하고 예외를 재던진다. 이벤트 루프 스레드에서 무심코 `get()`하면 비동기 구조를 다시 block시킨다.

실습: [day09_future.cpp](examples/day09_future.cpp)

## Day 10~11: Boost.Asio와 strand

`io_context`는 handler 실행 엔진이다. async operation은 즉시 결과를 돌려주는 것이 아니라 완료 handler를 나중에 실행한다. 세션은 handler가 끝날 때까지 살아 있어야 하므로 `enable_shared_from_this`와 캡처한 `shared_ptr`를 사용한다. 생성자 안에서 `shared_from_this()`를 호출하면 아직 소유 `shared_ptr`가 없어 잘못이다.

여러 스레드가 같은 `io_context.run()`을 호출하면 handler들이 병렬 실행될 수 있다. `strand`는 그 executor를 통해 제출된 handler들이 서로 겹쳐 실행되지 않게 직렬화한다. “lock-free”와 동의어가 아니며, 세션 외부에서 같은 데이터를 직접 건드리면 보호되지 않는다.

실습 모형: [day10_asio_lifetime.cpp](examples/day10_asio_lifetime.cpp), [day11_strand_model.cpp](examples/day11_strand_model.cpp)

## Day 12: 코루틴

C++20 coroutine은 중단 가능한 함수의 상태 기계를 컴파일러가 만든다. `co_await`는 OS 스레드를 붙잡고 기다리라는 뜻이 아니라 awaiter 계약에 따라 코루틴을 suspend할 수 있다. 지역 변수는 coroutine frame에 보존될 수 있으며, frame 소유권과 cancellation/exception 경로를 반드시 설계한다.

Boost.Asio의 `awaitable`, `co_spawn`, `use_awaitable`는 callback 연쇄를 순차 코드처럼 표현하지만 실제 I/O의 비동기 성질과 부분 I/O 규칙은 사라지지 않는다.

실습 모형(C++20): [day12_coroutine_model.cpp](examples/day12_coroutine_model.cpp)

## 디버깅 질문

- 이 task를 누가 소유하며 언제 폐기하는가?
- callback/coroutine이 캡처한 객체는 완료 시점까지 유효한가?
- queue가 가득 차면 생산자에게 어떤 신호가 돌아가는가?
- 종료 중 새 작업이 들어오면 거절되는가?
- 예외가 버려지지 않고 관찰되는가?
