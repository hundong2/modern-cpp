# 2026-08-13 이해 점검

## Modern C++와 아키텍처

- [ ] `shared_ptr`은 제어 블록의 강한 참조 수가 0일 때 객체를 파괴한다고 설명할 수 있다.
- [ ] `weak_ptr`가 순환 소유권을 끊고 `lock()`이 만료 여부를 안전하게 검사하는 이유를 말할 수 있다.
- [ ] `store`(lvalue), `std::move(name)`(xvalue), `make_shared` 결과(prvalue)를 구분한다.
- [ ] 복사, 이동, 객체 수명, 소유권, 반환값 복사 생략을 코드의 실제 식으로 설명한다.
- [ ] `struct`의 기본 public과 `class`의 기본 private, 생성자에 반환형이 없다는 점, `explicit`의 효과를 설명한다.
- [ ] 실제 명령 선택은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라져 특정 어셈블리로 단정할 수 없음을 안다.

## 오늘의 ICPC 문제

- 문제: [BOJ 1717 집합의 표현](https://www.acmicpc.net/problem/1717)
- 알고리즘: [`../algorithm/disjoint-set-union.md`](../algorithm/disjoint-set-union.md)
- [ ] `parent[root] == root` 불변식과 대표가 같으면 같은 집합이라는 근거를 설명한다.
- [ ] 경로 압축과 랭크 기준 합치기가 트리를 얕게 유지하는 과정을 손으로 추적한다.
- [ ] 초기화 O(n), m개 연산 O(m alpha(n)) 상각, 공간 O(n)을 설명한다.

## 초보자 검증 절차

1. 모든 실행 파일을 경고 없이 컴파일한다.
2. `daily_main`이 `study`, `daily_problem`이 `hello expired`를 출력하고 0으로 끝나는지 본다.
3. `ctest --output-on-failure`로 두 ICPC 사례를 포함한 전체 테스트를 통과시킨다.
4. 원소 1-2, 2-3을 합친 뒤 `parent` 변화와 `find(1) == find(3)`을 종이에 기록한다.
5. `find`의 재귀 대입을 제거해도 정답은 같지만 최악의 탐색이 길어질 수 있음을 설명한다.

