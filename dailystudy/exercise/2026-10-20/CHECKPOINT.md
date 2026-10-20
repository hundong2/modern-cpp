# 2026-10-20 초보자 검증 체크포인트

코드를 외우지 말고 먼저 **누가 문자열과 hash node를 소유하는지**, **어떤 참조·iterator가 빌리는지**를
그린다. 표준 라이브러리 호출은 반드시 다음 여섯 항목으로 답한다.

1. 정확한 수신 객체 타입과 호출 직전 상태
2. 선택된 시그니처/overload/template 인자
3. 모든 인자의 식·타입·값 범주·소유권 의미·허용값
4. 반환형·반환값 의미·사용 또는 폐기 여부
5. 호출 뒤 수신 객체와 인자의 상태
6. 전제/후조건, 복잡도, 할당, 무효화, 수명, 오류·예외·UB, thread 보장

## 1. 기초 문법과 타입

1. `<unordered_map>`, `<string>`, `<utility>`, `<vector>`, `<iostream>`, `<cstddef>`가 각각 무엇을 선언하는가?
2. `EndpointPriority priority{};`와 `RetryLimit retry_limit{};`의 초기값을 말하라.
3. 중괄호 초기화가 `double`에서 `int`로의 narrowing을 막는 짧은 예를 만들어라.
4. `using Storage = std::unordered_map<std::string, Endpoint>;`는 새 타입을 만드는가?
5. `Storage`의 명시적 template 인자 두 개와 생략된 hash/equality/allocator 정책을 말하라.
6. `struct Endpoint`와 `class EndpointRegistry`의 기본 접근 지정자는 각각 무엇인가?
7. 단순 설정 값에는 `struct`, 불변식을 가진 저장소에는 `class`를 쓴 이유는 무엇인가?
8. `public:`과 `private:`가 호출자가 볼 수 있는 API와 내부 표현을 어떻게 나누는가?
9. `explicit Endpoint(std::string address_value, EndpointPriority priority_value)`가 두 인자 copy-list 초기화/암시 변환을 어떻게 막는가?
10. `: address{std::move(address_value)}, priority{priority_value}`가 생성자 본문 대입과 다른 이유는 무엇인가?
11. 실제 멤버 초기화 순서는 초기화 목록 순서와 선언 순서 중 어느 것을 따르는가?
12. `bool register_endpoint(...)`의 반환형과 각 값 매개변수 타입을 말하라.
13. 값 매개변수에 lvalue와 prvalue 문자열을 넘길 때 복사/이동 선택이 어떻게 달라질 수 있는가?
14. `size() const` 뒤의 `const`는 어느 객체의 어떤 변경을 금지하는가?
15. `[[nodiscard]]`는 실행 시간 기능인가, 컴파일러 진단 요청인가?
16. `std::size_t`와 `int`의 signedness를 비교하고 컨테이너 크기에 전자를 쓰는 이유를 말하라.
17. `if`, `for`, `while`, `continue`, `return`의 제어 흐름을 각각 한 문장으로 설명하라.
18. range-`for`가 소스에 쓰이지 않은 `begin`, `end`, 역참조, 증가, 비교를 수행한다는 뜻은 무엇인가?

## 2. 포인터·참조·소유권·수명

19. 비교용 일반 개념으로 `std::string&`, `const std::string&`, `std::string*`, `std::unique_ptr<std::string>`의 null 가능성과 소유권을 비교하라.
20. 값 매개변수 `endpoint_name`은 호출자의 원본 문자열을 빌리는가, 독립 객체를 소유하는가?
21. map node 안의 `const std::string` key와 `Endpoint`는 누가 언제까지 소유하는가?
22. `try_emplace`가 반환한 iterator는 소유자인가 비소유 관찰자인가?
23. 함수가 iterator를 밖으로 반환하지 않고 bool/size 값만 돌려주는 수명상 장점은 무엇인가?
24. map 파괴 뒤 이전에 얻은 iterator, pointer, reference는 각각 사용할 수 있는가?
25. erase된 원소를 가리키는 pointer/reference와 다른 원소를 가리키는 관찰자는 어떻게 되는가?
26. rehash가 iterator를 무효화하면서 원소 pointer/reference는 유지하는 계약을 구분해 설명하라.
27. 같은 `unordered_map`을 한 thread가 쓰고 다른 thread가 읽으면 왜 외부 동기화가 필요한가?
28. 서로 다른 컨테이너 객체를 서로 다른 thread에서 조작하는 경우와 같은 객체를 공유하는 경우를 구분하라.

## 3. 값 범주·복사·이동·복사 생략

29. 이름 있는 `registry`, `endpoint_name`, `endpoint_address`, `insertion` 식의 값 범주는 무엇인가?
30. `std::string{"billing"}`와 `EndpointRegistry{}`의 값 범주는 무엇인가?
31. `std::move(endpoint_name)`의 반환 식은 어떤 값 범주이며 실제로 무엇을 하지 않는가?
32. 실제 key 문자열 이동 생성은 어느 조건에서 어느 객체를 만들 때 일어나는가?
33. 중복 key 경로에서 `endpoint_name`과 mapped 생성자 인자에서 이동하지 않는 이유를 `try_emplace` 계약으로 설명하라.
34. 그럼에도 `expensive_factory()` 인자 식은 중복 key에서 왜 실행되는가?
35. lvalue 문자열을 값 매개변수에 넘긴 뒤 함수 안에서 이동해도 호출자의 원본이 유지되는 이유는 무엇인가?
36. prvalue 문자열을 값 매개변수에 넘길 때 불필요한 중간 객체가 생략될 수 있는 지점을 말하라.
37. `return EndpointRegistry{};`의 prvalue가 호출자 객체를 직접 초기화하는 보장을 설명하라.
38. 보장된 prvalue 복사 생략과 이름 있는 지역 변수 반환의 선택적 NRVO는 같은가?
39. 삽입 성공 뒤 함수 매개변수의 moved-from 문자열에 대해 무엇까지 보장되며 무엇을 가정하면 안 되는가?
40. 중복 실패 뒤 매개변수 문자열이 이동되지 않았더라도 함수 종료 시 파괴되는 이유는 무엇인가?

## 4. `std::unordered_map::try_emplace` 호출 계약

41. 오늘 map의 정확한 `key_type`, `mapped_type`, `value_type`을 말하라.
42. `try_emplace(std::move(endpoint_name), std::move(endpoint_address), priority_value)`가 고르는 key rvalue overload의 형태를 적어라.
43. 첫 번째 인자 식의 타입·값 범주·소유권 의미를 말하라.
44. mapped 생성자 인자 pack의 정확한 추론 타입과 값 범주를 말하고, `priority_value`의 업무 범위를 오늘 코드가 별도로 검증하는지도 답하라.
45. 반환형 `std::pair<iterator, bool>`의 `first`와 `second`가 각각 뜻하는 것은 무엇인가?
46. 삽입 성공 시 반환 iterator와 bool 값, map size, key/mapped 객체 상태를 말하라.
47. 중복 key 시 반환 iterator와 bool 값, 기존 mapped 값, 인자 상태를 말하라.
48. 중복 key에서 기존 mapped 값을 덮어쓰지 않는 점을 `insert_or_assign`과 비교하라.
49. `emplace`와 달리 `try_emplace`가 move-only mapped 인자를 재사용하기 좋은 이유는 무엇인가?
50. 평균 복잡도 `O(1)`과 표준이 허용하는 최악 `O(n)`의 원인을 hash 충돌로 설명하라.
51. 새 node 삽입이 allocation과 rehash를 일으킬 수 있는 조건은 무엇인가?
52. rehash가 일어나면 호출 전에 얻은 iterator와 pointer/reference는 각각 어떻게 되는가?
53. string/equality/mapped 생성·할당 예외의 no-effect 보장과 사용자 `Hash`가 던지는 경우의 예외를 구분하라.
54. key와 mapped 생성자가 던지지 않아도 allocation은 어떤 예외를 낼 수 있는가?
55. hash와 equality가 동등성 규칙을 일관되게 만족해야 하는 이유는 무엇인가?
56. 같은 key를 동시에 두 thread가 try_emplace하면 데이터 경쟁을 막아 주는 내부 원자성이 보장되는가?
57. `insertion.second`는 함수 호출인가 단순 데이터 멤버 접근인가?
58. `insertion.first` iterator를 사용하지 않는 오늘 API가 왜 rehash-safe한 경계를 만드는가?

## 5. 관련 표준 라이브러리 계약

59. 오늘의 정확한 `std::unordered_map<std::string, Endpoint>` 기본 생성 직후 size와 원소 상태를 말하라. bucket 수를 특정 값으로 가정해도 되는가?
60. 오늘 기본 hash/equality/allocator 정책에서 기본 생성자가 `noexcept`인 이유와 정책 객체 수명을 설명하라.
61. `std::string{"billing"}`이 고르는 C 문자열 생성자의 입력 전제는 무엇인가?
62. null pointer 또는 null 종료되지 않은 문자 배열을 그 생성자에 넘기면 왜 위험한가?
63. 문자열 생성의 시간/공간 복잡도와 allocation 실패를 설명하라.
64. `std::move`의 template 인자 추론, 매개변수, 반환형, `noexcept`와 상태 불변을 말하라.
65. `std::string` 이동 생성 뒤 목적 문자열과 원본 문자열의 상태·수명을 말하라.
66. `unordered_map::size()`의 정확한 반환형, 복잡도, 상태 변화, 예외 여부를 말하라.
67. 확장 연습으로 `std::pair<iterator, bool>`을 구조적 바인딩으로 받는다면 숨은 `get`과 참조 성질이 어떻게 되는가?
68. `std::cout << first_insert << ' '` 연쇄에서 bool/size/char 각각 어느 계열 overload가 선택되는가?
69. 각 `operator<<`의 반환 `std::ostream&`가 다음 삽입에서 어떻게 사용되는가?
70. 출력 실패는 반환 bool이 아니라 어떤 stream 상태와 설정 시 예외로 보고되는가?
71. 출력 연쇄가 이전에 쓴 문자를 rollback하거나 한 레코드 thread 원자성을 보장하는가?

## 6. `std::vector`와 입력 stream 계약

72. `std::vector<std::vector<Vertex>>(node_count_size)`는 바깥과 안쪽 vector를 각각 어떤 상태로 만드는가?
73. `parent(n, -2)`와 `subtree_size(n, 1)`의 count/value 생성자 인자와 사후 상태를 설명하라.
74. `order.reserve(n)`가 size와 capacity에 각각 미치는 영향을 말하라.
75. `push_back`이 재할당할 때 어느 iterator/reference/pointer가 무효화되는가?
76. `operator[]`의 반환형, `O(1)` 복잡도, 유효 색인 전제와 위반 시 UB를 설명하라.
77. `back()`과 `pop_back()`의 non-empty 전제조건을 말하라.
78. `pop_back()` 뒤 마지막 원소를 가리키던 관찰자와 나머지 원소 관찰자는 어떻게 되는가?
79. `empty()`와 `size()`의 반환형·복잡도·상태 변화·예외 여부를 비교하라.
80. `std::ios::sync_with_stdio(false)`의 인자, 반환값 사용 여부, 호출 시점과 thread 주의를 설명하라.
81. `std::cin.tie(nullptr)`의 수신 객체, 인자, 반환 pointer와 호출 뒤 상태를 설명하라.
82. `std::cin >> vertex_count`의 반환 참조, 성공/형식 오류/범위 초과/EOF 때 상태 변화를 설명하라.
83. stream exceptions mask가 설정됐을 때 입력·출력 실패가 어떤 예외 경로를 가질 수 있는가?

## 7. 트리 센트로이드 알고리즘

84. 트리 센트로이드의 정의를 “정점을 제거한 뒤 연결 성분”으로 말하라.
85. 문제 설명의 “루트로 삼았을 때 각 부분 트리” 정의와 제거 성분 정의가 같은 이유는 무엇인가?
86. 반복 DFS의 `parent`와 `order`가 만족해야 하는 부모-선행 불변식을 말하라.
87. `order`를 역순으로 읽을 때 자식 부분 트리 크기가 이미 완성된 이유를 증명하라.
88. 정점 `v`의 자식 방향 조각과 부모 방향 조각의 크기 공식을 각각 적어라.
89. 가장 큰 조각이 `floor(n/2)` 이하이면 충분조건이면서 필요조건인 이유를 설명하라.
90. 모든 정점에서 모든 연결 성분을 새로 BFS하는 풀이의 시간 복잡도는 얼마인가?
91. 오늘 풀이에서 각 정점과 간선을 몇 단계에서 몇 번 읽으므로 `O(n)`인지 설명하라.
92. `n=1`일 때 부모, 순서, 부분 트리 크기와 출력 답을 추적하라.
93. 사슬 `1-2-3-4`의 센트로이드 둘을 찾고 오늘 구현이 어느 쪽을 고르는지 추적하라.
94. 별 트리에서 중심과 잎의 가장 큰 분할 조각 크기를 비교하라.
95. 센트로이드가 항상 존재하고 최대 두 개이며 둘이면 서로 인접한다는 사실을 직관적으로 설명하라.
96. 깊이 `200,000` 사슬에서 재귀 대신 명시적 vector stack을 쓰는 이식성 이유는 무엇인가?

## 8. 실제 식 해석

97. `endpoints_.try_emplace(std::move(endpoint_name), std::move(endpoint_address), priority_value)`를 평가 순서, 값 범주, 조건부 구성으로 나눠 읽어라.
98. `return EndpointRegistry{};`와 `EndpointRegistry registry{make_endpoint_registry()};`의 보장 복사 생략을 설명하라.
99. `std::cout << first_insert << ' ' << duplicate_insert << ' ' << count << '\n';`을 호출 연쇄로 풀어라.
100. `graph[from].push_back(to)`에서 outer `operator[]` 반환 참조와 inner `push_back` 수신 객체를 구분하라.
101. `subtree_size[static_cast<std::size_t>(parent_vertex)] += subtree_size[static_cast<std::size_t>(vertex)]`의 두 색인 전제와 load/add/store 가능성을 설명하라.
102. `if (largest_piece <= node_count_ / 2)`의 정수 나눗셈, 비교, 조건 분기와 센트로이드 정의를 연결하라.

## 통과 기준

- 1~40번을 코드 없이 설명하고 owner/observer/수명 그림을 그린다.
- 41~83번 중 임의의 표준 호출 여덟 개를 골라 위 6항목 계약을 빠짐없이 말한다.
- 84~96번을 작은 트리 세 개에 적용하고 정확성 및 `O(n)` 시간·공간을 증명한다.
- 97~102번에서 각 식의 lvalue/prvalue/xvalue, 반환값 사용, 무효화와 오류 경로를 표시한다.
- `main.cpp`, `problem.cpp`, ICPC 공식 예제 출력을 실행 전에 예측하고 CTest 전체를 통과한다.
