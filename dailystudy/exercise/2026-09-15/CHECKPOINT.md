# 2026-09-15 CHECKPOINT — 명시적 객체 매개변수, `std::forward_like`, 트리 rerooting DP

체크 표시는 답을 소리 내어 설명하고, 실제 객체와 참조의 화살표를 종이에 그린 뒤에만 한다. 단순히 “복사한다” 또는 “이동한다”로 끝내지 말고 **누가 저장소를 소유하는지, 어느 식이 lvalue/prvalue/xvalue인지, 호출 전후 어떤 참조가 여전히 유효한지**까지 답한다.

표준 라이브러리 호출 문항은 매번 아래 여섯 묶음을 빠짐없이 말해야 통과다.

1. 수신 객체의 정확한 타입·값 범주와 호출 직전 상태
2. 선택된 signature·overload·템플릿 인자
3. 각 인자 식의 타입·값 범주·허용값과 복사/이동/차용·소유권 의미
4. 반환형·반환값의 의미와 호출부가 저장·연쇄·검사·무시하는지
5. 호출 뒤 수신 객체·인자·backing storage의 상태 변화
6. 전제·후조건, 복잡도, 할당, 무효화, 수명, 오류·예외·UB와 스레드 보장

학습 순서는 [`main.cpp`](main.cpp) → [`problem.cpp`](problem.cpp) → [`icpc_problem.cpp`](icpc_problem.cpp)다.

## A. 초보자를 위한 기초 문법 검증

- [ ] 1. `<iostream>`, `<string>`, `<utility>`, `<vector>`가 `main.cpp`에서 직접 제공하는 오늘의 선언을 하나씩 연결하라.
- [ ] 2. `problem.cpp`에는 왜 `<vector>`가 필요 없고 `<iostream>`, `<string>`, `<utility>`만 필요한가?
- [ ] 3. `int`, `char`, `bool`과 `PortList`가 오늘 코드에서 표현하는 값을 찾고, 기본 타입과 사용자 별칭을 구분하라.
- [ ] 4. `using PortList = std::vector<int>;`와 `using Value = T;`가 새 타입을 만드는지, 원래 타입과 대입·overload 관점에서 같은 타입인지 답하라.
- [ ] 5. `std::vector<int>`의 타입 템플릿 인자 `int`, `Box<std::string>`의 타입 템플릿 인자 `std::string`, `template<class Self>`의 추론 인자가 각각 언제 결정되는가?
- [ ] 6. `struct DeploymentSnapshot`/`CopyMoveResult`와 `class DeploymentPlan`/`DeploymentPublisher`/`Box`의 기본 접근 지정자가 어떻게 다른가?
- [ ] 7. DTO에는 public 멤버를 가진 struct를, 불변식과 소유 상태를 감춘 wrapper에는 private 멤버를 가진 class를 선택한 이유를 설명하라.
- [ ] 8. `explicit DeploymentPlan(...)`과 `explicit Box(Value value)`가 막는 암시 변환/copy-list 초기화 예를 하나씩 만들라.
- [ ] 9. `service_{std::move(service)}`와 `ports_{std::move(ports)}`가 함수 본문 대입보다 먼저 실행되는 멤버 초기화 목록인 이유를 설명하라.
- [ ] 10. 멤버의 실제 초기화 순서는 초기화 목록에 쓴 순서와 선언 순서 중 어느 쪽을 따르는가? `service_`, `ports_`로 답하라.
- [ ] 11. `DeploymentPlan&& plan`, `const DeploymentPlan& audit_view`, `const DeploymentPlan* const audit_source`에서 각각 참조/포인터 대상, 수정 가능성, null 가능성, 소유권을 구별하라.
- [ ] 12. `const DeploymentPlan* const`에서 앞의 const와 뒤의 const가 각각 무엇을 고정하는지 설명하라.
- [ ] 13. 참조는 일반적인 정상 코드에서 null 상태를 모델링하지 않지만 포인터는 가능하다는 차이와, 오늘 `&audit_view`, `&box`가 null이 아닌 근거를 말하라.
- [ ] 14. `[[nodiscard]]`가 붙은 factory/accessor/publish 반환값을 버렸을 때 어떤 도움을 주며, 실행 의미를 강제로 바꾸는지 답하라.
- [ ] 15. `if (published.service != ... || ...)`의 단락 평가와 `return 0`/`return 1`이 운영체제에 전달하는 의미를 설명하라.

## B. C++23 명시적 객체 매개변수와 `std::forward_like`

- [ ] 16. 과거에 `content() &`, `content() const &`, `content() &&`, `content() const &&` 네 함수를 만들던 이유를 말하고 `this Self&& self`가 이를 어떻게 한 몸체로 합치는지 설명하라.
- [ ] 17. 명시적 객체 매개변수의 `Self`가 함수 호출 시 추론되는 템플릿 인자라는 점과 일반 매개변수 `T value`의 역할 차이를 말하라.
- [ ] 18. `self`의 선언 타입이 `Self&&`여도 이름 있는 식 `self` 자체는 왜 항상 lvalue인가?
- [ ] 19. `self.service_`와 `self.value_`가 lvalue인 이유를 멤버 접근 식의 값 범주 규칙으로 설명하라.
- [ ] 20. `std::forward_like<Self>(self.service_)`에서 명시적으로 주는 템플릿 인자와 함수가 추론하는 타입 인자를 구별하고, mutable owner의 `U=std::string&`와 const owner의 `U=const std::string&`를 설명하라.
- [ ] 21. 다음 표를 자료 없이 완성하라.

| 호출 객체 식 | 추론되는 `Self`의 const·참조 범주 | `service()`/`content()` 결과 |
|---|---|---|
| mutable lvalue |  |  |
| const lvalue |  |  |
| mutable xvalue |  |  |
| const xvalue |  |  |

- [ ] 22. `audit_source->service()`에서 `Self`가 const lvalue 경로가 되고 반환형이 `const std::string&`가 되는 과정을 말하라.
- [ ] 23. `observer->content()`와 `std::move(box).content()`가 같은 함수 몸체를 쓰면서 각각 복사 입력과 이동 입력이 되는 이유를 설명하라.
- [ ] 24. `std::forward_like` 자체는 객체를 복사하거나 이동하지 않는다는 말과, 반환된 xvalue로 뒤 이동 생성자가 실제 소유권을 옮긴다는 말을 구분하라.
- [ ] 25. `std::forward_like`가 pattern 타입의 const성과 lvalue/rvalue 성질을 결과 참조에 투영하되 pattern의 `volatile`은 새로 붙이지 않고 대상 타입의 기존 cv는 보존한다는 뜻을 타입 그림으로 보이라.
- [ ] 26. const rvalue에서 결과가 `const T&&`이면 보통 `T(T&&)` 이동 생성자보다 복사 경로가 선택될 수 있는 이유를 설명하라.
- [ ] 27. `auto&& dangling = make_trace_box().content();` 같은 코드를 함수 밖에 추가하면 세미콜론 뒤 왜 dangling 참조가 되는가? rvalue reference가 임시 Box 전체의 수명을 연장하는지 답하라.
- [ ] 28. explicit-object accessor가 참조를 반환하므로 owner인 Plan/Box의 파괴, 이동, 멤버 변경 중 무엇이 관찰 참조의 의미나 유효성에 영향을 주는가?
- [ ] 29. `decltype(auto)` 대신 단순 `auto` 반환을 쓰면 참조가 값으로 벗겨져 어떤 불필요한 복사가 생기고 rvalue 소비 정책이 어떻게 달라지는가?
- [ ] 30. `std::forward_like`가 `std::forward<T>`와 다른 질문에 답하는 도구임을 “인자의 원래 범주 복원”과 “다른 객체처럼 투영”으로 비교하라.

## C. 실제 식의 값 범주·복사·이동·수명 해석

- [ ] 31. 이름 있는 `plan`, `publisher`, `published`, `box`, `result`, `self`, `plan` 매개변수 식은 선언 타입과 무관하게 어떤 값 범주인가?
- [ ] 32. `make_plan()`, `make_trace_box()`, `PortList{8080, 8443}`, `DeploymentSnapshot{...}`의 값 범주를 말하라.
- [ ] 33. `std::move(plan)`과 `std::move(box)`의 결과 값 범주를 말하고, move 함수 호출 직후 원본 상태가 아직 바뀌지 않는 이유를 설명하라.
- [ ] 34. `std::move(plan).service()`와 `std::move(plan).ports()` 결과가 각각 어떤 타입의 xvalue인지 적어라.
- [ ] 35. `audit_source->service()`와 `observer->content()` 결과가 lvalue인 이유와 const reference에 바인딩되는 과정을 설명하라.
- [ ] 36. 문자열 리터럴 `"search-api"`와 `"trace"`의 타입·값 범주·저장 수명을 말하고, 배열-포인터 변환 뒤 어떤 생성자 인자가 되는지 설명하라.
- [ ] 37. `return DeploymentPlan{"search-api", PortList{8080, 8443}};`의 임시 string/vector 생성, 값 매개변수, 멤버 이동, Plan prvalue 반환 순서를 적어라.
- [ ] 38. `auto plan{make_plan()};`과 `auto box{make_trace_box()};`에서 C++17 보장 복사 생략이 왜 선택 사항인 NRVO와 다른가?
- [ ] 39. `return DeploymentSnapshot{...};`의 prvalue가 publish 함수 결과를 거쳐 `published`를 직접 초기화할 수 있는 이유를 설명하라.
- [ ] 40. publish의 매개변수 `DeploymentPlan&& plan`이 새 Plan을 소유하는 값 객체가 아니라 caller Plan의 별칭인 이유를 설명하라.
- [ ] 41. publish 안에서 이름 있는 `plan`에 다시 `std::move`가 필요한 이유와, 두 accessor가 서로 다른 subobject만 소비한다는 점을 설명하라.
- [ ] 42. publish 뒤 원본 `plan` 객체의 수명, `service_`/`ports_`의 상태, `audit_view`/`audit_source`의 포인터 대상은 각각 어떻게 되는가?
- [ ] 43. `CopyMoveResult{observer->content(), std::move(box).content()}`에서 initializer-clause 순서가 중요한 이유와 copied가 이동 뒤에도 `trace`인 근거를 말하라.
- [ ] 44. copied string은 원본과 독립 저장소를 소유하고 moved string은 원본 자원을 이어받을 수 있다는 차이를 객체 세 개의 화살표로 그려라.
- [ ] 45. string/vector의 소멸자가 main의 어느 역순으로 실행되며 RAII가 정상 return에서 자원을 회수하는지 설명하라.
- [ ] 46. 이동된 표준 객체의 “유효하지만 값은 미지정”이 빈 상태 보장과 다른 이유, 허용되는 파괴·대입과 피해야 할 값 의존 읽기를 구별하라.

## D. 표준 라이브러리 호출 계약

각 항목에 대해 문서 맨 위의 여섯 묶음을 모두 답한다.

- [ ] 47. `std::string{"trace"}`와 `"search-api"`에서 암시되는 `basic_string(const char*, allocator)` 생성자의 수신 전 상태, 포인터 인자, 반환 없음, 소유 결과, O(n)·할당·예외·NUL 전제를 설명하라.
- [ ] 48. `PortList{8080, 8443}`의 initializer-list vector 생성자에서 임시 const 원소 배열, 두 int 복사, size/capacity, O(n), 할당 실패와 독립 소유를 설명하라.
- [ ] 49. `std::move(service)`와 `std::move(ports)`의 템플릿 인자 추론, lvalue 인자, xvalue reference 반환, 상태 무변경, `noexcept`, 별도 표준 Complexity 항목과 새 소유 저장소 요구가 없다는 계약을 설명하라.
- [ ] 50. 위 xvalue 뒤 선택되는 string/vector 이동 생성자의 반환 없음, destination/source 상태, 복잡도, allocator 조건, 참조·반복자와 동시성 주의를 설명하라.
- [ ] 51. `std::forward_like<Self>(self.service_)`의 두 템플릿 타입, 인자 값 범주, 반환 const·참조 범주, alias 상태, `noexcept`·수명과 별도 표준 Complexity 항목/새 소유 저장소 요구가 없다는 계약을 설명하라.
- [ ] 52. `std::forward_like<Self>(self.ports_)`와 `std::forward_like<Self>(self.value_)`가 같은 함수 템플릿이지만 반환 대상 타입이 다른 이유를 설명하라.
- [ ] 53. `audited_ports.size()`의 정확한 const 수신자, 무인자 signature, size_type 반환값과 사용, 상태 무변경, O(1)·noexcept·무효화 없음 계약을 설명하라.
- [ ] 54. `published.ports.size()`가 source의 이동된 vector가 아니라 snapshot의 새 owner를 관찰한다는 점을 수신 객체 식부터 증명하라.
- [ ] 55. `std::cout << "audit=" << audited_service << ':' << audited_ports.size() << '\n'`에서 const-char-pointer/string/char/unsigned 정수 overload와 각 반환 `std::ostream&`의 연쇄 사용을 적어라.
- [ ] 56. 첫 출력 호출 전후 cout 상태, 입력 객체 상태, 문자 수 복잡도, buffering, 상태 비트, exception mask를 설명하라. synchronized 표준 stream은 동시 형식 출력에 data race가 없지만 문자가 섞일 수 있다는 두 보장을 구별하라.
- [ ] 57. 두 번째 main 출력과 problem의 두 번째 출력에는 마지막 newline이 없는 이유를 CMake exact-output 문자열과 연결하라. 정상 종료 시 flush와 외부 장치 오류 보장은 구별하라.
- [ ] 58. `published.service != "search-api"`의 string/const-char 비교, NUL 전제, bool 반환 사용, O(n), 무할당·무변경·단락 평가를 설명하라.
- [ ] 59. `result.copied != "trace" || result.moved != "trace"`에서 첫 결과가 true일 때 둘째 호출의 실행 여부와 객체 상태를 말하라.
- [ ] 60. `observer->content()`로 선택된 string 복사 생성자의 const lvalue 인자, 독립 ownership, O(n), 할당 실패 시 원본/미완성 destination 상태를 설명하라.
- [ ] 61. `std::move(box).content()` 뒤 선택된 string 이동 생성자의 xvalue 인자, source의 유효·미지정 상태, destination 소유권, 상수 시간과 과거 관찰자 주의를 설명하라.
- [ ] 62. string/vector `size`, 이동, 비교가 각각 어떤 포인터·참조·반복자를 무효화하는지 구별하고, “호출 자체가 무효화하지 않음”과 “뒤 이동 생성이 상태를 바꿈”을 분리하라.
- [ ] 63. 같은 `DeploymentPlan` 또는 `Box`를 한 스레드가 이동하는 동안 다른 스레드가 accessor로 읽으면 왜 `forward_like` 자체의 noexcept와 무관하게 데이터 경쟁인가?
- [ ] 64. 코드 주석의 “이동은 상수 시간”이 특정 포인터 세 개 교환 어셈블리를 보장하지 않는 이유를 allocator, 작은 문자열 최적화, ABI, CPU, compiler, 최적화 옵션으로 설명하라.

## E. 기계 실행 관점

- [ ] 65. const-lvalue accessor가 개념상 객체 주소로 subobject 위치를 계산해 reference를 돌려줄 수 있지만 최적화 뒤 별도 명령이 남지 않을 수도 있는 이유를 말하라.
- [ ] 66. `std::move`와 `std::forward_like`가 cast 성격이라 source code의 함수 이름이 실제 call instruction을 보장하지 않는 이유를 설명하라.
- [ ] 67. string/vector 이동 때 구현이 어떤 pointer/size/capacity load·store를 할 수 있고, 이를 모든 구현의 고정 레이아웃으로 단정하면 안 되는 이유를 말하라.
- [ ] 68. `if`의 문자열 비교와 size 비교가 load/compare/조건 분기로 번역될 수 있으나 상수 전파·inlining으로 합쳐지거나 사라질 수 있음을 설명하라.
- [ ] 69. `std::cout` 출력이 사용자 공간 버퍼와 운영체제 I/O를 거칠 수 있다는 점과, 소스의 `<<` 하나가 write system call 하나라는 보장이 없음을 말하라.

## F. CSES 1133 Tree Distances II — rerooting DP

- [ ] 70. 무방향 트리를 `vector<vector<int>>` 양방향 인접 리스트로 저장할 때 간선 하나를 왜 두 번 넣는가?
- [ ] 71. 반복 DFS의 `parent`, `order`, 명시적 stack이 각각 무엇을 저장하고, `order`에서 부모가 자식보다 먼저 온다는 불변식을 증명하라.
- [ ] 72. 재귀 대신 명시적 vector stack을 쓰면 정점 200,000개 경로 트리에서 어떤 호출 스택 위험을 피하는가?
- [ ] 73. root 깊이 `distance_from_root[v]`의 합이 root의 정답이 되는 이유를 말하고 `answer[root]` 초기식을 쓰라.
- [ ] 74. `subtree_size[v]`를 모두 1로 시작해 order 역순으로 부모에 더하면 각 subtree 크기가 완성되는 이유를 postorder 관점에서 설명하라.
- [ ] 75. 부모 p에서 자식 v로 root를 옮길 때 v subtree의 s개 정점은 거리가 1 줄고 나머지 n-s개는 1 늘어난다는 사실로 변화량 `(n-s)-s=n-2s`를 유도하라.
- [ ] 76. `answer[v] = answer[parent[v]] + n - 2 * subtree_size[v]`가 모든 정점의 거리 합을 정확히 만드는 귀납 증명을 적어라.
- [ ] 77. 첫 순회는 parent/depth/order, 역순은 subtree, 정순은 reroot answer를 만든다. 각 정점·간선이 몇 번 처리되는지 세어 O(n) 시간을 증명하라.
- [ ] 78. adjacency, parent, order, stack, subtree, depth, answer 배열을 합쳐 O(n) 공간임을 설명하라.
- [ ] 79. 최악 경로의 거리 합이 `n(n-1)/2`까지 커지는 것을 계산하고 n=200,000에서 int가 아닌 64-bit 정수가 필요한 이유를 수치로 보이라.
- [ ] 80. n=1, 네 정점 path, 다섯 정점 star를 손으로 계산해 각각 `0`, `6 4 4 6`, `4 7 7 7 7`을 얻어라.
- [ ] 81. ICPC 코드의 vector count/fill/default 생성자, `reserve`, `operator[]`, `push_back`, `empty`, `back`, `pop_back`, `size` 각각에 대해 인자·반환·복잡도·재할당/무효화·빈 상태 UB를 구별하라.
- [ ] 82. `ios::sync_with_stdio(false)`, `cin.tie(nullptr)`, 입력 `>>`, 출력 `<<`의 반환값 사용과 stream 상태·오류를 설명하라. 동기화를 끄면 synchronized 표준 stream의 동시 접근 data-race 예외 보장을 쓸 수 없다는 점도 포함하라.

## G. 손으로 고치고 실행하는 실기

- [ ] 83. `Box<std::string>`의 mutable lvalue, const lvalue, mutable rvalue, const rvalue 호출을 각각 하나씩 만들고 반환 참조에 값을 대입할 수 있는 경우만 표시하라. dangling 참조를 실제로 읽지는 않는다.
- [ ] 84. `content()`의 반환형을 임시로 `auto`로 바꾸어 복사 횟수와 원본 변경 가능성이 어떻게 달라지는지 컴파일·실행으로 비교한 뒤 원상 복구하라.
- [ ] 85. `Box<PortList>`를 추가해 lvalue에서는 원소 수를 빌려 읽고 rvalue에서는 새 vector owner로 이동하는 작은 검증을 작성하라.
- [ ] 86. main에서 publish 뒤 `audited_service`를 읽는 코드를 일부러 추가해 “유효한 참조”와 “의미 있는 이전 값”이 다르다는 것을 관찰한 뒤 제거하라. 특정 moved-from 출력값을 정답으로 고정하지 않는다.
- [ ] 87. factory의 `return Box<std::string>{...};`를 이름 있는 지역 반환으로 바꾸고, 보장된 prvalue 복사 생략과 선택적인 NRVO를 설명하되 주소나 로그 결과를 표준 보장으로 오해하지 않는다.
- [ ] 88. Tree Distances II를 n≤12 무작위 트리에서 각 정점 BFS oracle과 대조하고, path/star/random 사례를 각각 포함하라.
- [ ] 89. n=200,000 path를 실행해 recursion overflow가 없고 64-bit 양 끝 정답이 맞는지 확인하라.
- [ ] 90. w64devkit GCC 16.1.0, C++23, 높은 경고 설정으로 build한 뒤 CTest 6/6과 아래 stdout 전체 일치를 확인하라.

```text
audit=search-api:2
published=search-api:2
```

```text
copied=trace
moved=trace
```

- [ ] 91. `audit-standard-library-docs.ps1 -Scope latest`와 `-Scope all`, UTF-8, Markdown 링크, Mermaid, `git diff --check`를 검사하고 build 산출물이 stage되지 않았는지 확인하라.

## 답 확인 기준

- 문법 답은 키워드 번역에서 멈추지 않고 **접근 정책, 초기화 순서, 템플릿 추론 시점**까지 맞아야 한다.
- 값 범주 답은 이름 있는 rvalue-reference 변수가 lvalue라는 점, `std::move`/`std::forward_like`가 상태를 직접 바꾸지 않는 점, 뒤 생성자가 실제 복사·이동을 한다는 점을 구별해야 한다.
- 수명 답은 accessor 함수가 반환한 하위 객체 참조가 owner 수명을 늘리지 않으며, moved-from source를 가리키는 참조가 destination으로 재바인딩되지 않는다는 점을 포함해야 한다.
- 호출 계약 답은 매번 여섯 묶음을 모두 포함하고 string/vector의 복사·이동, `size`, 출력 연쇄, 비교의 반환 사용·무효화·오류·스레드 조건을 빠뜨리지 않아야 한다.
- 알고리즘 답은 parent-before-child order 불변식, 역순 subtree 누적, `n-2s` reroot 점화식, O(n) 시간·공간과 64-bit 필요성을 증명해야 한다.
- 마지막으로 자료를 보지 않고 아래 문장을 완성한다.

> 이름 있는 explicit-object 매개변수 `self`는 ______ 식이다. `std::forward_like<Self>(member)`는 Self의 ______와 ______ 성질을 member 참조에 투영하지만 실제 ______은 하지 않는다. lvalue wrapper에서는 값을 ______고 rvalue wrapper에서는 소유권을 ______ 수 있다. 트리 root를 parent에서 child로 옮길 때 subtree s개 거리는 ______, 나머지 n-s개 거리는 ______하므로 정답 변화량은 ______이고 전체 시간은 ______, 공간은 ______이다.
