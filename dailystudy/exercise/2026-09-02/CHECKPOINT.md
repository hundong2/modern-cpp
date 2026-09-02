# 2026-09-02 CHECKPOINT

자료를 닫고 답한다. 단순히 “호출한다”로 끝내지 말고 실제 식의 수신 객체, 모든 인자, 반환값, 호출 전후 상태, 복잡도, 수명·무효화·오류를 설명한다.

## Modern C++ 문법과 아키텍처

1. `struct StockRecord`와 `class ReservationService`가 접근 지정자를 생략하면 기본 접근이 각각 무엇인가?
2. 생성자에 반환형이 없는 이유와 `explicit ReservationService(StockRecord&)`가 막는 암시 변환, 허용하는 직접 초기화 예를 적는다.
3. `StockRecord& stock_`가 소유권·null 가능성·재바인딩·수명에 관해 어떤 계약을 만드는가?
4. `ScopeRollback<Rollback>`에서 템플릿 인자 Rollback은 무엇이며 람다마다 타입이 다른 이유를 설명한다.
5. `ScopeRollback rollback{lambda}`의 람다 prvalue, 값 매개변수, 이름 있는 매개변수 lvalue, `std::move` xvalue, 멤버 수명을 순서대로 추적한다.
6. callback의 `[this, units]`에서 두 캡처의 값/참조/포인터·소유권 차이와 dangling 조건을 말한다.
7. 복사·이동 생성자와 복사·이동 대입을 모두 delete한 이유를 rollback exactly-once와 스코프 고정 책임에 연결하고, 사용자 선언 소멸자가 암시적 이동 생성을 억제하는 규칙을 말한다.
8. 소멸자 `noexcept`와 callback `noexcept`가 필요한 이유, callback이 던질 때의 결과를 말한다.
9. 조기 `return false`에서도 guard가 실행되는 객체 파괴 순서와 `commit()` 뒤 실행되지 않는 상태 변화를 적는다.
10. 반환 bool prvalue와 지역 const bool 초기화에서 복사 생략 또는 값 전달을 설명한다.
11. load·store·비교·분기·callback 호출이 특정 어셈블리 명령을 보장하지 않는 이유를 CPU·ABI·컴파일러·최적화와 연결한다.

## 표준 라이브러리 호출 계약

### `std::move(rollback)`

12. 자유 함수이므로 수신 객체가 없다는 점, 템플릿 인자, 실제 인자 식의 타입·값 범주를 말한다.
13. 반환형·값 범주와 반환 참조 사용 위치, 함수 자체가 callback을 이동하지 않는 이유를 설명한다.
14. 선택된 멤버 이동 생성 뒤 매개변수와 `rollback_`의 소유 상태, 복잡도·예외 가능성을 말한다.

### `stack.reserve(static_cast<std::size_t>(city_count))`

15. 수신 객체의 정확한 타입과 호출 전 size/capacity, 한 인자의 타입·값·허용 범위를 말한다.
16. 반환형과 사용 여부, 호출 뒤 size/capacity, 재할당 시 관찰자 무효화, 시간·할당·예외 보장을 설명한다.

### `graph[from].push_back(to)`

17. 수신 `vector<int>` 상태, 인자 `to`의 타입·값 범주, 선택되는 복사 오버로드를 말한다.
18. void 반환과 호출 뒤 size/capacity, 원본 `to`, 새 원소 수명, 재할당·무효화·복잡도·예외를 설명한다.

### `stack.empty()` / `stack.back()` / `stack.pop_back()`

19. 각 호출의 인자 수, 반환형, 반환값 사용·무시, 상태 변화 유무를 말한다.
20. `back/pop_back` 전제조건을 어느 조건이 보장하며 `pop_back` 뒤 제거 원소·capacity·앞선 원소 관찰자·이전 past-the-end 반복자는 어떻게 되는가?

### `std::min(low[parent], low[child])`

21. 수신 객체 유무, 두 인자의 타입·값 범주·참조 바인딩, 반환형이 값이 아니라 참조라는 점을 설명한다.
22. 반환 참조를 즉시 복사 저장하는 이유, 호출 뒤 두 인자 상태, O(1)·할당·예외 계약을 말한다.

### 스트림 호출

23. `std::ios::sync_with_stdio(false)`의 인자, bool 반환값 무시, 전후 전역 입출력 상태, 피해야 할 C/C++ 입출력 혼용과 버퍼·상대 순서 보장을 잃는 이유를 설명한다.
24. `std::cin.tie(nullptr)`의 수신 객체 타입/상태, null 포인터 인자, `ostream*` 반환 무시, 전후 flush 연결을 설명한다.
25. `std::cin >> from >> to`의 두 연산자 호출이 각각 받는 피연산자와 첫 반환 `istream&`가 다음 호출에 쓰이는 과정을 말한다.
26. `std::cout << city << ' '`의 각 입력, 반환 참조 사용, cout 상태 변화, 출력 실패 표현과 소유권 유지를 말한다.

### vector 생성자·인덱싱

27. `std::vector<std::vector<int>> graph(static_cast<std::size_t>(city_count + 1))`에서 선택되는 count 생성자, 생략된 allocator, 유일한 인자의 타입·값 범주·허용 범위, 생성 뒤 size/원소 소유권, 시간·공간·예외를 설명한다.
28. `std::vector<int> discovered(count, 0)`의 두 인자와 fill 생성자, `std::vector<int> stack`의 무인자 기본 생성자를 비교하고 각 생성자에는 기존 수신 객체와 반환값이 없다는 점을 말한다.
29. `graph[static_cast<std::size_t>(from)].push_back(to)`를 바깥 `vector::operator[]`와 안쪽 `push_back`으로 나눠 수신 타입, 각 인자, 반환 `vector<int>&`/`void`, 범위 전제, 재할당·참조 무효화를 설명한다.
30. `discovered[static_cast<std::size_t>(root)]`가 반환하는 `int&`를 비교와 대입에서 각각 어떻게 쓰는지, 인덱스 범위·UB·O(1)·원소 수명 계약을 말한다.
31. `graph[vertex].size()` 반환형과 반환값 사용 위치를 설명하고, `std::size_t`가 부호 없는 이유와 음수 `int`를 검사 없이 변환할 때의 위험을 말한다.

## 단절점 low-link

32. `discovered[v]`, `low[v]`, `parent[v]`, `child_count[v]`, `next_edge[v]`를 한 문장씩 정의한다.
33. 트리 간선과 back edge를 구분하고 `low[v]`를 어떤 두 종류 값으로 갱신하는지 적는다.
34. 비루트 v가 자식 u에 대해 `low[u] >= discovered[v]`이면 v 제거 후 u 서브트리가 분리됨을 증명한다.
35. DFS 루트에는 같은 조건을 쓰지 않고 자식 수가 둘 이상인지 보는 이유를 삼각형과 별 그래프로 설명한다.
36. 무방향 간선에서 부모 간선을 back edge로 처리하면 low가 왜 잘못 내려갈 수 있는가?
37. `pop_back()` 직후에만 자식 low를 부모로 합치는 이유를 재귀 DFS의 함수 반환 시점과 연결한다.
38. 각 무방향 간선이 인접 목록에 두 번 저장되어도 전체 시간이 O(N+M)인 이유와 전체 공간을 계산한다.
39. 중복 간선이 허용되는 문제라면 `neighbor != parent`만으로 부모 간선을 건너뛰는 구현이 왜 부족할 수 있는가?
40. 단절점과 단절선의 low 조건 차이, block-cut tree로의 확장을 조사한다.

## 검증으로 이해 증명하기

41. 공개 예제에서 4와 5가 단절점이고 1과 2는 아닌 이유를 제거 후 연결성으로 설명한다.
42. N=5 일자 그래프에서 방문/low/부모와 단절점 판정을 종료 순서대로 적는다.
43. 작은 무작위 연결 그래프에서 각 정점을 제거하고 BFS로 연결 여부를 확인해 풀이와 비교하는 절차를 의사 코드로 적는다.
44. N=100,000 일자 그래프에서 재귀 대신 명시적 vector 스택을 쓴 이유와 두 저장 영역의 수명·한계를 비교한다.
45. guard 성공·실패 출력, 네 ICPC CTest, 무작위 대조, 경계 테스트, 표준 라이브러리 감사를 모두 통과해야 하는 이유를 적는다.

## 완료 기준

- 45문항을 자료 없이 설명한다.
- 실제 표준 호출마다 인자 수만큼 값 범주·소유권·상태 변화를 빠짐없이 답한다.
- root/non-root 단절점 판정을 직접 증명한다.
- 경고 포함 빌드, 두 학습 실행, CTest, 무작위·경계 검증, 전체 표준 라이브러리 감사에 통과한다.
