# 입출력, 파싱, 이동, 수치 유틸리티

## 타입 안전 문자열 형식화 `std::format` — `<format>`

`std::format`은 C++20의 자리표시자 기반 문자열 형식화 함수 템플릿이다. 대표 형태는 `template<class... Args> std::string format(std::format_string<Args...> fmt, Args&&... args);`다. 일반 문자열 연결보다 값 타입과 형식 지정의 대응을 분명히 하고, `std::format_string` 오버로드는 컴파일 시간에 형식 문자열을 검사한다.

- 첫 인자 `fmt`는 자리표시자와 형식 지정자를 담는다. 문자열 리터럴을 쓰는 기본 오버로드에서는 인자 타입과 자리표시자 수·종류가 맞지 않으면 컴파일 진단을 받을 수 있다.
- 뒤의 `args...`는 전달 참조 형태지만 “항상 이동한다”는 뜻이 아니다. lvalue는 lvalue로 읽히고 rvalue는 해당 값 범주로 전달되며, 형식화 과정이 일반적으로 호출자 인자의 소유권을 가져가지 않는다. 사용자 정의 formatter는 참조를 호출 뒤 저장하지 않아야 한다.
- 반환형은 문자를 독립 소유하는 `std::string`이다. 호출자는 원본 인자 수명과 별개로 결과를 보관할 수 있고, 원본 인자의 값과 소유권은 정상 호출 뒤 유지된다.
- 시간과 추가 공간은 대체로 결과 문자 수와 각 인자의 형식화 비용에 비례한다. 결과 버퍼 성장에서 동적 할당과 재할당이 일어날 수 있으며, 다른 객체의 포인터·참조·반복자를 무효화하지 않는다.
- 할당 실패는 `std::bad_alloc`, 런타임 형식 오류는 `std::format_error`로 표현될 수 있다. 컴파일 시간 형식 문자열은 많은 오류를 더 일찍 막지만 사용자 formatter의 실행 중 오류까지 모두 제거하지는 않는다.
- 기본 API는 새 문자열을 반환한다. 기존 버퍼·출력 반복자에 이어 쓰려면 `std::format_to`, 필요한 길이만 계산하려면 `std::formatted_size`를 검토하되 각 출력 반복자의 무효화·공간 계약을 별도로 확인한다.
- 표준 `format` 호출 자체가 여러 스레드가 공유하는 사용자 객체를 자동 동기화하지는 않는다. 불변 인자를 동시에 읽는 것은 각 타입 계약에 따르고, 사용자 formatter나 allocator가 공유 상태를 바꾸면 별도 동기화가 필요하다.

오늘 자료 [`../2026-08-31/main.cpp`](../2026-08-31/main.cpp)는 formatter가 입력 DTO를 `const` 참조로 읽고 결과 `string`만 반환하게 해 표현 정책과 출력 sink를 분리한다. 코드 인접 주석에서 다섯 인자의 값 범주·소유권과 결과 저장 위치를 확인한다.

## 표준 스트림 `std::cin`, `std::cout`, `std::cerr` — `<iostream>`

`std::istream`은 문자 입력 스트림의 기반 타입이고 `std::ostream`은 문자 출력 스트림의 기반 타입이다. `std::cin`은 입력 인터페이스, `std::cout`과 `std::cerr`는 출력 인터페이스를 사용한다. 추출 연산자 `operator>>`는 보통 `std::istream&`, 삽입 연산자 `operator<<`는 보통 `std::ostream&`를 반환한다. 따라서 `std::cin >> a >> b`와 `std::cout << a << b`는 첫 호출이 돌려준 같은 스트림 참조에 다음 호출을 이어 붙인다. 반환값은 새 스트림이나 복사본이 아니며 스트림 객체의 수명은 호출 전후로 계속 유지된다.

- `cin`은 표준 입력에 연결된 `istream`, `cout`은 일반 표준 출력, `cerr`는 오류 표준 출력에 연결된 `ostream` 객체다.
- `operator>>`는 대상 lvalue 참조를 입력으로 받아 형식화 입력을 수행하고 `std::istream&`를 반환하므로 `cin >> a >> b`처럼 연쇄한다. 성공하면 대상 값을 바꾸고 입력 위치를 전진시키며, 스트림 객체 자체의 수명과 소유권은 유지된다.
- 입력 실패 시 fail 상태가 설정되고 대상 값은 추출 계약에 따라 유지되거나 바뀔 수 있다. `if (!(cin >> value))`로 검사한다.
- `operator<<`는 값을 형식화해 쓰고 스트림 참조를 반환한다. `endl`은 개행과 flush를 함께 하고 `\n`은 보통 개행만 해 더 저렴하다.
- `cerr`는 진단용이며 표준 출력 정답과 섞지 않는다. 버퍼링 정책만 믿기보다 필요한 시점에 명시적으로 flush한다.

### 동시 레코드 출력 `std::osyncstream` — `<syncstream>`

`std::osyncstream`은 `std::basic_osyncstream<char>`의 별칭이다. 여러 스레드가 같은 최종 `streambuf`에 여러 조각을 쓰더라도 각 `osyncstream`이 모은 문자 덩어리가 다른 동기 스트림의 문자와 섞이지 않게 전달하는 C++20 출력 스트림이다. 레코드의 **내용 원자성**을 만들지만 스레드 사이 레코드 **순서**까지 고정하지는 않는다.

- 대표 생성 형태는 `explicit basic_osyncstream(ostream_type& wrapped)`다. 인자는 `std::ostream` lvalue 참조이며 스트림 객체나 그 버퍼를 소유권 이전하지 않는다. 생성된 `osyncstream`과 그 출력 작업이 끝날 때까지 wrapped와 wrapped의 stream buffer가 살아 있어야 한다.
- 생성자는 wrapped의 현재 stream buffer를 감싼 `basic_syncbuf`를 소유한다. 메모리 버퍼 할당이 실패하면 예외가 날 수 있고, 생성 성공 뒤 wrapped의 문자 시퀀스가 즉시 바뀐다고 가정하면 안 된다.
- `operator<<(value)`는 일반 `ostream`처럼 값을 형식화해 동기 버퍼에 추가하고 스트림 참조를 반환한다. 시간과 임시 공간은 기록 문자 수에 선형이며 버퍼 확장에서 할당이 일어날 수 있다.
- `emit()`의 대표 형태는 `void emit()`이다. 데이터 값 인자는 없고 반환값도 없다. 호출 전 동기 버퍼에 쌓인 문자를 wrapped stream buffer로 전달하며, 전달 중 오류는 스트림 상태나 예외 설정에 따라 표현될 수 있다.
- 소멸자는 남은 문자를 emit하는 RAII 경계다. 소멸자에서 예외가 전파된다고 기대해 오류 처리를 설계하지 말고, 엄격한 오류 관찰이 필요하면 명시적 `emit()`과 스트림 상태를 검사한다.
- 같은 최종 stream buffer에 접근하는 다른 코드도 `osyncstream`을 사용해야 레코드 비혼합 보장을 얻는다. 일반 `ostream` 직접 출력과 섞거나 wrapped 스트림을 동시에 읽고 쓰는 작업까지 자동으로 안전하게 만들지는 않는다.
- `osyncstream` 객체 자체를 여러 스레드가 동시에 공유해 무동기 접근하는 방식이 아니라, 각 스레드가 별도 지역 객체를 만들고 같은 wrapped buffer로 emit하는 방식이 기본이다.
- 객체는 내부 동기 버퍼를 소유하므로 복사할 수 없고 이동은 가능하다. 이동 뒤 원본은 유효하지만 자원 상태를 가정하지 않는다.

오늘 자료 [`../2026-08-28/main.cpp`](../2026-08-28/main.cpp)는 지역 `record` 수명을 로그 한 줄의 commit 경계로 쓴다. `jthread::join()` 뒤에만 wrapped `ostringstream`를 읽어 emit 완료와 단일 스레드 접근을 보장한다.

### 메모리 출력 `std::ostringstream`와 `str()` — `<sstream>`

- `std::ostringstream`는 `std::basic_ostringstream<char>`의 별칭이며, 문자 시퀀스를 내부 `std::string` 기반 버퍼에 소유하는 출력 스트림이다. 테스트 더블, 문자열 조립, 포맷 결과 캡처에 쓰지만 반복 연결이 매우 많은 성능 경로에서는 할당 비용을 측정한다.
- 기본 생성 `ostringstream()`는 빈 버퍼와 정상 스트림 상태를 만든다. 생성자는 반환값이 없고 버퍼 준비 중 할당 실패가 예외가 될 수 있다.
- `str() const &`는 현재 문자 시퀀스를 새 `std::string` 값으로 복사 반환한다. 데이터 값 인자는 없고 반환 문자열이 자기 버퍼를 소유하므로 원본 stream보다 오래 살 수 있다. 호출 뒤 stream 내용·위치·상태는 유지되며 문자 수에 선형 시간·공간이 들고 할당 실패 가능성이 있다.
- `str(string)` 계열 setter는 내부 시퀀스를 교체하므로 기존에 얻은 포인터·참조·뷰를 그대로 사용할 수 없다. 사용 중인 정확한 오버로드가 getter인지 setter인지 인자 수와 cv/ref 한정으로 구분한다.
- `ostringstream`에 여러 스레드가 직접 동시에 쓰는 것은 일반적으로 안전한 공유 접근 계약이 아니다. 오늘처럼 각 스레드가 별도 `osyncstream`으로 같은 wrapped buffer에 emit하고, 모든 join 뒤 한 스레드가 `str()`을 호출한다.

### `std::ios::sync_with_stdio(false)`와 `cin.tie(nullptr)`

- `sync_with_stdio(false)`는 C stdio와 C++ iostream의 동기화를 끌 수 있고 이전 설정을 `bool`로 반환한다.
- 표준 입출력 전에 한 번 호출하며 이후 C와 C++ 스트림을 같은 파일에서 임의로 섞지 않는다.
- `cin.tie(nullptr)`는 입력 전 `cout` 자동 flush 연결을 해제하고 이전 연결 스트림 포인터를 반환한다.
- 대화형 문제에서는 프롬프트가 보이도록 수동 flush가 필요할 수 있다.

### `std::getline(stream, string, delimiter)` — `<string>`, `<istream>`

- 대표 형태는 `template<class CharT, class Traits, class Allocator> basic_istream<CharT, Traits>& getline(basic_istream<CharT, Traits>& input, basic_string<CharT, Traits, Allocator>& output, CharT delimiter);`이며 구분자를 생략하면 개행을 쓴다.
- 첫 인자는 읽을 스트림 lvalue 참조, 둘째는 내용을 교체할 소유 문자열 lvalue 참조, 셋째는 값으로 받는 종료 문자다. 스트림이나 문자열의 소유권을 가져가지 않는다.
- 호출은 기존 output 내용을 지우고 구분자 전 문자를 저장한 뒤 구분자는 소비하지만 저장하지 않는다. 같은 입력 스트림 참조를 반환하므로 성공 여부를 bool 문맥에서 검사하거나 연쇄할 수 있다.
- 출력 문자열이 커지며 재할당되면 그 문자열의 기존 포인터·참조·반복자가 무효화된다. 다른 문자열은 바뀌지 않는다.
- 시간은 추출한 문자 수에 선형이고 문자열 저장 공간도 선형이다. 할당 실패는 예외, 입력 종료·실패는 스트림 상태로 표현된다.
- 앞서 `operator>>`를 사용했다면 남아 있는 개행을 첫 빈 줄로 읽을 수 있다. 입력 계약에 맞게 개행을 소비할지 결정한다.

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
