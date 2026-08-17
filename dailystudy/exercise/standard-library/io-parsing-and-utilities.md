# 입출력, 파싱, 이동, 수치 유틸리티

## 표준 스트림 `std::cin`, `std::cout`, `std::cerr` — `<iostream>`

`std::istream`은 문자 입력 스트림의 기반 타입이고 `std::ostream`은 문자 출력 스트림의 기반 타입이다. `std::cin`은 입력 인터페이스, `std::cout`과 `std::cerr`는 출력 인터페이스를 사용한다. 추출 연산자 `operator>>`는 보통 `std::istream&`, 삽입 연산자 `operator<<`는 보통 `std::ostream&`를 반환한다. 따라서 `std::cin >> a >> b`와 `std::cout << a << b`는 첫 호출이 돌려준 같은 스트림 참조에 다음 호출을 이어 붙인다. 반환값은 새 스트림이나 복사본이 아니며 스트림 객체의 수명은 호출 전후로 계속 유지된다.

- `cin`은 표준 입력에 연결된 `istream`, `cout`은 일반 표준 출력, `cerr`는 오류 표준 출력에 연결된 `ostream` 객체다.
- `operator>>`는 대상 lvalue 참조를 입력으로 받아 형식화 입력을 수행하고 `std::istream&`를 반환하므로 `cin >> a >> b`처럼 연쇄한다. 성공하면 대상 값을 바꾸고 입력 위치를 전진시키며, 스트림 객체 자체의 수명과 소유권은 유지된다.
- 입력 실패 시 fail 상태가 설정되고 대상 값은 추출 계약에 따라 유지되거나 바뀔 수 있다. `if (!(cin >> value))`로 검사한다.
- `operator<<`는 값을 형식화해 쓰고 스트림 참조를 반환한다. `endl`은 개행과 flush를 함께 하고 `\n`은 보통 개행만 해 더 저렴하다.
- `cerr`는 진단용이며 표준 출력 정답과 섞지 않는다. 버퍼링 정책만 믿기보다 필요한 시점에 명시적으로 flush한다.

### `std::ios::sync_with_stdio(false)`와 `cin.tie(nullptr)`

- `sync_with_stdio(false)`는 C stdio와 C++ iostream의 동기화를 끌 수 있고 이전 설정을 `bool`로 반환한다.
- 표준 입출력 전에 한 번 호출하며 이후 C와 C++ 스트림을 같은 파일에서 임의로 섞지 않는다.
- `cin.tie(nullptr)`는 입력 전 `cout` 자동 flush 연결을 해제하고 이전 연결 스트림 포인터를 반환한다.
- 대화형 문제에서는 프롬프트가 보이도록 수동 flush가 필요할 수 있다.

## 형식 조작자 `std::fixed`, `std::setprecision`

- `fixed`는 부동소수점 출력을 고정 소수점 표기로 바꾸는 조작자다.
- `setprecision(n)`은 `fixed` 상태에서는 소수점 뒤 자리 수, 기본 상태에서는 유효 숫자 수 의미가 된다.
- 스트림 형식 상태는 이후 출력에도 남으므로 라이브러리 함수가 전역 스트림 상태를 바꿀 때 주의한다.

## `std::from_chars`와 `std::errc` — `<charconv>`, `<system_error>`

- `from_chars(first,last,value,base)`는 문자 범위를 숫자로 파싱하고 `{ptr,ec}`를 반환한다.
- 로케일을 사용하지 않고 동적 할당을 요구하지 않으며 예외를 던지지 않는 저수준 변환 API다.
- `ec == std::errc{}`이면 변환 성공이다. `invalid_argument`는 시작부터 변환할 문자가 없고 `result_out_of_range`는 대상 타입 범위를 넘었다는 뜻이다.
- `ptr`은 변환을 멈춘 위치다. 전체 문자열이 숫자여야 하면 `ptr == last`도 검사한다.
- 문자열이 null 종료일 필요 없이 `[first,last)` 범위를 정확히 넘긴다.

## `std::to_string` — `<string>`

- 수치 값을 새 `std::string`으로 변환한다.
- 반환 문자열은 문자 메모리를 소유하며 할당이 일어날 수 있다.
- 형식과 로케일 세부 제어가 필요하면 `std::format`, 스트림 또는 `to_chars`를 검토한다.

## `std::move`, `std::forward`, `std::exchange` — `<utility>`

### `std::move`

- 실제 이동을 수행하는 함수가 아니라 인자를 xvalue로 캐스팅하는 함수 템플릿이다.
- 이후 선택된 이동 생성자·이동 대입이 자원을 옮길 수 있다. 타입에 이동 연산이 없으면 복사가 선택될 수도 있다.
- 이동된 표준 라이브러리 객체는 유효하지만 값은 보통 미지정 상태다. 파괴·대입처럼 계약이 허용한 연산만 한다.
- `const` 객체에 `move`를 적용하면 `const T&&`가 되어 일반적인 `T&&` 이동 생성자와 맞지 않아 복사될 수 있다.

### `std::forward<T>`

- 전달 참조로 받은 식의 원래 lvalue/rvalue 범주를 복원하는 조건부 캐스트다.
- 템플릿 인자 `T`를 추론된 그대로 사용해야 한다. 일반 코드에서 무조건 `forward`하면 수명과 다중 사용 문제가 생긴다.
- 한 인자를 여러 번 forward하면 첫 호출에서 자원이 이동된 뒤 다시 사용할 수 있어 주의한다.

### `std::exchange`

- `old = std::move(object)`에 해당하는 옛 값을 반환하고 `object = new_value`로 바꾼다.
- 핸들 이동, 상태 전이, 카운터 교체를 한 식으로 표현한다.

## 수치 도구

### `std::numeric_limits<T>` — `<limits>`

- 타입의 최솟값·최댓값·무한대 지원 등 구현 속성을 컴파일 시간 상수로 제공한다.
- `max()`는 가장 큰 유한 값이다. 최단거리 INF로 그대로 쓰고 가중치를 더하면 오버플로할 수 있어 여유 있게 나누거나 덧셈 전에 검사한다.
- 부동소수점 `min()`은 가장 작은 양의 정규값이고 가장 낮은 음수는 `lowest()`다.

### `std::min`, `std::max`, `std::abs`

- `min/max`는 비교해 선택한 값/참조를 반환한다. 서로 다른 타입을 무심코 섞으면 템플릿 추론이 실패할 수 있다.
- 정수 `abs`는 가장 작은 음수를 양수로 표현할 수 없는 경우 오버플로 문제가 있다. 입력 범위를 먼저 확인한다.
- 올바른 오버로드 헤더는 타입에 따라 `<cstdlib>` 또는 `<cmath>`다.

### 고정 폭 정수와 크기 타입

- `std::uint64_t`는 구현이 정확히 64비트 부호 없는 정수 타입을 제공할 때 존재한다.
- `std::size_t`는 객체 크기와 컨테이너 인덱스에 쓰는 부호 없는 타입이다. 음수 인덱스와 비교하면 암시 변환에 주의한다.
- `std::ptrdiff_t`는 같은 배열 안 포인터·반복자 차이를 나타내는 부호 있는 타입이다.
- `std::uintmax_t`는 구현이 제공하는 가장 넓은 부호 없는 정수 타입이며 파일 크기 API에서 쓰인다.
- `std::byte`는 정수 산술보다 원시 바이트 의미를 나타내는 enum class다.

## 상수와 비교

- `std::numbers::pi_v<T>`는 `<numbers>`의 타입별 원주율 상수 템플릿이다. `pi_v<double>`처럼 타입을 명시한다.
- `std::strong_ordering`은 [`ownership-and-vocabulary-types.md`](ownership-and-vocabulary-types.md)를 참고한다.

## 종료 함수 `std::exit`, `std::terminate`

- `std::exit(code)`는 정상 프로그램 종료 절차 일부를 수행하지만 현재 스택의 자동 객체 소멸자는 실행하지 않는다. RAII 지역 자원이 남을 수 있어 일반 흐름에서는 `return`을 선호한다.
- `std::terminate()`는 복구 없이 종료 처리기로 전달한다. `noexcept` 위반, 처리되지 않은 스레드 예외 등에서도 호출될 수 있다.
- 둘 다 `[[noreturn]]` 성격이므로 뒤 코드가 실행된다고 가정하지 않는다.

## 최소 파싱 예제

```cpp
#include <charconv>
#include <string_view>
#include <system_error>

int main() {
    constexpr std::string_view text{"42"};
    int value{};
    const auto result{std::from_chars(text.data(), text.data() + text.size(), value)};
    const bool complete{result.ec == std::errc{} && result.ptr == text.data() + text.size()};
    return complete && value == 42 ? 0 : 1;
}
```

## 직접 검증

1. `from_chars("12x")`에서 성공 코드와 반환 포인터를 모두 검사해야 하는 이유를 설명한다.
2. `std::move(const_string)`이 보통 문자열 버퍼를 이동하지 못하는 이유를 생성자 매개변수 타입으로 말한다.
3. `numeric_limits<long long>::max()`에 양의 가중치를 더하는 다익스트라 코드의 문제를 설명한다.
4. `exit`와 `return`이 현재 함수의 자동 RAII 객체 소멸에 미치는 차이를 비교한다.
