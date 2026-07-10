# 2026-07-08 Daily Study: `volatile` MMIO 시뮬레이션과 K회 이상 문자열

## 오늘의 주제

브리핑은 하드웨어 MMIO 레지스터와 구조체를 정적 리플렉션으로 연결하는 내용을 다룬다. 실제 물리 주소 `0x40001000`에 쓰는 코드는 일반 Windows 사용자 프로그램에서 안전하지 않으므로, 실습은 `volatile` 필드를 가진 mock 레지스터 구조체로 구성했다.

## 왜 필요한가

디바이스 드라이버나 임베디드 코드는 변수를 읽고 쓰는 행위 자체가 하드웨어 동작을 일으킨다. 컴파일러가 “안 쓰는 값”이라고 판단해 읽기/쓰기를 제거하면 장치가 동작하지 않는다. `volatile`은 그런 최적화 제거를 막기 위한 최소 장치다.

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-08
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_08.exe
.\build\daily_problem_2026_07_08.exe
```

## 기본 문법 복습

- `volatile`: 읽기/쓰기 명령을 컴파일러가 임의로 제거하지 못하게 한다.
- `std::uint32_t`: 크기가 명확한 32비트 정수다.
- `std::expected`: 성공 또는 실패 이유를 값으로 반환한다.
- `std::span`: 연속 메모리를 복사 없이 바라보는 뷰다.
- `std::array<int, 26>`: 크기가 고정된 배열이다.

## 실무 아키텍처 포인트

실제 하드웨어 접근 코드는 반드시 한 계층에 모아야 한다. 나머지 비즈니스 로직이 직접 레지스터를 건드리면 테스트가 어렵고 장비가 없을 때 개발이 막힌다. 이번 예제처럼 mock 레지스터를 주입하면 안전하게 학습하고 테스트할 수 있다.

## 알고리즘 문제

문자열 안에서 모든 문자가 최소 `k`번 이상 등장하는 가장 긴 부분 문자열 길이를 구한다.

핵심은 알파벳 소문자가 26개뿐이라는 점이다. 목표 고유 문자 수를 1부터 26까지 고정하고, 각 경우에 슬라이딩 윈도우를 수행한다.

복잡도:

- 시간: `O(26N)`, 즉 `O(N)`
- 공간: `O(1)`

