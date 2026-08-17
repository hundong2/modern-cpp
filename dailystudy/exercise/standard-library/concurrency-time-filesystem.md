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

## `std::shared_mutex`, `std::shared_lock`

- 여러 독자의 공유 잠금과 한 작성자의 독점 잠금을 제공한다.
- `shared_lock<shared_mutex>`는 읽기 경로에서 공유 잠금을 RAII로 소유한다.
- `unique_lock<shared_mutex>`는 쓰기 경로에서 독점 잠금을 소유한다.
- 읽기 잠금 아래 얻은 내부 참조를 잠금 해제 뒤 반환하면 다른 작성자가 컨테이너를 바꿔 무효화할 수 있다. 값 스냅샷 반환을 검토한다.
- 공정성·기아 방지 정책은 구현에 따라 다를 수 있다.

## `std::jthread`, `std::stop_token`, `std::this_thread::sleep_for`

- `jthread`는 실행 스레드를 소유하고 소멸 시 중지를 요청한 뒤 합류한다. `thread`의 join 누락 문제를 RAII로 줄인다.
- 생성자가 작업 함수의 첫 인자로 받을 수 있으면 `stop_token`을 전달한다.
- `stop_token::stop_requested()`는 취소 요청 여부를 관찰한다. 취소는 강제 종료가 아니며 작업이 안전 지점에서 확인해야 한다.
- `request_stop()`은 공유 중지 상태에 요청을 기록하고 요청을 처음 성공시켰는지 `bool`을 반환한다.
- `sleep_for(duration)`는 최소한 지정 기간 정도 현재 스레드 실행을 양보하지만 정확한 기상 시각은 스케줄러에 따라 늦어질 수 있다.

## `std::chrono`

- `duration<Rep,Period>`는 숫자 표현 타입과 한 틱의 단위를 타입에 담는다.
- `milliseconds`, `microseconds`는 자주 쓰는 duration 별칭이다.
- `time_point<Clock,Duration>`은 특정 시계의 기준점부터 지난 duration을 나타낸다.
- `steady_clock`은 단조 증가해 경과 시간 측정에 적합하다. 달력 시각이나 로그 타임스탬프에는 `system_clock`을 검토한다.
- `duration_cast<To>(value)`는 시간 단위를 명시 변환한다. 더 거친 단위로 바꾸면 나머지가 잘릴 수 있다.
- `chrono_literals`를 가져오면 `1500ms`, `2s` 같은 리터럴을 쓸 수 있다. 넓은 헤더/전역 namespace 오염을 피하고 작은 범위에서 사용한다.

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
