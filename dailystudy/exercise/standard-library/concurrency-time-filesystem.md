# 동시성, 시간, 파일 시스템, 코루틴

## `std::atomic<T>` — `<atomic>`

- `T`는 원자 객체가 직접 보관하는 값 타입이다. `std::atomic<std::uint64_t>`는 정수를, `std::atomic<std::shared_ptr<const Config>>`는 공유 포인터 값을 원자적으로 다루며, 포인터가 가리키는 `Config`의 모든 멤버를 자동으로 원자화하지는 않는다.
- 원자 객체는 복사 생성·복사 대입할 수 없다. 값을 전달하려면 `load`, `store`, `exchange` 같은 명시적 원자 연산으로 어떤 메모리 순서를 쓸지 정한다.
- `load`, `store`, `exchange`, `fetch_add` 같은 연산을 다른 스레드가 중간 상태로 관찰하지 못하게 한다.
- 원자 타입이라고 모든 복합 식이 하나의 원자 연산은 아니다. `counter = counter + 1`보다 `fetch_add(1)`이 읽기-수정-쓰기를 한 연산으로 표현한다.
- `is_lock_free()` 결과는 타입과 플랫폼에 따라 달라진다. 원자 API가 반드시 단일 CPU 명령이나 lock-free 구현임을 뜻하지 않는다.
- 원자성은 가리키는 객체나 주변 비원자 메모리의 안전까지 자동 제공하지 않는다.

### 호출 계약표

아래 대표 형태에서 `order`는 읽거나 쓸 **데이터 값**이 아니라, 컴파일러와 CPU에 요구하는 원자 연산의 **메모리 순서 규칙**이다. 생략하면 기본값은 `std::memory_order_seq_cst`다.

| 대표 멤버 형태 | 입력 매개변수 | 반환값 | 원자 객체에 미치는 영향 |
|---|---|---|---|
| `T load(memory_order order = seq_cst) const` | 읽기 순서. `relaxed`, `consume`, `acquire`, `seq_cst`를 사용한다. `release`, `acq_rel`은 읽기 연산에 맞지 않아 전달하지 않는다. | 호출 시 관찰한 `T` 값의 스냅샷 | 저장된 값을 바꾸지 않는다. `const`는 이 성질을 나타낸다. |
| `void store(T desired, memory_order order = seq_cst)` | `desired`는 새로 저장할 값, `order`는 쓰기 순서. `relaxed`, `release`, `seq_cst`를 사용한다. `consume`, `acquire`, `acq_rel`은 단순 쓰기에 전달하지 않는다. | 없음(`void`) | 기존 값을 `desired`로 교체한다. |
| `T exchange(T desired, memory_order order = seq_cst)` | 새 값과 읽기-수정-쓰기 순서 | 교체되기 전 `T` 값 | 한 원자 연산에서 옛 값을 읽고 새 값으로 교체한다. |
| `T fetch_add(T difference, memory_order order = seq_cst)` | 더할 차이와 읽기-수정-쓰기 순서 | 덧셈 전의 옛 값 | 한 원자 연산에서 현재 값에 차이를 더한다. 정수·포인터 등 지원되는 특수화에서만 제공된다. |
| `bool compare_exchange_weak(T& expected, T desired, ...)` | `expected`는 예상값이면서 실패 출력 자리, `desired`는 성공 시 새 값 | 교환 성공 여부 | 같으면 `desired`를 저장하고 `true`; 다르거나 약한 비교가 허위 실패하면 현재 값을 `expected`에 써 주고 `false`다. 보통 반복문에서 쓴다. |
| `bool compare_exchange_strong(T& expected, T desired, ...)` | weak와 같지만 허위 실패를 허용하지 않는 강한 비교 | 교환 성공 여부 | 한 번만 시도하는 분기에 적합하다. 경쟁에 의한 실제 실패는 가능하다. |
| `bool is_lock_free() const` | 없음 | 이 객체의 원자 연산이 잠금 없이 구현되는지 | 저장값을 바꾸지 않는다. `true`여도 wait-free 시간 상한을 뜻하지 않는다. |
| `void wait(T old, memory_order order = seq_cst) const` | 값이 `old`와 달라질 때까지 기다릴 비교값과 읽기 순서 | 없음 | 값을 바꾸지 않는다. 깨어난 뒤에도 조건을 다시 확인하는 구조가 안전하다. |
| `void notify_one()` / `notify_all()` | 없음 | 없음 | 대기 중인 스레드 일부/전부에 재확인 기회를 주며 원자 값 자체는 바꾸지 않는다. |

선택된 오버로드와 `T`에 따라 실제 매개변수 타입은 달라진다. 따라서 코드 주석에는 함수 이름만 번역하지 말고, 수신 객체의 정확한 `atomic<T>` 타입, 현재 전달한 각 인자, 반환값을 사용하는지, 호출 뒤 저장값이 바뀌는지를 함께 적는다.

### `load(order)` 상세

- 수신 객체는 읽을 `std::atomic<T>`이고 명시 인자는 `order` 하나다. 예를 들어 `counter.load(std::memory_order_relaxed)`에서 `relaxed`는 카운터 값이 아니라 읽기 순서다.
- 반환형은 `T`이며 호출 시 그 원자 객체에서 관찰한 값의 복사본이다. 반환값이 “항상 가장 최근 벽시계 시각의 값”이라는 뜻은 아니고, C++ 메모리 모델이 허용하는 수정 순서의 값이다.
- `load`는 원자 객체의 저장값을 바꾸지 않는다. 정수 `T`는 값이 복사되고, `std::atomic<std::shared_ptr<U>>`의 반환값은 `shared_ptr<U>` 소유권 한 몫을 얻어 pointee 수명을 연장한다.
- `memory_order_acquire`는 같은 원자 객체의 release 계열 쓰기가 게시한 값을 실제로 읽을 때 그 이전 쓰기와 동기화한다. 단순히 acquire를 적었다는 사실만으로 관련 없는 저장과 동기화되지는 않는다.
- API의 작업량은 컨테이너 크기에 비례하지 않지만, lock-free·wait-free 여부와 실제 지연은 타입, 표준 라이브러리, 하드웨어, 경쟁 정도에 따라 달라진다.

### `store(desired, order)` 상세

- 첫 매개변수 `desired`는 새 원자 값이다. 두 번째 `order`는 쓰기 순서이며 반환형은 `void`라 옛 값을 돌려주지 않는다. 옛 값이 필요하면 `exchange`를 검토한다.
- `std::atomic<std::shared_ptr<U>>::store(std::move(ptr), order)`에서는 값 매개변수 `desired`가 `ptr`의 공유 소유권 한 몫을 이동받고, 성공 뒤 원자 객체가 새 `shared_ptr` 값을 보관한다. 이동된 원본 `ptr`는 유효하지만 보통 빈 상태다.
- 기존 원자 값은 교체된다. `shared_ptr` 특수화에서는 이전 pointee의 참조 횟수 감소와 소멸이 원자 값 교체 뒤 일어날 수 있으므로, 사용자 소멸자가 원자 갱신 내부에서 반드시 끝난다고 가정하지 않는다.
- `memory_order_release`는 이 호출보다 앞선 쓰기를, 이 저장값을 읽는 acquire 계열 독자에게 게시한다. store 자체는 읽기 결과를 반환하지 않는다.

### `fetch_add(difference, order)` 상세

- 첫 매개변수 `difference`는 현재 원자 값에 더할 차이다. 두 번째 매개변수 `order`는 읽기와 쓰기가 결합된 연산의 메모리 순서다.
- 반환형은 원자 값 타입이며 **증가 전 값**을 반환한다. 증가 후 값이 필요하면 반환값에 같은 차이를 지역 계산으로 더한다.
- `std::atomic<std::uint64_t>`의 부호 없는 덧셈은 최대값 다음에 0으로 순환한다. 원자성은 오버플로를 포화 처리로 바꾸지 않는다.
- `memory_order_relaxed`를 쓰면 해당 원자 값의 원자성과 수정 순서는 유지하지만 주변 데이터 게시를 위한 동기화 관계는 만들지 않는다.

### 메모리 순서

- `std::memory_order_relaxed`: 해당 원자 값의 수정 순서와 원자성만 보장한다. 독립 통계에 적합하다.
- `std::memory_order_release`: 이 저장 이전의 쓰기를 대응하는 acquire 독자에게 공개하는 게시 지점으로 쓴다.
- `std::memory_order_acquire`: 대응하는 release 값을 읽으면 그 이전 쓰기를 이후 코드가 관찰하게 한다.
- 메모리 순서는 “빠른/느린 옵션”보다 프로그램의 happens-before 계약이다. 더 약한 순서로 바꾸기 전에 정확성 증명이 필요하다.

### `fetch_add`

- 이전 값을 반환하면서 원자 값에 인자를 더한다.
- 새 값을 원하면 `fetch_add(1)+1`처럼 계산한다. 이 `+1`은 지역 반환값 계산이지 두 번째 원자 갱신이 아니다.
- 정수 원자에서 유용하며 메모리 순서를 명시할 수 있다.

## `std::mutex`, `std::scoped_lock`, `std::unique_lock`

- `mutex`는 한 번에 한 스레드만 임계 구역에 들어가게 한다. 직접 `lock` 후 예외가 나면 `unlock` 누락 위험이 있어 RAII 잠금 객체를 사용한다.
- `scoped_lock`은 생성 시 하나 이상의 mutex를 잠그고 소멸 시 모두 해제한다. 이동·복사가 불가능한 범위 잠금이다.
- `unique_lock`은 지연 잠금, 조건 변수 대기, 수동 unlock/relock 같은 유연한 상태를 제공한다. 그만큼 상태 확인이 필요하다.
- 잠금 객체 수명은 보호 구역의 정확한 범위와 일치시킨다. 잠금 아래에서 외부 콜백을 호출하면 교착·지연 위험이 있다.

## `std::condition_variable` — `<condition_variable>`

조건 변수는 공유 상태가 특정 **술어(predicate)** 를 만족할 때까지 스레드가 mutex를 양보하고 기다리게 한다. 알림 자체는 상태를 저장하지 않는다. 생산자–소비자 큐에서는 `queue가 비지 않음`, `queue에 공간이 있음`, `종료됨` 같은 상태를 mutex 아래에서 바꾸고, 대기자는 같은 mutex 아래에서 술어를 재검사해야 lost wakeup과 허위 깨움을 안전하게 처리한다.

| 대표 형태 | 수신 객체·각 입력 | 반환값 | 호출 뒤 상태·계약 |
|---|---|---|---|
| `template<class Predicate> void wait(unique_lock<mutex>& lock, Predicate pred)` | 수신 `condition_variable`, 같은 공유 상태를 보호하며 현재 잠금을 소유한 `unique_lock<mutex>` lvalue, 인자 없이 bool 문맥 결과를 내는 predicate 값. callable은 내부에서 호출되며 캡처 대상은 대기 동안 살아야 한다. | 없음(`void`) | `pred()`가 false면 mutex unlock과 대기 등록을 원자적인 대기 단계로 수행한다. 알림·허위 깨움 뒤 mutex를 다시 lock하고 pred를 반복 검사한다. 반환 시 lock은 mutex를 소유하고 pred가 true다. |
| `void notify_one() noexcept` | 수신 condition_variable만 사용하며 데이터 인자는 없다. | 없음(`void`) | 현재 대기자 하나를 unblock할 수 있다. 공유 상태와 mutex 소유권은 바뀌지 않고, 대기자가 즉시 실행되거나 어떤 대기자가 선택될지 보장하지 않는다. |
| `void notify_all() noexcept` | 수신 condition_variable만 사용하며 데이터 인자는 없다. | 없음(`void`) | 현재 대기자를 모두 unblock할 수 있다. 모두가 같은 mutex를 동시에 소유하는 것은 아니며, 각 스레드는 잠금을 경쟁한 뒤 술어를 다시 검사한다. |

- `wait(lock, pred)`는 개념적으로 `while (!pred()) wait(lock);`와 같다. `if` 한 번이나 술어 없는 wait만 사용하면 허위 깨움, 다른 소비자의 선점, 알림이 대기보다 먼저 발생하는 실행을 안전하게 다루기 어렵다.
- `lock`은 `std::unique_lock<std::mutex>`여야 하고 호출 시 mutex를 소유해야 한다. `scoped_lock`/`lock_guard`는 wait가 요구하는 일시 unlock/relock API를 제공하지 않는다.
- 공유 상태 변경과 술어 평가는 같은 mutex 아래에서 수행한다. notify는 잠금 안팎에서 호출할 수 있으나, 상태를 먼저 갱신한 뒤 호출해야 한다. 알림 자체만으로 payload를 게시하거나 데이터 경쟁을 없애지 않는다.
- timed wait는 timeout도 정상 결과로 모델링해야 하며 정확한 벽시계 마감·공정성·깨우기 순서를 보장하지 않는다. predicate나 lock 재획득에서 예외가 날 수 있는 구조는 상태 불변식과 예외 정책을 별도로 설계한다.
- condition_variable을 파괴할 때 대기 중이거나 곧 접근할 스레드가 남아 있으면 안 된다. 소유 계층은 종료 상태를 게시하고 필요한 대기자를 알린 뒤 모든 작업 스레드를 join해야 한다.
- 호출 작업량은 데이터 크기와 무관한 상수 규모지만 실제 지연은 mutex 경쟁, 스케줄러, 운영체제 구현에 좌우된다. 특정 atomic/커널 명령이나 lock-free 동작은 보장하지 않는다.
- 오늘 자료 [`../2026-08-30/main.cpp`](../2026-08-30/main.cpp)는 두 술어와 `notify_one/notify_all`로 용량 제한 queue와 close 프로토콜을 만들고, [`../2026-08-30/problem.cpp`](../2026-08-30/problem.cpp)는 one-shot 결과 게시를 연습한다.

## `std::shared_mutex`, `std::shared_lock`

- 여러 독자의 공유 잠금과 한 작성자의 독점 잠금을 제공한다.
- `shared_lock<shared_mutex>`는 읽기 경로에서 공유 잠금을 RAII로 소유한다.
- `unique_lock<shared_mutex>`는 쓰기 경로에서 독점 잠금을 소유한다.
- 읽기 잠금 아래 얻은 내부 참조를 잠금 해제 뒤 반환하면 다른 작성자가 컨테이너를 바꿔 무효화할 수 있다. 값 스냅샷 반환을 검토한다.
- 공정성·기아 방지 정책은 구현에 따라 다를 수 있다.

## `std::jthread`, `std::stop_token`, `std::this_thread::sleep_for`

- `jthread`는 실행 스레드를 소유하고 소멸 시 중지를 요청한 뒤 합류한다. `thread`의 join 누락 문제를 RAII로 줄인다.
- `jthread(F&& f, Args&&... args)`는 호출 가능 객체와 인자를 decay-copy 또는 이동해 새 스레드가 소유하게 하고 실행을 시작한다. 호출 가능하면 `stop_token`을 첫 인자로 넣는 형태를 우선 사용하며, 아니면 저장한 인자만으로 호출한다. 생성 성공 시 객체가 joinable 스레드 하나를 소유하고, 운영체제 스레드를 만들지 못하면 `std::system_error`를 던진다.
- `jthread`는 복사할 수 없고 이동할 수 있다. 이동은 실행 스레드 소유권을 옮기며, 이동된 원본은 joinable하지 않은 유효 상태가 된다. 참조 캡처나 `std::ref`로 전달한 객체는 자동 소유되지 않으므로 작업 종료까지 살아 있어야 한다.
- 소멸자는 joinable이면 `request_stop()` 뒤 `join()`한다. 작업이 중지 요청을 관찰하지 않거나 끝나지 않으면 소멸도 기다릴 수 있다. 스레드가 자기 자신을 합류하려는 구조, 소유 객체보다 오래 쓰는 참조, 예외가 작업 함수 밖으로 빠지는 구조를 피한다.
- `bool joinable() const noexcept`는 데이터 인자 없이 수신 jthread가 실행 스레드 소유권을 가졌는지 반환한다. 수신 상태를 바꾸지 않고 할당·예외가 없으며, `true`가 작업 함수가 아직 실행 중이라는 뜻은 아니다. 작업이 끝났어도 아직 `join`하지 않았다면 joinable일 수 있다.
- `void join()`은 데이터 인자와 반환값 없이 연결된 스레드가 끝날 때까지 기다린 뒤 소유 연결을 해제한다. 성공 뒤 `joinable()==false`다. joinable하지 않거나 자기 자신을 합류하려 하면 `std::system_error`가 발생한다. 합류 완료는 작업 스레드의 평가들과 join 이후 평가 사이에 동기화 관계를 만든다.
- 생성자가 작업 함수의 첫 인자로 받을 수 있으면 `stop_token`을 전달한다.
- `stop_token::stop_requested()`는 취소 요청 여부를 관찰한다. 취소는 강제 종료가 아니며 작업이 안전 지점에서 확인해야 한다.
- `request_stop()`은 공유 중지 상태에 요청을 기록하고 요청을 처음 성공시켰는지 `bool`을 반환한다.
- `sleep_for(duration)`는 최소한 지정 기간 정도 현재 스레드 실행을 양보하지만 정확한 기상 시각은 스케줄러에 따라 늦어질 수 있다.

## `std::latch` — `<latch>`

`std::latch`는 내부 카운터가 0이 될 때까지 하나 이상의 스레드를 기다리게 하는 **일회성** 동기화 객체다. 시작 준비, 작업 종료 집계처럼 참가자가 정해지고 한 단계만 통과하는 경계에 적합하다. 카운터가 0이 되면 영구히 열린 상태이며 다시 설정할 수 없다. 반복 단계에는 `std::barrier`, 조건 기반 통신에는 조건 변수나 atomic wait를 검토한다.

| 대표 형태 | 수신 객체·입력 | 반환값 | 호출 뒤 상태·계약 |
|---|---|---|---|
| `explicit latch(ptrdiff_t expected)` | 새 latch와 값 복사되는 초기 카운터 `expected`. `0 <= expected <= max()`가 전제조건이다. | 초기화된 비복사 latch 객체 | 카운터를 expected로 만든다. 0이면 처음부터 열린 상태다. 전제조건 위반은 미정의 동작이며 동적 할당 여부는 구현 세부다. |
| `void count_down(ptrdiff_t update = 1) const` | 수신 latch와 감소량. `0 <= update <= 현재 카운터`가 전제조건이고 생략 시 1이다. | 없음(`void`) | 카운터를 원자적으로 update만큼 줄인다. 0이 되는 감소는 0을 관찰한 wait/try_wait와 동기화하며 latch는 다시 닫히지 않는다. |
| `void wait() const` | 수신 latch만 사용하고 데이터 인자는 없다. | 없음(`void`) | 카운터를 바꾸지 않고 0이 될 때까지 기다린다. 이미 0이면 즉시 돌아온다. 공정성·깨우는 순서·lock-free 구현은 보장하지 않는다. |
| `void arrive_and_wait(ptrdiff_t update = 1) const` | 수신 latch와 감소량. 생략 시 1이고 count_down과 같은 범위 전제조건이 있다. | 없음(`void`) | 자신의 도착분을 줄인 뒤 카운터가 0이 될 때까지 기다린다. 한 호출로 도착과 대기를 묶지만 latch를 재사용 가능하게 만들지는 않는다. |
| `bool try_wait() const noexcept` | 수신 latch만 사용하고 데이터 인자는 없다. | 카운터 0을 관찰했는지 나타내는 bool | 수신 상태를 바꾸지 않고 즉시 반환한다. false가 앞으로의 진행이나 다른 참가자의 도착을 보장하지 않는다. |

- `count_down`과 `arrive_and_wait`의 감소량이 현재 카운터보다 크면 전제조건 위반이다. 참가자 수와 각 호출 횟수를 설계 단계에서 맞춘다.
- 카운터가 0이 되는 감소는 그 호출 이전 평가들과, 0을 관찰하고 반환한 대기 이후 평가 사이의 동기화 경계를 만든다. latch가 공유 데이터 자체를 자동 보호하지는 않으므로 카운터가 0이 되기 전에도 동시에 읽고 쓰는 데이터에는 원자 연산이나 잠금이 필요하다.
- `wait`와 `arrive_and_wait`는 스케줄러 대기로 이어질 수 있어 벽시계 시간 상한이 없다. 카운터 갱신 자체의 작업량은 참가자 수와 무관한 상수 규모지만 실제 구현은 원자 연산·스핀·운영체제 대기를 조합할 수 있다.
- latch를 파괴할 때 아직 멤버 함수를 실행 중인 스레드가 있으면 안 된다. 비소유 참조로 공유했다면 소유 객체와 모든 작업의 수명을 명시적으로 정렬한다.

## `std::barrier<CompletionFunction>` — `<barrier>`

`std::barrier`는 정해진 참가자들이 한 **단계(phase)** 에 도착할 때까지 기다린 뒤, 완료 함수를 한 번 실행하고 다음 단계에 재사용되는 C++20 동기화 객체다. 일회성 `latch`와 달리 반복 파이프라인, 시뮬레이션 tick, 병렬 계산의 단계 경계에 적합하다.

| 대표 형태 | 수신 객체·입력 | 반환값 | 호출 뒤 상태·계약 |
|---|---|---|---|
| `explicit barrier(ptrdiff_t expected, Completion completion = {})` | 새 barrier, 값으로 복사되는 예상 참가자 수, barrier가 이동 소유할 완료 callable | 초기화된 비복사 barrier 객체 | 현재·다음 단계 예상 수를 expected로 만든다. `0 <= expected <= max()`가 전제조건이며 0이면 객체는 파괴만 할 수 있다. 명시된 예외는 callable 이동 생성 예외다. |
| `void arrive_and_wait()` | 현재 단계의 barrier 수신 객체만 사용하며 데이터 인자는 없다. | 없음(`void`) | 현재 단계의 미도착 수를 1 줄이고 단계 완료까지 기다린다. 마지막 도착 시 완료 함수를 한 번 실행하며 완료 뒤 다음 단계 카운트가 다시 열린다. |
| `arrival_token arrive(ptrdiff_t update = 1)` | 감소량 update. 생략 시 1이며 현재 예상 수를 넘지 않아야 한다. | 현재 단계 도착을 나타내는 이동 전용 token | 도착만 기록하고 기다리지 않는다. token은 같은 barrier의 `wait`에 넘겨야 하며 barrier보다 오래 살 수 없다. |
| `void wait(arrival_token&& token) const` | 같은 barrier의 이전 단계 token xvalue | 없음(`void`) | token이 가리키는 단계가 끝날 때까지 기다린다. 다른 barrier token이나 잘못된 수명은 전제조건 위반이다. |
| `void arrive_and_drop()` | 현재 단계 barrier만 사용하며 데이터 인자는 없다. | 없음(`void`) | 현재 단계 도착 수와 이후 단계의 예상 참가자 수를 각각 1 줄인다. 호출한 참가자는 다음 단계부터 빠져야 한다. |

- 단계 완료 시점에는 완료 함수가 실행되고, 완료 함수 종료는 그 단계에서 풀려나는 모든 호출의 반환보다 강하게 happens-before한다. 완료 함수가 읽는 데이터도 해당 단계 도착 전 쓰기와 이 관계를 통해 연결할 수 있다.
- 완료 함수는 barrier 객체가 소유한다. 참조 캡처 대상은 barrier와 모든 참가자 호출보다 오래 살아야 한다. `is_nothrow_invocable_v<Completion&>`가 true여야 하므로 호출 연산자를 `noexcept`로 만든다.
- `arrive_and_wait()`의 반환형은 `void`이며 도착 token을 노출하지 않는다. 도착과 다른 일을 겹치려면 `arrive()` 반환 token을 저장했다가 `wait(std::move(token))`한다.
- 어느 참가자가 약속한 단계에 도착하지 않으면 나머지는 무기한 기다릴 수 있다. 조기 종료 참가자는 설계된 지점에서 `arrive_and_drop()`을 호출해야 하며, 파괴 중인 barrier에 다른 스레드가 접근하면 안 된다.
- barrier가 공유 payload의 모든 접근을 자동 원자화하지는 않는다. 단계 안에서 동시에 같은 객체를 읽고 쓰면 별도 원자·잠금·분할 소유가 필요하다. 단계 전후 순서만 barrier 동기화로 증명한다.
- API 호출 수 자체는 원소 수와 무관한 상수 규모지만 실제 지연은 가장 느린 참가자, 경쟁, OS 스케줄링에 좌우되어 시간 상한을 보장하지 않는다. lock-free 구현이나 특정 CPU 명령도 보장하지 않는다.
- 오늘 자료 [`../2026-08-27/main.cpp`](../2026-08-27/main.cpp)는 expected에 `weights.size()`를 `ptrdiff_t`로 변환해 넣고, 완료 lambda를 xvalue로 이동 소유시킨다. 두 번의 `arrive_and_wait()`는 준비와 게시 단계를 분리하고 완료 함수는 총 두 번 실행된다.

## `std::counting_semaphore<LeastMaxValue>`, `std::binary_semaphore` — `<semaphore>`

세마포어는 음수가 아닌 내부 counter를 가진 C++20 동기화 객체다. `acquire`는 permit 하나를 얻을 때까지 기다리고 counter를 줄이며, `release`는 permit을 반환해 counter를 늘린다. `std::binary_semaphore`는 `std::counting_semaphore<1>`의 별칭이라 닫힘/열림 신호나 단일 handoff에 적합하다. payload를 소유하는 큐는 아니며, 여러 값을 보관하려면 별도 저장소와 동시성 계약이 필요하다.

| 대표 형태 | 수신 객체·각 입력 | 반환값 | 호출 뒤 상태·계약 |
|---|---|---|---|
| `explicit counting_semaphore(ptrdiff_t desired)` | 새 `counting_semaphore<LeastMaxValue>`와 값 복사되는 초기 counter `desired`. `0 <= desired <= max()`가 전제조건이다. | 초기화된 비복사·비이동 semaphore 객체 | counter가 desired가 된다. 전제조건 위반은 미정의 동작이다. 동적 할당·특정 OS 객체·lock-free 여부는 보장하지 않는다. |
| `void acquire()` | 수신 semaphore만 사용하고 데이터 인자는 없다. | 없음(`void`) | counter가 양수가 될 때까지 막힐 수 있고 성공 시 1 줄인다. 대응 `release` 이전 평가는 성공한 `acquire` 이후 평가와 동기화한다. 대기 시간 상한·공정성·깨우기 순서는 보장하지 않는다. |
| `bool try_acquire()` | 수신 semaphore만 사용하고 데이터 인자는 없다. | 즉시 permit을 얻었는지 나타내는 bool | true면 counter를 1 줄인다. false면 상태를 바꾸지 않지만 counter가 양수여도 허위 실패할 수 있으므로 정확한 현재 크기 조회로 쓰지 않는다. |
| `template<class Rep, class Period> bool try_acquire_for(const chrono::duration<Rep,Period>& rel_time)` | 수신 semaphore와 const 참조로 빌린 상대 duration. duration은 호출 동안만 읽힌다. | 제한 시간 안에 permit을 얻었는지 bool | true면 counter를 1 줄이고 false면 permit을 얻지 않는다. 허위 실패·스케줄러 지연이 가능해 정확한 실시간 마감 보장은 아니다. |
| `void release(ptrdiff_t update = 1)` | 수신 semaphore와 값 복사되는 증가량 update. 생략하면 1이고 `0 <= update <= max()-현재 counter`가 전제조건이다. | 없음(`void`) | counter를 update만큼 늘리고 대기자가 진행할 수 있게 한다. 최대값을 넘기거나 얻지 않은 permit을 중복 반환하면 전제조건을 깨뜨릴 수 있다. |

- `LeastMaxValue`는 최소한 필요한 최대 counter를 컴파일 시간에 표현한다. 실제 `max()`는 이 값 이상일 수 있지만 코드가 그 여유를 몰래 사용해서는 안 된다.
- `release`는 그 호출 이전 평가와, 그 release로 permit을 얻어 성공한 `acquire` 이후 평가 사이에 strongly happens-before 관계를 만든다. 오늘 mailbox는 이 관계로 비원자 `slot_` payload를 한 번 안전하게 넘긴다.
- semaphore는 공유 payload에 대한 상호 배제를 자동으로 제공하지 않는다. permit이 여러 개면 여러 스레드가 동시에 임계 구역에 들어가므로 같은 객체를 쓴다면 별도 분할 소유·원자·잠금이 필요하다.
- semaphore 객체를 파괴할 때 다른 스레드가 멤버 함수를 실행하거나 기다리고 있으면 안 된다. 비소유 참조/포인터로 감싼 RAII Lease는 semaphore보다 먼저 모두 파괴되어야 한다.
- `acquire` 성공 뒤 모든 반환·예외 경로에서 정확히 한 번 `release`하도록 이동 전용 RAII guard를 쓰면 permit 누수와 이중 반환을 줄일 수 있다.
- API 작업량은 컨테이너 원소 수와 무관한 상수 규모지만 실제 지연은 경쟁·스케줄러·구현에 좌우된다. 특정 atomic 명령, futex, 커널 대기 방식으로 단정하지 않는다.
- 오늘 자료 [`../2026-08-29/main.cpp`](../2026-08-29/main.cpp)는 binary semaphore의 0→1 release와 1→0 acquire로 payload 게시를 증명하고, [`../2026-08-29/problem.cpp`](../2026-08-29/problem.cpp)는 counting semaphore permit 반환 책임을 이동 전용 소멸자에 둔다.

## `std::chrono`

- `duration<Rep,Period>`는 숫자 표현 타입과 한 틱의 단위를 타입에 담는다.
- `std::chrono::milliseconds`, `std::chrono::microseconds`는 자주 쓰는 duration 별칭이다.
- `std::chrono::time_point<Clock,Duration>`은 특정 시계의 기준점부터 지난 duration을 나타낸다.
- `std::chrono::steady_clock`은 단조 증가해 경과 시간 측정에 적합하다. 달력 시각이나 로그 타임스탬프에는 `system_clock`을 검토한다.
- `std::chrono::duration_cast<To>(value)`는 시간 단위를 명시 변환한다. 더 거친 단위로 바꾸면 나머지가 잘릴 수 있다.
- `std::chrono_literals`를 가져오면 `1500ms`, `2s` 같은 리터럴을 쓸 수 있다. 넓은 헤더/전역 namespace 오염을 피하고 작은 범위에서 사용한다.

## `std::filesystem`과 `std::error_code`

- `std::filesystem::path`는 운영체제 경로 구문을 값으로 모델링한다. `/` 연산자는 구분자를 직접 이어 붙이는 대신 경로 요소를 결합한다.
- `path::is_absolute()`는 경로 문법상 루트가 완전한지 확인해 `bool`을 반환한다. 파일 시스템을 조회하지 않으므로 경로가 실제로 존재하거나 접근 가능한지는 보장하지 않으며, 판정 규칙은 운영체제 경로 문법의 영향을 받는다.
- `lexically_normal()`은 파일 시스템 접근 없이 `.`·`..`와 중복 구분자를 어휘적으로 정리한다. 심볼릭 링크를 해석한 보안 검증은 아니다.
- `directory_iterator`는 디렉터리 항목을 한 번 순회하는 입력 반복자 성격을 갖는다. 순회 중 파일 시스템이 바뀌면 관찰 결과가 달라질 수 있다.
- `directory_entry::is_regular_file(error)`와 `file_size(error)` 오버로드는 예상 가능한 OS 실패를 예외 대신 `error_code`에 기록한다.
- `std::error_code`는 오류 값과 범주를 묶는다. `if (error)`로 실패 여부를 검사하고 `message()`는 사람이 읽는 구현별 문자열을 만든다.
- 오류 코드 오버로드 호출 전후 같은 `error_code`를 재사용할 때 성공 시 값이 지워지는지 각 함수 계약을 확인한다.
- 상대 경로의 `..` 거부만으로 symlink, junction, TOCTOU 공격을 모두 막을 수 없다.

## 코루틴 지원 타입 — `<coroutine>`

- `std::coroutine_handle<Promise>`는 코루틴 프레임을 가리키는 작고 복사 가능한 핸들이다. 기본적으로 프레임 소유권을 자동 관리하지 않는다.
- `resume()`은 중단된 코루틴을 다음 중단/종료 지점까지 실행한다. 완료된 프레임을 재개하면 안 된다.
- `destroy()`는 코루틴 프레임을 파괴한다. 정확히 한 번 호출하고 이후 핸들을 사용하지 않는다.
- `done()`은 최종 중단 상태인지 확인한다.
- `std::suspend_always`는 `await_ready()`가 거짓이라 항상 중단하는 awaiter다.
- `std::exchange(obj,new_value)`는 옛 값을 이동해 반환하고 객체에 새 값을 대입한다. 이동 생성자에서 핸들 소유권을 넘기고 원본을 빈 상태로 만들 때 유용하다.
- 처리되지 않은 예외 경로에서 `std::terminate()`를 호출하면 복구 없이 프로그램이 종료된다.

## 최소 예제

```cpp
#include <atomic>
#include <memory>

struct Config {
    int limit{};
};

int main() {
    std::atomic<std::shared_ptr<const Config>> current{
        std::make_shared<const Config>(Config{3})};
    current.store(std::make_shared<const Config>(Config{5}),
                  std::memory_order_release);
    const std::shared_ptr<const Config> snapshot{
        current.load(std::memory_order_acquire)};
    return snapshot->limit == 5 ? 0 : 1;
}
```

## 직접 검증

1. relaxed 카운터 두 개를 따로 읽은 값이 같은 시점의 스냅샷이 아닐 수 있는 실행 순서를 그린다.
2. `shared_lock`으로 찾은 `unordered_map` 원소의 참조를 반환할 때 잠금 해제 뒤 위험을 설명한다.
3. `steady_clock` 대신 `system_clock`으로 성능 구간을 재면 시스템 시각 보정이 어떤 영향을 줄 수 있는지 말한다.
4. 코루틴 핸들의 이동 생성자에서 `exchange(other.handle,nullptr)`가 이중 파괴를 막는 과정을 설명한다.
