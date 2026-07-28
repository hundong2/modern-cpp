# 01. cppreference 페이지 읽는 법

## 페이지의 정체

cppreference는 C++ 표준의 내용을 검색하기 좋게 정리한 참고서입니다. 표준의 모든 문장을
순서대로 가르치는 강의가 아닙니다. 개별 API 페이지는 대개 아래 구조입니다.

1. 이름과 소속: `std::atomic<T>::is_lock_free`
2. 필요한 헤더와 표준 버전
3. 하나 이상의 선언(overload)
4. 한 줄 설명
5. Parameters
6. Return value
7. Complexity
8. Exceptions
9. Notes
10. Example / See also / Defect reports

섹션이 없으면 “중요하지 않다”가 아니라, 상수 시간이 당연하거나 예외 규칙이 상위 페이지에
있는 등 여러 이유가 있을 수 있습니다.

## 색·괄호·표의 의미

- `(since C++17)`: C++17부터 표준 기능입니다.
- `(until C++20)`: C++20부터는 그 형태가 더 이상 해당하지 않습니다.
- `(deprecated in C++17)`: 사용 중단을 권고하지만 아직 존재합니다.
- `(removed in C++20)`: C++20 표준에서는 제거되었습니다.
- `(constexpr since C++20)`: 같은 API가 C++20부터 상수 평가에 쓰일 수 있습니다.
- `(1)`, `(2)`: 서로 다른 overload 번호입니다. 실행 순서가 아닙니다.
- `T`, `U`, `Allocator`: 문맥에서 정해지는 타입 매개변수입니다.
- `/* ... */`: 설명용 자리표시자이거나 exposition-only 이름일 수 있습니다.
- `LegacyForwardIterator`, `ranges::input_range`: 이름 있는 요구 조건 또는 concept입니다.
- `UB`: undefined behavior. [타입·값·수명](04-types-values-lifetime.md#정의되지-않은-동작-ub) 참고.
- `DR`, `CWG`, `LWG`: 나중에 발견한 표준 결함과 수정 기록입니다.
- `PxxxxRn`, `Nxxxx`: 표준화 제안서/문서 번호입니다.
- `feature-test macro`: 구현 지원 여부를 전처리 때 검사하는 매크로입니다.

## 선언 표를 읽는 순서

다음 선언을 봅시다.

```cpp
bool is_lock_free() const volatile noexcept;
```

한꺼번에 이해하려 하지 말고 다섯 칸으로 나눕니다.

| 부분 | 질문 | 답 |
|---|---|---|
| `bool` | 무엇을 반환하는가? | 참/거짓 값 |
| `is_lock_free` | 이름은? | 멤버 함수 이름 |
| `()` | 입력은? | 명시적 매개변수 없음 |
| `const volatile` | 어떤 `*this`에서 호출 가능한가? | const이면서 volatile인 객체 |
| `noexcept` | 예외 명세는? | 예외를 밖으로 던지지 않음 |

`const volatile`은 반환값이나 함수 자체가 아니라 암시적 객체 매개변수인 `*this`를
한정합니다. 자세한 문법은 [선언 해독](03-declarations-and-signatures.md), 의미는
[동시성 문서](07-concurrency-volatile-atomic.md)로 연결됩니다.

## 이름을 읽는 법

```text
std::atomic<T>::is_lock_free
│    │       │
│    │       └─ 클래스의 멤버 이름
│    └───────── T를 받는 클래스 템플릿
└────────────── 표준 라이브러리 namespace
```

- `::`는 scope resolution operator입니다.
- `<T>`는 “작다” 비교가 아니라 템플릿 인자 목록입니다.
- `std::atomic<int>`는 `T`가 `int`인 구체 타입입니다.
- `std::atomic<int>{}`는 그 타입의 임시 객체를 값 초기화하는 표현식입니다.

## “Effects”와 “Equivalent to” 읽기

- **Effects**: 호출이 관찰 가능한 상태에 무엇을 하는지 설명합니다.
- **Equivalent to**: 제시한 코드와 같은 의미/요구를 갖는다는 뜻입니다. 실제 구현을
  그대로 공개한다는 뜻은 아닙니다.
- **Returns**: 결과값의 의미입니다.
- **Throws**: 던질 수 있는 예외입니다.
- **Complexity**: 입력 크기에 따른 작업량입니다.
- **Preconditions**: 호출 전에 사용자가 만족해야 하는 조건입니다.
- **Postconditions**: 정상 완료 후 보장되는 조건입니다.
- **Remarks/Notes**: overload 참여 조건, 미묘한 규칙, 구현상 주의점을 포함할 수 있습니다.

## 표준 버전 선택

C++17 코드를 읽을 때는 페이지의 현재 선언만 보지 말고 `(since ...)`, `(until ...)`,
버전 선택기를 함께 봅니다. 최신 페이지의 예제가 C++17에서 컴파일된다고 가정하면 안 됩니다.

```cpp
#if __cplusplus >= 202002L
// C++20 이상에서만 사용하는 코드
#endif
```

MSVC는 설정에 따라 `__cplusplus` 값 갱신에 `/Zc:__cplusplus`가 필요할 수 있습니다.
라이브러리 기능은 `__cpp_lib_optional` 같은
[기능 테스트 매크로](10-glossary.md#기능-테스트-매크로)를 우선 확인합니다.

## Example을 읽는 5단계

1. `#include`마다 어떤 이름을 제공하는지 표시합니다.
2. 타입과 객체를 동그라미, 함수 호출을 네모라고 생각하고 구분합니다.
3. 각 객체의 소유권과 수명 종료 지점을 표시합니다.
4. 출력 결과를 먼저 손으로 씁니다.
5. 타입 하나, 인자 하나, 한정자 하나만 바꿔 다시 컴파일합니다.

예제 출력은 가능한 결과일 수 있습니다. 주소, 스레드 순서, 난수, lock-free 여부처럼
구현/실행마다 달라지는 값은 정답 하나가 아닙니다.

## 검색 순서

어떤 API가 막히면 아래 순서로 좁힙니다.

1. 페이지 제목의 소속 타입(예: `std::atomic`)을 먼저 읽습니다.
2. 선언의 낯선 토큰을 [색인](10-glossary.md)에서 찾습니다.
3. Parameters와 Return value를 읽습니다.
4. Requirements/Notes에서 수명·무효화·동시성 조건을 찾습니다.
5. See also로 비슷한 API의 차이를 확인합니다.
6. 버전 페이지와 Paper는 “왜 추가됐는가”가 필요할 때 읽습니다.
