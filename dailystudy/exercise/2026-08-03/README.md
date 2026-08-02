# 2026-08-03 — `std::variant`와 타입 안전 명령 디스패치

오늘은 상속 계층 대신 `std::variant`로 닫힌 명령 집합을 표현하고 `std::visit`으로 애플리케이션 경계에서 처리한다. `main.cpp`는 `CommandHandler` 아키텍처 예제, `problem.cpp`는 작은 방문자 연습이다.

## 핵심 문법과 실행 관점

- `struct`는 기본 `public`, `class`는 기본 `private`이며 `public:`/`private:`로 인터페이스와 상태를 나눈다.
- 생성자에는 반환형이 없고, `explicit`은 원치 않는 암시적 변환을 막는다. `CommandHandler handler{true}`처럼 직접 초기화한다.
- 이름 있는 객체는 lvalue, `Start{"backup"}`은 prvalue, `std::move(x)`가 만드는 식은 xvalue이다. `variant` 대안 교체 시 이전 객체 수명이 끝나고 새 객체가 내부 저장소에 생성된다.
- `std::visit`은 활성 태그에 따라 호출한다. 기계 수준에서는 로드·비교·조건 분기·함수 호출이 될 수 있지만 정확한 명령은 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.

## 오늘의 ICPC 문제

- ID/제목: Kattis `shortestpath1`, **Single source shortest path, non-negative weights**
- 출처: [Kattis](https://open.kattis.com/problems/shortestpath1)
- 핵심 알고리즘: 다익스트라 — 공용 문서 [`../algorithm/dijkstra.md`](../algorithm/dijkstra.md)
- 복잡도: 테스트 케이스당 시간 `O((n+m) log n + q)`, 공간 `O(n+m)`
- 검증: CTest가 공식 형식의 도달·미도달 질의를 실행해 `0, 2, 4, Impossible`과 케이스 구분 빈 줄을 정확히 비교한다.

## 빌드와 직접 해보기

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

1. `Operation`에 `Multiply`를 추가하고 방문자가 모든 타입을 처리하게 한다.
2. `Command`를 값으로 전달할 때와 `const&`로 전달할 때 복사 횟수와 객체 수명을 설명한다.
3. ICPC 예제의 힙과 거리 배열을 완화 단계마다 손으로 기록한다.
