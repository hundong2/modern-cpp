# 2026-07-10 Daily Study: 정적 DI와 최소 윈도우 문자열

## 오늘의 주제

브리핑은 정적 리플렉션을 이용한 컴파일 타임 DI 컨테이너를 다룬다. 실습은 C++23에서 템플릿 기반 정적 DI를 구현한다. 드라이버 타입이 템플릿 인자로 결정되기 때문에 virtual 함수나 런타임 서비스 로케이터가 필요 없다.

## 왜 필요한가

DI는 객체가 직접 의존 객체를 만들지 않고 외부에서 주입받게 하는 설계 방식이다. 테스트에서는 `MockNetworkDriver`, 운영에서는 `FiberNetworkDriver`를 넣을 수 있다. 템플릿 DI는 이 결합을 컴파일 타임에 확정하므로 런타임 맵 조회나 virtual 호출 비용이 없다.

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-10
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_10.exe
.\build\daily_problem_2026_07_10.exe
```

## 기본 문법 복습

- 포인터 `T*`: 객체 주소를 저장하는 변수다. `nullptr`일 수 있고 다른 주소로 바뀔 수 있다.
- 참조자 `T&`: 기존 객체의 별명이다. 생성 후 다른 객체를 가리키게 바꿀 수 없다.
- 템플릿 `template <typename T>`: 타입을 컴파일 타임 인자로 받는다.
- 생성자 `explicit AsyncGateway(Driver driver)`: 객체를 만들 때 필요한 의존성을 받는다.
- 멤버 변수 `driver_`: 게이트웨이가 사용할 드라이버를 보관한다.
- `std::string_view`: payload 문자열을 복사하지 않고 읽는다.

## 실무 아키텍처 포인트

`AsyncGateway<Driver>`는 드라이버가 어떤 타입인지 컴파일 타임에 안다. 그래서 `driver_.send_packet()` 호출은 직접 호출로 최적화되기 쉽다. 실무에서는 성능이 중요한 경로에서 추상 인터페이스 대신 이런 정적 조립 방식을 고려할 수 있다.

단, 템플릿 DI는 타입 조합이 많아지면 빌드 시간이 늘고 바이너리가 커질 수 있다. 외부 플러그인처럼 런타임 교체가 필요한 곳은 virtual 인터페이스가 더 적합하다.

## 알고리즘 문제

문자열 `s`에서 문자열 `t`의 모든 문자(중복 포함)를 담는 가장 짧은 부분 문자열을 찾는다.

```text
s = "ADOBECODEBANC", t = "ABC"
answer = "BANC"
```

핵심은 `target` 빈도표와 현재 윈도우 빈도표를 유지하면서, 조건이 만족되면 왼쪽을 최대한 줄이는 것이다.

복잡도:

- 시간: `O(N)`
- 공간: `O(1)`

