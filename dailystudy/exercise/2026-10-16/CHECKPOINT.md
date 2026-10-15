# 2026-10-16 초보자 검증 체크포인트

코드를 외우지 말고 객체·소유권·수명 그림을 먼저 그린다. 표준 라이브러리 호출 문제는 반드시
**수신 객체와 호출 전 상태, 정확한 overload/template 인자, 모든 인자의 타입·값 범주·소유권,
반환형과 사용 여부, 호출 뒤 상태, 전제·후조건·복잡도·할당·무효화·수명·오류·thread 보장**을 포함한다.

## 1. 기초 문법

1. `struct Ledger`와 `class LedgerService`의 기본 접근 지정자는 각각 무엇인가?
2. `AccountId`는 새 강한 타입인가, `std::size_t`의 다른 이름인가?
3. `int amount{};`의 초기값은 무엇이며 미초기화 선언보다 안전한 이유는 무엇인가?
4. `TransferCommand{0, 1, 30}`에서 세 값이 어느 멤버에 들어가는지 선언 순서로 말하라.
5. 중괄호 초기화가 narrowing 변환을 막는 예를 하나 만들어라.
6. `explicit LedgerService(Ledger& ledger)`가 막는 암시 변환은 무엇인가?
7. `: ledger_{ledger}`가 생성자 본문 안 대입과 다른 이유를 참조 멤버 관점에서 설명하라.
8. `Ledger&`, `const Ledger&`, `Ledger*`, `std::unique_ptr<Ledger>`의 null 가능성과 소유권을 비교하라.
9. `bool apply(const TransferCommand& command)`의 반환형과 매개변수 계약을 설명하라.
10. `view() const noexcept`에서 뒤의 `const`와 `noexcept`가 각각 약속하는 것은 무엇인가?
11. `[[nodiscard]]`는 실행 시간 동작인가 컴파일러 진단 요청인가?
12. `template<class ExitFunction>`에서 template 인자가 결정하는 저장 멤버 타입은 무엇인가?
13. `ScopeExit rollback{lambda};`에서 CTAD가 추론하는 것은 무엇인가?
14. 복사 생성자/대입을 `= delete`한 이유를 rollback 중복 실행과 연결해 설명하라.
15. `if`, `for`, `continue`, `return`, 조건 연산자 `?:`의 제어 흐름을 각각 한 문장으로 설명하라.

## 2. RAII와 트랜잭션 수명

16. RAII가 자원 획득뿐 아니라 “실패 시 상태 복구”에도 적용되는 이유는 무엇인가?
17. `before`를 `rollback`보다 먼저 선언해야 하는 이유를 지역 객체 파괴 역순으로 설명하라.
18. 색인·금액·출금 잔액 사전 검증의 `return false`에서는 왜 `before`와 `rollback` 소멸자가 실행되지
    않으며, 기존 vector 원소 관찰자도 유지되는가?
19. 성공 경로에서 `rollback.release()`가 바꾸는 단 하나의 상태는 무엇인가?
20. `release()` 뒤에도 lambda 객체가 guard 안에 남아 있는데 실행되지 않는 이유는 무엇인가?
21. rollback lambda가 `this`를 값으로 캡처할 때 무엇을 소유하고 무엇을 소유하지 않는가?
22. `&before` 캡처가 guard보다 먼저 dangling이 되지 않는 이유는 무엇인가?
23. scope-exit callback이 `noexcept` 소멸자 밖으로 예외를 던지면 어떤 문제가 생기는가?
24. snapshot 복사가 실패하면 원장이 이미 바뀌었는가? 강한 예외 보장 관점에서 답하라.
25. 출금 차감 뒤 목적지 상한 검사가 실패할 때 값 복구 순서와 기존 vector 원소 관찰자 무효화를
    설명하고, `maximum_balance - command.amount` 검사가 뒤의 signed 덧셈 overflow를 막는 이유를 증명하라.
26. `problem.cpp`에서 첫 차감 뒤 두 번째 검사가 실패하면 `remaining_` 값이 어떻게 변하는지 시간순으로 적어라.
27. 같은 `Quota` 객체를 두 thread에서 동시에 바꾸면 ScopeExit만으로 안전해지는가?

## 3. 값 범주·복사/이동·소유권

28. 이름 있는 `exit_function`, `before`, `current` 식의 값 범주는 무엇인가?
29. lambda 식, `TransferCommand{...}`, `make_unique` 반환식은 어떤 값 범주인가?
30. `std::move(exit_function)`이 수행하는 일과 실제로 수행하지 않는 일을 구분하라.
31. `exit_function_`의 이동 생성은 어느 식이 촉발하며 moved-from 매개변수는 언제 파괴되는가?
32. `std::vector<int> before{ledger_.balances}`는 복사와 이동 중 어느 것인가? 두 저장소는 독립인가?
33. `ledger_.balances = std::move(before)` 뒤 각 vector의 유효 상태와 버퍼 소유자를 설명하라.
34. vector 이동 대입이 기존 `ledger_.balances` 원소 관찰자를 무효화하는 이유는 무엇인가?
35. `make_unique<ColorBag>(...)` 반환 prvalue와 `current` 초기화에서 보장 복사 생략이 적용되는 부분은 무엇인가?
36. 보장된 prvalue 직접 초기화와 이름 있는 local의 NRVO는 같은 보장인가?
37. `current.swap(bags[child])`가 `ColorBag`과 hash node 자체를 이동하는가?
38. `bags[vertex] = std::move(current)` 뒤 `current`와 pointee의 상태를 각각 설명하라.
39. `unique_ptr::operator->` 반환 raw pointer가 pointee 수명을 늘리는가?

## 4. `std::vector` 호출 계약

40. `vector<int>(count, 0)`에서 두 위치 인자의 타입·값과 생성 후 원소 수를 설명하라.
41. `vector<vector<int>>(count)`에서 내부 vector들은 어떤 상태로 시작하는가?
42. `vector<unique_ptr<ColorBag>>(count)`가 ColorBag도 count개 생성하는가?
43. `colors[index]`가 반환하는 형식과 유효 색인 전제조건은 무엇인가?
44. 범위 밖 `operator[]`는 예외를 던지는가, 미정의 동작인가?
45. `graph[from].push_back(to)`에서 outer `operator[]` 반환값은 다음 호출에서 어떻게 사용되는가?
46. `push_back(const int&)`가 재할당하면 어느 iterator/reference/pointer가 무효화되는가?
47. `order.reserve(n)`가 `size`와 `capacity`에 각각 어떤 영향을 주는가?
48. reserve 실패 시 기존 vector 상태와 관찰자는 어떻게 되는가?
49. `order.size()` 반환형·복잡도·예외 여부·상태 변화를 말하라.
50. `ScopeExit` 예제의 initializer-list vector 생성과 ICPC 풀이의 count/value 생성자를 구분하라.

## 5. 해시 집합과 소유 포인터 호출 계약

51. `std::unordered_set<int>`의 명시·기본 template 인자를 모두 말하라.
52. 기본 생성 뒤 size, bucket 저장 가능성, 소유 key 상태를 설명하라.
53. `colors.insert(color)`가 선택하는 overload, 반환 `pair`의 두 요소와 오늘의 사용 여부를 말하라.
54. insert의 평균/최악 복잡도를 hash 충돌과 연결하라.
55. insert가 rehash하면 iterator와 pointer/reference는 각각 어떻게 되는가?
56. `unordered_set::merge(source)`의 정확한 수신 객체와 source 인자 값 범주를 말하라.
57. merge가 목적지에 이미 존재하는 key를 source에서 제거하는가?
58. merge 성공으로 옮겨진 node에서 key 객체의 복사/이동 생성자가 호출되는가?
59. merge 전 두 allocator가 같아야 하는 이유와 오늘 코드가 이를 만족하는 근거를 말하라.
60. merge 뒤 옮겨진 원소의 pointer/reference와 iterator, 목적지의 기존 iterator는 어떻게 되는가?
61. source에 남은 중복 node가 언제 파괴되는가?
62. `unordered_set::size()`의 반환형이 `int`가 아닌 이유와 오늘 변환이 안전한 이유는 무엇인가?
63. `make_unique<ColorBag>`의 template 인자, 전달 인자, 반환형, 할당 실패를 설명하라.
64. `unique_ptr::swap` 호출 뒤 deleter/pointer 소유 관계와 pointee 주소를 설명하라.
65. `unique_ptr::reset()`의 생략된 기본 인자와 호출 뒤 pointer 상태를 설명하라.
66. reset 뒤 source bag을 가리키던 관찰자가 왜 모두 무효인지 설명하라.

## 6. 스트림 호출 계약

67. `std::ios::sync_with_stdio(false)`의 매개변수와 반환값을 오늘 코드는 어떻게 처리하는가?
68. 이 설정을 첫 입출력 전에 해야 하는 이유는 무엇인가?
69. `std::cin.tie(nullptr)`의 인자/반환 pointer와 호출 뒤 연결 상태를 설명하라.
70. `!(std::cin >> node_count)`에서 `operator>>`와 `operator!` 반환값이 어떻게 이어지는가?
71. 정수 추출 실패 시 대상 정수, 입력 위치와 stream 상태 비트가 어떻게 될 수 있는가?
72. `std::cout << answer[index]`가 선택하는 overload와 반환 `ostream&` 사용 여부를 말하라.
73. 이어 출력하는 char overload의 인자 값 범주와 실패 알림 방법을 설명하라.
74. stream 오류가 앞서 기록한 문자를 자동 rollback하는가?

## 7. small-to-large 알고리즘

75. 왜 부모-선행 `order`를 역순으로 읽으면 모든 자식 bag이 먼저 완성되는가?
76. 무방향 인접 리스트에서 `parent[child] == vertex`인 이웃만 합쳐야 하는 이유는 무엇인가?
77. 처리 시작 직후 current bag의 `colors`와 `represented_nodes` 값을 말하라.
78. `represented_nodes`를 distinct color 수와 별도로 두는 이유는 무엇인가?
79. source population이 destination 이하일 때 merge 후 source 정보의 대표 population이 왜 두 배 이상인가?
80. 각 원래 정점이 작은 쪽에 속하는 횟수가 `O(log n)`인 과정을 `1,2,4,...`로 적어라.
81. source의 서로 다른 색 수가 source population보다 클 수 없는 이유는 무엇인가?
82. 평균 `O(n log n)` 시간 상한을 전체 hash 검사 횟수로 유도하라.
83. 한 merge의 표준 최악 `O(source_size * destination_size + source_size)`를 전체 트리에서 합치면
    왜 보수적으로 `O(n^2)`까지 악화될 수 있는가?
84. `std::set` 대안의 결정적 시간 상한과 예상 성능 상수를 비교하라.
85. 재귀 DFS 대신 cursor 기반 FIFO vector 순서를 사용한 이유를 최대 깊이 200,000과 연결하고,
    이 순서와 명시적 stack DFS가 공유해야 하는 부모-선행 불변식을 말하라.
86. 모든 색이 같은 사슬, 모두 다른 별, 중복 색이 많은 균형 트리에서 답과 merge 동작을 비교하라.

## 8. 실제 식 해석

87. `ScopeExit rollback{[this, &before]() noexcept { ... }};`를 타입 추론, capture, 값 범주, 수명으로 나눠 읽어라.
88. `ledger_.balances = std::move(before);`에서 수신 객체, 함수 인자, 두 반환값 중 사용 여부를 설명하라.
89. `if (command.from >= count || ... )`의 단락 평가와 조기 `return`이 뒤의 범위 밖 접근을 어떻게 막는가?
90. `std::cout << first << ' ' << second`를 여러 함수 호출의 연쇄로 풀어 써라.
91. `auto current{std::make_unique<ColorBag>(colors[index])};`의 template 인자와 결과 객체를 설명하라.
92. `current->colors.merge(bags[child]->colors);`에서 두 `operator->`와 merge 수신/인자를 구분하라.
93. `answer[index] = static_cast<int>(current->colors.size());`의 두 표준 호출과 정수 변환 근거를 말하라.
94. `std::cout << (vertex == node_count ? '\n' : ' ');`에서 조건식과 두 결과식의 공통 타입은 무엇인가?

## 통과 기준

- 1~39번을 코드 없이 설명하고 owner/observer 수명 그림을 그린다.
- 40~74번 중 임의의 표준 호출 여섯 개를 골라 6항목 호출 계약을 빠짐없이 말한다.
- 75~86번의 불변식·정확성·평균/최악 복잡도를 작은 트리로 증명한다.
- 87~94번에서 lvalue/prvalue/xvalue, 반환값 사용, 무효화와 오류 경로를 표시한다.
- `main.cpp`, `problem.cpp`, 공식 예제 출력을 실행 전에 예측하고 CTest 전체를 통과한다.
