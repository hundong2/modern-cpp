# 7단계: C++11·14·17·20·23 동시성 기능의 진화

> **한 줄 요약:** C++11이 메모리 모델과 기본 동시성 어휘를 세웠고, 14/17은 reader lock과 RAII 조합을 다듬었으며, 20은 구조적 종료·대기 primitive를 크게 확장했고, 23은 그 기반 위에 진단·오류·callable 도구를 보강했다.

- 선행 지식: [메모리 모델](./02-memory-model-atomics.md)~[`extern "C"`](./06-extern-c-abi-ffi.md)
- 초보자 우선: **한눈에 보는 표**, 각 버전의 **실무 기준**
- 전문가 목표: 프로젝트 최소 표준에 따라 같은 설계를 안전하게 backport한다.

## 한눈에 보는 버전 표

| 영역 | C++11 | C++14 | C++17 | C++20 | C++23 |
|---|---|---|---|---|---|
| 메모리 모델 | data race/atomic/order 표준화 | 기반 유지 | 간섭 크기 상수 | `atomic_ref`, atomic wait/notify, `atomic<shared_ptr>` | 핵심 모델 유지·라이브러리 통합 성숙 |
| mutex guard | `mutex`, timed/recursive, `lock_guard`, `unique_lock`, `lock` | `shared_timed_mutex`, `shared_lock` | `scoped_lock`, `shared_mutex`, CTAD | 기존 도구 유지 | 기존 도구 유지 |
| thread/task | `thread`, `async`, future/promise | 기반 유지 | parallel algorithms | `jthread`, stop token | `move_only_function` 등 task 표현 보강 |
| 대기/동기화 | condition variable, `call_once` | 기반 유지 | 기반 유지 | semaphore, latch, barrier | 기반 유지 |
| 성능/진단 | chrono, atomic lock-free 질의 | generic lambda | hardware interference size, execution policies | `source_location` 등 진단 보조 | `stacktrace`, `expected`, `mdspan` 등 실무 보조 |

“기반 유지”는 가치가 없다는 뜻이 아니라 기존 primitive의 표준 계약이 계속 중심이라는 뜻이다. C++23이 새 mutex를 추가하지 않았다고 C++23 동시성 코드가 달라지지 않는 것은 아니다. 오류 모델, callable ownership, 진단 도구가 worker/task 설계를 개선한다.

## C++11: 공통 메모리 언어의 탄생

### 추가된 핵심

- 표준 memory model, data race와 happens-before
- `std::atomic`, memory order, fences
- `std::thread`, thread ID, hardware concurrency hint
- `std::mutex`, `timed_mutex`, recursive variants
- `std::lock_guard`, `std::unique_lock`, `std::lock`
- `std::condition_variable`, `condition_variable_any`
- `std::future`, `promise`, `packaged_task`, `async`
- `std::call_once`, `once_flag`, thread-safe local static initialization
- `thread_local`, lambda, move semantics, variadic template

### 왜 실무가 바뀌었는가

이전에는 pthread/Win32/compiler intrinsic에 직접 의존하던 핵심 개념을 표준 C++ 타입으로 표현할 수 있게 됐다. Compiler도 C++ data-race-free contract 아래 최적화와 atomic lowering을 수행할 공통 의미를 얻었다.

### 실무 기준

- 범위 잠금은 `lock_guard`, 조건 대기는 `unique_lock`.
- 새 `std::thread`의 모든 경로에 join/detach ownership을 정한다.
- lock-free가 꼭 필요하지 않으면 mutex로 시작한다.
- C++11 다중 mutex는 deferred `unique_lock` + `std::lock`으로 묶는다.

## C++14: 공유 읽기와 표현력 보강

### 추가된 핵심

- `std::shared_timed_mutex`
- `std::shared_lock`
- generic lambda와 init-capture가 worker/callback ownership 표현을 개선
- chrono literal이 timeout 표현을 읽기 쉽게 함

### 주의

Reader가 많다는 이유만으로 shared lock을 쓰지 않는다. writer starvation과 관리 비용을 측정한다. 값 init-capture는 C++11의 수동 functor보다 lifetime ownership을 분명히 만들 수 있다.

### C++11 대안

Platform reader-writer lock 또는 일반 mutex를 사용한다. 일반 mutex가 더 단순하고 충분히 빠른 경우가 많다.

## C++17: 여러 lock의 RAII와 배치 힌트

### 추가된 핵심

- `std::scoped_lock`: 여러 mutex의 deadlock-avoiding RAII 획득
- `std::shared_mutex`: timeout 없는 reader/writer mutex
- `std::hardware_destructive_interference_size`, `hardware_constructive_interference_size`
- Class Template Argument Deduction(CTAD): `std::lock_guard lock{mutex};`
- standard parallel algorithms와 execution policy
- `std::invoke`, `is_invocable` 계열이 generic task wrapper를 정교화

### 실무 기준

- 두 객체를 swap/transfer할 때 `scoped_lock`을 기본으로 고려한다.
- false sharing 회피에 interference size를 쓰더라도 실제 layout과 benchmark를 확인한다.
- `par` 정책은 사용 implementation/backend, exception 정책, callable thread safety를 확인한다.

### C++14 대안

두 mutex에는 deferred `unique_lock` 둘과 `std::lock`을 사용한다. CTAD 없이 template argument를 명시한다.

## C++20: 구조적 종료와 효율적 대기

### 추가된 핵심

- `std::jthread`, `stop_source`, `stop_token`, `stop_callback`
- `std::counting_semaphore`, `binary_semaphore`
- `std::latch`, `std::barrier`
- `atomic::wait`, `notify_one`, `notify_all`, `atomic_flag::wait/notify`
- `std::atomic_ref`
- `std::atomic<std::shared_ptr<T>>`, `atomic<weak_ptr<T>>` 특수화
- `condition_variable_any`의 stop-aware wait 조합
- coroutine language feature: 비동기 control flow의 재료지만 scheduler는 직접 제공하지 않음

### 실무 기준

- 직접 만든 joining-thread wrapper를 `jthread`로 단순화한다.
- polling loop를 atomic wait/notify 또는 적합한 blocking primitive로 바꾼다.
- capacity는 semaphore, phase rendezvous는 latch/barrier, 임의 상태는 CV로 표현한다.
- stop 요청이 모든 blocking point를 깨우는지 검토한다.

### C++17 대안

- `jthread`: stop atomic + RAII joining wrapper
- semaphore/latch: mutex + condition variable로 구현하되 destruction/overflow/fairness 계약 추가
- atomic wait: condition variable 또는 platform primitive wrapper

## C++23: 기반 위의 실무 구성 보강

C++23 동시성의 중심 primitive는 여전히 C++11~20에서 온다. 다음 도구가 주변 설계를 개선한다.

- `std::expected`: worker/FFI 내부에서 예상 가능한 실패를 값으로 전달
- `std::move_only_function`: `unique_ptr` 같은 move-only capture를 가진 task를 type erase
- `std::stacktrace`: deadlock watchdog/오류 지점 진단 데이터 수집에 활용 가능
- `std::mdspan`: 연속/다차원 데이터 view를 통해 cache-friendly layout 실험
- ranges/library 개선: 작업 준비와 결과 처리 코드를 명확히 함

### 주의

- `expected`나 `stacktrace`를 C ABI에 직접 노출하지 않는다.
- stacktrace 수집은 느리거나 할당할 수 있으므로 hot path에서 무조건 수행하지 않는다.
- `move_only_function` 지원 compiler/library 버전을 CI에서 확인한다.
- C++23이라고 memory order 규칙이나 mutex correctness가 달라지는 것은 아니다.

## 최소 표준 선택

| 제약 | 권장 기준 |
|---|---|
| 오래된 embedded/toolchain | C++11/14 primitive + 작은 검증 wrapper |
| 넓은 server/desktop 호환 | C++17: `scoped_lock`, `shared_mutex`, CTAD |
| 제어 가능한 최신 toolchain | C++20: `jthread`, semaphore, atomic wait를 활용 |
| 최신 library 기능을 제품에서 검증 가능 | C++23: 오류/task/진단 표현까지 채택 |

표준 버전 숫자보다 compiler와 standard library의 실제 feature support가 중요하다. `__cpp_lib_jthread`, `__cpp_lib_semaphore`, `__cpp_lib_atomic_wait` 같은 feature-test macro와 CI compile test로 확인한다.

## 버전 migration 실습

1. C++11의 두 `unique_lock + std::lock` 코드를 C++17 `scoped_lock`로 바꾼다.
2. atomic flag polling + sleep 코드를 C++20 `atomic::wait`로 바꾼다.
3. 수동 stop atomic + thread join guard를 `jthread`로 바꾼다.
4. 직접 semaphore wrapper의 shutdown/overflow test를 작성한 뒤 C++20 표준 타입과 비교한다.
5. C++23 `move_only_function<void()>` task queue를 만들고 C++17 `std::function` 제약과 비교한다.

## 완료 기준

- [ ] 각 표준의 새 기능을 목록이 아니라 해결한 문제로 설명한다.
- [ ] C++20 기능의 C++17 대안을 선택할 수 있다.
- [ ] C++23의 보조 기능과 core synchronization primitive를 구별한다.
- [ ] 다음 문서인 [성능 실습](./08-performance-labs.md)으로 이동한다.
