# 2026-07-05 Daily Study: 비트 마스킹과 네덜란드 국기 문제

## 오늘의 주제

여러 boolean 상태를 개별 변수로 들고 다니지 않고, 하나의 바이트 안에 플래그 비트로 압축한다. 브리핑의 정적 리플렉션/pack indexing 부분은 C++23에서 실행 가능한 수동 inspector와 `std::tuple`로 대체했다.

## 왜 필요한가

네트워크 패킷, 권한, 상태 플래그는 매우 자주 검사된다. `encrypted`, `urgent`, `compressed` 같은 상태를 비트 하나씩에 넣으면 메모리를 아끼고, 여러 조건을 한 번의 `&` 비교로 검사할 수 있다.

## 파일 구성

```text
main.cpp      # 비트 플래그 기반 패킷 검사
problem.cpp   # 0, 1, 2 배열 제자리 정렬
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-05
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_05.exe
.\build\daily_problem_2026_07_05.exe
```

## 기본 문법 복습

- `static constexpr`: 컴파일 타임 상수다. 플래그 값처럼 바뀌면 안 되는 값에 쓴다.
- `std::uint8_t`: 정확히 8비트 크기의 정수 타입이다.
- `|`: 비트 OR. 여러 플래그를 하나로 조합한다.
- `&`: 비트 AND. 특정 플래그가 켜져 있는지 검사한다.
- `std::swap`: 두 원소의 값을 교환한다.
- `std::tuple`: 여러 타입의 값을 하나로 묶는다.

## 실무 아키텍처 포인트

비트 플래그 검사는 `has_all_flags()`로 분리했다. 조건식이 프로젝트 곳곳에 흩어지면 나중에 플래그 의미가 바뀔 때 수정 지점이 늘어난다. 작고 이름이 명확한 함수로 감싸면 의도를 보존할 수 있다.

## 알고리즘 문제

0, 1, 2로만 구성된 배열을 `std::sort` 없이 한 번의 순회로 정렬한다.

핵심 포인터:

- `low`: 다음 0이 들어갈 위치
- `mid`: 현재 검사 위치
- `high`: 다음 2가 들어갈 위치

복잡도:

- 시간: `O(N)`
- 공간: `O(1)`

