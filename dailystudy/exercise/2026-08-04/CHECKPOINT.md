# 2026-08-04 이해 체크포인트

## Modern C++와 아키텍처

- `DiscountPolicy`가 포트이고 `FixedRatePolicy`가 어댑터인 이유를 말한다.
- `struct`/`class` 기본 접근, `public`/`private`, 반환형 없는 생성자, 생성자 매개변수와 멤버 초기화 목록을 코드에서 가리킨다.
- `explicit`이 막는 `FixedRatePolicy policy = 20;`과 올바른 `FixedRatePolicy policy{20};`를 비교한다.
- 이름 있는 `policy`는 lvalue, `std::move(policy)`는 xvalue, `Money{10000}`은 prvalue임을 설명한다.
- `unique_ptr` 이동 뒤 원래 포인터가 소유하지 않으며, 서비스가 정책 수명을 소유함을 설명한다.
- 반환 prvalue의 복사 생략과 임시 객체의 const 참조 바인딩 수명을 설명한다.
- 가상 호출이 간접 호출이 될 수 있지만 실제 기계 명령은 CPU·ABI·컴파일러·최적화에 따라 다름을 말한다.

## 직접 검증

1. 세 실행 파일을 빌드하고 `daily_main`은 8000, `daily_problem`은 42를 출력하는지 확인한다.
2. 정책 생성에서 `std::move`를 제거해 복사 금지 진단을 읽고 다시 복원한다.
3. 정액 할인 어댑터를 추가해 서비스 코드를 바꾸지 않고 정책만 교체한다.

## 오늘의 ICPC 검증

- 문제: BOJ 1197 [최소 스패닝 트리](https://www.acmicpc.net/problem/1197)
- 문서: [`../algorithm/kruskal-minimum-spanning-tree.md`](../algorithm/kruskal-minimum-spanning-tree.md)
- 핵심: 오름차순 간선 중 서로 다른 컴포넌트를 잇는 것만 선택하는 크루스칼과 DSU다.
- 불변식: 선택 간선은 항상 사이클 없는 숲이고, 처리한 간선 범위에서 어떤 MST로 확장 가능하다.
- 복잡도: 시간 `O(E log E)`, 공간 `O(V+E)`.
- 검증 과정: 공식 예제의 MST 간선 `(1,2)`, `(2,3)` 합이 3인지 CTest가 확인한다. 추가로 음수 간선, 평행 간선, V-1번째 선택 뒤 종료를 손으로 추적한다.
