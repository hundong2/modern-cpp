# 07. `volatile`, `std::atomic`, 동시성

## 먼저 세 개를 분리하기

- atomicity: 연산이 다른 스레드에 쪼개진 중간 상태로 관찰되지 않는 성질
- synchronization/ordering: 여러 스레드의 작업이 어떤 순서로 보이는지 정하는 관계
- lock-free: 구현이 mutex 같은 lock에 의존하지 않고 진행됨을 뜻하는 구현 특성

atomic이라고 반드시 lock-free는 아니며, lock-free라고 모든 알고리즘이 wait-free인 것도
아닙니다.

## thread와 data race

둘 이상의 스레드가 같은 메모리 위치에 동시에 접근하고, 적어도 하나가 쓰기이며, 적절한
synchronization이 없고 atomic 접근도 아니라면 data race가 발생합니다. C++에서 data race는
undefined behavior입니다.

```cpp
int counter = 0;
// 여러 스레드가 counter++ 실행 -> read-modify-write가 겹쳐 data race
```

race condition은 실행 순서에 따라 논리 결과가 달라지는 더 넓은 개념입니다. data race가
없어도 check-then-act 같은 논리 race는 존재할 수 있습니다.

## mutex

`std::mutex`는 한 번에 한 스레드만 임계 구역에 들어가게 합니다. 직접 `lock()`/`unlock()`보다
`std::lock_guard`나 `std::scoped_lock`으로 RAII를 적용합니다.

```cpp
std::mutex m;
int counter = 0;

void increment() {
    std::lock_guard<std::mutex> guard(m);
    ++counter;
}
```

여러 mutex의 잠금 순서, deadlock, lock 범위, condition variable의 predicate를 별도 검토해야
합니다.

## `volatile`

### 뜻

`volatile` 객체 접근은 구현이 관찰 가능한 side effect로 취급해야 하는 특수 접근입니다.
주 사용처는 메모리 매핑 하드웨어 레지스터, 신호 처리 같은 구현 의존 저수준 코드입니다.

```cpp
volatile std::uint32_t* device_register = /* 장치 주소 */;
auto status = *device_register; // 실제 접근이 필요하다는 의도를 표현
```

### 하지 않는 것

`volatile`은 다음을 보장하지 않습니다.

- 여러 스레드 사이의 atomicity
- happens-before 관계
- CPU 메모리 장벽
- 복합 연산 `++x`의 원자성
- data race 방지

즉 “다른 스레드가 값을 바꾼다”는 이유만으로 `volatile`을 쓰면 안 됩니다. 스레드 공유
상태에는 mutex 또는 `std::atomic`과 올바른 메모리 순서를 사용합니다.

### cv 조합

- `const T`: 프로그램의 그 접근 경로로 수정하지 않음
- `volatile T`: volatile 접근 의미가 필요
- `const volatile T`: 둘 다 적용

멤버 함수 뒤의 `const volatile`은 그 함수를 `const volatile` 객체에서 호출할 수 있다는
뜻입니다.

## `std::atomic<T>`

`<atomic>` 헤더의 클래스 템플릿입니다.

```cpp
std::atomic<int> count{0};
count.fetch_add(1);
int snapshot = count.load();
```

같은 atomic 객체에 대한 atomic 연산끼리는 data race 없이 수행됩니다. 그러나 여러 atomic
변수에 걸친 불변식이 자동으로 하나의 transaction이 되지는 않습니다.

일반 `std::atomic<T>`의 `T`는 trivially copyable여야 하며 표준 버전에 따라
copy/move constructible/assignable 관련 형식 요구가 더 명시됩니다. 아무 클래스나
`atomic<클래스>`로 감싼다고 안전해지는 것이 아닙니다. 큰 사용자 타입은 구현 내부 lock을
쓸 가능성도 있으므로 요구 조건과 `is_lock_free` 결과를 각각 확인합니다.

주요 연산:

- `load`: 읽기
- `store`: 쓰기
- `exchange`: 새 값 저장 + 이전 값 반환
- `compare_exchange_weak/strong`: 예상 값과 같을 때 교체
- `fetch_add/sub/and/or/xor`: atomic read-modify-write
- C++20 `wait/notify_one/notify_all`: 값 변경 대기/통지

`atomic_flag`는 항상 lock-free여야 하는 최소 atomic boolean-like 타입입니다. 일반
`atomic<T>`의 lock-free 여부는 타입과 플랫폼/정렬/구현에 따라 다를 수 있습니다.

## 메모리 순서 입문

기본값 `std::memory_order_seq_cst`는 가장 이해하기 쉬운 단일 전역 순서 모델을 제공합니다.
성능 측정과 증명 없이 약한 순서로 낮추지 마세요.

- `relaxed`: 해당 객체의 원자성/수정 순서는 보장, 다른 메모리와 동기화하지 않음
- `acquire`: 이 load 뒤 작업이 앞당겨지는 것을 제한, 대응 release의 이전 쓰기를 봄
- `release`: 이 store 앞 작업이 뒤로 밀리는 것을 제한
- `acq_rel`: read-modify-write에 acquire + release
- `seq_cst`: acquire/release 성질 + seq_cst 연산 간 단일 총순서

전형적인 메시지 전달:

```cpp
int data = 0;
std::atomic<bool> ready{false};

// 생산자
data = 42;
ready.store(true, std::memory_order_release);

// 소비자
if (ready.load(std::memory_order_acquire)) {
    // release/acquire가 연결되면 data == 42를 안전하게 관찰
}
```

이 코드를 실제 여러 스레드로 만들 때는 반복/수명/종료까지 설계해야 합니다.

## lock-free의 단계

- `a.is_lock_free()`: 이 타입 객체의 atomic 연산이 현재 객체/실행에서 lock-free인지 질의
- `std::atomic<T>::is_always_lock_free`(C++17): 해당 구현에서 타입이 항상 lock-free인지
  컴파일 타임 상수로 질의
- `ATOMIC_*_LOCK_FREE` 매크로: 특정 기본 atomic 타입군의 보장 수준(0/1/2)

lock-free는 대개 “한 스레드가 멈춰도 시스템 전체 일부 작업은 진행”이라는 계열의
progress guarantee입니다. wait-free(각 연산이 유한 단계 내 완료)와 같지 않습니다.
lock-free 자료구조는 ABA 문제, 메모리 회수, cache contention까지 다뤄야 하므로 초급
최적화 수단이 아닙니다.

## 예시 선언을 다시 읽기

cppreference에는 두 overload가 있습니다.

```cpp
bool is_lock_free() const noexcept;
bool is_lock_free() const volatile noexcept;
```

두 번째를 해석하면:

1. `bool`: `true`/`false` 반환
2. `is_lock_free`: lock-free 구현 여부를 묻는 멤버
3. `()`: 명시적 인자 없음
4. `const`: 이 호출로 atomic 객체의 논리 값을 변경하지 않음
5. `volatile`: volatile-qualified atomic 객체에서도 호출할 overload
6. `noexcept`: 예외를 호출자에게 전파하지 않음

여기서 `volatile`이 atomicity를 제공하는 것이 아닙니다. 타입이 이미 `std::atomic<T>`라서
atomic 연산 의미를 갖고, overload의 `volatile`은 그런 cv 객체에서 호출 가능하게 할
뿐입니다. 일반 코드에서는 비-volatile overload를 주로 사용합니다.

페이지 Notes의 핵심은 다음입니다.

- `atomic_flag`를 제외한 atomic 타입은 내부적으로 lock을 사용할 수 있습니다.
- 같은 타입도 정렬 등 실행 조건에 따라 lock-free 여부가 달라질 수 있습니다.
- 따라서 결과는 플랫폼 독립 상수가 아닐 수 있습니다.

## compare-exchange 읽기

```cpp
bool compare_exchange_weak(T& expected, T desired);
```

현재 값이 `expected`와 같으면 `desired`로 바꾸고 true를 반환합니다. 다르면 현재 값을
`expected`에 써 주고 false를 반환합니다. 즉 `expected`는 입력이면서 출력입니다.
`weak`은 값이 같아도 spuriously fail할 수 있어 보통 loop에 적합하고, `strong`은 단발
시도에 편리합니다.

## 동시성 문서 체크리스트

- [ ] 공유되는 메모리 위치는 무엇인가?
- [ ] 모든 접근이 같은 mutex로 보호되거나 atomic인가?
- [ ] 객체 수명이 모든 스레드보다 긴가?
- [ ] 어떤 연산이 read-modify-write여야 하는가?
- [ ] 필요한 happens-before 관계는 무엇인가?
- [ ] 기본 `seq_cst`로 먼저 정확성을 확인했는가?
- [ ] lock-free 여부가 정확성 조건이 아니라 최적화 조건인가?
- [ ] false sharing, 정렬, 메모리 회수는 측정/설계했는가?

실습: [`08_atomic.cpp`](../examples/08_atomic.cpp)
