# 2026-10-23 CHECKPOINT

오늘의 목표는 두 가지다.

- 실무 코드에서는 `CapabilitySet`을 **자원을 직접 소유하는 값 타입**으로 설명하고, 실제 식마다 값 범주·참조·복사·이동·수명을 구분한다.
- CSES 2138 **Reachable Nodes**에서는 DAG의 역위상 순서와 64비트 word bitset 폐쇄가 왜 맞고 얼마나 비싼지 증명한다.

자료를 덮고 먼저 말이나 종이에 답한다. “대충 안다”가 아니라 **정확한 타입, 값 범주, 소유자, 수명 끝, 전제조건, 반환값 사용 여부, 무효화, 오류, 복잡도**를 실제 식에 붙일 수 있어야 통과다.

표준 라이브러리 호출 계약을 묻는 문항에서는 항상 다음 여섯 항목을 순서대로 답한다.

1. 수신 객체의 정확한 타입과 호출 직전 상태
2. 선택된 signature, overload, template 인자
3. 각 인자의 식·타입·값 범주·소유권 의미·허용 범위
4. 반환형·반환값의 의미·실제 사용 여부
5. 호출 뒤 수신 객체와 인자의 상태 변화
6. 전제조건·후조건·복잡도·할당·무효화·수명·오류/예외·thread 보장

아래 식은 오늘 `main.cpp`의 실제 API를 축약한 값 범주 예시다.

```cpp
using BitWord = std::uint64_t;

CapabilitySet enabled{BitWord{0}};
enabled.grant(Capability::inspect);
enabled.grant(Capability::deploy);
const CapabilitySet& view = enabled;
const auto total = enabled.count();
```

## 1. 기초 문법과 타입

1. `#include <bit>`가 오늘 코드에 필요한 직접적인 이유는 무엇인가?
2. `#include <cstdint>`가 제공하는 `std::uint64_t`는 모든 구현에 반드시 존재하는가?
3. `#include <vector>`와 `#include <iostream>`이 각각 선언하는 핵심 타입을 두 개씩 말한다.
4. `using Word = std::uint64_t;`는 새 강한 타입을 만드는가, 기존 타입의 별칭을 만드는가?
5. `std::vector<Word>`에서 `Word`는 어떤 종류의 template 인자인가?
6. `Word word{};`의 중괄호 초기화 결과와 초기화하지 않은 지역 `Word word;`의 차이는 무엇인가?
7. 중괄호 초기화가 narrowing을 막는 간단한 반례를 하나 만든다.
8. `struct`와 `class`의 기본 멤버 접근 지정자는 각각 무엇인가?
9. `public`과 `private`는 객체의 runtime 메모리 보호 기능인가, compile-time 접근 규칙인가?
10. `explicit CapabilitySet(BitWord initial_bits)`가 막는 암시 변환을 식으로 보인다.
11. 생성자에 반환형을 쓰지 않는 이유는 무엇인가?
12. 멤버 초기화 목록은 생성자 본문보다 언제 실행되는가?
13. 멤버 초기화 목록에 적은 순서와 class에 선언한 순서가 다르면 어느 순서로 초기화되는가?
14. `count() const`의 뒤 `const`와 `const CapabilitySet& view`의 앞 `const`를 각각 설명한다.
15. pointer와 reference를 null 가능성, 재바인딩 가능성, 소유권 표현 관점에서 비교한다.
16. `std::size_t`가 배열 첨자에 흔히 쓰이는 이유와 signed 정수와 섞을 때의 위험은 무엇인가?
17. `Word{1} << bit_offset`에서 왼쪽 피연산자를 unsigned 64비트 타입으로 만드는 이유는 무엇인가?
18. `bit_offset`이 64 이상이면 shift 식에 어떤 문제가 생기며, 나머지 연산으로 어떻게 막는가?

## 2. `CapabilitySet` 값 타입과 불변식

19. “값 타입”이라는 말을 복사 가능성, 독립 상태, 동등성, 소유권 관점에서 설명한다.
20. `CapabilitySet`의 `bits_`는 누가 소유하며 그 기본 타입 subobject의 수명은 언제 시작하고 끝나는가?
21. `Capability::inspect`, `deploy`, `audit`의 기저 값과 실제 64비트 mask를 각각 쓴다.
22. `bit_for(capability)`의 switch가 각 열거자를 어떤 고정 shift 식으로 바꾸는지 타입·값 순서로 해석한다.
23. shift의 왼쪽 피연산자를 `BitWord{1}`로 만든 이유와 평범한 `1`을 쓸 때의 위험을 말한다.
24. `grant(capability)`에 선언되지 않은 enum 값이 들어왔을 때 0 mask가 되는 후조건과 수신 상태가 유지되는 이유를 말한다.
25. `contains(capability)`가 상태를 바꾸지 않아야 하는 이유와 멤버 함수 선언에 드러나는 표시는 무엇인가?
26. `count()`가 `int`를 반환해도 안전한 최댓값을 오늘의 64비트 저장소와 연결해 설명한다.
27. `grant` 뒤 수신 집합과 값 매개변수 `capability`의 상태를 각각 설명한다.
28. 같은 capability를 두 번 `grant`해도 결과가 같은 성질의 이름은 무엇인가?
29. bitwise OR의 교환법칙·결합법칙·멱등성이 권한 집합과 ICPC 도달 집합에서 각각 어떻게 쓰이는가?
30. `static_cast<Capability>(64U)`를 강제로 만들어 `grant`에 넘겨도 현재 switch 구현이 shift UB 없이 안전하게 무시하는 제어 흐름을 말한다.
31. public 함수가 raw 정수가 아니라 `Capability`를 받는 것이 실수를 줄이되 강제 cast까지 완전히 막지는 못하는 이유는 무엇인가?
32. 외부에 mutable `BitWord&`를 그대로 노출하면 캡슐화와 열거자 mapping 불변식이 어떻게 깨지는가?
33. `operator==`를 자동 생성한다면 private `bits_` 하나의 비교가 값 의미를 충분히 표현하는가?
34. 새 열거자를 추가할 때 기저 값, 64비트 범위, 이전 직렬화/ABI 기대 중 무엇을 검토해야 하는가?
35. 권한 수가 64를 넘는 제품으로 확장해 `vector<BitWord>`를 소유한다면 universe 크기와 padding bit라는 어떤 새 불변식이 필요한가?

## 3. 실제 식, 값 범주, 참조 바인딩

36. 선언 뒤 이름으로 쓴 `enabled` 식의 값 범주는 무엇인가?
37. `CapabilitySet{BitWord{0}}` 식의 값 범주는 무엇인가?
38. `std::move(enabled)`의 값 범주와 반환 타입 모양은 무엇인가?
39. `std::move` 자체가 bit 저장소를 이동시키는가?
40. `view`가 reference 변수여도 식 `view`의 값 범주가 lvalue인 이유는 무엇인가?
41. `const CapabilitySet& view = CapabilitySet{BitWord{0}};`에서 임시 객체 수명은 어디까지 연장되는가?
42. 함수가 임시 객체에 묶인 reference를 반환하면 호출자까지 수명이 연장되는가?
43. `Word{1}`과 `bit_offset`과 `(Word{1} << bit_offset)`의 값 범주를 각각 말한다.
44. 멤버 함수 안의 `bits_` 식과 `BitWord{1}` 식의 값 범주는 각각 무엇인가?
45. `contains`의 `(bits_ & bit_for(capability)) != BitWord{0}` 전체 식의 타입과 값 범주를 말한다.
46. `bits_ |= bit_for(capability);`에서 개념적으로 읽히고 쓰이는 객체는 무엇인가?
47. `const auto total = enabled.count();`에서 `auto`가 추론하는 타입과 top-level `const`의 효과는 무엇인가?
48. 작은 `Capability` enum을 값 매개변수로 받는 것과 `const Capability&`로 받는 것을 복사 비용과 reference 수명 관점에서 비교한다.
49. ICPC 코드에서 `outgoing` reference를 가진 동안 그 내부 vector에 `push_back`하면 언제 안전하지 않은가? 재할당 무효화와 연결한다.
50. `print_policy(const SessionPolicy& policy)`가 policy를 복사하지 않으면서도 소유하지 않는다는 뜻을 설명한다.
51. 이름 있는 rvalue-reference 변수 `CapabilitySet&& sink`를 식으로 쓰면 어떤 값 범주인가?
52. 51번의 객체를 다시 이동 생성에 넘기려면 왜 `std::move(sink)`가 필요한가?

## 4. 복사·이동·수명·소유권·RVO

53. `CapabilitySet backup = enabled;`에서 64비트 `bits_` subobject가 어떻게 복사되는가?
54. 53번 뒤 `backup.grant(Capability::audit)`이 `enabled`를 바꾸지 않아야 하는 이유는 무엇인가?
55. `CapabilitySet transferred = std::move(enabled);`에서 어떤 생성자가 선택되는가?
56. 오늘처럼 scalar 멤버 하나뿐인 암시적 move 뒤 `enabled.bits_`의 값이 유지되는 이유와, 이를 모든 class의 moved-from 상태로 일반화하면 안 되는 이유는 무엇인가?
57. `CapabilitySet`의 암시적 copy/move가 동적 할당이나 dangling view를 만들지 않는 이유는 무엇인가?
58. `SessionPolicy` 이동에서는 `std::string` 원본 값이 미지정일 수 있지만 `CapabilitySet` scalar 값은 복사되는 차이를 설명한다.
59. 기본 allocator의 string 이동이 보통 저장소를 넘길 수 있어도 SSO와 allocator 조건 때문에 항상 단순 pointer swap이라고 단정하면 안 되는 이유는 무엇인가?
60. `std::move`는 cast이고 실제 자원 이전은 어느 생성자/대입 연산에서 일어나는가?
61. `CapabilitySet make_set() { return CapabilitySet{BitWord{0}}; }`에서 C++17 보장 복사 생략을 설명한다.
62. `CapabilitySet result{BitWord{0}}; return result;`의 NRVO는 61번과 똑같이 반드시 적용되는가?
63. 복사 생략이 일어나도 반환 객체 생성자가 접근 가능하고 destructor가 유효해야 하는 이유는 무엇인가?
64. `const CapabilitySet&`를 멤버로 저장하는 observer class를 만들 때 소유 객체보다 observer가 오래 살면 어떤 문제가 생기는가?
65. ICPC 함수가 `const Graph&`나 `const std::vector<Word>&`를 받으면 소유권이 넘어가는가? owner 파괴·재할당이 reference를 어떻게 dangling으로 만들 수 있는가?
66. `CapabilitySet`을 함수 값 매개변수로 받는 설계가 복사본을 수정하는 함수와 ownership을 소비하는 함수에서 각각 언제 유용한가?
67. `SessionPolicy`의 string 또는 ICPC vector 할당이 `std::bad_alloc`으로 실패하면 미완성 객체와 이미 생성된 멤버 정리는 누가 맡는가? `CapabilitySet` 자체는 왜 할당하지 않는가?

## 5. `std::popcount`, `std::vector`, `iostream` 호출 계약

68. `std::popcount(word)`의 template 인자 `T`, 인자 타입/값 범주, 반환형과 반환 범위를 말한다.
69. `std::popcount`에 signed `int`를 그대로 넘길 수 있는가? overload 참여 조건으로 설명한다.
70. `std::popcount(word)`가 수신 객체 없이 동작할 때 여섯 항목 중 “수신 객체” 항목을 어떻게 기록해야 하는가?
71. `std::popcount`의 `constexpr`와 `noexcept`가 각각 보장하는 것과 보장하지 않는 것은 무엇인가?
72. `std::popcount` 호출이 입력 word, iterator/reference, 소유권, thread에 미치는 영향을 여섯 항목으로 말한다.

다음 vector 식은 각각 여섯 항목을 모두 답한다.

73. `std::vector<Word> words(word_count, Word{0});`
74. `words.size()`
75. non-const 객체의 `words[word_index]`
76. const 객체의 `words[word_index]`
77. `words.at(word_index)`
78. `words.begin()`과 `words.end()`
79. `words.push_back(word)`에서 선택되는 overload
80. `words.push_back(Word{0})`에서 선택되는 overload
81. `words.reserve(required_words)`
82. `std::vector<Word> reachability(node_count * word_count, Word{0});`
83. `std::vector<std::vector<int>> edges(n);`
84. `edges[from].push_back(to)`를 바깥 `operator[]`와 안쪽 `push_back` 두 호출로 나누어 설명한다.
85. 오늘 코드가 range-for 대신 index loop를 썼다. range-for로 바꾸면 숨은 begin/end, 비교, 역참조, 증가 연산에 어떤 반환·무효화 계약이 추가되는가?

다음 stream 식도 각각 여섯 항목을 모두 답한다.

86. `std::ios::sync_with_stdio(false)`
87. `std::cin.tie(nullptr)`
88. `std::cin >> n`
89. `std::cin >> from >> to`의 왼쪽 결합과 첫 호출 반환값 사용을 설명한다.
90. 오늘 코드는 judge의 유효 입력을 전제한다. `if (!(std::cin >> n >> m))` 검사를 추가하면 stream의 `operator!`와 상태 비트 계약이 어떻게 달라지는가?
91. `std::cout << answer`
92. `std::cout << ' '`
93. `std::cout << '\n'`이 `std::endl`과 달리 보장하지 않는 동작은 무엇인가?
94. 출력 연쇄 `std::cout << answer << separator`에서 첫 `operator<<` 반환값이 어떻게 두 번째 수신 객체가 되는가?
95. 입력 실패와 출력 실패가 기본 예외 mask에서 보통 어떻게 보고되며, 언제 예외가 던져질 수 있는가?
96. 같은 stream 또는 같은 vector를 여러 thread가 동시에 읽거나 수정할 때 “서로 다른 객체” 규칙과 data race 가능성을 구분한다.

## 6. CSES 2138: 역위상 word-bitset 폐쇄

97. 문제에서 “노드 `v`에서 도달 가능”은 길이 0인 경로를 포함하는가? 따라서 자기 자신 bit는 어떻게 초기화해야 하는가?
98. 입력 그래프가 DAG라는 조건이 오늘 점화식에 왜 중요한가?
99. 간선 `v -> u`가 있을 때 집합 점화식 `Reach(v)`를 합집합으로 쓴다.
100. topological order에서 모든 간선의 출발점과 도착점 순서는 어떻게 되는가?
101. 왜 forward topological order가 아니라 reverse topological order로 처리하는가?
102. 정점 `v`를 처리할 때 모든 outgoing neighbor `u`의 `Reach(u)`가 이미 완성됐다는 불변식을 말한다.
103. 정점 5개와 간선 `1->2`, `1->3`, `2->4`, `3->4`, `4->5`의 한 topological order를 쓴다.
104. 103번 그래프를 역순으로 처리하며 각 정점의 도달 집합을 차례대로 적는다.
105. 103번에서 정점 1이 정점 4를 두 경로로 만나도 popcount가 4를 한 번만 세는 이유는 무엇인가?
106. `v -> u`를 처리할 때 word `k`에 수행하는 핵심 대입 식을 쓴다.
107. 정점 ID를 0-based로 바꿨을 때 정점 `v`의 자기 bit를 설정하는 word index와 mask를 쓴다.
108. `n`이 64의 배수가 아닐 때 마지막 word의 padding bit를 어떻게 취급해야 하는가?
109. 모든 row를 0으로 시작하고 실제 정점 bit만 설정하면 padding mask를 매번 적용하지 않아도 되는 이유는 무엇인가?
110. 중복 간선이 있어도 정답이 바뀌지 않는 이유와 실행 비용에는 미치는 영향을 말한다.
111. isolated vertex의 출력이 0이 아니라 1인 이유는 무엇인가?
112. sink vertex의 도달 집합과 count는 무엇인가?
113. source vertex가 반드시 모든 정점에 도달하는가? 반례를 하나 만든다.
114. 입력이 DAG가 아니면 Kahn topological order의 길이가 어떻게 cycle 존재를 드러내는가?
115. 일반 directed graph에 같은 아이디어를 적용하려면 SCC 압축 뒤 각 bit가 “원래 정점”인지 “SCC”인지에 따라 count를 어떻게 조정해야 하는가?

### 정확성 증명

116. 역위상 순서에 대한 induction의 base case를 sink 정점으로 서술한다.
117. induction hypothesis를 “이미 처리된 정점”과 “outgoing neighbor”를 사용해 정확히 쓴다.
118. soundness: OR 결과에 들어온 모든 bit가 실제로 `v`에서 도달 가능함을 경로 연결로 증명한다.
119. completeness: `v`에서 도달 가능한 모든 정점이 자기 자신 또는 첫 간선의 neighbor closure를 통해 포함됨을 증명한다.
120. 118번과 119번, 자기 bit 초기화가 함께 equality를 보이는 이유는 무엇인가?

### 복잡도와 메모리

121. `W = ceil(n / 64)`라 할 때 한 번의 집합 OR과 한 row의 count 비용을 word 연산 수로 쓴다.
122. topological sort, 자기 bit 초기화, 모든 간선 OR, 모든 row popcount를 합친 시간 복잡도를 `n`, `m`, `W`로 쓴다.
123. closure 저장소와 adjacency/topological 보조 구조를 합친 공간 복잡도를 쓴다.
124. `n = 50,000`이면 `W`는 몇 개인가?
125. 124번에서 raw word 저장소 `n * W * 8` byte를 계산하고 대략적인 MiB로 바꾼다.
126. `std::vector<std::vector<Word>>` 행렬 방식이 raw word 저장소 외에 row별 vector object와 할당 metadata 비용을 더 갖는 이유는 무엇인가?
127. 하나의 flat `std::vector<Word>`를 `n * W` 크기로 잡는 방식의 장점과 index 계산 단점을 말한다.
128. 각 시작점에서 DFS/BFS를 새로 하는 방법의 최악 시간과 bitset closure 방법의 차이를 비교한다.
129. `std::bitset<MAX_N>`과 동적 `std::vector<Word>`를 compile-time 크기, padding, 저장 위치, 입력 유연성 관점에서 비교한다.
130. 행의 popcount 합을 모든 OR 뒤 한 번만 계산하는 것과 OR마다 다시 계산하는 것의 비용 차이는 무엇인가?
131. cached count를 유지하며 OR하는 최적화가 새 bit 수를 구하기 위해 어떤 추가 계산과 불변식을 요구하는가?
132. word OR 순회가 시간상 병목일 때 간선 순서, row 연속성, cache locality가 왜 중요할 수 있는가?

## 7. 기계 실행 관점과 단정 금지

133. `destination[k] |= source[k]`를 개념적인 load, OR, store 단계로 나누어 설명한다.
134. `std::popcount(word)`가 target CPU의 단일 명령으로 내려갈 수도, software sequence나 library call이 될 수도 있는 이유는 무엇인가?
135. `contains(capability)`는 개념적으로 어떤 enum-to-unsigned 변환, shift, word load, AND, 비교, 조건 결과를 만들 수 있는가?
136. `operator[]`와 `at()`는 bounds check와 분기 관점에서 어떤 차이를 의도하지만 optimizer 결과를 왜 단정할 수 없는가?
137. Kahn algorithm의 `if (--indegree[to] == 0)`에는 개념적으로 어떤 load/store/비교/조건 분기가 있는가?
138. vector의 data가 연속이라는 보장과 모든 row의 data가 서로 연속이라는 주장은 왜 다른가?
139. 오늘 값 타입이나 closure 코드에 virtual 함수가 없다면 가상 간접 호출 비용을 주장하면 왜 틀리는가?
140. Debug와 Release, target ISA, compiler version에 따라 inlining·vectorization·unrolling·popcount lowering이 달라질 수 있는 이유는 무엇인가?
141. source 한 줄과 assembly 한 줄이 항상 일대일 대응하지 않는 이유는 무엇인가?
142. bitset OR의 성능을 말할 때 Big-O뿐 아니라 memory bandwidth와 cache miss도 측정해야 하는 이유는 무엇인가?

## 8. 직접 수정·실험 과제

1. `CapabilitySet::revoke(Capability)`와 `clear()`를 추가하고 기존 불변식을 보존하는 최소 test를 작성한다.
2. `intersection_with`를 구현하고 합집합과 다른 bitwise 연산, 후조건, 복잡도를 주석으로 쓴다.
3. `static_cast<Capability>(64U)`가 들어와도 현재 switch 기반 경계가 0 mask로 처리함을 test한다. 옛 unchecked shift 버전은 직접 실행하지 말고 왜 UB인지 식으로 설명하며, 더 확인하려면 별도 임시 파일과 UBSan으로만 격리 실험한다.
4. moved-from `SessionPolicy`를 다시 대입해 정상 사용하는 test를 만들고, string과 CapabilitySet의 moved-from 차이를 관찰한다.
5. `Capability::audit`를 추가/제거하며 `count()`와 `contains()`가 함께 바뀌는 회귀 test를 만든다.
6. `std::popcount`에 signed 타입을 넘기는 compile-fail 예제를 별도 임시 파일에서 확인하고 unsigned cast가 의미상 안전한 조건을 적는다.
7. 예제 DAG를 종이에 역위상 순서로 추적한 뒤 program 출력과 대조한다.
8. 정점 8개 이하의 무작위 DAG를 생성하고 각 정점 DFS oracle과 bitset 결과를 최소 1,000회 비교한다.
9. 중복 간선, isolated vertex, chain, star, 서로 분리된 두 DAG를 고정 회귀 test로 추가한다.
10. 간선 하나를 뒤집어 cycle을 만든 입력에서 topological-order 길이 검사가 실패를 잡는지 확인한다.
11. row별 vector 구현과 flat storage 구현을 같은 입력에서 비교하되 Release build, 동일 compiler 옵션, 여러 회 반복을 기록한다.
12. 최대 입력 stress에서 실행 시간과 peak memory를 측정하고 이론값과 차이가 나는 원인을 적는다.
13. `operator[]`를 임시로 `at()`으로 바꾸어 test를 돌리고 잘못된 index가 exception으로 드러나는 반례를 만든다.
14. 출력 마지막 공백을 없애는 두 방법을 구현하고 branch/readability trade-off를 설명한다.
15. strict warning build, CTest, 무작위 oracle, 문서 감사 후 build 산출물이 Git stage에 없는지 확인한다.

## 9. 완료 체크리스트

- [ ] 1~35를 자료 없이 답하고 class 불변식을 세 문장으로 요약했다.
- [ ] 36~52의 모든 실제 식에 lvalue/prvalue/xvalue와 cv/ref를 붙였다.
- [ ] “`std::move`가 이동한다”가 아니라 실제 move constructor/assignment가 자원을 이전한다고 설명했다.
- [ ] 보장 복사 생략과 선택적인 NRVO를 구분했다.
- [ ] 68~96의 각 호출을 여섯 항목 계약으로 빠짐없이 설명했다.
- [ ] `std::popcount`의 unsigned 제약, 반환형, `noexcept`, 상태 불변을 설명했다.
- [ ] vector 재할당 시 reference/iterator 무효화와 예외 가능성을 설명했다.
- [ ] stream 반환 reference가 연쇄의 다음 수신 객체가 되는 과정을 설명했다.
- [ ] 103번 DAG를 손으로 끝까지 추적했다.
- [ ] reverse-topological 불변식으로 soundness와 completeness를 각각 증명했다.
- [ ] `O(n + m + mW + nW)` 시간과 `O(nW + n + m)` 공간을 직접 유도했다.
- [ ] 최대 입력의 raw bit 저장소 byte와 MiB를 계산했다.
- [ ] 기계 실행 설명마다 CPU/compiler/최적화 옵션에 따라 달라질 수 있음을 덧붙였다.
- [ ] 고정 test, 무작위 oracle, 최대 stress를 모두 통과했다.
- [ ] 관련 없는 변경과 build 산출물이 staging되지 않았음을 확인했다.

<details>
<summary>핵심 답 힌트 — 먼저 스스로 푼 뒤 펼치기</summary>

### 기초와 값 타입

- `<bit>`는 `std::popcount`, `<cstdint>`는 선택적 고정 폭 정수 별칭, `<vector>`는 연속 동적 배열, `<iostream>`은 표준 문자 stream을 선언한다.
- `Word`는 별칭일 뿐 새 타입이 아니다. `std::vector<Word>`의 `Word`는 원소 타입 template 인자다.
- 오늘 권한 값 타입은 `std::uint64_t bits_` 하나를 소유한다. switch의 각 case가 알려진 열거자를 `BitWord{1} << 0U/1U/2U`의 안전한 고정 mask로 바꾸고, 미지의 값은 0을 반환한다.
- 값 타입은 내부 값을 스스로 소유하고 복사본이 독립적이며 연산 뒤 class 불변식이 유지돼야 한다. raw 생성자는 알려진 세 bit만 남기고, switch 경계는 선언되지 않은 enum을 무시한다.
- OR은 결합적·교환적·멱등적이다. 이 때문에 처리 순서 일부와 중복 경로/간선이 정답을 바꾸지 않는다.

### 값 범주·복사·이동·수명

- 이름 있는 객체·reference 변수 식은 lvalue, `CapabilitySet{BitWord{0}}`은 prvalue, `std::move(enabled)`는 xvalue다. `std::move`는 rvalue cast일 뿐이다.
- `bits_`는 멤버 함수 안에서 lvalue이고, ICPC vector의 `operator[]` 결과도 원소 lvalue reference다. const vector이면 `const Word&`다.
- 임시를 local `const&`에 직접 묶으면 그 reference scope까지 살지만, 함수 return reference가 임시 수명을 호출자까지 연장하지는 않는다.
- CapabilitySet 복사/이동은 scalar 비트를 복사해 source도 유지된다. 그러나 SessionPolicy의 string 이동 뒤 source 값은 미지정이므로 이를 모든 타입으로 일반화하지 않는다.
- `return CapabilitySet{BitWord{0}};`은 C++17 보장 복사 생략 대상이다. 이름 있는 `return result;`의 NRVO는 일반적으로 허용되지만 의무는 아니다.
- reference는 소유하지 않는다. owner 파괴나 관찰 중인 vector 재할당 뒤에는 dangling이 될 수 있다.

### 핵심 호출 계약

- `std::popcount<T>(T)`는 unsigned integer 타입에만 참여하고 `int`를 반환한다. 결과는 0부터 `numeric_limits<T>::digits`까지이며 입력을 바꾸지 않고 `noexcept`다. 수신 객체·할당·무효화는 없고, 같은 값의 독립 호출은 공유 mutable state가 없다.
- `vector(count, value)`는 `count`개 복사 원소를 만들며 선형 시간이고 할당/`length_error`/`bad_alloc` 가능성이 있다. 생성자에는 반환값이 없다.
- `size()`는 `size_type`, 상수 시간, 상태 불변이다. `operator[]`는 상수 시간 reference를 주며 범위 밖이면 전제조건 위반이다. `at()`은 범위 밖에서 `out_of_range`를 던진다.
- `push_back`은 lvalue면 `const T&`, prvalue/xvalue면 `T&&` overload가 선택된다. 재할당되면 모든 iterator/reference가 무효화되고, 아니면 과거 `end()`는 적어도 무효화된다.
- `reserve`는 요청이 capacity보다 클 때만 재할당한다. 재할당 시 모든 iterator/reference가 무효화될 수 있으며 `length_error`/할당 실패가 가능하다.
- `vector<vector<Word>>(n, vector<Word>(W, 0))`는 안쪽 vector 값을 n번 깊은 복사한다. 각 row는 독립 저장소를 가져야 하므로 큰 closure에는 많은 개별 할당이 생길 수 있다.
- `sync_with_stdio(false)`는 이전 동기화 상태를 `bool`로 반환한다. 표준 stream I/O 전에 호출해야 이식 가능한 의도가 분명하다. `cin.tie(nullptr)`는 과거 tied stream pointer를 반환하고 소유권을 이전하지 않는다.
- `operator>>`와 `operator<<`는 stream reference를 반환해 연쇄한다. 실패는 state bit로 기록되고 exception mask가 요청할 때 예외가 날 수 있다. `\n`은 newline 문자일 뿐 flush를 보장하지 않는다.
- 같은 container를 한 thread가 수정하며 다른 thread가 읽으면 외부 동기화 없이는 data race가 될 수 있다. 서로 다른 객체라는 사실과 같은 내부 저장소를 공유하는지는 별도로 확인한다.

### 알고리즘과 증명

- `Reach(v) = {v} union (union over edges v->u of Reach(u))`다. reverse topological 순서에서는 모든 `Reach(u)`가 먼저 완성된다.
- 예제의 집합은 `R5={5}`, `R4={4,5}`, `R3={3,4,5}`, `R2={2,4,5}`, `R1={1,2,3,4,5}`다.
- soundness는 자기 bit 또는 실제 간선 뒤 실제 경로만 합친다는 사실, completeness는 모든 nonempty 경로의 첫 간선이 어떤 outgoing neighbor로 간다는 사실에서 나온다.
- `W=(n+63)/64`. 시간은 `O(n+m+mW+nW)`, 보통 `O((n+m)W)`로 쓴다. 공간은 `O(nW+n+m)`이다.
- `n=50,000`이면 `W=782`, raw words는 `50,000*782*8 = 312,800,000` byte, 약 `298.3 MiB`다. container/allocator/adjacency overhead는 별도다.
- cycle이면 Kahn 결과 길이가 `n`보다 작다. 일반 directed graph는 SCC로 압축할 수 있지만 SCC 크기 가중치 또는 원래 정점 bit 설계가 필요하다.

### 기계 관점

- OR loop는 대략 source/destination word load, OR, destination store를 반복할 수 있고 popcount는 hardware 명령 또는 software lowering일 수 있다.
- `operator[]`에는 언어 수준 bounds 검사 계약이 없고 `at()`은 검사/예외 계약이 있지만, 실제 분기·inlining·vectorization 형태는 compiler, target ISA, 최적화 옵션에 따라 달라진다.
- 큰 closure는 계산량뿐 아니라 수백 MiB memory traffic과 cache locality가 성능을 좌우한다. 반드시 Release 조건에서 측정한다.

</details>
