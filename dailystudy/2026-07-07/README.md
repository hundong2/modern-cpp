# 2026-07-07 Daily Study: `std::variant` 정적 다형성과 고유 문자 윈도우

## 오늘의 주제

브리핑은 가상 함수 테이블(vtable)을 정적 리플렉션으로 제거하는 흐름을 다룬다. 실습 코드는 현재 컴파일 가능한 C++23의 `std::variant`와 `std::visit`으로 같은 방향의 정적 다형성을 연습한다.

## 왜 필요한가

`virtual` 기반 다형성은 유연하지만 객체마다 vtable 포인터가 붙고 호출 시 간접 참조가 생긴다. 프로토콜 타입의 종류가 빌드 시점에 정해져 있다면 `std::variant`가 더 단순하고 캐시 친화적일 수 있다.

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-07
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_07.exe
.\build\daily_problem_2026_07_07.exe
```

## 기본 문법 복습

- `std::variant<A, B>`: `A` 또는 `B` 중 하나를 안전하게 담는다.
- `std::visit`: variant 안에 실제로 들어 있는 타입에 맞춰 함수를 호출한다.
- `template <typename T>`: 타입을 인자로 받는 함수/클래스를 만든다.
- `std::string_view`: 문자열 복사를 피한다.
- `std::unordered_map`: 키 기반 빈도 장부를 평균 `O(1)`에 관리한다.

## 실무 아키텍처 포인트

패킷 타입이 제한적이고 빈번히 처리된다면 상속 계층보다 `variant` 기반 라우터가 읽기 쉽고 빠를 수 있다. 새 타입을 추가할 때는 variant 타입 목록과 처리 visitor만 갱신하면 된다.

## 알고리즘 문제

문자열 전체에 등장하는 모든 고유 문자를 최소 한 번씩 포함하는 가장 짧은 부분 문자열 길이를 구한다.

```text
s = "AABCBBCADEB"
answer = 5   # "BCADE"
```

복잡도:

- 시간: `O(N)`
- 공간: 알파벳 범위에서는 `O(1)`

