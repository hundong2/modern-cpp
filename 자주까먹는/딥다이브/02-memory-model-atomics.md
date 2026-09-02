# 2단계: C++ 메모리 모델과 atomic

> **한 줄 요약:** atomic의 핵심은 “찢어지지 않는 값”만이 아니라 어떤 쓰기가 어떤 읽기보다 happens-before가 되는지를 프로그램 전체에서 증명하는 것이다.

- 선행 지식: [소스에서 CPU까지](./01-machine-compiler-memory.md)
- 초보자 우선: **관계 네 가지**, **publish 예제**, **메모리 순서 선택 규칙**
- 전문가 목표: release sequence, CAS 실패 순서, fence, lifetime race까지 검토한다.

## 가장 먼저 구별할 네 관계

| 관계 | 같은 스레드? | 뜻 |
|---|---:|---|
| sequenced-before | 예 | 한 평가 A가 같은 스레드의 B보다 앞서도록 언어가 정한 관계 |
| modification order | 객체별 | 하나의 atomic 객체에 가해진 모든 modification의 단일 총순서 |
| synchronizes-with | 스레드 사이 | 예: release store의 값을 acquire load가 읽을 때 생기는 동기화 간선 |
| happens-before | 둘 다 | sequenced-before와 synchronizes-with 등을 이은 전이적 “먼저 일어남” 관계 |

벽시계 시간이 먼저였다는 사실만으로 happens-before가 되지 않는다. 디버거에서 A가 먼저 실행된 것처럼 보여도 표준 동기화 간선이 없으면 다른 실행에서는 실패할 수 있다.

## Data race의 정확한 조건

서로 다른 스레드의 두 potentially concurrent action이 같은 memory location에 conflict하고, **적어도 하나가 atomic 연산이 아니며**, happens-before로 정렬되지 않으면 data race다. conflict는 대체로 하나 이상이 쓰기이거나 객체 수명을 시작/끝내는 경우다. C++ data race는 Undefined Behavior(UB, 정의되지 않은 동작)다.

```cpp
int counter = 0;      // 비원자 공유 객체다.
// thread A: ++counter; // read + add + write이며 동기화가 없다.
// thread B: ++counter; // A와 충돌하므로 단순 lost update를 넘어 data race/UB다.
```

`++counter`는 소스 한 연산처럼 보여도 원자적이라는 뜻이 아니다. 컴파일러는 정상 C++ 프로그램에 data race가 없다고 가정해 값을 레지스터에 오래 보관하거나 접근을 합칠 수 있다.

## Atomic이 보장하는 세 축

1. **원자성(atomicity):** 해당 atomic 객체에 대한 연산이 중간 상태로 관찰되지 않는다.
2. **수정 순서(modification order):** 그 atomic 객체의 쓰기/RMW는 모두 같은 순서를 따른다.
3. **메모리 순서(ordering):** 주변의 다른 메모리 접근이 스레드 사이에서 어떤 관계를 갖는지 지정한다.

Atomic은 여러 객체를 한 트랜잭션으로 묶지 않는다. `atomic<int> x, y`의 각각은 원자적이어도 `(x, y)` 쌍의 불변식은 별도 프로토콜이 필요하다.

## Publish 패턴: release와 acquire

```cpp
std::string message;                      // mutex 없이 직접 공유할 비원자 payload다.
std::atomic<bool> ready{false};            // publish 상태를 운반하는 atomic이다.

// producer thread
message = "완료";                         // (1) payload 쓰기: 같은 스레드에서 (2)보다 앞선다.
ready.store(true, std::memory_order_release); // (2) release store: 앞의 쓰기를 publish한다.

// consumer thread
if (ready.load(std::memory_order_acquire)) {  // (3) (2)의 true를 읽으면 synchronizes-with가 생긴다.
    use(message);                         // (4) (1)이 (4)에 happens-before이므로 읽기가 적법하다.
}
```

관계는 `(1) sequenced-before (2) synchronizes-with (3) sequenced-before (4)`다. 전이성으로 (1)은 (4)에 happens-before다.

```mermaid
flowchart LR
    A["생산자: message 쓰기"] -->|"sequenced-before"| B["ready.store<br/>release"]
    B -->|"값을 읽으면 synchronizes-with"| C["ready.load<br/>acquire"]
    C -->|"sequenced-before"| D["소비자: message 읽기"]
```

`ready`를 relaxed로 바꾸면 ready 값 자체는 안전하게 읽지만 `message`의 비원자 쓰기/읽기를 연결하는 synchronizes-with 간선이 없다. 이 publish 프로토콜에는 부족하다.

## `std::memory_order` 선택표

| 순서 | 주 용도 | 주변 접근에 대한 핵심 제약 | 초보자 지침 |
|---|---|---|---|
| `relaxed` | 통계 카운터, 고유 ID | 해당 atomic의 원자성과 modification order만 | 다른 데이터를 publish하지 않을 때만 |
| `acquire` | publish된 상태 읽기, lock 획득 | 뒤의 접근이 acquire 앞으로 넘어가 관찰을 깨지 못함 | 대응 release가 무엇인지 표시 |
| `release` | 상태 publish, lock 해제 | 앞의 접근이 release 뒤로 넘어가 관찰을 깨지 못함 | 무엇을 publish하는지 표시 |
| `acq_rel` | 성공한 RMW가 읽고 publish도 함 | acquire + release | `fetch_*`, CAS 성공에 자주 사용 |
| `seq_cst` | 가장 단순한 전역 추론 | acquire/release에 더해 seq_cst 연산의 단일 총순서 | 기본값; 근거 없이 약화하지 않기 |
| `consume` | 의존성 기반 순서 | 구현·명세 역사가 복잡함 | C++11~23 실무에서는 보통 acquire 사용 |

먼저 mutex 또는 기본 `seq_cst`로 정확하게 만들고, 실제 병목이 증명된 경우에만 약한 순서로 바꾼다. 약화 PR에는 반드시 happens-before 증명과 대상 아키텍처별 성능 측정이 있어야 한다.

## Atomic 연산 종류

| 분류 | 예 | 성질 |
|---|---|---|
| load | `a.load(order)` | 값만 읽는다. release order를 쓸 수 없다 |
| store | `a.store(v, order)` | 값만 쓴다. acquire order를 쓸 수 없다 |
| exchange | `a.exchange(v, order)` | 이전 값을 반환하는 Read-Modify-Write(RMW) |
| fetch | `fetch_add`, `fetch_or` | 이전 값을 반환하는 RMW |
| CAS | `compare_exchange_weak/strong` | 예상값과 같을 때만 교체하는 RMW |
| wait/notify | `wait`, `notify_one/all`(C++20) | 값이 같을 동안 효율적으로 기다릴 수 있게 함 |

### CAS에서 `expected`가 바뀌는 이유

```cpp
bool compare_exchange_weak(
    T& expected,
    T desired,
    std::memory_order success,
    std::memory_order failure) noexcept;
```

- 현재 값이 `expected`와 같고 성공하면 `desired`를 저장하고 `true`를 반환한다.
- 실패하면 실제 현재 값을 `expected`에 써 주고 `false`를 반환한다.
- `weak`는 값이 같아도 spuriously fail할 수 있어 반복문에 적합하다.
- `strong`은 그런 허위 실패가 없으므로 한 번만 시도하는 분기에 편하다.
- failure order는 store를 포함할 수 없으므로 `release`, `acq_rel`을 쓸 수 없고 success order보다 강할 수 없다.

```cpp
int expected = state.load(std::memory_order_relaxed);
while (!state.compare_exchange_weak(
    expected,
    expected + 1,
    std::memory_order_acq_rel,
    std::memory_order_relaxed)) {
    // 실패할 때 expected가 최신 값으로 갱신되므로 그 값으로 desired를 다시 계산한다.
}
```

## Release sequence와 RMW

Release store 뒤에 같은 atomic의 RMW들이 modification order에서 이어질 때, acquire가 그 sequence의 값을 읽어 최초 release가 publish한 데이터까지 볼 수 있는 규칙이 있다. 정확한 release sequence 정의는 표준 판본에 따라 정리 방식이 달라졌으므로, 복잡한 다중 생산자 프로토콜에서는 “어느 write/RMW 값을 읽었는지”를 도식화하고 해당 대상 표준 문구를 확인한다.

실무 지침:

- CAS chain에 release sequence를 암묵적으로 기대하면 코드 옆에 불변식과 간선을 쓴다.
- 값이 wraparound하거나 ABA 문제가 가능한지 별도로 검토한다.
- 단순한 mutex가 충분하면 검증 비용까지 포함해 mutex를 선택한다.

## Fence는 마지막 수단

`std::atomic_thread_fence(order)`는 atomic 객체 자체의 연산이 아니라 앞뒤 평가의 순서를 연결하는 fence다. 올바른 fence 동기화에는 별도의 atomic read/write가 관계를 운반해야 한다. fence 하나를 아무 위치에 추가해 data race를 치료할 수 없다.

`std::atomic_signal_fence`는 같은 스레드와 signal handler 관점의 compiler ordering 도구이며 CPU 간 동기화 fence가 아니다.

## C++20 `atomic::wait/notify`

```cpp
std::atomic<int> state{0};

// waiter
state.wait(0, std::memory_order_acquire); // 값이 0인 동안 block 가능; 반환 후 반드시 상태를 다시 해석한다.

// publisher
payload = make_payload();
state.store(1, std::memory_order_release);
state.notify_one();
```

`wait(old)`는 현재 값이 `old`와 다를 때 반환한다. 구현은 짧게 spin한 뒤 OS 대기를 사용할 수 있다. ABA처럼 값이 바뀌었다가 다시 `old`가 되면 변화 자체를 관찰하지 못할 수 있으므로 세대 번호나 상태 머신을 사용한다.

## `std::atomic_ref`(C++20)

기존 객체를 atomic처럼 참조한다. 다음 조건을 지켜야 한다.

- 참조 대상의 수명과 정렬 요구 사항을 만족한다.
- 같은 객체에 겹치는 `atomic_ref`가 살아 있는 동안 모든 동시 접근은 atomic 경로를 사용한다.
- lock-free 여부와 required alignment는 타입/구현에 따라 다르다.

기존 wire format이나 배열 레이아웃을 유지해야 할 때 유용하지만, 일반 코드에서는 처음부터 `std::atomic<T>`로 의도를 드러내는 편이 안전하다.

## `is_lock_free`와 진행 보장

- `is_always_lock_free`: 해당 `atomic<T>`가 그 구현의 모든 실행에서 lock-free인지 compile-time 상수로 알려 준다.
- `is_lock_free()`: 현재 객체/실행 환경에 대한 runtime 질의다.
- lock-free는 시스템 전체의 어떤 연산이 진행한다는 성질이지 wait-free나 starvation-free가 아니다.
- 내부 lock을 쓰지 않는다고 캐시 line 경합, 재시도 폭증, 우선순위 역전이 자동 해결되지 않는다.

## 수명도 race의 일부다

Atomic pointer가 가리키는 객체의 주소를 안전하게 읽었다고 pointee의 수명이 안전해지는 것은 아니다. 다른 스레드가 그 객체를 해제할 수 있다면 hazard pointer, epoch reclamation, RCU 계열 또는 적절한 shared ownership 같은 별도 reclamation 프로토콜이 필요하다. `shared_ptr` atomic 연산도 reference count 관리 비용과 객체 내부 데이터 동기화를 구별해야 한다.

## 올바름 검토 템플릿

1. 공유되는 memory location을 모두 적는다.
2. 각 접근이 read/write/RMW/lifetime start/end 중 무엇인지 적는다.
3. 비원자 충돌 쌍마다 happens-before 경로를 그린다.
4. acquire가 정확히 어느 release 또는 release sequence의 값을 읽는지 적는다.
5. relaxed 값을 기반으로 비원자 메모리에 접근하지 않는지 본다.
6. 객체 수명과 reclamation이 접근보다 길다는 증거를 적는다.
7. ThreadSanitizer는 보조 증거로 사용하되 테스트 미실행 경로가 안전하다고 추론하지 않는다.

## 자주 하는 실수

- `atomic<bool> ready`가 있으니 주변의 모든 데이터가 자동 안전하다고 생각한다.
- relaxed 카운터 값을 다른 배열의 index/수명 protocol로 쓰면서 ordering을 증명하지 않는다.
- `compare_exchange_weak` 실패 시 바뀐 `expected`를 무시한다.
- atomic 여러 개의 snapshot이 일관된 시점이라고 가정한다.
- x86-64에서 우연히 동작한 코드를 ARM64에서도 표준상 올바르다고 결론낸다.

## 완료 기준

- [ ] publish 예제의 네 간선을 말로 설명한다.
- [ ] relaxed와 acquire/release의 차이를 “캐시 flush”라는 부정확한 표현 없이 설명한다.
- [ ] CAS의 `expected`, desired, success/failure order를 설명한다.
- [ ] 다음 문서인 [mutex와 RAII](./03-locks-and-raii.md)로 이동한다.
