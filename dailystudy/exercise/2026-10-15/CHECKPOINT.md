# 2026-10-15 초보자 검증 체크포인트

답을 코드에서 그대로 베끼지 말고 먼저 말과 그림으로 설명한다. 각 호출 문제는 **수신 객체, 선택된
오버로드, 위치별 인자, 반환값 사용 여부, 호출 뒤 상태, 복잡도·할당·무효화·수명·오류·스레드 보장**을
빠뜨리지 않아야 통과다.

## 1. 기초 문법

1. `struct LegacyMetrics`와 `class AtomicCounterView`에서 아무 접근 지정자도 쓰지 않았을 때 기본 접근은
   각각 무엇인가?
2. `std::uint64_t completed{};`와 `std::uint64_t next_id{1};`의 초기값은 무엇인가?
3. 중괄호 초기화가 미초기화 기본 정수와 narrowing 실수를 줄이는 이유를 설명하라.
4. `using Value = std::uint64_t;`는 새 강한 타입을 만드는가? `Value`와 원래 타입의 관계는 무엇인가?
5. `std::atomic_ref<Value>`의 template 인자 `Value`가 결정하는 것은 무엇인가?
6. `explicit AtomicCounterView(Value& storage)`의 `explicit`이 막는 변환은 무엇인가?
7. `: completed_{storage}`가 생성자 본문 안 대입과 다른 점은 무엇인가?
8. `Value&`와 `Value*`의 null 가능성, 재바인딩, 수명 연장 여부를 비교하라.
9. `increment() const`에서 `const`인 것은 wrapper인가 underlying 숫자인가?
10. 함수 반환형 `void`, `Value`, `CounterReport`가 호출자에게 주는 결과 차이를 설명하라.
11. `for (std::uint64_t i{}; i < iterations; ++i)`의 네 실행 단계를 순서대로 말하라.
12. `return condition ? 0 : 1;`에서 조건식과 두 결과식의 역할은 무엇인가?

## 2. 값 범주·복사/이동·수명·소유권

13. 식 `storage.completed`가 lvalue인 이유는 무엇인가?
14. 식 `AtomicCounterView{storage.completed}`는 어떤 값 범주이며 무엇을 소유하는가?
15. 이름 있는 `counter`의 선언 타입이 이동 가능한 타입이어도 식 `counter` 자체는 어떤 값 범주인가?
16. `std::move(counter)`가 실제로 수행하는 일과 수행하지 않는 일을 각각 말하라.
17. `AtomicCounterView` 복사 생성 뒤 두 wrapper가 서로 다른 숫자 복사본을 가리키는가?
18. 오늘 wrapper 이동 생성 뒤 source가 같은 binding을 계속 가진다는 사실이 owning `vector` 이동과 어떻게
    다른가?
19. lambda의 값 capture `ids = std::move(first_view)`가 소유하는 것과 소유하지 않는 것을 구분하라.
20. `CounterReport{counter_.load()}` 반환에서 C++17 보장 복사 생략이 적용되는 결과 객체를 그려라.
21. 이름 있는 local을 `return local;` 하는 NRVO와 20번의 prvalue 직접 초기화는 같은 보장인가?
22. `collect()`가 반환되기 전에 사라지는 객체를 생성 역순으로 적어라.
23. `LegacyMetrics`가 atomic view보다 먼저 파괴되면 어떤 종류의 오류가 되는가?

## 3. `std::atomic_ref` 호출 계약

24. `std::atomic_ref<Value>(storage)` 생성자의 정확한 인자 타입·값 범주·소유권 의미를 말하라.
25. 생성자의 반환형이 없는 이유와 호출 뒤 `storage` 값이 바뀌는지 설명하라.
26. `Value`가 만족해야 하는 타입 조건은 무엇인가?
27. `required_alignment`는 왜 `alignof(Value)`와 같다고 단정할 수 없는가?
28. atomic_ref가 하나라도 살아 있는 epoch에 `storage`를 plain read하면 왜 안 되는가?
29. 복사 생성한 atomic_ref의 원자 연산이 같은 modification order에 참여하는 이유는 무엇인가?
30. atomic_ref 복사 대입이 삭제되어 있다는 사실은 재바인딩 설계에 어떤 제약을 주는가?
31. `fetch_add(1, std::memory_order_relaxed)`에서 수신 객체의 정확한 타입과 호출 전 상태를 설명하라.
32. 첫째 인자 `1`의 타입 변환, 값, 값 범주, 소유권 의미를 설명하라.
33. 둘째 인자는 데이터 값인가 순서 정책인가? `relaxed`가 허용되는 이유는 무엇인가?
34. `fetch_add`는 증가 전 값과 증가 후 값 중 무엇을 반환하는가? 두 소스는 그 값을 각각 어떻게 쓰는가?
35. 호출 뒤 handle binding과 underlying 값은 각각 어떻게 되는가?
36. 정수 최대값에서 `fetch_add`가 수행하는 동작과 포화 덧셈의 차이는 무엇인가?
37. `load(std::memory_order_relaxed)`의 반환형·의미·사용 위치를 설명하라.
38. load가 허용하지 않는 memory order 두 가지와 그 이유를 말하라.
39. atomic_ref 연산이 항상 lock-free 한 CPU 한 명령이라고 말할 수 없는 이유는 무엇인가?
40. 잘못된 정렬·끝난 owner 수명·plain/atomic 혼합은 예외와 UB 중 어느 쪽인가?
41. atomic_ref가 `LegacyMetrics`의 다른 필드까지 자동으로 보호하는가?

## 4. `std::jthread`, 완료 동기화와 기계 실행

42. 두 jthread 생성에서 callable과 capture가 값/참조 중 어떤 방식으로 전달되는지 구분하라.
43. lambda가 `std::stop_token`을 받지 않을 때 선택되는 jthread 호출 형태는 무엇인가?
44. 둘째 thread 생성이 예외를 던져도 첫째 thread가 누수되지 않는 이유는 무엇인가?
45. joinable jthread 소멸자가 수행하는 두 단계를 순서대로 말하라.
46. 오늘 lambda가 stop 요청을 직접 관찰하지 않아도 종료하는 이유는 무엇인가?
47. worker의 쓰기 완료를 뒤의 `load`/plain read가 볼 수 있게 하는 관계는 relaxed인가 join인가?
48. `problem.cpp`의 `first_sum`과 `second_sum`이 non-atomic이어도 데이터 경쟁이 없는 이유는 무엇인가?
49. worker 함수 밖으로 예외가 탈출하면 어떤 종료 동작이 생길 수 있는가?
50. source의 `fetch_add`가 실제 lock instruction인지 library lock인지 단정하지 말아야 하는 이유를 적어라.

## 5. 컨테이너·스트림 호출 계약

51. `vector<vector<Edge>>(n+1)`에서 count 인자, 반환, 초기 내부 원소 상태, 시간·공간 비용을 설명하라.
52. `graph[from]`의 전제조건과 반환형은 무엇인가? 범위 밖이면 예외인가 UB인가?
53. `push_back(Edge{...})`가 선택하는 overload와 재할당 시 무효화되는 관찰자를 말하라.
54. `vector<int>(n+1, 0)`의 두 위치 인자가 각각 무엇이며 원소가 몇 개 생기는가?
55. `answers.reserve(k)`가 바꾸는 것과 바꾸지 않는 것을 `size`와 `capacity`로 설명하라.
56. reserve가 성공·실패할 때 기존 원소와 관찰자의 상태는 어떻게 되는가?
57. `priority_queue`의 기본 방향과 `CandidateGreater`가 최소 힙을 만드는 비교식을 설명하라.
58. `frontier.top()`의 반환형·수명과 바로 복사한 뒤 `pop()`해야 하는 이유를 말하라.
59. 빈 priority_queue에 `top()` 또는 `pop()`을 적용하지 못하게 하는 코드 전제는 어디에 있는가?
60. `std::ios::sync_with_stdio(false)`의 bool 반환값을 오늘 코드는 쓰는가?
61. `std::cin.tie(nullptr)`가 반환하는 포인터와 호출 뒤 바뀌는 연결을 설명하라.
62. `!(std::cin >> n >> m >> k)`에서 각 `>>`와 `operator!`의 반환값이 어떻게 연결되는가?
63. `std::cout << value << separator`의 각 호출 반환형과 마지막 반환값 사용 여부를 말하라.
64. stream 오류가 발생했을 때 이미 출력한 문자가 자동 rollback되는가?

## 6. k개 최단 walk 알고리즘

65. path와 walk의 정점·간선 재방문 차이를 설명하라.
66. `Candidate{cost, vertex}`는 “vertex의 최단 거리가 확정됐다”는 뜻인가, 아니면 무엇인가?
67. `accepted[v] == r`이 보장하는 비용 순위 불변식을 말하라.
68. 왜 accepted를 push할 때가 아니라 유효 pop할 때 증가시켜야 하는가?
69. 같은 `(cost, vertex)` 후보를 합치면 안 되는 공식 문제 조건은 무엇인가?
70. `(k+1)`번째 `s->v` 접두사가 전체 앞 k개 답에 필요 없다는 대체-prefix 논증을 써라.
71. 도착점의 첫 pop이 아니라 k번째 유효 pop까지 기다려야 하는 이유는 무엇인가?
72. k번째 답 전에는 도착점의 나가는 간선도 확장해야 하는 반례를 하나 만들어라.
73. 음수 간선이 들어오면 최소 힙의 어떤 불변식이 깨지는가?
74. 모든 정점이 최대 k번 확장될 때 인접 리스트 전체 순회가 최대 `km`인 이유를 식으로 보여라.
75. 힙 삽입/제거까지 포함한 `O(km log(km))` 시간 상한을 유도하라.
76. 힙의 최악 저장량을 포함한 `O(n+m+km)` 공간 상한을 유도하라.
77. `current.cost > MAX - edge.cost`를 덧셈보다 먼저 검사해야 하는 이유는 무엇인가?
78. 공식 예제의 답 `4 4 7`에서 두 비용 4에 대응하는 서로 다른 간선열을 적어라.

## 7. 실제 식 해석

79. `alignas(std::atomic_ref<std::uint64_t>::required_alignment) std::uint64_t completed{};`를
    선언 specifier, template 인자, 정적 멤버, 타입, 이름, initializer로 나눠 읽어라.
80. `static_cast<void>(completed_.fetch_add(1, std::memory_order_relaxed));`에서 반환값을 의도적으로
    버린다는 사실을 어떻게 표현했는가?
81. `[this, iterations]() noexcept { ... }`에서 capture 두 개의 소유/수명 위험을 비교하라.
82. `[ids = std::move(first_view), attempts_per_worker, &first_sum]`의 세 capture를 값 범주와 소유권으로
    각각 해석하라.
83. `while (!frontier.empty() && answers.size() < k)`의 short-circuit가 보장하는 것을 설명하라.
84. `const Candidate current{frontier.top()}; frontier.pop();`에서 참조 무효화 문제를 피하는 순서를 말하라.
85. `const std::vector<Edge>& outgoing{graph[vertex_index]};`가 복사 없이 빌리는 객체와 유효 기간을 적어라.
86. `index + 1U == answers.size() ? '\n' : ' '`가 선택하는 타입과 출력 목적을 설명하라.

## 통과 기준

- 1~23번을 코드 없이 설명한다.
- 24~64번 중 임의의 표준 호출 다섯 개를 골라 여섯 항목 계약을 빠짐없이 말한다.
- 65~78번의 불변식·증명·복잡도를 작은 그래프와 함께 설명한다.
- 79~86번의 식에서 lvalue/prvalue/xvalue, 반환값 사용, 수명·무효화를 표시한다.
- `main.cpp`, `problem.cpp`, CSES 공식 예제의 출력을 실행 전에 맞히고 CTest 7/7을 통과한다.
