# 2026-08-14 이해 점검

## Modern C++와 아키텍처
- [ ] `struct` 기본 public과 `class` 기본 private를 설명한다.
- [ ] 생성자에 반환형이 없고 멤버 초기화 목록이 먼저 실행됨을 설명한다.
- [ ] `explicit`과 `PercentDiscount{20}` 직접 초기화를 설명한다.
- [ ] lvalue `policy`, xvalue `std::move(policy)`, prvalue `make_unique` 결과를 구분한다.
- [ ] `unique_ptr` 이동, 객체 수명, 소유권, 복사 생략, 가상 소멸자를 설명한다.

## 오늘의 ICPC 문제
- 문제: [BOJ 1916 최소비용 구하기](https://www.acmicpc.net/problem/1916)
- 문서: [`../algorithm/dijkstra.md`](../algorithm/dijkstra.md)
- [ ] 최신 최소 후보의 거리가 확정된다는 불변식을 설명한다.
- [ ] 오래된 힙 항목을 버려도 정확한 이유를 설명한다.
- [ ] 시간 `O((V+E) log V)`, 공간 `O(V+E)`를 유도한다.

## 초보자 검증
1. 세 실행 파일을 경고 없이 컴파일한다.
2. 학습 실행 파일이 각각 `800`, `42`를 출력하고 0으로 끝나는지 본다.
3. CTest 공식 예제 `4`와 평행 간선 사례 `9`를 통과시킨다.
4. `1→4→5`의 완화 과정을 종이에 추적한다.
5. 음수 간선에서는 다익스트라를 쓰면 안 되는 이유를 반례로 보인다.
