# 2026-07-04 Daily Study: `std::atomic_ref`와 중복 없는 부분 문자열

## 오늘의 주제

브리핑의 핵심은 일반 구조체 멤버에 필요한 순간만 원자적 뷰를 씌우는 `std::atomic_ref`다. C++26 정적 리플렉션과 pack indexing 문법은 현재 로컬 GCC 16.1.0에서 안정적으로 컴파일하기 어렵기 때문에, 실습 코드는 **C++23에서 실행 가능한 형태**로 작성했다.

## 왜 필요한가

실무에서 패킷 수, 오류 수, 요청 수 같은 카운터는 여러 스레드가 동시에 갱신한다. 뮤텍스를 매번 잠그면 대기 시간이 커진다. `std::atomic_ref`는 기존 구조체 레이아웃을 유지하면서 특정 멤버만 원자적으로 갱신할 수 있어, ABI가 중요한 네트워크/금융/임베디드 코드에 유용하다.

## 파일 구성

```text
CMakeLists.txt
main.cpp      # atomic_ref 기반 메트릭 게이트웨이
problem.cpp   # 중복 없는 가장 긴 부분 문자열
README.md
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-04
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_04.exe
.\build\daily_problem_2026_07_04.exe
```

## 기본 문법 복습

- `struct`: 관련 데이터를 하나의 타입으로 묶는다.
- `std::string_view`: 문자열을 복사하지 않고 읽기 전용으로 바라본다. 원본보다 오래 살면 안 된다.
- `std::atomic_ref<int>`: 이미 존재하는 `int`를 원자적으로 접근한다.
- `memory_order_relaxed`: 카운터처럼 숫자 자체만 정확하면 되는 경우 불필요한 메모리 장벽을 줄인다.
- `std::thread`: 여러 실행 흐름을 만든다. `join()`으로 끝날 때까지 기다려야 한다.
- `std::tuple`, `std::get<1>()`: C++26 `args...[1]`의 현재 사용 가능한 대체 방식이다.

## 실무 아키텍처 포인트

`main.cpp`는 위험한 동시성 처리를 `update_network_metrics()` 한 곳에 모았다. 이렇게 하면 나머지 코드는 카운터가 어떻게 동기화되는지 몰라도 된다. 실무에서는 동시성 경계를 작게 만들수록 테스트와 장애 분석이 쉬워진다.

## 알고리즘 문제

문자열에서 중복 문자가 없는 가장 긴 부분 문자열의 길이를 구한다.

```text
입력: "abcabcbb"
출력: 3
```

핵심은 각 문자의 마지막 등장 위치를 `last_seen[256]`에 저장하고, 중복이 윈도우 안에 들어오면 `left`를 한 번에 점프시키는 것이다.

복잡도:

- 시간: `O(N)`
- 공간: `O(1)`

