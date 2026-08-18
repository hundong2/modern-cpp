# 비트, 바이트, 객체 표현 유틸리티

이 문서는 실질적으로 같은 표준 기능을 날짜별로 반복하지 않고 `<bit>`, `<cstdint>`, `<cstddef>`의 저수준 표현 도구를 한곳에서 설명한다. 객체 표현은 전송 프로토콜 자체가 아니므로 필드 폭·순서·버전·유효성 검사는 별도 경계가 책임져야 한다.

## `std::bit_cast<To>(from)` — `<bit>` 함수 템플릿

- 대표 형태: `template<class To, class From> constexpr To bit_cast(const From& from) noexcept;`
- `To`는 호출부가 명시하는 목적 타입이고 `From`은 인자에서 추론된다.
- `sizeof(To) == sizeof(From)`이고 두 타입이 trivially copyable이어야 한다. 조건을 만족하지 않으면 호출 후보가 되지 않는다.
- `from`은 `const From&`로 읽을 뿐 수정하거나 소유하지 않는다. 반환형은 새 `To` 값이며 원본과 독립적으로 수명을 가진다.
- 같은 크기의 객체 표현 비트를 복사한다. 엔디언 변환, 구조체 패딩 제거, 직렬화 버전 관리는 하지 않는다.
- 올바른 별칭 규칙을 깨는 포인터 `reinterpret_cast` 역참조 대신 값 복사에 쓸 수 있다.
- 고정 크기 복사라 시간·추가 공간은 `O(sizeof(To))`이고 오늘의 4바이트 타입에서는 상수다. 동적 할당과 표준 예외는 없다.
- 대상 타입에서 값이 없는 비트 표현이나 수명 밖의 하위 객체를 읽는 상황은 별도 규칙을 확인해야 한다. 임의 바이트를 어떤 타입으로든 바꿔도 안전하다는 허가가 아니다.

## `std::byteswap(value)` — `<bit>` 함수 템플릿(C++23)

- 대표 형태: `template<class T> constexpr T byteswap(T value) noexcept;`
- `T`는 padding bit가 없는 정수 타입이어야 한다. `bool`, 부동소수점, 포인터, 임의 구조체에 쓰는 도구가 아니다.
- 값 매개변수를 복사해 바이트 순서를 반대로 배치한 같은 타입 prvalue를 반환한다. 호출자의 인자는 변하지 않는다.
- 동적 할당과 예외가 없고 고정 폭 정수에서는 `O(sizeof(T))`, 오늘의 32비트 값에서는 상수 시간·공간이다.
- 실제로 단일 바이트 교환 명령, 시프트 조합, 컴파일 시간 계산이 될지는 CPU와 최적화에 달려 있다.

## `std::endian` — `<bit>` 열거형

- `std::endian::little`, `std::endian::big`은 알려진 작은/큰 바이트 순서를 나타낸다.
- `std::endian::native`는 실행 구현의 스칼라 바이트 순서를 나타내는 컴파일 시간 값이다.
- native가 little 또는 big과 같지 않은 혼합 엔디언 구현도 표준상 가능하다. 지원하지 않으면 오늘 코드처럼 `static_assert`로 명시적으로 거부한다.
- 열거자 비교는 값을 읽을 뿐 전역 상태나 객체를 바꾸지 않고 할당·예외가 없다.
- 네트워크 바이트 순서는 관례상 big-endian이지만 파일/프로토콜 명세가 정한 순서를 항상 우선한다.

## `std::uint32_t`, `std::byte`, `std::size_t`

### `std::uint32_t` — `<cstdint>` 타입 별칭

- 구현이 정확히 32비트인 부호 없는 정수 타입을 제공할 때 존재한다.
- 모듈 간 메모리 ABI를 무조건 보장하는 것이 아니라 수치 폭을 표현한다. 직렬화에는 바이트 순서와 필드 배치 계약이 추가로 필요하다.
- 부호 없는 산술은 2의 비트 수 제곱을 법으로 순환한다. 범위 검증 없이 오버플로를 오류 탐지로 사용하지 않는다.

### `std::byte` — `<cstddef>` enum class

- 원시 저장 단위를 정수 값과 구분한다. 암시적으로 정수로 변환되지 않아 바이트에 우연히 산술하는 일을 줄인다.
- 비트 연산은 지원하지만 숫자로 읽으려면 `std::to_integer` 같은 명시적 변환이 필요하다.
- `array<byte, N>`은 바이트를 값으로 소유한다. 컨테이너 수명과 함께 원소 수명도 끝난다.

### `std::size_t` — `<cstddef>` 타입 별칭

- 객체 크기와 표준 컨테이너 크기·인덱스에 쓰는 부호 없는 타입이다.
- 음수 `int`를 암시 변환하면 매우 큰 값이 될 수 있으므로 범위 검사와 변환 위치를 명시한다.

## 와이어 경계의 전제조건과 후조건

- 호출 전: 필드 폭과 허용 범위, 프로토콜 바이트 순서, 버전을 알아야 한다.
- 호출 후: 반환 바이트 값은 원본 객체와 독립적이어야 하며 원본 소유권과 수명은 유지된다.
- 고정 `array`는 재할당하지 않으므로 원소 포인터·참조는 배열 수명 동안 유지되지만, 범위 밖 `operator[]`는 미정의 동작이다.
- 저수준 함수 자체는 오류 코드를 만들지 않는다. 잘못된 길이·버전·필드 값은 코덱 계층이 `expected`, 오류 코드 등으로 표현해야 한다.
- 공유 가변 객체를 동시에 읽고 쓰는 스레드 안전은 제공하지 않는다. 독립된 값 객체끼리의 변환만 데이터 경쟁이 없다.

## 최소 실행 예제

```cpp
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>

int main() {
    static_assert(std::endian::native == std::endian::little ||
                  std::endian::native == std::endian::big);
    const std::uint32_t host{0x01020304U};
    const std::uint32_t network{
        std::endian::native == std::endian::little ? std::byteswap(host) : host};
    const auto bytes{std::bit_cast<std::array<std::byte, 4>>(network)};
    const auto copied{std::bit_cast<std::uint32_t>(bytes)};
    const std::uint32_t restored{
        std::endian::native == std::endian::little ? std::byteswap(copied) : copied};
    return restored == host ? 0 : 1;
}
```

## 흔한 실수와 직접 검증

1. `bit_cast`가 엔디언까지 바꾼다고 오해하지 않는다.
2. 패딩이 있을 수 있는 구조체 전체를 안정된 파일 형식이라고 가정하지 않는다.
3. `reinterpret_cast<T*>(bytes.data())` 역참조의 정렬·별칭·수명 문제를 설명한다.
4. 0x01020304를 little/big-endian 메모리에 놓았을 때 주소 증가 순서의 바이트를 적는다.
5. 16비트와 64비트 필드용 코드를 만들고 왕복 값과 정확한 출력 바이트를 각각 검증한다.
