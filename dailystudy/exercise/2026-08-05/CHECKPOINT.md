# 2026-08-05 이해 점검

## 초보자 검증

- [ ] `std::pmr::vector<int>`의 템플릿 인자와 일반 `std::vector<int>`와의 차이를 말할 수 있다.
- [ ] 생성자에 반환형이 없고, `explicit`이 암시적 변환을 막으며 `SampleService service{&resource}`는 허용되는 직접 초기화임을 설명한다.
- [ ] `resource`가 서비스·컨테이너보다 오래 살아야 하는 이유를 비소유 포인터와 객체 수명으로 설명한다.
- [ ] lvalue `numbers`, prvalue 함수 결과, `std::move(numbers)`가 만든 xvalue와 이동 후 상태를 구분한다.
- [ ] 실제 기계 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라져 특정 명령으로 단정할 수 없음을 말한다.

## ICPC 문제 검증

- 문제: BOJ 11724 「연결 요소의 개수」 (<https://www.acmicpc.net/problem/11724>)
- 알고리즘 문서: [`../algorithm/graph-traversal-connected-components.md`](../algorithm/graph-traversal-connected-components.md)
- 핵심: 미방문 정점을 찾을 때마다 답을 증가시키고 반복형 DFS로 그 요소 전체를 표시한다.
- 복잡도: 시간 `O(N+M)`, 공간 `O(N+M)`.

- [ ] 간선이 0개면 답이 N인 이유를 설명한다.
- [ ] 방문 표시를 스택에서 꺼낼 때가 아니라 넣을 때 해야 중복 삽입을 막는 이유를 설명한다.
- [ ] DFS 종료 뒤 시작점에서 도달 가능한 정점이 모두 방문됐다는 불변식으로 정확성을 증명한다.
- [ ] 예제 2개, 단일 정점 `1 0`, 완전 연결 그래프를 실행한다.
- [ ] `cmake --build build`와 `ctest --test-dir build --output-on-failure`가 모두 성공한다.

## 직접 수정

반복 DFS를 `std::queue<int>` 기반 BFS로 바꾸고 시간·공간 복잡도와 연결 요소 개수가 유지되는 이유를 주석으로 적는다.
