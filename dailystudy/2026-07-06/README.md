# 2026-07-06 Daily Study: 원자적 CAS와 곱 윈도우

## 오늘의 주제

여러 스레드가 공유 포인터를 바꾸는 상황에서 뮤텍스 없이 안전하게 교체하려면 CAS(compare-and-swap)가 필요하다. C++에서는 `std::atomic<T*>`와 `compare_exchange_strong()`으로 표현한다.

## 왜 필요한가

라우팅 테이블, 설정 스냅샷, 읽기 전용 인덱스는 읽는 스레드가 매우 많고 갱신은 가끔 일어난다. 매번 뮤텍스를 걸면 읽기 경로가 느려진다. CAS 기반 교체는 “내가 본 값이 아직 그대로라면 새 값으로 바꾼다”는 조건부 갱신을 하드웨어 원자 명령으로 수행한다.

## 파일 구성

```text
main.cpp      # atomic pointer CAS 라우팅 노드 교체
problem.cpp   # 곱이 K 미만인 연속 부분 배열 개수
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-06
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_06.exe
.\build\daily_problem_2026_07_06.exe
```

## 기본 문법 복습

- `alignas(64)`: 구조체를 64바이트 경계에 맞춘다. 캐시 라인 경합을 줄이는 데 쓰인다.
- `std::atomic<T*>`: 포인터 값을 원자적으로 읽고 쓴다.
- `load()`: 현재 값을 안전하게 읽는다.
- `compare_exchange_strong(expected, desired)`: 현재 값이 `expected`와 같을 때만 `desired`로 바꾼다.
- `std::unique_ptr`: 실제 노드 메모리를 소유한다. 예제에서는 atomic 포인터가 소유권을 갖지 않게 분리했다.

## 실무 아키텍처 포인트

`RoutingTable` 클래스가 원자 포인터 접근을 감싼다. 실무에서는 atomic 연산을 여러 파일에 흩뿌리지 말고, 작은 소유 객체 안에 모아야 memory order와 소유권 규칙을 통제하기 쉽다.

## 알고리즘 문제

양의 정수 배열에서 곱이 `k` 미만인 연속 부분 배열의 개수를 구한다.

```text
nums = [10, 5, 2, 6], k = 100
answer = 8
```

윈도우가 유효할 때 `right`에서 끝나는 유효 부분 배열 개수는 `right - left + 1`개다.

복잡도:

- 시간: `O(N)`
- 공간: `O(1)`

