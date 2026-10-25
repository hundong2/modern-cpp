# 2026-10-26 CHECKPOINT — `optional::or_else`와 암시적 트립

코드를 다시 읽기 전에 말과 종이로 답한다. “대충 fallback”, “대충 균형 tree”가 아니라 실제 식의 타입·값 범주·수명·불변식까지 설명해야 통과다.

## 1. 초보 문법 확인

- [ ] 1. `<optional>`, `<string>`, `<string_view>`, `<utility>`, `<vector>`, `<iostream>`, `<cstdint>`, `<cstddef>`, `<chrono>`, `<random>`이 오늘 어떤 선언을 직접 제공하는가?
- [ ] 2. `struct Endpoint`와 `class EndpointRepository`의 기본 접근 지정자는 각각 무엇인가?
- [ ] 3. `public:`과 `private:`가 runtime 보안 장치가 아닌 이유를 말하라.
- [ ] 4. `Value value{};`, `int root_{kNull};`, `bool reversed{};`가 각각 어떤 값으로 초기화되는가?
- [ ] 5. 중괄호 초기화가 narrowing을 막는 예 하나를 작성하라.
- [ ] 6. `using Value = std::int64_t;`가 새 독립 타입을 만드는가, 별칭만 만드는가?
- [ ] 7. `std::vector<Endpoint>`와 `std::optional<std::string>`의 template 타입 인자를 각각 말하라.
- [ ] 8. 함수의 반환형, 매개변수, 함수 뒤 `const`, `noexcept`, `[[nodiscard]]`의 역할을 구분하라.
- [ ] 9. 생성자에 반환형이 없는 이유와 `explicit`이 막는 변환을 설명하라.
- [ ] 10. 멤버 초기화 목록이 본문보다 먼저 실행된다는 뜻과 실제 멤버 초기화 순서를 말하라.
- [ ] 11. pointer와 reference가 표현할 수 있는 상태(null, 재바인딩)와 소유권 여부를 비교하라.
- [ ] 12. `if (endpoint.service == service)`의 비교 결과 타입과 어느 block이 실행되는지 말하라.
- [ ] 13. `for (const Endpoint& endpoint : endpoints_)`가 숨기는 begin/end/비교/역참조/증가를 적어라.
- [ ] 14. `{}` block을 벗어날 때 지역 객체가 어떤 순서로 파괴되는가?
- [ ] 15. `return 0;`이 `main`에서 뜻하는 운영체제 관찰 상태를 말하라.

## 2. 소유권·값 범주·객체 수명

- [ ] 16. `repository`, `found`, `entries`, `endpoint` 식을 lvalue/prvalue/xvalue 중 분류하라.
- [ ] 17. `std::string{"billing"}`, `Endpoint{...}`, `primary_.find(service)`의 결과 값 범주를 말하라.
- [ ] 18. `std::move(entries)`가 vector 원소를 직접 옮기는지, 실제 이동은 어느 연산이 하는지 설명하라.
- [ ] 19. moved-from vector/string/optional에 공통으로 보장되는 것과 보장되지 않는 것을 구분하라.
- [ ] 20. `const Endpoint&`가 복사 없이 바인딩되는 대신 어떤 수명 의존성을 갖는가?
- [ ] 21. `string_view service`가 문자를 소유하거나 원본 수명을 늘리는가?
- [ ] 22. fallback lambda가 `std::string{service}`를 만드는 이유를 dangling 관점에서 설명하라.
- [ ] 23. `[this, service]`가 캡처하는 두 값의 소유/비소유 의미를 구분하라.
- [ ] 24. `EndpointResolver::primary_`가 안전하려면 repository와 resolver 중 누가 오래 살아야 하는가?
- [ ] 25. `main`의 선언 순서와 역순 파괴가 위 조건을 어떻게 만족하는가?
- [ ] 26. `return or_else-result-prvalue;`의 보장 복사 생략과 이름 있는 지역 `return local;`의 NRVO를 구분하라.
- [ ] 27. string 이동이 반드시 heap pointer 하나만 옮긴다고 말하면 왜 틀릴 수 있는가?
- [ ] 28. optional 결과가 Endpoint를 소유해도 Endpoint 내부가 string_view라면 무엇이 달라지는가?
- [ ] 29. `operator*`가 반환한 reference를 optional 파괴 뒤 보관하면 왜 잘못인가?
- [ ] 30. `std::vector` 재할당이 Node reference/pointer/iterator에 미치는 영향을 말하라.

## 3. `std::optional::or_else` 여섯 항목 계약

다음 실제 식을 보지 않고 여섯 항목을 완성한다.

```cpp
primary_.find(service).or_else([this, service] {
    return std::optional<Endpoint>{
        Endpoint{std::string{service}, fallback_url_}};
});
```

- [ ] 31. 수신 객체의 정확한 타입, 값 범주와 호출 전 두 상태를 말하라.
- [ ] 32. 선택되는 `or_else`의 cv/ref-qualified signature와 template 인자 `F`를 말하라.
- [ ] 33. lambda 식의 타입/값 범주, `F&&`가 forwarding reference인 이유를 설명하라.
- [ ] 34. callable의 데이터 매개변수 수와 허용되는 반환 타입을 말하라.
- [ ] 35. engaged 수신자에서 callable 호출 횟수, 반환값 구성, 원본 상태를 말하라.
- [ ] 36. disengaged 수신자에서 callable 호출 횟수와 최종 소유 객체들을 말하라.
- [ ] 37. 반환형과 반환 prvalue가 이 함수에서 어떻게 사용되는가?
- [ ] 38. 상태 검사·callable·string 복사 각각의 복잡도와 할당 가능성을 나눠라.
- [ ] 39. `or_else`가 일반적으로 `noexcept`가 아닌 이유와 가능한 예외를 말하라.
- [ ] 40. 같은 optional/captured object의 동시 읽기·쓰기에서 어떤 외부 동기화가 필요한가?

다음 두 식도 비교한다.

```cpp
std::move(configured).or_else(factory);
const_configured.or_else(factory);
```

- [ ] 41. 첫 식에서 이름 있는 `configured` 자체와 `std::move(configured)` 결과의 값 범주는 각각 무엇인가?
- [ ] 42. mutable/const lvalue, mutable rvalue, const rvalue가 각각 `const &`/`&&` 중 어느 overload를 고르고 contained `T`를 복사/이동 중 무엇으로 새 결과에 넣는가?
- [ ] 43. rvalue overload 뒤 원본 optional이 자동으로 빈 상태가 되는가?
- [ ] 44. factory가 plain `Endpoint`나 `optional<Other>`를 반환하면 어떤 단계에서 거부되는가?
- [ ] 45. `or_else`와 `transform`, `and_then`, `value_or`가 각각 답하는 질문을 한 문장씩 구분하라.
- [ ] 45-a. engaged 경로에서도 lambda 객체와 capture는 먼저 만들어지지만 lambda 본문만 건너뛴다는 차이를 설명하라.

## 4. 다른 표준 호출 계약

- [ ] 46. vector initializer-list 생성에서 initializer_list 원소가 const인 점과 복사 횟수 의미를 설명하라.
- [ ] 47. vector 이동 생성 후 기본 allocator 조건에서 소유권과 원소 관찰자 상태를 설명하라.
- [ ] 48. `vector::reserve(n)`의 인자/반환/후조건/할당/예외/무효화 계약을 말하라.
- [ ] 49. capacity 안에서 `push_back(Node&&)`와 capacity 초과 시 계약 차이를 말하라.
- [ ] 50. `nodes_[index]`가 `at(index)`와 달리 범위 위반을 검사하지 않을 때 결과는 무엇인가?
- [ ] 51. `std::swap(current.left,current.right)`의 template 인자, 두 매개변수, 반환형과 사후 상태를 말하라.
- [ ] 52. `std::pair<int,int>` 반환 뒤 structured binding의 숨은 객체와 `std::get<0/1>(pair&&)`가 돌려주는 참조·수명을 설명하고, tree node가 아니라 root index 값만 전달됨을 보여라.
- [ ] 53. `std::nullopt`의 타입과 optional 생성 뒤 상태를 말하라.
- [ ] 54. disengaged optional의 `operator*`와 `value()`가 각각 어떤 실패 계약을 갖는가?
- [ ] 55. string/string_view `operator==`의 입력 소유권·복잡도·dangling 전제조건을 말하라.
- [ ] 56. `ios::sync_with_stdio(false)`와 `cin.tie(nullptr)`의 매개변수·반환값 사용 여부·사후 상태를 말하라.
- [ ] 57. `operator>>` 실패 뒤 대상 값을 계속 쓰는 것이 왜 문제인가?
- [ ] 58. `cout << sum << '\n'`과 `std::endl`의 flush 차이를 설명하라.
- [ ] 58-a. `random_device` 생성/두 호출의 수신 객체·무인자 시그니처·반환값 사용, 호출 뒤 내부 상태 변화 가능성과 반환 정수의 독립 수명, 무효화 없음, 실제 entropy·비용·blocking·할당·예외의 구현 의존성, 같은 객체의 병행 호출에 필요한 동기화, catch 뒤 clock fallback을 여섯 계약 축으로 설명하라.
- [ ] 58-b. `steady_clock::now().time_since_epoch().count()`의 각 수신자·무인자 시그니처·세 반환형·사용, 임시 수명과 상태/무효화 없음, 표준이 점근 비용·할당을 지정하지 않는 점, 선언의 `noexcept`와 TrivialClock의 실제 비throw 요구 차이, thread 동기화 효과 없음, 단조성·epoch·tick 단위·보안 난수 비보장을 설명하라.

## 5. 암시적 트립 불변식

- [ ] 59. 외부 key가 없는데 현재 위치를 어떻게 `size(left)`에서 계산하는가?
- [ ] 60. 중위 순회와 logical sequence 사이의 불변식을 말하라.
- [ ] 61. `(priority,id)`를 함께 비교하는 이유를 설명하라.
- [ ] 62. `size`와 `sum`의 점화식을 적어라.
- [ ] 63. reverse가 합과 원소 수를 바꾸지 않으므로 `apply_reverse`에서 pull이 필요 없는 이유를 말하라.
- [ ] 64. `apply_reverse`가 child swap과 bool toggle을 모두 해야 하는 이유를 적어라.
- [ ] 65. 같은 subtree에 reverse tag를 두 번 적용했을 때 결과를 말하라.
- [ ] 66. `push` 전에 child 방향/left size를 읽으면 어떤 논리 오류가 생기는가?
- [ ] 67. link 변경 뒤 `pull`을 빼먹으면 어느 다음 연산들이 깨지는가?
- [ ] 68. `split(root,k)`의 정확한 반환 의미를 반열린 index 구간으로 표현하라.
- [ ] 69. `k <= size(left)`와 그 반대 branch에서 재귀 인자가 각각 무엇인가?
- [ ] 70. `merge(A,B)`가 요구하는 위치 전제조건을 말하라.
- [ ] 71. merge가 더 높은 priority root를 선택해 heap을 유지하는 이유를 증명하라.
- [ ] 72. `[a,b]`를 분리할 때 첫 split 크기와 두 번째 split 크기를 적어라.
- [ ] 73. 합 질의도 link를 바꾸는데 관찰상 배열이 그대로인 이유를 설명하라.
- [ ] 74. `root = merge(left, merge(middle,right))`의 결합 순서를 바꿔도 되는지 전제조건으로 답하라.

## 6. 정확성·복잡도·기계 실행

- [ ] 75. `L,u,R`의 전체 역순 식으로 lazy reverse 정확성을 증명하라.
- [ ] 76. tree 높이에 대한 귀납법으로 split의 앞 k개/나머지 정확성을 증명하라.
- [ ] 77. merge가 sequence 순서와 heap을 동시에 지킨다는 것을 두 root priority 경우로 나눠 증명하라.
- [ ] 78. 두 split와 두 merge에서 구간 연산 전체 정리를 도출하라.
- [ ] 79. 무작위 priority에서 기대 높이와 병적 priority에서 최악 높이를 각각 말하라.
- [ ] 80. 오늘 구축/연산/전체 시간과 arena/recursion stack 공간 복잡도를 적어라.
- [ ] 81. `2*10^5*10^9`를 계산해 32비트 합이 부족함을 보이라.
- [ ] 82. 공개된 `priority=mix(id)`를 reverse로 단조 priority 순서에 재배열할 수 있는 이유와, 입력 밖 실행별 seed가 기대 복잡도 분석에 필요한 이유를 말하라.
- [ ] 83. arena index가 현재 sequence position과 같지 않은 이유를 말하라.
- [ ] 84. `split`에서 예상되는 load/compare/branch/store를 나열하라.
- [ ] 85. `or_else` lambda와 treap 함수가 inline될 수 있어도 실제 assembly를 단정할 수 없는 이유를 말하라.
- [ ] 86. 오늘 코드에 virtual 함수가 없으므로 요구되지 않는 호출 형태가 무엇인가?

## 7. 직접 해보기

- [ ] 87. `problem.cpp`의 fallback을 `anonymous`로 바꾸고 실행 전 출력을 예측했다.
- [ ] 88. `or_else` lambda 반환을 plain `std::string`으로 바꿔 compiler 제약 진단을 읽었다가 복구했다.
- [ ] 89. `std::move(configured)`를 지우고 어느 overload와 복사 경로가 선택되는지 확인했다.
- [ ] 90. 결과 Endpoint가 `string_view`를 저장하도록 바꾼 위험 예를 실행하지 않고 수명선으로 설명했다.
- [ ] 91. `apply_reverse`의 bool toggle을 지운 뒤 double-reverse test가 무엇을 잡는지 확인했다가 복구했다.
- [ ] 92. `push(index)` 하나를 지운 변형이 random oracle에서 실패하는 seed를 찾았다가 복구했다.
- [ ] 93. `pull(index)` 하나를 지운 변형이 size/sum 중 무엇부터 깨는지 확인했다가 복구했다.
- [ ] 94. 작은 배열의 모든 reverse/sum을 `std::vector` oracle과 최소 1,000회 비교했다.
- [ ] 95. `n=200,000`, `m=100,000` stress에서 실행 시간과 최대 tree 깊이를 기록했다.
- [ ] 96. 공용 algorithm 문서의 code block을 독립 컴파일·실행했다.
- [ ] 97. CMake Release build와 CTest 7/7을 직접 통과했다.
- [ ] 98. `audit-standard-library-docs.ps1 -Scope latest`와 `-Scope all`을 통과했다.
- [ ] 99. `git diff --check`와 링크 검사를 통과하고 build 산출물이 staging되지 않았음을 확인했다.
- [ ] 100. `icpc_problem.cpp`를 보지 않고 split/merge/reverse/query를 다시 구현했다.

## 최종 구술 기준

다음 문장을 빈칸 없이 말한다.

> `or_else`는 optional이 ______일 때만 callable을 최대 ______회 실행한다. 오늘 prvalue 수신자는 ______ overload를 선택하고, 성공값은 결과로 ______되지만 원본 optional이 자동으로 ______되지는 않는다. fallback의 `string_view`는 ______하지 않으므로 결과 DTO는 이를 `string`으로 ______한다. 암시적 트립의 중위 순회는 현재 ______이고 위치는 왼쪽 subtree의 ______로 계산한다. `[a,b]`는 크기 ______와 ______로 두 번 split하며 reverse tag는 child ______과 bool ______로 합성한다. 각 연산은 기대 시간 ______, 병적 최악 ______이고 전체 node 공간은 ______이다.
