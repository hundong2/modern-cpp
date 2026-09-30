# 2026-10-01 CHECKPOINT — `views::enumerate`와 트리 차분 경로 계수

코드를 보지 않고 먼저 답한 뒤 실제 식과 주석으로 검증한다. “무엇을 한다”만 말하지 말고 타입, 값 범주, 소유권, 수명, 상태 변화와 실패 조건을 함께 설명한다.

## 1. 기초 문법과 타입

- [ ] 1. `<cstddef>`, `<iostream>`, `<ranges>`, `<string>`, `<tuple>`, `<utility>`, `<vector>`가 `main.cpp`에서 직접 제공하는 선언을 하나씩 말한다.
- [ ] 2. `int`, `bool`, `std::size_t`, `long long`의 역할과 signed/unsigned 차이를 설명한다.
- [ ] 3. `ReleaseStep step{};`에서 `name`과 `state`가 각각 어떻게 초기화되는가?
- [ ] 4. `enum class StepState`가 평범한 정수 상수보다 안전한 이유는?
- [ ] 5. `struct ReleaseStep`과 `class ReleasePlan`의 기본 접근 지정자는 무엇인가?
- [ ] 6. DTO에는 struct, owner에는 class를 고른 설계 이유를 말한다.
- [ ] 7. `using Steps = std::vector<ReleaseStep>;`가 새 강한 타입인지 별칭인지 답한다.
- [ ] 8. `std::vector<ReleaseStep>`와 `std::vector<NumberedStep>`의 template 인자 `T`는 무엇인가?
- [ ] 9. `explicit ReleasePlan(Steps steps)`가 막는 암시 변환 식을 하나 쓴다.
- [ ] 10. 생성자에 반환형이 없는 이유와 멤버 초기화 목록이 본문 대입과 다른 점을 말한다.
- [ ] 11. 멤버 초기화 목록의 표기 순서와 실제 초기화 순서 중 무엇이 우선하는가?
- [ ] 12. `Snapshot make_snapshot() const &`에서 반환형, 매개변수 목록, 함수 const, `&` 참조 한정자를 구분한다.
- [ ] 13. `void indexed_steps() const && = delete;`가 막는 최소 위험 식을 작성한다.
- [ ] 14. `[[nodiscard]]` 반환값을 버렸을 때 언어가 보장하는 것과 컴파일러 진단 가능성을 구분한다.
- [ ] 15. `constexpr const char* state_name(...) noexcept`의 세 specifier/타입 의미를 설명한다.
- [ ] 16. `switch`, `if`, `continue`, `for`, `while`이 오늘 코드에서 만드는 조건 분기 위치를 찾는다.
- [ ] 17. `const ReleaseStep&`와 `const ReleaseStep*`의 null 가능성, 재바인딩, 접근 문법, 소유권을 비교한다.
- [ ] 18. `const`가 동시 쓰기를 막는 잠금이 아닌 이유를 별칭 관점에서 말한다.

## 2. 값 범주, 참조 바인딩, 복사·이동, 수명

- [ ] 19. 이름 있는 `seed`, `steps`, `steps_`, `plan`, `snapshot`, `rows`, `enumerated` 식의 값 범주는 무엇인가?
- [ ] 20. `ReleasePlan::Steps{...}`, `NumberedStep{...}`, `plan.make_snapshot()` 결과 식의 값 범주는 무엇인가?
- [ ] 21. `std::move(seed)`의 값 범주와 그 식이 여전히 가리키는 객체를 말한다.
- [ ] 22. lvalue, prvalue, xvalue를 오늘 코드의 실제 식 하나씩으로 구분한다.
- [ ] 23. `std::move`가 실제 원소를 옮기는지, 실제 저장소 이전은 어느 연산이 수행하는지 설명한다.
- [ ] 24. `ReleasePlan plan{std::move(seed)}`에서 값 매개변수와 `steps_`까지 소유권 경계를 추적한다.
- [ ] 25. rvalue reference로 초기화되었더라도 이름 있는 생성자 매개변수 `steps` 식이 lvalue인 이유는?
- [ ] 26. 이동 뒤 seed에 허용되는 연산 두 개와 의존하면 안 되는 상태를 말한다.
- [ ] 27. `auto enumerated = indexed_steps();`가 원소 vector를 복사하는가? 실제로 무엇을 보관하는가?
- [ ] 28. `auto&& [zero_based, step]`에서 바깥 임시와 두 바인딩의 수명을 구분한다.
- [ ] 29. `step`이 원소를 복사하지 않는 이유와 `step.name`이 결과 string을 복사하는 이유를 말한다.
- [ ] 30. owner 파괴, vector 재할당, 원소 값만 읽기, 동시 쓰기가 enumerate view에 주는 영향을 각각 말한다.
- [ ] 31. `return rows;`에서 NRVO가 가능한 이유와 적용되지 않을 때 허용되는 이동을 말한다.
- [ ] 32. 이름 있는 지역 반환이면 복사·이동 0회가 항상 보장된다고 말할 수 없는 이유는?
- [ ] 33. `const Snapshot snapshot = plan.make_snapshot();`의 보장된 복사 생략 지점을 설명한다.
- [ ] 34. `Snapshot copied{snapshot};`에서 vector와 string이 어떤 독립 저장소를 얻는가?
- [ ] 35. `state_name`의 `const char*`가 안전한 기간과 enumerate 원소 참조가 안전한 기간이 다른 이유는?
- [ ] 36. `make_snapshot()` 결과가 plan 파괴 뒤에도 유효한 이유를 소유권으로 설명한다.

## 3. `views::enumerate` 의미와 안전한 설계

- [ ] 37. `std::views::enumerate(steps_)`가 생성하는 각 원소의 두 성분을 말한다.
- [ ] 38. index가 0-based인지 1-based인지, 부호 있는지 부호 없는지 답한다.
- [ ] 39. `static_cast<std::size_t>(zero_based) + 1U`에서 변환 전제가 안전한 이유는?
- [ ] 40. 빈 범위와 원소 하나인 범위는 각각 몇 번 순회되는가?
- [ ] 41. view 구성 시 원소 순회·복사·할당이 일어나는지와 실제 원소 접근 시점을 구분한다.
- [ ] 42. `filter | enumerate`와 `enumerate | filter`에서 index 의미가 어떻게 다른가?
- [ ] 43. `const auto view = enumerate(mutable_vector)`만으로 원소가 const가 되지 않는 이유는?
- [ ] 44. 오늘 코드는 어떤 식을 enumerate에 넘겨 원소를 읽기 전용으로 만드는가?
- [ ] 45. ref-view가 소유하는 것과 소유하지 않는 것을 말한다.
- [ ] 46. tuple-like 역참조 결과와 `NumberedStep` 결과 객체의 소유권을 비교한다.
- [ ] 47. range-`for`가 개념적으로 호출하는 `begin`, `end`, 비교, 역참조, 증가, 구조적 바인딩 get을 순서대로 적는다.
- [ ] 48. 끝 iterator 역참조와 서로 다른 범위 iterator 비교가 왜 전제조건 위반인지 설명한다.
- [ ] 49. 길이 n에서 view 구성, 전체 순회, 결과 materialize의 시간/공간을 계산한다.
- [ ] 50. `const &` accessor와 삭제한 `const &&` overload가 제거하는 댕글링 경로를 그린다.
- [ ] 51. UI 행 번호, validation 위치, 감사 로그 중 enumerate가 적합한 사례와 원본 ID가 더 적합한 사례를 하나씩 든다.

## 4. 표준 라이브러리 호출 계약 여섯 항목

아래 각 식에 대해 반드시 다음 여섯 부분을 빠짐없이 답한다.

1. 수신 객체의 **정확한 타입**과 호출 직전 상태
2. 선택된 시그니처·overload·template 인자
3. 각 매개변수 식의 타입·값 범주·소유권 의미·허용값
4. 반환형·반환값 의미·실제 사용 또는 폐기 여부
5. 호출 뒤 수신 객체와 각 인자의 상태 변화
6. 전제조건·후조건·복잡도·할당·iterator/reference 무효화·수명·오류/예외·스레드 보장

- [ ] 52. `std::string name{}`의 기본 생성
- [ ] 53. seed 안 문자열 리터럴에서 각 `std::string`을 만드는 생성
- [ ] 54. `std::vector<ReleaseStep>` initializer-list 생성
- [ ] 55. `std::move(seed)`와 값 매개변수 vector 이동 생성
- [ ] 56. `std::move(steps)`와 멤버 vector 이동 생성
- [ ] 57. `std::views::enumerate(steps_)`
- [ ] 58. enumerate range-`for`의 숨은 반복 연산과 `std::get<0/1>`의 정확한 overload, tuple 인자의 xvalue 범주, 반환 참조 축약
- [ ] 59. `Snapshot rows{}`의 vector 기본 생성
- [ ] 60. `steps_.size()`
- [ ] 61. `rows.reserve(steps_.size())`
- [ ] 62. `rows.push_back(NumberedStep{...})`와 내부 string 복사
- [ ] 63. `Snapshot copied{snapshot}`의 vector/string 복사 생성
- [ ] 64. const vector range-`for`의 숨은 반복 연산
- [ ] 65. main 출력의 `std::cout << row.number << ':' << row.name ...`
- [ ] 66. ICPC 코드의 `std::ios_base::sync_with_stdio(false)`
- [ ] 67. `std::cin.tie(nullptr)`
- [ ] 68. `std::cin >> node_count >> path_count`
- [ ] 69. `std::vector<std::vector<int>> graph(node_count+1)`
- [ ] 70. `graph[first].push_back(second)`의 바깥 `operator[]`와 안쪽 push
- [ ] 71. `parent/depth(count,0)` fill 생성과 `order/stack{}` 기본 생성
- [ ] 72. `order.reserve(n)`과 `stack.reserve(n)`
- [ ] 73. `stack.empty()`, `stack.back()`, `stack.pop_back()`
- [ ] 74. `for (const int neighbor : graph[vertex])`의 숨은 호출
- [ ] 75. 중첩 vector 조상 표의 안쪽/바깥 fill 생성
- [ ] 76. `std::cout << ' '`, long long 삽입, 마지막 `std::cout << '\n'`

## 5. 호출 계약의 세부 확인

- [ ] 77. vector initializer-list 생성자가 backing array 원소를 복사하는지 이동하는지 설명한다.
- [ ] 78. 기본 allocator vector 이동 생성의 대표 시간·할당·예외 계약과 기존 원소 참조의 새 대상을 말한다.
- [ ] 79. vector 복사 생성이 allocator와 원소/string에 대해 수행하는 일을 말한다.
- [ ] 80. `reserve(n)`이 size를 바꾸는지, 재할당 시 어떤 iterator/reference가 무효화되는지 답한다.
- [ ] 81. 충분한 reserve 뒤 `push_back`이 기존 관찰자를 유지하는 조건과 과거 `end()`의 변화를 구분한다.
- [ ] 82. `push_back`이 `bad_alloc` 또는 원소 이동/복사 예외를 낼 수 있는 조건과 예외 보장을 말한다.
- [ ] 83. `vector::operator[]`의 반환형, 복잡도, 범위 검사 여부와 범위 밖 동작을 말한다.
- [ ] 84. `empty()`와 `size()==0`의 의미는 같지만 오늘 back/pop 전 empty를 쓴 의도를 말한다.
- [ ] 85. 빈 vector의 `back()`/`pop_back()` 동작과 성공 뒤 무효화 범위를 말한다.
- [ ] 86. stream 추출 실패 때 대상 값, 상태 비트, 반환 istream&에 어떤 일이 생기는가?
- [ ] 87. stream 삽입 실패 때 상태 비트와 `exceptions()` mask에 따른 예외를 설명한다.
- [ ] 88. `sync_with_stdio(false)`의 반환값과 호출 시점, C stdio와 섞을 때 주의점을 말한다.
- [ ] 89. `cin.tie(nullptr)`가 반환하는 이전 포인터의 사용 여부와 자동 flush 변화를 말한다.
- [ ] 90. 같은 vector를 한 스레드가 수정하는 동안 다른 스레드가 읽으면 왜 데이터 경쟁인가?

## 6. 실제 Expression 해석

- [ ] 91. `.number = static_cast<std::size_t>(zero_based) + 1U`의 읽기·변환·덧셈·저장을 순서대로 설명한다.
- [ ] 92. `.name = step.name`이 참조만 저장하는지 문자를 복사하는지 답한다.
- [ ] 93. `auto enumerated = indexed_steps();`의 auto가 참조 타입인지 view 값 타입인지 말한다.
- [ ] 94. `for (auto&& [zero_based, step] : enumerated)`의 `auto&&`가 forwarding-reference 형태로 결과에 어떻게 바인딩되는가?
- [ ] 95. `if (item.approved) continue;`의 load, 비교, 조건 분기를 언어 의미 수준에서 설명한다.
- [ ] 96. `parent[neighbor] = vertex`에서 두 인덱스 변환과 store가 요구하는 범위 전제를 말한다.
- [ ] 97. `up[level][vertex] = up[level-1][up[level-1][vertex]]`를 `2^level` 조상 의미로 해석한다.
- [ ] 98. `(difference & (1 << level)) != 0`이 깊이 차의 어느 정보를 검사하는가?
- [ ] 99. `++path_difference[first]`의 lvalue 수정과 전위 증가 반환값 사용 여부를 말한다.
- [ ] 100. `path_difference[parent[vertex]] += path_difference[vertex]`의 두 load와 add/store를 설명한다.

## 7. Binary Lifting과 노드 경로 차분

- [ ] 101. 트리에서 두 정점 사이 단순 경로가 유일한 이유는?
- [ ] 102. 반복 DFS에서 부모만 건너뛰어도 visited 배열이 필요 없는 전제는 무엇인가?
- [ ] 103. `order`에서 부모가 자식보다 앞선다는 불변식을 귀납적으로 증명한다.
- [ ] 104. `up[0][v]`와 `up[k][v]`의 의미 및 점화식을 적는다.
- [ ] 105. `n=200000`에 `max_log=19`가 충분한 이유를 2의 거듭제곱으로 보인다.
- [ ] 106. 깊이 맞춤 단계가 조상-자손 LCA를 어떻게 즉시 처리하는가?
- [ ] 107. 큰 level부터 조상이 다를 때만 함께 올리면 왜 LCA 바로 아래에서 멈추는가?
- [ ] 108. 경로 `(u,v)`의 네 표식 공식을 정확히 적는다.
- [ ] 109. `u==v`일 때 표식과 누적 결과가 그 정점에서 1인지 계산한다.
- [ ] 110. `LCA==root`일 때 부모 감산을 생략하지 않으면 root 답이 어떻게 틀리는가?
- [ ] 111. 정점 x가 u-LCA 구간 위, v-LCA 구간 위, LCA, LCA의 조상, 경로 밖일 때 서브트리 표식 합을 각각 계산한다.
- [ ] 112. 여러 경로의 표식을 한 배열에 합쳐도 되는 이유를 선형성으로 설명한다.
- [ ] 113. 역순 order에서 자식 값을 부모에 더할 때 서브트리 합이 완성되는 귀납 증명을 말한다.
- [ ] 114. root까지 같은 누적문을 실행하면 `delta[root] += delta[root]`가 되는 이유는?
- [ ] 115. 노드 공식과 간선 공식의 LCA 감산 차이를 설명한다.
- [ ] 116. 전처리, 질의, 누적 비용을 합쳐 `O((n+m) log n)`을 유도한다.
- [ ] 117. 인접 목록, 상태 배열, 조상 표를 포함해 `O(n log n)` 공간을 유도한다.
- [ ] 118. 최종 답이 int에 들어가도 `long long`을 쓴 이유와 unsigned가 부적합한 이유는?

## 8. 경계 사례와 실수 찾기

- [ ] 119. `n=1`, 경로 `(1,1)` 세 개의 차분과 출력 3을 손으로 계산한다.
- [ ] 120. 일자 5개에서 `(1,5)`, `(2,4)`, `(3,3)` 결과 `1 2 3 2 1`을 계산한다.
- [ ] 121. 별 트리의 리프-리프 두 경로에서 root 횟수를 계산한다.
- [ ] 122. `delta[lca]-=2`로 바꾸면 공식 예제에서 어느 값이 틀리는가?
- [ ] 123. order를 정방향으로 누적하면 자식 정보가 늦게 도착하는 반례를 만든다.
- [ ] 124. 일반 그래프에서 parent만 건너뛰면 다시 방문할 수 있는 최소 사이클 반례를 만든다.
- [ ] 125. 길이 200000 일자 트리에서 재귀 DFS가 환경에 따라 위험한 이유는?
- [ ] 126. 조상 표 level 하나를 줄였을 때 가장 깊은 차이를 올리지 못하는 입력을 구성한다.
- [ ] 127. 경로마다 실제 정점을 걷는 풀이의 최악 연산량을 n=m=200000에서 계산한다.
- [ ] 128. parent/depth/order/stack의 int, path_difference의 long long 선택을 각 범위와 연결한다.

## 9. 기계 실행 관점

- [ ] 129. enumerate 순회에서 가능한 index 증가, 끝 비교, 주소 계산, 원소 load를 소스 식과 연결한다.
- [ ] 130. string 깊은 복사에서 길이 검사, 할당 가능성, 문자 load/store를 설명한다.
- [ ] 131. LCA 한 level에서 depth/up 배열 load, 비트 검사, 조건 분기가 어떻게 반복되는가?
- [ ] 132. 역순 누적의 두 배열 load, 정수 add, 부모 칸 store를 설명한다.
- [ ] 133. 연속 vector 조상 행이 cache locality에 줄 수 있는 이점과 `vector<vector<int>>`의 행별 할당을 말한다.
- [ ] 134. 오늘 domain class에 virtual 호출이 없고 stream 구현에는 간접 호출이 있을 수 있음을 구분한다.
- [ ] 135. CPU·ABI·라이브러리·컴파일러·최적화 옵션 없이 특정 명령 수나 분기 형태를 단정할 수 없는 이유는?

## 10. 직접 실행하는 검증

- [ ] A. `main.cpp`의 세 출력 줄을 실행 전에 예측하고 stdout 전체와 비교한다.
- [ ] B. `problem.cpp`가 위치 2와 3만 출력하는 이유를 손으로 설명한다.
- [ ] C. 임시 `ReleasePlan{...}.indexed_steps()`가 컴파일되지 않는 최소 검사를 작성한다.
- [ ] D. `enumerate | filter`와 `filter | enumerate`의 index 차이를 보이는 작은 코드를 작성한다.
- [ ] E. CSES 공식 예제와 단일 정점·일자·별·균형 트리 CTest를 실행한다.
- [ ] F. 고정 seed 작은 무작위 트리를 BFS 경로 복원 oracle과 대조한다.
- [ ] G. 최대 일자/별 트리 스트레스를 Release 빌드로 실행하고 시간·메모리를 기록한다.
- [ ] H. 알고리즘 문서의 C++ 예제를 별도로 컴파일해 `1 2 3 2 1`을 확인한다.
- [ ] I. CMake C++23 빌드와 모든 CTest의 실제 종료 코드·개수를 기록한다.
- [ ] J. 전체 표준 라이브러리 감사에서 미문서화 심볼·헤더·호출 계약이 0인지 확인한다.

## 통과 기준

- enumerate의 index 타입·0-based 의미, tuple-like 비소유 참조, owner 수명과 vector 무효화를 설명한다.
- lvalue/prvalue/xvalue, 참조 바인딩, 복사·이동, 이동 후 상태, NRVO와 보장 복사 생략을 실제 식으로 설명한다.
- 모든 최초 표준 라이브러리 호출을 여섯 계약 항목과 실제 인자 수에 맞춰 설명한다.
- Binary Lifting 조상 표, 노드 경로 차분 네 항, root 예외, 역순 서브트리 누적을 증명한다.
- 전체 시간 `O((n+m) log n)`, 공간 `O(n log n)`과 `O(nm)` 반례를 스스로 유도한다.
- 세 실행 파일 빌드, CTest, oracle/stress, 문서 예제, 전체 감사를 실제로 통과한 뒤에만 완료로 표시한다.
