# 2026-08-03 이해 체크포인트

## 말로 증명하기

- `variant`가 잘못된 타입 캐스트 없이 닫힌 명령 집합을 표현하는 이유를 설명한다.
- `struct`/`class` 기본 접근, 생성자의 무반환형, `explicit`, 멤버 초기화 목록을 코드에서 가리킨다.
- `command` lvalue, `Stop{7}` prvalue, `std::move(command)`이 있다면 xvalue가 되는 이유와 참조 바인딩·복사·이동·수명을 설명한다.
- `std::visit`의 간접 동작을 특정 어셈블리 명령으로 단정할 수 없는 이유를 말한다.

## 직접 검증하기

1. `problem.cpp`에 `Multiply`를 추가해 컴파일하고 결과를 검사한다.
2. 방문자에서 `const auto&`를 `auto`로 바꾸어 대안 복사 가능성을 설명한다.
3. 세 실행 파일과 CTest 세 개가 모두 성공하는지 확인한다.

## 오늘의 ICPC 검증

- 문제: Kattis `shortestpath1` — https://open.kattis.com/problems/shortestpath1
- 알고리즘 문서: [`../algorithm/dijkstra.md`](../algorithm/dijkstra.md)
- 불변식: `distance[v]`는 발견된 경로 중 최솟값이며, 힙에서 현재 값과 일치하는 최소 후보는 음이 아닌 가중치 때문에 확정된다.
- 복잡도: 시간 `O((n+m) log n + q)`, 공간 `O(n+m)`
- 검증 과정: 도달 가능한 시작점·중간점, 두 간선을 잇는 경로, 도달 불가능한 정점을 한 입력에서 검사하고 종료 센티널 및 케이스 빈 줄까지 비교한다.
