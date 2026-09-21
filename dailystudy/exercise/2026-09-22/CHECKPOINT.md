# 2026-09-22 CHECKPOINT

코드를 보지 않고 먼저 말로 답한 뒤, 막힌 항목만 [`main.cpp`](main.cpp), [`problem.cpp`](problem.cpp), [`icpc_problem.cpp`](icpc_problem.cpp), [`../algorithm/topological-sort.md`](../algorithm/topological-sort.md)을 다시 확인한다. “대충 한다”가 아니라 **타입·값 범주·소유자·수명·반환값·복잡도·실패 방식**을 실제 식과 함께 설명해야 통과다.

## 1. 기초 문법과 객체 모델

1. `long long delta_cents{};`에서 `long long`과 `{}`가 각각 뜻하는 것을 말하고 초기값을 답하라.
2. `bool passed{};`의 초기값은 무엇이며 `if (check.passed)`는 어떤 기계적 제어 흐름이 될 수 있는가?
3. `struct LedgerEntry`와 `class BatchSummary`의 기본 접근 지정자는 각각 무엇인가?
4. 오늘 코드가 입력 레코드에는 `struct`, 불변식이 있는 결과에는 `class`를 쓴 이유를 설명하라.
5. `explicit BatchSummary(long long, std::size_t, std::string)`에서 `explicit`이 막는 암시 변환 예를 하나 써라.
6. `: net_cents_{net_cents}, ...` 멤버 초기화 목록과 생성자 본문의 대입은 객체 수명 관점에서 어떻게 다른가?
7. `const std::vector<LedgerEntry>& entries`에서 `const`, `&`, 매개변수 이름의 역할을 각각 설명하라.
8. `[[nodiscard]]` 반환값을 버렸을 때 언어가 반드시 컴파일을 실패시키는가, 아니면 무엇을 요구하는가?
9. `noexcept`인 관찰자 함수가 예외를 밖으로 내보내면 어떤 일이 일어나는가?
10. `BatchSummary::include(...) &&`의 `&&`는 반환형/매개변수 참조와 무엇이 다른가?
11. `using` 별칭과 새로운 `class` 타입 정의의 차이를 설명하라. `std::vector<LedgerEntry>`의 템플릿 인자도 지목하라.
12. ICPC 풀이의 `for`, `while`, `if`, `continue`가 각각 반복/분기 흐름을 어떻게 바꾸는지 실제 한 줄씩 찾아 설명하라.

## 2. lvalue·prvalue·xvalue, 복사·이동·수명

13. `entries`, `summary`, `entry.source`, `BatchSummary{...}`, `summarize(entries)`의 값 범주를 각각 분류하라.
14. 이름 있는 매개변수의 선언 타입이 `T&&`여도 그 이름을 식으로 쓰면 왜 lvalue인가?
15. `std::move(summary)`가 실제로 수행하는 일과 수행하지 않는 일을 구분하라.
16. `std::move(summary).include(entry)`에서 어떤 참조 한정 overload가 선택되는가?
17. 이동 뒤 원본 객체가 “유효하지만 값 미지정”이라는 표현은 파괴/대입/내용 의존 관점에서 무엇을 허용하는가?
18. `return BatchSummary{...};`에서 C++17의 보장된 prvalue 직접 구성은 무엇을 생략하는가?
19. `return report;`의 `report`가 함수 매개변수일 때 NRVO가 적용되는가? C++23 move-eligible 규칙이 식의 값 범주와 생성자 선택을 어떻게 바꾸는가?
20. vector initializer-list의 원소가 `const`이므로 `LedgerEntry`가 보통 복사된다는 사실을 설명하라.
21. `include`가 `entry.source`를 결과 문자열로 복사하는 이유를 입력과 결과의 독립 수명으로 설명하라.
22. `last_source()` 반환 참조를 `summary` 파괴 뒤 사용하면 어떤 문제가 생기는가?
23. 결과 보고서에 `string_view`를 저장했을 때 `fold_left`가 보고서를 값으로 반환하는 것만으로 입력 문자열 수명이 연장되는가?
24. 함수 포인터 `&fold_entry`는 무엇을 소유하고 무엇을 소유하지 않는가? 가리키는 함수 코드의 수명은 얼마인가?
25. `const&` 입력, 값 누산기, 소유 문자열 결과가 각각 어느 객체의 수명에 묶이는지 그림으로 그려라.

## 3. 표준 라이브러리 호출 계약

26. `std::ranges::fold_left(entries, std::move(initial), &fold_entry)`의 세 인자를 위치별로 타입·값 범주·소유권과 함께 설명하라.
27. 같은 호출의 반환형과 빈 범위에서의 반환값을 설명하라. 반환값은 호출부에서 사용되는가?
28. `fold_left`는 reducer를 몇 번 호출하며 시간 복잡도는 무엇인가? reducer의 문자열 복사 비용은 어디에 더해지는가?
29. fold 도중 reducer가 같은 vector에 `push_back`하면 iterator/참조와 동작에 어떤 위험이 생기는가?
30. reducer 또는 문자열 할당이 예외를 던졌을 때 입력 범위와 이미 일어난 외부 부수 효과는 자동 rollback되는가?
31. `failed_count_ == std::size_t{1}`이 빈 이름도 첫 실패로 보존하는 이유를 설명하고, 그 분기 안 문자열 복사 대입의 수신 타입·인자·반환값·무효화 계약을 말하라.
32. `first_failure_ = check.name`은 어느 string 대입 overload를 고르며, 두 문자열의 소유권과 수명은 호출 뒤 어떻게 되는가?
33. 문자열 복사 대입 뒤 재할당 여부와 무관하게 기존 pointer/reference/iterator는 어떻게 될 수 있는가?
34. vector initializer-list 생성자의 반환값은 무엇인가? 성공/실패 뒤 대상 상태와 예외 가능성을 설명하라.
35. `std::cout << "net=" << value`의 각 삽입 호출은 무엇을 반환하며 왜 연쇄 호출이 가능한가?
36. `operator<<(std::ostream&, char)`에서 char 인자의 값 범주와 소유권, 출력 오류 표현, 복잡도를 말하라.
37. ICPC 코드의 `std::ios::sync_with_stdio(false)`는 이전 상태를 무엇으로 반환하며 호출부가 그 값을 쓰는지 확인하라.
38. `std::cin.tie(nullptr)`의 인자 타입/허용값, 반환값, 호출 뒤 상태, 수명·동시성 주의를 설명하라.
39. ICPC 코드의 첫 vector `count/fill 생성자`를 찾아 원소 수·초깃값·할당/예외/복잡도를 말하라.
40. `graph[u].push_back(v)`의 수신 vector와 인자, 반환형, 크기/용량 변화, 재할당 무효화, 상각 복잡도를 설명하라.
41. `queue::front()`와 `queue::pop()`의 전제조건을 말하라. 빈 queue에서 수행하면 왜 안전하지 않은가?
42. 경로 vector의 `size()` 또는 인덱싱 호출이 돌려주는 타입과 범위 전제조건을 실제 식으로 설명하라.

## 4. DAG 최장 경로 알고리즘

43. 위상 순서의 정의를 간선 `u -> v`에 대한 문장으로 써라.
44. Kahn 알고리즘에서 `indegree[v]`가 의미하는 “아직 남은” 간선 집합을 정확히 말하라.
45. 준비 queue에는 어떤 정점만 들어가며, 각 정점이 최대 한 번만 들어가는 이유는 무엇인가?
46. DAG에는 남은 부분 그래프가 비어 있지 않다면 왜 진입 차수 0인 정점이 적어도 하나 존재하는가?
47. `distance[u]`가 도달 불가 sentinel이면 `u -> v`를 완화하지 않아야 하는 이유를 설명하라.
48. 정점 `u`를 위상 순서에서 처리할 때 `distance[u]`가 이미 최종 최장값인 이유를 귀납적으로 증명하라.
49. `distance[u] + 1 > distance[v]`일 때 `parent[v]`도 함께 바꿔야 하는 이유는 무엇인가?
50. 동률 `==`에서는 parent를 바꾸지 않아도 정답인 이유와, 사전순 최소 경로가 요구된다면 왜 추가 규칙이 필요한지 말하라.
51. n번에서 parent를 거슬러 만든 순서는 왜 역순인가? 오늘 구현이 별도 `reverse` 없이 이를 어떻게 저장하는가?
52. n번은 다른 컴포넌트에서도 들어오는 간선을 가질 수 있다. 그래도 1번에서 도달 불가인지 판별하는 기준은 무엇인가?
53. 전체 시간 `O(n+m)`을 정점 처리 횟수, queue 연산 횟수, 간선 완화 횟수로 나누어 증명하라.
54. 전체 공간 `O(n+m)`을 인접 리스트, 진입 차수, DP, parent, queue, 경로로 나누어 설명하라.
55. 재귀 DFS 대신 Kahn을 쓰면 `n=100,000` 사슬에서 어떤 호출 스택 위험을 피하는가?
56. 문제의 DAG 보장이 없다면 처리 정점 수로 순환을 어떻게 검출할 수 있는가?

## 5. 실기 검증

57. `main.cpp`의 세 번째 금액을 `-900`으로 바꾸고 예상 출력과 실제 출력을 비교하라.
58. `problem.cpp`를 “첫 실패”가 아니라 “마지막 실패”를 소유하도록 고치고, 왜 string 수명이 안전한지 설명하라.
59. 빈 원장/빈 점검 vector를 fold할 때 identity 결과가 무엇인지 테스트를 추가하라.
60. ICPC 입력 `4 3 / 1 2 / 2 3 / 3 4`의 DP와 parent 배열을 각 위상 단계 뒤에 손으로 적어라.
61. `1`에서 `n`으로 갈 수 없지만 다른 정점에서 `n`으로 가는 입력을 만들고 `IMPOSSIBLE`을 확인하라.
62. 길이가 같은 두 최장 경로가 있는 DAG를 만들고 출력된 경로가 유효하며 최대 길이인지 검증하라.
63. 작은 DAG에서 모든 1→n 단순 경로를 열거하는 oracle을 작성해 실행 파일과 무작위 대조하라.
64. `n=100,000` 사슬을 생성해 재귀 없이 제한 시간/메모리 안에 끝나는지 확인하라.

## 통과 기준

- 1~56번을 코드 없이 구체적인 타입과 식으로 설명한다.
- 26~42번은 여섯 요소(수신 상태, 시그니처/overload, 인자, 반환, 사후 상태, 전제·복잡도·할당/무효화/수명/오류/스레드)를 빠뜨리지 않는다.
- 48번은 “DAG라서 된다” 한 문장이 아니라 위상 순서 불변식으로 증명한다.
- 57~64번 중 적어도 네 개를 실제 컴파일/실행하고, 63번 무작위 oracle 대조는 반드시 통과한다.
