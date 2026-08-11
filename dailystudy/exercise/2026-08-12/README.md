# 2026-08-12 Modern C++ 학습

## 오늘의 문법과 아키텍처

`main.cpp`는 RAII 스코프 타이머로 `std::chrono::steady_clock`, `std::string_view`, 생성자·소멸자와 객체 수명을 연결한다. `problem.cpp`는 `ClockPort` 인터페이스와 `FakeClock` 구현을 분리하고 `std::unique_ptr` 의존성 주입으로 소유권을 명시한다. 코드 가까운 주석에서 `struct`/`class`, `public`/`private`, 생성자와 멤버 초기화 목록, `explicit`, `const`, 가상 호출, lvalue·prvalue·xvalue, 이동, 수명과 복사 생략을 확인한다.

## 오늘의 ICPC 문제

- 문제: BOJ 1806 **부분합**
- 출처: [Baekjoon Online Judge](https://www.acmicpc.net/problem/1806)
- 핵심 알고리즘: 양수 수열의 두 포인터/슬라이딩 윈도우
- 문서: [`../algorithm/two-pointers-sliding-window.md`](../algorithm/two-pointers-sliding-window.md)
- 복잡도: 시간 `O(N)`, 입력 저장 공간 `O(N)`, 알고리즘 추가 공간 `O(1)`

양수라는 조건 때문에 오른쪽 끝을 늘리면 합이 감소하지 않고 왼쪽 끝을 늘리면 합이 감소한다. 합이 목표 이상인 동안 왼쪽을 당겨 현재 오른쪽 끝에서 가장 짧은 구간을 만들며, 두 인덱스가 각각 최대 N번만 이동한다.

## 빌드와 검증

```powershell
cmake -S dailystudy/exercise/2026-08-12 -B build/2026-08-12 -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=tools/w64devkit/bin/g++.exe
cmake --build build/2026-08-12
ctest --test-dir build/2026-08-12 --output-on-failure
```

CTest는 두 학습 프로그램의 정상 종료와 ICPC 공식 예제, 단일 원소 성공, 불가능한 경우를 검증한다.

## 직접 해보기

1. `ScopeTimer timer = "x";`가 컴파일되지 않는 이유와 `ScopeTimer timer{"x"};`가 되는 이유를 설명한다.
2. `std::move(clock)`을 지우면 왜 `unique_ptr` 복사 오류가 나는지 확인한다.
3. ICPC 입력을 음수가 포함된 수열로 바꾸면 슬라이딩 윈도우 불변식이 왜 깨지는지 반례를 만든다.
4. `icpc_problem.cpp`를 보지 않고 두 포인터 풀이를 다시 작성하고 세 CTest를 통과시킨다.
