# 02. 핵심 문법

## 프로그램이 만들어지는 큰 흐름

1. 전처리: `#include`, `#define`, 조건부 컴파일을 처리합니다.
2. 컴파일: 각 `.cpp` 번역 단위를 기계어 목적 파일로 바꿉니다.
3. 링크: 목적 파일과 라이브러리의 이름 정의를 연결해 실행 파일을 만듭니다.

`declaration`은 이름과 타입을 알리고, `definition`은 함수 몸체나 객체 저장 공간까지
제공합니다. 하나의 프로그램에는 보통 한 정의만 있어야 한다는 ODR(One Definition Rule)이
적용됩니다. 템플릿, `inline`, 클래스 내부 정의에는 세부 예외가 있습니다.

## 토큰과 scope

- identifier: `count`, `Widget` 같은 이름
- keyword: `if`, `class`, `const`처럼 예약된 단어
- literal: `42`, `3.14`, `'x'`, `"text"`, `true`, `nullptr`
- operator/punctuator: `+`, `=`, `::`, `;`, `{}`, `[]`
- scope: 이름이 보이고 의미가 유지되는 영역

```cpp
namespace app {             // namespace scope
int global_count = 0;

void run(int parameter) {   // parameter scope
    int local = parameter;  // block scope
    if (local > 0) {
        int only_here = 1;  // 이 중괄호 안에서만 보임
    }
}
}
```

## 타입, 객체, 값, 변수

- 타입(type): 가능한 값과 연산의 규칙. 예: `int`, `std::string`
- 객체(object): 저장 공간을 차지하고 수명이 있는 실체
- 값(value): 객체가 나타내는 정보
- 변수(variable): 이름이 붙은 객체 또는 참조

기본 타입에는 `bool`, 문자 타입, 정수 타입, 부동소수점 타입, `void`가 있습니다.
각 타입의 정확한 비트 수는 일부만 고정되어 있습니다. 필요하면 `<cstdint>`의
`std::int32_t` 존재 여부를 확인하거나 `std::uint64_t` 등을 씁니다.

## 초기화는 대입과 다르다

```cpp
int a;       // 기본 초기화: 지역 int의 값은 불확정
int b = 3;   // 복사 초기화
int c(3);    // 직접 초기화
int d{3};    // 직접 목록 초기화: narrowing을 막아 줌
int e{};     // 값 초기화: int는 0
b = 4;       // 이미 존재하는 객체에 대입
```

초심자에게는 `{}`가 안전한 기본 선택이지만, `std::initializer_list` overload가 우선되는
경우처럼 괄호와 의미가 달라질 수 있습니다.

## 표현식과 문

- 표현식(expression)은 값을 계산하거나 부수 효과를 냅니다: `a + b`, `f()`, `x = 3`
- 문(statement)은 실행 단위입니다: `x = 3;`, `return x;`, `if (...) {...}`
- 선언문은 새 이름을 도입합니다: `int x = 3;`

연산자 우선순위를 전부 외우지 말고 의도가 보이도록 괄호를 씁니다. 특히 `&&`와 `||`는
왼쪽 결과만으로 답이 정해지면 오른쪽을 평가하지 않는 short-circuit 연산자입니다.

## 제어문

```cpp
if (condition) { ... } else { ... }
switch (value) { case 1: ...; break; default: ...; }
for (초기화; 조건; 증감) { ... }
for (auto& element : range) { ... } // range-for
while (condition) { ... }
do { ... } while (condition);
```

`break`는 가장 가까운 반복문/`switch`를 끝내고, `continue`는 다음 반복으로 갑니다.
`return`은 함수를 끝냅니다.

## 함수의 기본

```cpp
int add(int left, int right) {
    return left + right;
}
```

- `int`: 반환 타입
- `add`: 함수 이름
- `(int left, int right)`: 매개변수 목록
- `{...}`: 함수 정의
- `add(1, 2)`: 두 인자(argument)를 전달하는 호출 표현식

parameter는 선언에 있는 입력 변수이고 argument는 호출할 때 넘기는 실제 표현식입니다.
값 전달은 복사, `T&`는 수정 가능한 참조, `const T&`는 복사 없이 읽기, `T&&`는 이동/전달
문맥에 쓰입니다. 작은 숫자 타입은 보통 값으로 넘깁니다.

## 자동 타입과 별칭

```cpp
auto n = 42;                 // int
using Counter = long long;   // 새 타입이 아니라 별칭
Counter total = 0;
```

`auto`는 타입이 없어지는 것이 아니라 초기화 식에서 정적으로 추론됩니다. top-level
`const`와 참조가 어떻게 추론되는지는 [선언 문서](03-declarations-and-signatures.md#auto와-decltype)를
참고하세요.

## 변환

- implicit conversion: 컴파일러가 문맥에 맞게 자동 변환
- explicit conversion: `static_cast<double>(n)`처럼 의도를 표시
- promotion: 작은 정수 타입이 `int` 등으로 승격
- narrowing: 정보를 잃을 수 있는 변환; `{}` 초기화는 많은 narrowing을 거부

`static_cast`, `dynamic_cast`, `const_cast`, `reinterpret_cast`는 목적이 다릅니다. C 스타일
캐스트 `(T)x`는 어떤 변환을 했는지 숨기므로 피합니다.

## 배열, enum, namespace

```cpp
int raw[3]{1, 2, 3};               // 크기가 타입의 일부인 내장 배열
std::array<int, 3> fixed{1, 2, 3}; // 값 타입처럼 다루기 쉬운 고정 배열

enum class Color { red, green, blue };
Color color = Color::green;         // scoped enum은 이름과 변환이 안전함

namespace project::math {           // C++17 nested namespace
int square(int value) { return value * value; }
}
```

일반 코드에서는 내장 배열보다 `std::array`/`std::vector`, unscoped `enum`보다
`enum class`가 의도를 더 잘 보존합니다. `union`은 같은 저장 공간에 여러 멤버를 겹치지만
활성 멤버 수명 규칙이 까다롭습니다. 닫힌 타입 대안에는 C++17 `std::variant`를 우선
검토합니다.

## 전처리와 include guard

전처리기는 C++ 타입을 알기 전에 텍스트 수준에서 동작합니다.

```cpp
#pragma once
#include <vector>

#if defined(PROJECT_DEBUG)
// debug에서만 포함할 선언
#endif
```

매크로는 scope, 타입 검사, 정상적인 디버깅을 우회하므로 상수는 `constexpr`, 작은 함수는
함수/템플릿을 우선합니다. 이식 가능한 헤더에는 `#ifndef` include guard도 널리 씁니다.

## I/O와 예외 입문

```cpp
try {
    throw std::runtime_error{"reason"};
} catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
}
```

`std::cout`은 보통 출력, `std::cerr`는 진단 출력에 씁니다. 예외는 값으로 던지고 보통
`const&`로 잡습니다. 소멸자는 기본적으로 예외를 밖으로 던지지 않아야 합니다. 모든 실패를
예외로 표현하는 것은 아니므로 API의 Returns/Throws와 오류 코드 overload를 확인합니다.

## 오류의 네 종류

- compile-time error: 문법/타입 규칙 위반
- link error: 선언은 찾았지만 정의를 연결하지 못함
- runtime error/exception: 실행 중 감지된 문제
- undefined behavior: 표준이 결과를 보장하지 않는 실행

다음: [선언과 call signature 해독](03-declarations-and-signatures.md)
