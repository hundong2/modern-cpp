# C++·C#·Rust·Python 동시성 발전과 실무 비교

> **한 줄 요약:** 네 언어 모두 결국 운영체제와 CPU 위에서 실행되지만, C++은 명시적 수명과 표준 메모리 모델, C#은 managed runtime과 task/async, Rust는 소유권·`Send`/`Sync` 정적 검사, Python은 runtime/프로세스·async 선택이 설계를 크게 좌우한다.

- 선행 지식: [메모리 모델](./02-memory-model-atomics.md), [잠금](./03-locks-and-raii.md), [`extern "C"`](./06-extern-c-abi-ffi.md)
- 초보자 우선: **개념 대응표**, **언어별 발전**, **FFI 수명 번역**
- 범위: C++가 중심이며, C#과 Rust의 실무 대안 및 Python에서 가장 가까운 관용구까지 비교한다.

## 개념 대응표

| 목적 | C++ | C#/.NET | Rust 표준 | Python(주로 CPython) |
|---|---|---|---|---|
| OS thread | `std::thread`, C++20 `jthread` | `Thread`; 보통 ThreadPool/Task 선호 | `std::thread::spawn`, `scope` | `threading.Thread` |
| 범위 상호 배제 | `mutex` + guard | `lock` 문, `Monitor`; 최신 runtime의 `System.Threading.Lock` | `Mutex<T>` → `MutexGuard<T>` | `threading.Lock` + `with` |
| reader/writer | `shared_mutex`/`shared_lock` | `ReaderWriterLockSlim` | `RwLock<T>` | 표준에 직접 대응 없음 |
| permit N개 | C++20 semaphore | `SemaphoreSlim`, async wait 가능 | 표준 `Semaphore` 없음; channel/생태계 async semaphore | `threading.Semaphore`, `asyncio.Semaphore` |
| 조건 대기 | `condition_variable` | `Monitor.Wait/Pulse`, event 계열 | `Condvar` | `threading.Condition` |
| atomic | `std::atomic<T>` | `Interlocked`, `Volatile` | `Atomic*`, `Ordering` | 언어 수준 portable atomic 부재 |
| 소유권 공유 | 명시적 객체 수명, 필요 시 `shared_ptr` | GC reference | `Arc<T>` | GC/reference count 구현 |
| 취소 | C++20 stop token | `CancellationToken` | 표준 thread 강제 취소 없음; flag/channel/token 생태계 | event/task cancellation |
| FFI | C ABI export | P/Invoke/`LibraryImport`, `SafeHandle` | `unsafe extern "C"`, `#[repr(C)]`, safe wrapper | `ctypes`, C extension, cffi |

## C++의 발전 축 요약

1. C++11: OS별 API 위에 표준 thread/mutex/atomic/memory model을 세웠다.
2. C++14: reader/writer lock과 lambda capture 표현을 보강했다.
3. C++17: `scoped_lock`, `shared_mutex`, hardware interference hint로 조합과 배치를 개선했다.
4. C++20: `jthread`, stop token, semaphore/latch/barrier, atomic wait/notify로 종료와 효율적 대기를 구조화했다.
5. C++23: 기존 synchronization 기반 위에 `expected`, `move_only_function`, `stacktrace` 같은 task/오류/진단 도구를 보강했다.

상세 표는 [C++ 버전별 진화](./07-version-evolution.md)에 있다.

## C#/.NET은 어떻게 발전했는가

### 1. Thread와 Monitor 중심

초기 .NET 실무는 `System.Threading.Thread`, `Monitor`, OS handle 기반 `Mutex/Event/Semaphore`를 직접 조합했다. C# `lock(obj)` 문은 예외가 발생해도 해제되도록 compiler가 동기화 진입/`try/finally` 종료 패턴으로 내린다.

```csharp
private readonly object _gate = new(); // 다른 용도로 공개하지 않는 전용 monitor 대상이다.

lock (_gate)                           // compiler가 예외 안전한 진입/해제 범위를 만든다.
{
    _balance += amount;                // lock 범위에서 복합 상태를 갱신한다.
}
```

C++ `lock_guard`와 목적은 비슷하지만 차이가 있다.

- C++ guard는 library object의 destructor가 unlock한다.
- C# `lock`은 language statement이고 managed exception 경로에 맞춰 생성된 코드가 해제한다.
- 일반 object monitor는 reentrant다. C++ `std::mutex`는 non-recursive다.
- C#에서도 `_gate`가 무엇을 보호하는지 별도 규약이 필요하다. GC가 data race를 막지 않는다.

### 2. ThreadPool과 Task Parallel Library

요청마다 raw thread를 만드는 대신 runtime이 관리하는 ThreadPool, `Task`, continuation, parallel loop가 작업 단위 모델을 넓혔다. `Task`는 OS thread 그 자체가 아니며 완료/결과/예외를 나타낸다. C++ `std::future`와 일부 역할이 비슷하지만 .NET scheduler/runtime 통합이 더 중심적이다.

### 3. `async`/`await`와 cooperative cancellation

I/O 대기 중 thread를 붙잡지 않는 task-based async가 일반화됐다. `CancellationToken`이 API 전반의 협력적 취소 계약이 됐다. C++20 `stop_token`과 철학은 비슷하지만, C++ 표준 coroutine은 scheduler/I/O runtime을 직접 제공하지 않는다.

`lock` 본문에서는 `await`할 수 없다. async 경로의 동시 통과 수 제한에는 `SemaphoreSlim.WaitAsync(token)` + `try/finally Release()` 같은 패턴을 사용한다.

```csharp
await _slots.WaitAsync(token); // permit까지 비동기 대기하며 cancellation도 관찰한다.
try
{
    await UseResourceAsync(token); // permit 소유 중 resource를 사용한다.
}
finally
{
    _slots.Release();              // 예외·취소에서도 permit을 반환한다.
}
```

`SemaphoreSlim`은 thread/task identity에 소유권을 묶지 않으므로 C++ semaphore처럼 Wait/Release pairing을 애플리케이션이 지켜야 한다.

### 4. 현대적 전용 lock과 native interop

.NET 9/C# 13부터 일반 상호 배제에는 전용 `System.Threading.Lock` 객체를 C# `lock` 문과 함께 쓰는 지침이 추가됐다. compiler는 정확히 그 타입일 때 scope 기반 경로를 사용한다. 이전 target에서는 외부에 노출하지 않는 전용 object를 잠근다. [`System.Threading.Lock` 공식 설계](https://learn.microsoft.com/en-us/dotnet/csharp/language-reference/proposals/csharp-13.0/lock-object)

Native interop은 전통적인 `DllImport`에 더해 .NET 7부터 `LibraryImport` source-generated P/Invoke가 도입되어 marshalling 코드를 compile time에 생성할 수 있다. [`LibraryImport` 공식 안내](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/pinvoke-source-generation)

### C# 메모리와 CPU 관계

- C# source는 IL(Intermediate Language)로 빌드된 뒤 CLR의 JIT/AOT가 native instruction으로 내린다.
- `lock`/Monitor, `Interlocked`, `Volatile`은 runtime/JIT에 ordering 의미를 전달한다.
- `Interlocked.Increment`는 원자 RMW에 가깝지만 여러 field invariant를 자동 묶지 않는다.
- GC는 객체 수명을 추적하지만 unmanaged handle, pinning, native callback lifetime은 `SafeHandle`, `Dispose`, explicit unregister/join으로 관리한다.
- JIT tiering과 runtime version에 따라 기계어가 달라질 수 있어 배포 runtime에서 profile한다.

## Rust는 어떻게 발전했는가

Rust는 처음부터 소유권/borrow, `Send`/`Sync`, 표준 RAII guard를 중심으로 “공유 가능한 타입인가?”를 compile time에 더 많이 검사하는 방향을 택했다.

### `Mutex<T>`가 보호 데이터까지 감싼다

```rust
use std::sync::{Arc, Mutex};                 // thread-safe 공유 소유권과 상호 배제를 가져온다.

let total = Arc::new(Mutex::new(0_i64));     // Arc가 소유권을 공유하고 Mutex가 i64 접근을 보호한다.
let cloned = Arc::clone(&total);             // reference count를 늘린 별도 owner를 worker로 이동한다.
let worker = std::thread::spawn(move || {    // move closure가 cloned ownership을 가져간다.
    let mut guard = cloned.lock().unwrap();  // Result에서 RAII MutexGuard를 얻고 poison 정책을 선택한다.
    *guard += 1;                             // DerefMut을 통해 lock 보유 중 T를 바꾼다.
});                                         // guard drop으로 unlock하고 worker closure가 끝난다.
worker.join().unwrap();                      // panic 결과를 확인하며 worker 완료를 기다린다.
```

C++에서는 mutex와 데이터가 별도 멤버이고 규약으로 연결되는 경우가 많다. Rust `Mutex<T>`는 보호 데이터 접근 자체가 guard를 통해 나오므로 API 구조가 규약을 더 강하게 표현한다.

### `Send`와 `Sync`

- `Send`: 값의 ownership을 다른 thread로 옮겨도 안전함을 나타내는 auto trait다.
- `Sync`: `&T`를 여러 thread가 공유해도 안전함을 나타낸다.
- `Rc<T>`는 atomic reference count가 아니므로 thread 간 공유용이 아니다. `Arc<T>`를 쓴다.
- `Arc<Mutex<T>>`라고 내부 logic invariant가 자동 올바른 것은 아니지만 많은 잘못된 공유 타입은 compile time에 거절된다.
- FFI wrapper에 `unsafe impl Send/Sync`를 직접 추가하는 순간 native 계약의 thread safety를 작성자가 증명해야 한다.

### Panic과 poisoning

표준 `Mutex::lock()`은 `LockResult<MutexGuard<T>>`를 반환한다. lock 보유 중 panic이 나면 poison 상태를 보고해 보호 데이터 불변식이 깨졌을 가능성을 caller가 처리하게 한다. Poisoning은 advisory이므로 memory safety의 유일한 근거로 삼지 않는다. [Rust `Mutex` 공식 문서](https://doc.rust-lang.org/std/sync/struct.Mutex.html)

C++ mutex는 예외가 발생했다는 poison 상태를 자동 기록하지 않는다. RAII unlock 뒤 불변식이 유효한지는 C++ 설계자가 commit/rollback으로 보장해야 한다.

### Scoped thread와 수명 발전

일반 `thread::spawn` closure는 대체로 `'static` ownership을 요구한다. 이후 안정화된 `std::thread::scope`는 scope 종료 전에 모든 scoped thread가 join됨을 수명으로 표현하여 local을 안전하게 borrow할 수 있게 했다. 이는 C++의 “참조 capture 후 반드시 join” 규약을 type/lifetime API로 더 강하게 만든 형태다. [Rust scoped thread 공식 문서](https://doc.rust-lang.org/std/thread/fn.scope.html)

### Atomic과 memory order

Rust 표준 atomic ordering은 `Relaxed`, `Acquire`, `Release`, `AcqRel`, `SeqCst`를 제공하며 공식 문서가 C++20 ordering과 같은 모델임을 명시한다. C++의 `memory_order_consume`에 직접 대응하는 안정 variant는 없다. [Rust atomic `Ordering` 문서](https://doc.rust-lang.org/std/sync/atomic/enum.Ordering.html)

Ownership 검사는 atomic protocol의 논리 오류를 모두 잡지 못한다. `unsafe`, raw pointer reclamation, relaxed ordering은 C++와 마찬가지로 happens-before 증명이 필요하다.

### Rust 2015 → 2018 → 2021 → 2024 Edition 관점

Edition은 생태계가 호환 migration을 하면서 문법/진단의 기본값을 현대화하는 경계다. 동시성의 핵심 `Mutex/Arc/atomic/channel`은 일찍 자리 잡았고, 이후 scoped thread, one-time initialization 등 표준 API가 보강됐다.

FFI에서는 Rust 2024 Edition부터 외부 함수 선언 block을 `unsafe extern "C"`로 써야 한다. signature가 실제 C ABI와 맞는지 compiler가 검증할 수 없다는 proof obligation을 문법에 드러낸 변화다. [Rust 2024 unsafe extern 안내](https://doc.rust-lang.org/edition-guide/rust-2024/unsafe-extern.html)

## Python에서는 무엇이 가장 가까운가

```python
lock = threading.Lock()       # runtime lock 객체를 만든다.
with lock:                    # context manager가 예외에서도 release한다.
    balance += amount         # 같은 lock 규약을 지키는 코드와 상호 배제된다.
```

- `with lock`은 C++ RAII guard/C# lock과 scope 안전성 목적이 비슷하다.
- CPython의 GIL(Global Interpreter Lock, 전역 인터프리터 잠금)은 애플리케이션의 여러 field invariant를 보호하는 mutex가 아니다.
- I/O 동시성은 `asyncio`, blocking 작업은 thread pool, CPU 병렬성은 process/native extension 등 workload에 맞춰 선택한다.
- Python object 수명은 GC/reference counting 구현에 맡기지만 native buffer/view/callback은 별도 수명 계약이 필요하다.
- C extension이 GIL을 해제하거나 다른 Python 구현을 쓰면 “GIL 때문에 안전”이라는 추론은 더 쉽게 깨진다.

## 같은 문제를 푸는 방식 비교

### 범위 잠금

- C++: `std::lock_guard<std::mutex> guard{mutex};`
- C#: `lock (_gate) { ... }`
- Rust: `let mut guard = mutex.lock()?;`
- Python: `with lock: ...`

모두 scope 종료에서 해제를 목표로 한다. 차이는 C++/Rust가 guard object의 deterministic destruction/Drop을 쓰고, C# compiler statement가 `try/finally`를 구성하며, Python context manager의 `__exit__`가 해제한다는 점이다.

### 보호 데이터의 모양

- C++: `struct { mutex m; T data; }` + coding convention/facade
- C#: `private readonly gate` + private field + method contract
- Rust: `Mutex<T>`가 guard를 통해서만 `T` 접근 제공
- Python: lock과 object가 대개 별도이므로 convention 필요

### 실패/예외

- C++ exception 중 guard는 unlock하지만 invariant 복구는 설계자 책임이다.
- C# exception 중 `lock`은 해제되며 task exception은 await/observe해야 한다.
- Rust panic 중 guard는 drop되고 표준 mutex는 poisoning을 보고할 수 있다.
- Python exception 중 context manager는 release하지만 invariant rollback은 별도다.

## FFI 수명 번역 표

| C ABI 계약 | C++ 구현 | C# wrapper | Rust wrapper |
|---|---|---|---|
| `create -> handle*` | `new`/factory, 예외를 null/error로 변환 | `SafeHandle` 생성 | `NonNull`을 소유하는 struct |
| `destroy(handle*)` | 같은 module에서 `delete` | `ReleaseHandle`/`Dispose` | `Drop::drop` |
| `(ptr,len)` 입력 | `span/string_view`로 비소유 view, 호출 중만 사용 | pin 또는 marshalling 수명 | slice를 raw parts로 변환, 호출 중 borrow |
| callback + context | trampoline에서 예외 차단 | delegate와 `GCHandle` 보유 | `Box` context, panic 차단 |
| status + out value | exception → error enum | exception/Result-like wrapper로 변환 | `Result<T, Error>` |

## 다른 언어 사용자가 C++에서 자주 하는 실수

### C# 개발자

- GC처럼 native object가 참조되는 동안 자동으로 살아 있다고 가정한다.
- `std::thread` destructor가 join할 것이라 생각한다.
- C++ `std::mutex`를 C# Monitor처럼 reentrant라고 생각한다.
- C# class/reference semantics를 C++ 값 복사와 혼동한다.

### Rust 개발자

- C++ type system이 `Send/Sync`를 자동 검사할 것이라 기대한다.
- mutex와 보호 데이터가 떨어져 있어도 모든 접근 경로가 강제될 것이라 생각한다.
- moved-from 객체가 Rust move 후 변수처럼 사용 불가라고 생각한다. C++ moved-from 객체는 유효하지만 값은 타입 계약에 따른다.
- C++ 예외와 destructor 정책을 Rust panic/Drop과 동일시한다.

### Python 개발자

- `++`, container 한 번 호출 같은 표현이 GIL/표준 때문에 portable atomic이라고 생각한다.
- 참조 capture/view의 native 수명을 GC reference처럼 생각한다.
- CPU-bound thread 수를 늘리면 선형 향상할 것이라 가정한다.

## 선택 가이드

- Native hot path/embedded/세밀한 layout과 ABI control이 핵심이면 C++가 강하다. 대신 수명과 memory protocol 검증 부담이 크다.
- Managed service와 풍부한 async/runtime 관측성이 핵심이면 C#의 Task/async/CancellationToken 생태계가 자연스럽다.
- Native 성능과 compile-time ownership/data-race 방지의 균형이면 Rust가 강하다. FFI/unsafe 영역은 여전히 수동 증명이 필요하다.
- 빠른 orchestration과 I/O glue에는 Python이 유리하지만 CPU 병렬 hot path는 process/native/vectorized 대안을 검토한다.

언어 선택과 관계없이 가장 큰 최적화는 공유 상태를 줄이고, bounded work와 cancellation을 설계하고, production workload로 측정하는 것이다.
## 공식 참고 자료

- [.NET threading objects and features](https://learn.microsoft.com/en-us/dotnet/standard/threading/threading-objects-and-features)
- [.NET `SemaphoreSlim`](https://learn.microsoft.com/en-us/dotnet/api/system.threading.semaphoreslim)
- [C# `lock` statement](https://learn.microsoft.com/en-us/dotnet/csharp/language-reference/statements/lock)
- [Rust `std::sync`](https://doc.rust-lang.org/std/sync/)
- [Rust `std::thread`](https://doc.rust-lang.org/std/thread/)
- [Rust FFI Nomicon](https://doc.rust-lang.org/nomicon/ffi.html)
