# 09. C++20 문서 읽기 가이드

원문: [cppreference C++20](https://cppreference.com/cpp/20)

## 큰 그림

C++20의 중심은 템플릿 요구 조건을 언어로 표현하는 concepts, 범위 조합을 바꾸는 ranges,
비동기 함수의 기반인 coroutines, 번역 단위 모델을 확장하는 modules입니다. 여기에
`span`, formatting, calendar/time zone, `jthread`, 동기화 primitive가 추가됩니다.

기능별 컴파일러·표준 라이브러리 지원 시점이 다르므로 C++20 플래그만 켰다고 모든 기능이
있다고 가정하지 말고 C++20 페이지의 compiler support와 feature-test macro를 확인합니다.

## 추천 읽기 순서

### 1. 바로 이해할 수 있는 언어 기능

#### designated initializer

```cpp
struct Point { int x; int y; };
Point p{.x = 1, .y = 2};
```

C++에서는 aggregate에 선언 순서대로 지정해야 하는 등 C와 규칙이 다릅니다.

#### range-for initializer

```cpp
for (auto values = make_values(); int x : values) {
    // values가 loop 동안 살아 있음
}
```

세미콜론 앞 initializer로 원본의 수명을 loop에 묶을 수 있습니다.

#### defaulted comparison과 `<=>`

```cpp
struct Version {
    int major;
    int minor;
    auto operator<=>(const Version&) const = default;
};
```

three-way comparison은 ordering category를 반환합니다. 모든 타입이 strong ordering인 것은
아니며 부동소수점에는 unordered가 가능한 partial ordering이 관련됩니다.

#### 새 attribute

- `[[likely]]`, `[[unlikely]]`: branch 가능성에 대한 최적화 힌트
- `[[no_unique_address]]`: 빈 멤버 등의 주소 공간 중첩을 허용

정확성을 바꾸는 명령이 아니며 측정 없이 남용하지 않습니다.

#### `consteval`, `constinit`, 확장된 `constexpr`

- `constexpr`: 상수 평가가 가능할 수 있음
- `consteval`: 호출이 반드시 상수 평가되어야 함
- `constinit`: static/thread 객체의 정적 초기화를 강제하며 const를 뜻하지 않음

#### `char8_t`

UTF-8 코드 단위를 위한 별도 타입입니다. C++17의 `u8"..."` 타입과 C++20의 타입이 달라
기존 `const char*` API와 호환 문제가 생길 수 있습니다.

### 2. concepts

```cpp
template<std::integral T>
T add(T a, T b) { return a + b; }

auto twice(std::integral auto value) {
    return value + value;
}
```

같은 제약은 `requires std::integral<T>`, requires-clause, requires-expression 등 여러
형태로 나타납니다. concept은 올바른 연산 형태와 의미론적 요구를 이름 붙입니다.
단순히 template 코드를 짧게 쓰는 기능이 아니라 overload 후보와 오류 메시지를 개선합니다.

읽을 것:

- constraint normalization과 satisfaction
- subsumption과 constrained overload 순서
- `same_as`, `convertible_to`, `derived_from`
- `integral`, `floating_point`
- `assignable_from`, `constructible_from`, `regular`
- `invocable`, `predicate`, `relation`
- iterator/range concepts

### 3. ranges

```cpp
auto result = values
            | std::views::filter([](int x) { return x % 2 == 0; })
            | std::views::transform([](int x) { return x * x; });
```

- range: `begin`/`end`로 순회할 수 있는 것
- view: 보통 비소유·lazy range adapter
- sentinel: 끝 표시가 iterator와 같은 타입일 필요 없음
- projection: 비교 전에 요소에서 key를 뽑는 호출 가능 객체
- borrowed range: 임시 range가 사라져도 iterator가 유효할 수 있는지 나타냄
- dangling: 수명 안전을 위해 iterator 대신 반환될 수 있는 표시 타입

view pipeline은 결과 컨테이너를 즉시 만들지 않습니다. 원본 수명, 반복 중 변경, 한 번만
순회 가능한 input range인지 확인하세요.

### 4. 유용한 라이브러리

#### `std::span<T>`

연속된 T 구간을 보는 비소유 view입니다. pointer+size 인자를 하나로 묶지만 원본 수명을
연장하지 않습니다. compile-time extent와 dynamic extent가 있습니다.

#### `<format>`

타입 안전한 포맷 문자열 기반 문자열 생성을 제공합니다. 구현 지원과 compile-time format
검사를 확인합니다.

#### `<bit>`

`std::bit_cast`, `std::endian`, `popcount`, `countl_zero`, `bit_width`, power-of-two 연산 등이
있습니다. `bit_cast`는 크기와 trivially copyable 요구 조건을 확인하며 type punning 만능
도구가 아닙니다.

#### `<numbers>`

`std::numbers::pi_v<T>` 등 수학 상수를 제공합니다.

#### `source_location`

호출 지점의 파일/줄/함수 정보를 기본 인자로 캡처해 logging에 활용합니다. 매크로를 줄일 수
있습니다.

#### 문자열과 constexpr

`starts_with`/`ends_with`, 많은 algorithm/utility의 constexpr 확대, `std::ssize`,
`std::midpoint`, `std::lerp`, `std::to_address`, `std::remove_cvref`가 추가됩니다.

### 5. 동시성

- `std::jthread`: 소멸 시 join하며 cooperative stop과 통합
- `stop_token`, `stop_source`, `stop_callback`: 취소 요청 전달
- `atomic::wait/notify_*`: 값 변화 대기/통지
- `std::counting_semaphore`: permit 개수
- `std::latch`: 한 번만 0까지 세는 barrier
- `std::barrier`: 여러 phase에 재사용 가능한 rendezvous
- `std::basic_osyncstream`: 여러 스레드 출력 조각을 섞이지 않게 방출
- floating-point atomic, `shared_ptr` atomic specialization

취소는 강제 thread 종료가 아니라 협력적 요청입니다. blocking 작업이 stop을 관찰하는지
확인합니다.

### 6. coroutines

`co_await`, `co_yield`, `co_return`을 포함한 함수가 coroutine이 될 수 있습니다. C++20은
언어 메커니즘을 제공하지만 범용 `task`/`generator` 타입은 직접 또는 라이브러리로
제공해야 합니다.

필수 선행 개념:

- coroutine frame과 수명
- promise type
- awaitable/awaiter의 `await_ready`, `await_suspend`, `await_resume`
- initial/final suspend
- exception과 cancellation

함수가 어느 thread에서 재개되는지는 `co_await` 자체가 자동 결정하지 않습니다.

### 7. modules

`export module`, `import`로 인터페이스와 구현을 구성합니다. 텍스트 include를 단순 치환하는
기능이 아니며 build system과 compiler별 BMI/IFC 관리가 필요합니다. 먼저 일반 번역 단위,
ODR, linkage를 이해하고 도구 체인의 공식 module 지원 방법을 따르세요.

## 새 언어 기능 전체 점검표

- [ ] feature-test macro
- [ ] `<=>`와 defaulted `operator==`
- [ ] designated initializer
- [ ] range-for init-statement/initializer
- [ ] `char8_t`
- [ ] `[[no_unique_address]]`, `[[likely]]`, `[[unlikely]]`
- [ ] lambda init-capture의 pack expansion
- [ ] 일부 문맥에서 `typename` 생략
- [ ] `consteval`, `constinit`, relaxed `constexpr`
- [ ] signed integer가 2's complement
- [ ] bitwise shift 규칙 통일
- [ ] 괄호를 이용한 aggregate initialization
- [ ] coroutines
- [ ] modules
- [ ] constraints/concepts, abbreviated function template
- [ ] array new의 배열 크기 추론(DR)

## 새 헤더 전체

일반:

`<bit>`, `<compare>`, `<concepts>`, `<coroutine>`, `<format>`, `<numbers>`,
`<ranges>`, `<source_location>`, `<span>`, `<syncstream>`, `<version>`.

동시성:

`<barrier>`, `<latch>`, `<semaphore>`, `<stop_token>`.

## 라이브러리 기능 추가 점검표

- [ ] concepts/ranges/formatting library
- [ ] chrono calendar와 time zone
- [ ] `source_location`, `span`
- [ ] endian/bit operations/`bit_cast`
- [ ] `make_shared` array, overwrite factory
- [ ] `remove_cvref`, `to_address`, `assume_aligned`
- [ ] floating atomic, shared_ptr atomic
- [ ] barrier/latch/semaphore/jthread/stop
- [ ] syncstream
- [ ] `char8_t` 연계
- [ ] 더 많은 constexpr library
- [ ] `starts_with`/`ends_with`
- [ ] `bind_front`, allocator 보조 함수
- [ ] heterogeneous unordered lookup
- [ ] `execution::unseq`
- [ ] `midpoint`, `lerp`, `ssize`
- [ ] bounded/unbounded array traits
- [ ] uniform `erase`/`erase_if`
- [ ] `<numbers>` 상수

실습: [`09_cpp20.cpp`](../examples/09_cpp20.cpp)
