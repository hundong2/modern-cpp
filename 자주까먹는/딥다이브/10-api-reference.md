# 10단계: 동시성 API·문법·비용 빠른 사전

> **한 줄 요약:** 자주 쓰는 표준 함수의 매개변수, 반환값, 상태 변화, 예외와 조합을 한곳에서 다시 찾는다.

- 선행 지식: [mutex](./03-locks-and-raii.md), [대기](./04-waiting-and-semaphores.md), [thread](./05-threads-and-lifecycle.md)
- 초보자 우선: 표에서 함수를 찾고 반드시 “호출 뒤 ownership” 열을 확인한다.
- 주의: 정확한 `noexcept`, 오류 조건, clock 요구는 표준 버전과 타입 요구 사항을 함께 확인한다. 아래 signature는 학습용 핵심 형태다.

## Mutex와 guard

| 표현 | 매개변수 | 반환 | 호출 뒤 상태 | 대표 오류/비용 |
|---|---|---|---|---|
| `m.lock()` | 없음 | `void` | 현재 thread가 `m` 소유 | block 가능; 재귀 lock 금지 |
| `m.try_lock()` | 없음 | `bool` | true면 소유 | false는 정상 제어 흐름 |
| `m.unlock()` | 없음 | `void` | 소유 해제 | 비소유 unlock은 잘못 |
| `lock_guard<M>{m}` | `M&` | 객체 생성 | 즉시 lock, 소멸 때 unlock | copy/move 불가 |
| `lock_guard<M>{m, adopt_lock}` | 이미 잠근 `M&`, tag | 객체 생성 | 기존 ownership을 guard가 관리 | 전제 위반 위험 |
| `unique_lock<M>{m}` | `M&` | 객체 생성 | 즉시 lock | movable, 상태 저장 |
| `unique_lock<M>{m, defer_lock}` | `M&`, tag | 객체 생성 | mutex 연결, 미소유 | 나중에 lock 필요 |
| `unique_lock<M>{m, try_to_lock}` | `M&`, tag | 객체 생성 | 성공 여부를 `owns_lock`으로 확인 | 실패 정상 |
| `scoped_lock{m1, m2}` | mutex 참조들 | 객체 생성 | 모두 획득, 소멸 때 모두 해제 | C++17, 순서 직접 가정 금지 |

### `std::unique_lock` 자주 쓰는 멤버

| 함수 | 반환 | 의미 |
|---|---|---|
| `lock()` | `void` | 연결 mutex를 획득하고 ownership=true |
| `try_lock()` | `bool` | 즉시 시도; 결과에 따라 ownership 변경 |
| `unlock()` | `void` | 소유 mutex 해제, 연결은 유지 |
| `owns_lock()` | `bool` | 현재 ownership 여부 |
| `mutex()` | `mutex_type*` | 연결 mutex 주소 또는 null |
| `release()` | `mutex_type*` | 연결을 끊고 pointer 반환; mutex를 unlock하지 않음 |
| move construction | 새 lock object | mutex 연결과 ownership 이전; 원본 비소유 |

`release()`는 ownership 관리만 포기하므로 거의 항상 특별한 low-level integration에서만 쓴다. `unlock()`과 혼동하면 lock leak이 생긴다.

### 자유 함수 `std::lock`과 `std::try_lock`

```cpp
template<class L1, class L2, class... Ls>
void lock(L1& l1, L2& l2, Ls&... ls);

template<class L1, class L2, class... Ls>
int try_lock(L1& l1, L2& l2, Ls&... ls);
```

- `std::lock`: 여러 Lockable을 deadlock avoidance 알고리즘으로 모두 획득한다. 반환값 없음.
- `std::try_lock`: 순서대로 try하며 모두 성공하면 `-1`, 실패하면 실패한 인자의 0 기반 index를 반환하고 이미 획득한 것들을 unlock한다.
- 알고리즘의 시도 횟수와 fairness를 가정하지 않는다.

## Condition variable

```cpp
cv.wait(lock, predicate);
bool ok = cv.wait_for(lock, duration, predicate);
bool ok = cv.wait_until(lock, deadline, predicate);
cv.notify_one();
cv.notify_all();
```

| 함수 | 매개변수 | 반환 | 핵심 |
|---|---|---|---|
| `wait(lock)` | 소유 중인 `unique_lock<mutex>&` | `void` | unlock+대기+relock; spurious wake 가능 |
| `wait(lock, pred)` | lock, bool처럼 평가되는 callable | `void` | pred가 true일 때 반환 |
| `wait_for(..., pred)` | lock, duration, predicate | `bool` | 제한 내 pred true면 true |
| `wait_until(..., pred)` | lock, time_point, predicate | `bool` | deadline까지 pred true면 true |
| `notify_one()` | 없음 | `void` | waiter 하나를 깨울 수 있음 |
| `notify_all()` | 없음 | `void` | 모든 waiter를 깨움 |

Predicate는 lock을 보유한 상태로 호출되므로 빠르고 예외 정책이 명확해야 한다.

## Semaphore(C++20)

```cpp
std::counting_semaphore<8> slots{8};
slots.acquire();
bool acquired = slots.try_acquire();
slots.release();
```

| 함수 | 매개변수 | 반환 | 핵심 |
|---|---|---|---|
| constructor | desired count | 객체 | `0 <= desired <= max()` 전제 |
| `acquire()` | 없음 | `void` | permit 하나까지 block |
| `try_acquire()` | 없음 | `bool` | 즉시 성공 여부 |
| `try_acquire_for(d)` | duration | `bool` | 상대 시간 제한 |
| `try_acquire_until(t)` | time point | `bool` | 절대 시각 제한 |
| `release(update=1)` | 추가 permit 수 | `void` | counter 증가와 waiter unblock 가능 |
| `max()` | 없음 | `ptrdiff_t` | 구현이 지원하는 최대 counter |

`LeastMaxValue`는 template parameter인 최소 지원 최대값이지 초기 permit 수가 아니다. 실제 maximum은 `max()`로 질의한다.

## Atomic

| 함수 | 매개변수 | 반환 | 자주 쓰는 순서 |
|---|---|---|---|
| `load(order)` | load에 허용되는 order | 현재 값 | acquire/relaxed/seq_cst |
| `store(value, order)` | 값, store order | `void` | release/relaxed/seq_cst |
| `exchange(value, order)` | 새 값, order | 이전 값 | acq_rel/seq_cst |
| `fetch_add(arg, order)` | 증가량, order | 증가 전 값 | 카운터 relaxed, protocol은 증명 |
| `compare_exchange_weak` | expected ref, desired, orders | 성공 `bool` | loop |
| `compare_exchange_strong` | expected ref, desired, orders | 성공 `bool` | 단발 분기 |
| `wait(old, order)` | 비교할 이전 값 | `void` | 값이 달라질 때까지 대기(C++20) |
| `notify_one/all()` | 없음 | `void` | waiter 깨우기(C++20) |

`fetch_add(1)`의 반환은 **증가 전 값**이다. 증가 후 값이 필요하면 반환값에 1을 더하되 overflow semantics와 타입을 확인한다.

## Thread(C++11)

```cpp
std::thread t{callable, args...};
bool needs_join = t.joinable();
t.join();
```

| 함수/표현 | 반환 | 핵심 |
|---|---|---|
| constructor | 새 `thread` 객체 | callable/args 저장 후 실행 시작; 생성 실패 예외 가능 |
| `get_id()` | `std::thread::id` | 대표 thread ID; 기본 객체는 특별한 ID |
| `joinable()` | `bool` | join/detach 책임이 남았는지 |
| `join()` | `void` | 완료 대기 후 non-joinable; system_error 가능 |
| `detach()` | `void` | 대표권 분리 후 non-joinable |
| `native_handle()` | 구현 정의 타입 | OS API 연동; 이식성 낮음 |
| `hardware_concurrency()` | `unsigned` | hardware thread 수 힌트; 0 가능 |
| `this_thread::yield()` | `void` | scheduler에 실행 양보 힌트 |
| `sleep_for/until` | `void` | 적어도 지정 조건까지 sleep; 정확한 wake 시각 보장 아님 |

## `std::jthread`와 stop(C++20)

| 함수 | 반환 | 핵심 |
|---|---|---|
| destructor | 없음 | joinable이면 stop request 후 join |
| `request_stop()` | `bool` | 이번 호출이 stop 상태를 새로 바꿨는지 |
| `get_stop_token()` | `stop_token` | 관찰 token |
| `get_stop_source()` | `stop_source` | 요청 source |
| `stop_token::stop_requested()` | `bool` | 중지 요청 상태 |
| `stop_token::stop_possible()` | `bool` | 연결 stop state가 요청 가능 상태인지 |

Callable의 첫 인자가 `stop_token`을 받을 수 있으면 `jthread`가 token을 전달하는 overload를 선택한다. 오버로드/forwarding이 복잡하면 명시 lambda로 경계를 단순화한다.

## Latch와 barrier(C++20)

| 타입/함수 | 반환 | 의미 |
|---|---|---|
| `latch(count)` | 객체 | 일회성 counter |
| `count_down(n=1)` | `void` | counter 감소 |
| `wait()` | `void` | 0까지 대기 |
| `arrive_and_wait(n=1)` | `void` | 감소 후 대기 |
| `barrier(expected, completion)` | 객체 | 반복 phase 참가자 수 |
| `arrive()` | arrival token | 현재 phase 도착 등록 |
| `wait(token)` | `void` | token의 phase 완료 대기 |
| `arrive_and_wait()` | `void` | 도착+대기 |
| `arrive_and_drop()` | `void` | 도착 후 다음 phase 참가자에서 영구 이탈 |

Latch/barrier는 살아 있는 동안 참가 thread가 객체 수명을 넘지 않고, destruction 전에 더 이상 method를 호출하지 않게 외부 lifetime을 관리한다.

## `extern "C"` 문법 사전

```cpp
extern "C" int f(int);       // 함수 이름과 함수 타입에 C language linkage를 지정하는 선언이다.

extern "C" {                 // 여러 선언에 C language linkage를 적용하는 linkage specification이다.
    int g(double);
    void h(void* context);
}
```

- 매개변수: runtime 매개변수가 아니라 declaration에 적용되는 language-linkage 문자열이다.
- 반환값: 문법 자체에는 반환값이 없고 각 함수의 return type이 별도로 있다.
- 함수 정의에도 직접 붙일 수 있다.
- namespace scope에서 사용한다.
- C++ compiler 기능이므로 C header에서는 `#ifdef __cplusplus`로 가린다.
- overload, template, member function, C++ object ABI를 C API처럼 노출하는 해결책이 아니다.

## 자주 함께 쓰는 헤더

| 헤더 | 주요 타입/함수 |
|---|---|
| `<thread>` | `thread`, `jthread`, `this_thread` |
| `<mutex>` | mutex 계열, guard, `lock`, `call_once` |
| `<shared_mutex>` | `shared_mutex`, `shared_timed_mutex`, `shared_lock` |
| `<condition_variable>` | condition variable 계열 |
| `<atomic>` | atomic, memory order, fences, wait/notify |
| `<semaphore>` | counting/binary semaphore(C++20) |
| `<latch>` | latch(C++20) |
| `<barrier>` | barrier(C++20) |
| `<future>` | future, promise, packaged_task, async |
| `<stop_token>` | stop source/token/callback(C++20) |
| `<functional>` | ref/cref, invoke, move_only_function(C++23) |

## 빠른 결정 규칙

1. 복합 공유 상태이면 mutex부터 시작한다.
2. 가장 단순한 범위는 lock_guard, 여러 mutex는 scoped_lock, 대기/수동 해제는 unique_lock.
3. 조건은 CV predicate, 용량은 semaphore, phase는 latch/barrier.
4. 작은 독립 상태만 atomic; memory order를 약화하면 proof와 benchmark를 남긴다.
5. thread 수명은 join scope 안에 묶고 C++20이면 jthread를 우선 검토한다.
6. 언어 경계는 versioned C ABI + 언어별 safe wrapper로 격리한다.
