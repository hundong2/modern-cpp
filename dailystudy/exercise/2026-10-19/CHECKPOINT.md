# 2026-10-19 초보자 이해 점검

답을 코드에서 그대로 베끼지 말고, 각 식의 타입·값 범주·소유권·전제조건을 소리 내어 설명한다.

## 1. 기초 문법과 타입

1. `<algorithm>`, `<vector>`, `<utility>`, `<iostream>`, `<cstddef>`, `<cstdint>`를 각각 왜 포함하는가?
2. `using UserId = int;`는 `int`와 암시 변환되지 않는 새 타입인가, 별칭인가?
3. `enum class Action`이 일반 정수와 뜻하지 않게 섞이는 것을 어떻게 줄이는가?
4. `UserId user_id{};`에서 `{}`가 만드는 초기값은 무엇인가?
5. `AccessRequest{42, Action::deploy}`의 멤버 초기화 순서와 값 범주를 말하라.
6. 함수 선언 `bool authorize(const AccessRequest& request) const`의 반환형, 매개변수, 두 `const`의
   역할을 각각 설명하라.
7. `[[nodiscard]]`와 `noexcept`는 각각 호출자·예외에 대해 무엇을 약속하거나 요청하는가?
8. `struct`와 `class`의 기본 접근 지정자 차이는 무엇이며 그 외 표현 능력도 다른가?
9. `public` API와 `private` 저장소를 분리하면 어떤 불변식을 보호할 수 있는가?
10. `explicit AllowList(std::vector<UserId>)`가 막는 암시 변환 예를 하나 들라.
11. 멤버 초기화 목록과 생성자 본문 중 멤버가 먼저 생성되는 곳은 어디인가?
12. 실제 멤버 초기화 순서는 초기화 목록의 표기 순서인가, 멤버 선언 순서인가?
13. `std::vector<UserId>`의 명시 template 인자와 숨은 기본 template 인자를 구분하라.
14. `if`, `for`, `while`, `continue`가 오늘 알고리즘에서 맡는 제어 흐름을 하나씩 말하라.

## 2. 포인터·참조·값 범주·수명

15. `const AllowList&`와 `const AllowList*`의 null 가능성, 재지정 가능성, 소유권을 비교하라.
16. `allow_list_{&allow_list}`가 owner 수명을 늘리는가? owner가 먼저 파괴되면 어떤 문제가 생기는가?
17. `allow_list`, `users`, `request`, `coins[index]`는 왜 lvalue인가?
18. `AccessRequest{...}`, `std::vector<UserId>{...}`, `AllowList{...}`는 왜 prvalue인가?
19. `std::move(users)`의 결과 값 범주는 무엇이며 move 함수 자체가 버퍼를 옮기는가?
20. vector 이동 생성 뒤 값 매개변수 `users`에 대해 보장되는 상태와 보장되지 않는 내용을 말하라.
21. 임시 요청이 `const AccessRequest&`에 결합됐을 때 수명은 언제까지인가?
22. `return AllowList{...};`의 보장 복사 생략과 이름 있는 local 반환의 NRVO를 구분하라.
23. gate와 allow-list의 선언 순서를 뒤집으면 파괴 역순 관점에서 무엇을 조심해야 하는가?
24. 읽기 전용 service가 container iterator를 반환하지 않는 것이 수명 안전성에 어떻게 도움이 되는가?

## 3. `std::ranges::contains` 호출 계약

25. `std::ranges::contains(users_, user_id)`에는 사용자 정의 수신 객체가 있는가?
26. 첫 번째 식의 정확한 타입·cv/ref·값 범주와 소유권 의미를 말하라.
27. 두 번째 식의 타입·값 범주와 `const T&` 바인딩을 말하라.
28. 생략된 projection의 타입과 역할은 무엇인가?
29. 반환형과 true/false의 의미, 오늘 반환값 사용 위치를 말하라.
30. 호출 뒤 vector의 size/capacity/원소와 user_id가 바뀌는가?
31. 원소 수를 n이라 할 때 최선/최악 비교 횟수와 추가 공간을 말하라.
32. `int == int` 대신 사용자 정의 비교/projection이 예외를 던지면 어떻게 되는가?
33. 검색 중 다른 thread가 같은 vector를 구조 변경하면 어떤 보장이 부족한가?
34. `contains`가 iterator를 반환하지 않는 것이 무효화 위험을 없애는 범위와 없애지 못하는 범위를 말하라.
35. `request.action == required_ && ...`에서 오른쪽 검색이 생략되는 조건은 무엇인가?

## 4. vector 생성·멤버 호출 계약

36. `std::vector<UserId>{7,42,99}`가 선택하는 생성자, 각 입력 값, 반환값 유무를 말하라.
37. initializer-list backing array와 결과 vector 원소 저장소의 수명은 독립인가?
38. `std::vector<std::int64_t> coins(vertex_count)`의 초기 원소 값과 실패 가능성을 말하라.
39. `Graph graph(vertex_count)`에서 바깥·안쪽 vector의 각각 size와 소유권을 말하라.
40. `visited(vertex_count, 0)` fill 생성의 매개변수 두 개와 복잡도를 말하라.
41. `std::vector<std::size_t> order{}`의 size/capacity와 동적 할당 여부 보장을 말하라.
42. `order.reserve(vertex_count)`의 반환형·후조건·예외 보장·관찰자 무효화를 설명하라.
43. `graph[from].push_back(to)`에서 바깥/안쪽 `operator[]`, push 인자, 재할당 범위를 구분하라.
44. `coins[index]`의 유효 전제조건과 위반 시 동작을 말하라.
45. `graph[vertex].size()`의 반환형, 복잡도, 상태 변화를 말하라.
46. `stack.empty()`의 반환값을 `!`로 뒤집은 while 조건을 해석하라.
47. `stack.back()`이 안전한 근거와 반환 참조가 무효화되는 두 사건을 말하라.
48. `stack.pop_back()`의 전제조건, 반환형, 제거 뒤 무효화 범위를 말하라.
49. `std::vector<std::int64_t> best{component_coins}`는 initializer-list인가 copy 생성인가?
50. copy 생성 성공 뒤 두 vector가 같은 저장소를 공유하는가?

## 5. 스트림 계약

51. `std::ios::sync_with_stdio(false)`의 bool 인자·반환값·호출 시점 전제를 말하라.
52. `std::cin.tie(nullptr)`의 pointer 인자와 반환 pointer를 오늘 왜 버리는가?
53. 두 설정이 입출력 속도를 반드시 특정 배수로 빠르게 만든다고 단정할 수 있는가?
54. `std::cin >> vertex_count >> edge_count`에서 각 호출의 반환 참조는 어디에 쓰이는가?
55. 추출 성공/실패 뒤 입력 변수와 stream 상태를 구분하라.
56. `std::cout << answer << '\n'`에서 선택되는 정수/문자 overload와 반환값 사용을 말하라.
57. 출력 실패는 상태 비트와 예외 중 어떤 방식으로 관찰될 수 있는가?
58. stream 설정/호출이 여러 thread의 복합 레코드 원자성을 자동 보장하는가?

## 6. SCC와 압축 DAG 정확성

59. 서로 도달 가능한 관계가 동치 관계인 이유를 반사·대칭·추이성으로 설명하라.
60. 첫 DFS에서 발견 순서가 아니라 종료 순서가 필요한 이유를 작은 그래프로 보이라.
61. `(vertex,next_edge)` frame이 재귀 호출의 어떤 정보를 대체하는가?
62. 종료 순서 역순에서 역그래프 DFS 하나가 다른 미배정 SCC로 새지 않는 이유를 말하라.
63. SCC 내부 모든 동전을 하나의 합으로 바꿔도 되는 이유에 “동전이 양수” 조건이 어떻게 쓰이는가?
64. SCC 압축 그래프에 방향 사이클이 있다고 가정해 모순을 도출하라.
65. 자기 loop는 SCC 합과 압축 DAG에 각각 어떤 영향을 주는가?
66. 같은 SCC 쌍 사이 중복 간선을 유지해도 Kahn indegree가 정확히 0이 되는 이유를 말하라.
67. `best[c]`의 뜻을 한 문장으로 쓰고 초기값이 `component_coins[c]`인 이유를 설명하라.
68. `candidate = best[current] + component_coins[next]`가 표현하는 경로를 말하라.
69. 진입 차수가 0이 되어 queue에 들어갈 때 `best[next]`가 확정되는 이유를 말하라.
70. 그래프가 서로 분리되어 있어도 시작 방이 자유라서 모든 source SCC를 queue에 넣어야 하는 이유는?
71. 전체 시간 `O(n+m)`을 그래프 구성, 두 DFS, 압축, 위상 DP로 나눠 합산하라.
72. 전체 공간 `O(n+m)`에 포함되는 큰 자료구조를 열거하라.
73. 재귀 DFS와 명시적 stack DFS의 최악 보조 공간은 같은 빅오인데도 후자를 택한 실무 이유는?
74. `10^5 * 10^9`를 32-bit int로 누적하면 어떤 문제가 생기며 signed overflow의 언어 규칙은?

## 7. 실제 식 해석

75. `return request.action == required_ && allow_list_->contains(request.user_id);`를 평가 순서대로 풀어라.
76. `return AllowList{std::vector<UserId>{7, 42, 99}};`에서 생성되는 객체와 소유권 이동을 순서대로 써라.
77. `DfsFrame& frame{stack.back()};`에서 참조 바인딩 대상과 유효 기간을 말하라.
78. `const std::size_t next{graph[vertex][frame.next_edge]};`의 두 색인 전제조건을 각각 말하라.
79. `for (std::size_t position{order.size()}; position > 0; --position)`가 unsigned underflow를 피하는 이유는?
80. `order[position - 1]`에서 뺄셈과 접근이 안전한 근거는 무엇인가?
81. `component_coins[component_id] += coins[vertex];`의 두 load, 덧셈, store와 overflow 상한을 설명하라.
82. `--indegree[next]; if (indegree[next] == 0)`가 중복 간선에서도 underflow하지 않는 불변식을 말하라.
83. `queue[head]` 참조를 저장하지 않고 값으로 복사한 뒤 `queue.push_back`하는 이유를 무효화와 연결하라.
84. 오늘 코드에 virtual 함수가 있는가? 없다면 가상 간접 호출을 반드시 만들지 않는다고 말할 수 있는가?
85. 소스 한 줄을 특정 CPU 명령 한 개와 1:1로 단정할 수 없는 이유를 세 가지 이상 말하라.

## 통과 기준

- 85문항 중 기초·수명·호출 계약 문항을 포함해 최소 75개를 근거와 함께 답한다.
- 공식 예제와 경계 테스트의 출력이 왜 맞는지 SCC와 DP 불변식으로 직접 추적한다.
- 호출 계약 답변마다 수신/상태, signature/overload, 인자/값 범주/소유권, 반환/사용,
  사후 상태, 전제·복잡도·할당·무효화·수명·오류·thread 보장을 빠뜨리지 않는다.
