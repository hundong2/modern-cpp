# 2026-08-09 이해 점검

## Modern C++와 아키텍처

- [ ] `vector`는 메모리를 소유하지만 `span`은 소유하지 않는다는 차이를 설명한다.
- [ ] `std::span<const int>`가 원소 변경을 막되 원본 수명을 연장하지 않음을 설명한다.
- [ ] `ScoreService`의 `explicit` 생성자, 멤버 초기화 목록, `public`/`private` 역할을 코드에서 찾는다.
- [ ] `owned`는 lvalue, `ScoreSummary{...}`는 prvalue이며 반환 시 복사 생략되는 지점을 찾는다.
- [ ] `std::move`가 필요 없는 작은 `array` 복사와, 뷰를 이동해도 원본 소유권은 이동하지 않는 이유를 말한다.

## 오늘의 ICPC 문제

- 문제: BOJ 1149 **RGB거리** — <https://www.acmicpc.net/problem/1149>
- 핵심: 마지막 색을 상태로 둔 동적 계획법
- 문서: [`../algorithm/dynamic-programming-state-transition.md`](../algorithm/dynamic-programming-state-transition.md)
- 복잡도: 시간 `O(N)`, DP 추가 공간 `O(1)`

## 정확성 검증

- [ ] `previous[c]`가 처리한 마지막 집의 색이 `c`인 최소 비용이라는 불변식을 설명한다.
- [ ] 현재 빨강 상태가 이전 초록·파랑 상태만 참조해야 하는 이유를 설명한다.
- [ ] 각 상태가 가능한 모든 직전 색 중 최소를 택하므로 최적해를 놓치지 않음을 귀납적으로 증명한다.
- [ ] 공식 예제의 상태를 손으로 계산해 마지막 최솟값 96을 확인한다.
- [ ] CTest의 교대 최적 사례 3과 단일 집 경계 5가 나오는지 확인한다.

## 초보자 실습

1. `current` 없이 `previous[0]`부터 즉시 갱신하면 나머지 점화식이 왜 오염되는지 작은 예로 보인다.
2. 색이 K개라면 `O(NK^2)` 전이로 일반화하고, 최소·두 번째 최소를 이용한 `O(NK)` 개선을 조사한다.
3. 비용 합이 `int`를 넘는 제약으로 바뀐다면 상태 타입을 `long long`으로 바꾼다.
