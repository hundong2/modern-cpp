# 2026-08-02 이해 체크포인트

## 1. 말로 설명하기

- `optional<User>`가 `User`와 `-1` 조합보다 실패 계약을 명확히 하는 이유를 말한다.
- `std::move(repository)`가 즉시 객체를 움직이는 함수가 아니라 xvalue 표현을 만드는 것임을 말한다.
- `UserRepository`는 정책 계약, `MemoryUserRepository`는 저장 기술, `GreetingService`는 사용 사례를 맡는다고 설명한다.
- 가상 호출이 간접 호출일 수 있지만 실제 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다고 설명한다.

## 2. 코드에서 찾기

- 생성자에 반환형이 없는 곳, `explicit`, 멤버 초기화 목록, `public`/`private`, `const` 참조를 각각 찾는다.
- lvalue(`result`), prvalue(`User{...}`), xvalue(`std::move(repository)`)를 찾고 수명과 소유권 변화를 설명한다.
- `icpc_problem.cpp`에서 거리 불변식, 간선 완화, 오래된 힙 항목 제거를 가리킨다.

## 3. 직접 수정하고 검증하기

1. `problem.cpp`의 `ch - '0'`을 잠시 `0`으로 바꾸어 테스트 실패를 관찰한 뒤 복구한다.
2. 입력을 `"x"`, `"12"`, `"0"`으로 바꾸어 실패·성공 분기를 확인한다.
3. 빌드 후 세 실행 파일을 실행하고 CTest 3개가 통과하는지 확인한다.

## 4. 오늘의 ICPC 검증

- 문제: BOJ 1753 **최단경로** — https://www.acmicpc.net/problem/1753
- 알고리즘 문서: [`../algorithm/dijkstra.md`](../algorithm/dijkstra.md)
- 복잡도: 시간 `O((V+E) log V)`, 공간 `O(V+E)`
- 정확성 근거: 양의 가중치에서 힙의 최솟값으로 확정한 정점은 더 짧아질 수 없고, 모든 성공한 완화를 다시 후보에 넣는다.
- 예제 검증: CTest가 공식 예제 입력을 전달하여 `0, 2, 3, 7, INF` 출력을 줄 단위로 비교한다.
- 추가 검증: 간선이 없는 정점, 시작 정점 자신, 평행 간선 중 더 작은 가중치가 선택되는 경우를 손으로 시험한다.
