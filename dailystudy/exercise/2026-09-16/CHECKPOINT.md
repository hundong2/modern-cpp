# 2026-09-16 CHECKPOINT — `std::any`, 소유 타입 소거, min-plus 행렬 지수승

이 문서는 [`main.cpp`](main.cpp) → [`problem.cpp`](problem.cpp) → [`icpc_problem.cpp`](icpc_problem.cpp) 순서로 읽은 뒤 자료를 닫고 푼다. 답을 “복사한다”, “빠르다”처럼 한 단어로 끝내지 말고 **누가 값을 소유하는지, 식의 값 범주는 무엇인지, 호출 전후 어떤 객체와 관찰자가 유효한지**까지 설명한다.

표준 라이브러리 호출 문항은 매번 아래 여섯 묶음을 빠짐없이 말해야 통과다.

1. 수신 객체의 정확한 타입·cv 한정·값 범주와 호출 직전 상태
2. 선택된 signature·overload·템플릿 인자
3. 각 인자 식의 타입·값 범주·허용값과 복사/이동/차용·소유권 의미
4. 반환형·반환값 의미와 호출부가 저장·연쇄·검사·무시하는지
5. 호출 뒤 수신 객체·인자·contained object/backing storage의 상태 변화
6. 전제·후조건, 복잡도, 할당, 무효화, 수명, 오류·예외·UB와 스레드 보장

## A. 초보자를 위한 기초 문법 검증

- [ ] 1. `<any>`, `<string>`, `<unordered_map>`, `<utility>`, `<iostream>`이 `main.cpp`에서 직접 제공하는 선언을 하나씩 연결하라.
- [ ] 2. `<cstddef>`, `<cstdint>`, `<iostream>`, `<vector>`가 `icpc_problem.cpp`에 필요한 이유를 각각 설명하라.
- [ ] 3. 오늘 코드의 `int`, `bool`, `std::size_t`, `std::int64_t`가 표현하는 값을 찾고 signed/unsigned와 폭의 차이를 말하라.
- [ ] 4. `using Storage = std::unordered_map<std::string, std::any>;`와 `using Cost = std::int64_t;`가 새 타입을 만드는지, 원래 타입과 overload 관점에서 같은 타입인지 답하라.
- [ ] 5. `std::unordered_map<std::string, std::any>`의 두 타입 템플릿 인자와 `std::vector<Cost>`의 타입 템플릿 인자가 각각 어떤 역할인지 말하라.
- [ ] 6. `put<std::string>`과 `get<int>`의 명시적 함수 템플릿 인자 `T`가 언제 결정되고 함수 몸체에서 어디에 쓰이는가?
- [ ] 7. `class MetadataBag`, `class RequestContext`, `class AnySlot`, `class MinPlusMatrix`의 기본 접근 지정자는 무엇이며, 소유 상태를 `private`로 감춘 이유는 무엇인가?
- [ ] 8. 같은 멤버를 `struct`에 선언했을 때 달라지는 기본 접근과, class/struct가 모두 사용자 정의 타입이라는 공통점을 설명하라.
- [ ] 9. `explicit MetadataBag(size_type expected_entries)`에서 `explicit`가 금지하는 암시 변환/copy-initialization 예를 쓰고 직접 초기화 예와 비교하라.
- [ ] 10. `AnySlot(std::any payload) : payload_{std::move(payload)}`에서 매개변수 수명, 멤버 초기화 목록 실행 시점, 본문 대입과의 차이를 설명하라.
- [ ] 11. 여러 멤버가 있을 때 실제 초기화 순서는 초기화 목록에 쓴 순서와 클래스의 선언 순서 중 무엇을 따르는가?
- [ ] 12. `std::string{"trace"}`, `int{3}`, `Cost{0}`의 중괄호 초기화가 표현하는 타입과 narrowing 방지 역할을 설명하라.
- [ ] 13. `void put(std::string key, T value)`에서 반환형, 두 매개변수의 전달 방식과 각 지역 객체의 소유권을 말하라.
- [ ] 14. `const T* get(const std::string& key) const`에서 앞의 `const`, 포인터 `*`, 참조 `&`, 함수 뒤 `const`가 각각 무엇을 제한하는가?
- [ ] 15. `const T*`는 null을 표현하지만 정상 바인딩된 `const T&`는 null 상태를 모델링하지 않는다는 차이를 설명하라.
- [ ] 16. `if (found == values_.end())`가 iterator 역참조 전에 필요한 이유와 즉시 `return nullptr` 뒤 나머지 문장이 실행되지 않는 이유를 말하라.
- [ ] 17. `missing_key ? "true" : "false"`에서 조건 연산자가 평가하는 branch 수와 결과 타입을 말하고, 문자열 리터럴의 타입·저장 기간도 설명하라.
- [ ] 18. `return 0`과 CTest가 관찰하는 프로세스 종료 성공의 관계를 설명하라.

## B. `std::any`와 소유 타입 소거

- [ ] 19. `std::any`의 두 논리 상태를 말하고, non-empty 상태에서 contained object를 누가 소유하고 언제 파괴하는가?
- [ ] 20. “타입 소거”가 런타임 타입 정보까지 없앤다는 뜻이 아닌 이유를 `any_cast<T>`와 연결해 설명하라.
- [ ] 21. `std::any{std::string{"req-42"}}`가 문자열 문자를 빌리는지 소유하는지, 원래 임시 문자열 수명이 끝난 뒤에도 값이 남는 이유를 말하라.
- [ ] 22. 저장 타입이 `int`일 때 `any_cast<long>`은 실패하지만 pointer `any_cast<const int>`는 성공할 수 있는 이유를 `typeid`의 최상위 cv 규칙과 값 변환 부재로 설명하라.
- [ ] 23. 저장 타입 `const char*`와 `std::string`이 문자 내용이 같아도 다른 조회 타입인 이유를 말하라.
- [ ] 24. `using TraceId = std::string`과 `struct TraceId { std::string value; };` 중 어느 것이 별도 stored type이 되는지 설명하라.
- [ ] 25. `std::any_cast<T>(&found->second)`가 성공할 때와 실패할 때의 반환값을 쓰고 pointer overload가 타입 불일치를 어떻게 보고하는가?
- [ ] 26. 값/reference overload `std::any_cast<T>(value)`와 pointer overload의 실패 방식 차이를 `std::bad_any_cast`와 null로 비교하라.
- [ ] 27. `get<T>`가 key 누락과 type mismatch 모두 null을 반환하더라도 내부에서는 어느 두 단계에서 서로 다른 원인이 생기는가?
- [ ] 28. `get<T>`가 돌려준 `const T*`가 새 객체를 소유하지 않는다는 것을 주소 화살표로 그리고 owner가 누구인지 표시하라.
- [ ] 29. 반환 포인터를 무효로 만드는 같은 key의 덮어쓰기, mapped `any::reset`, 원소 erase, bag 파괴를 구별하라.
- [ ] 30. `unordered_map` rehash가 iterator는 무효화하지만 원소 reference/pointer는 무효화하지 않는 규칙과, 기존 key에 `insert_or_assign`한 경우 contained object 포인터가 무효화되는 이유를 구분하라.
- [ ] 31. `std::any copied{source}`가 contained `std::string`까지 독립적으로 복사하는 이유와, 이후 한 문자열 변경이 다른 문자열 값을 바꾸지 않는 이유를 설명하라.
- [ ] 32. `std::any moved{std::move(source)}`가 선택하는 생성자와 `moved`가 얻는 논리 값을 설명하라.
- [ ] 33. 위 이동 뒤 `source.has_value() == false`를 표준 보장으로 단정할 수 없는 이유를 말하라.
- [ ] 34. `source.reset(); const bool source_reset{!source.has_value()};`가 이동 결과 관찰이 아니라 **명시적 빈 상태 전환** 검증인 이유를 설명하라.
- [ ] 35. `reset()` 뒤 source 객체 자체의 수명과 contained object의 수명을 구분하고, source에 새 값을 대입할 수 있는지 답하라.
- [ ] 36. `std::any`의 contained type이 CopyConstructible이어야 하는 이유를 any 자체의 copyable 계약과 연결하라.
- [ ] 37. contained object 저장이 항상 heap allocation 또는 항상 inline이라고 단정할 수 없는 이유를 타입 크기·정렬·구현 최적화 관점에서 말하라.
- [ ] 38. 필드 집합이 고정된 경우 `struct`, 닫힌 후보 집합인 경우 `std::variant`, 열린 부가 메타데이터인 경우 `std::any`를 선택할 때 얻고 잃는 것을 비교하라.

## C. 실제 식의 값 범주·복사·이동·수명 해석

- [ ] 39. 이름 있는 `context`, `key`, `value`, `source`, `copied`, `moved`, `found`, `adjacency`, `result`는 식으로 쓰일 때 어떤 값 범주인가?
- [ ] 40. `std::string{"trace"}`, `std::any{std::move(value)}`, `Cost{0}`, `from - 1`의 값 범주를 각각 말하라.
- [ ] 41. `std::move(key)`, `std::move(value)`, `std::move(source)`의 결과 타입과 값 범주를 말하고, 호출 자체가 객체를 바꾸지 않는 이유를 설명하라.
- [ ] 42. `put<std::string>(std::string{"trace"}, std::string{"req-42"})`에서 두 prvalue가 두 값 매개변수를 초기화하고, 이름 있는 매개변수가 다시 lvalue가 되는 과정을 적어라.
- [ ] 43. `values_.insert_or_assign(std::move(key), std::any{std::move(value)})`에서 첫 인자와 둘째 인자의 타입·값 범주·최종 소유자를 순서대로 적어라.
- [ ] 44. `put<int>(std::string{"retries"}, 3)`에서 `std::move(value)`가 int에 적용돼도 자원 포인터 이전 같은 의미가 없는 이유를 말하라.
- [ ] 45. `const auto found{values_.find(key)}`에서 `auto`가 추론하는 const_iterator 계열 타입과, iterator 값 자체의 복사가 map 원소 복사가 아닌 이유를 설명하라.
- [ ] 46. `found->second`의 값 범주와 타입을 쓰고, 주소를 취했을 때 어떤 `any_cast` pointer overload가 선택되는가?
- [ ] 47. `std::any& source{source_slot.value()}`가 새 any를 만들지 않는다는 점, source_slot과 source의 수명 관계를 설명하라.
- [ ] 48. `std::any copied{source}`에서 source lvalue가 const-reference copy constructor 입력에 바인딩되는 과정을 설명하라.
- [ ] 49. `std::any moved{std::move(source)}`에서 xvalue가 rvalue-reference move constructor 입력에 바인딩되고, `std::move`와 생성자의 역할이 어떻게 다른가?
- [ ] 50. `std::any_cast<std::string>(&copied)`와 `&moved` 결과 포인터를 저장하는 동안 copied/moved가 먼저 파괴되면 안 되는 이유를 말하라.
- [ ] 51. main의 지역 객체가 정상 return에서 생성 역순으로 파괴되고 map·any·string 자원이 RAII로 회수되는 순서를 큰 객체부터 그려라.
- [ ] 52. C++17에서 prvalue가 같은 타입 목적 객체를 직접 초기화하는 보장 복사 생략과 `return local;`의 선택적 NRVO를 구별하라.
- [ ] 53. 오늘 `get`의 포인터 반환과 `min_plus_multiply`의 output-reference 방식이 왜 RVO/NRVO에 정확성을 의존하지 않는가?
- [ ] 54. `MinPlusMatrix& output`, `const MinPlusMatrix& left`, `Cost& best`가 각각 소유 객체인지 비소유 별칭인지, 수정 가능성과 수명을 구별하라.
- [ ] 55. `vector::swap` 뒤 vector 객체 자체를 가리키는 참조와 원소를 가리키는 참조가 각각 어느 객체/원소를 계속 가리키는지 말하라.

## D. 표준 라이브러리 호출 계약

각 항목에 대해 문서 맨 위의 여섯 묶음을 모두 답한다. 특히 **매개변수, 반환값 사용 여부, 무효화, 오류, 스레드 조건**을 생략하지 않는다.

- [ ] 56. `std::string{"trace"}`의 `basic_string(const char*, allocator)` 생성자에서 아직 없는 destination, 배열-포인터 변환 인자, 반환 없음, 독립 소유, O(n), NUL 전제, 할당 실패를 설명하라.
- [ ] 57. 빈 `Storage values_`의 기본 생성과 `values_.reserve(expected_entries)`에서 수신 상태, `size_type` 값 인자, void 반환, bucket 변화, 평균/최악 rehash 비용, iterator와 reference/pointer 무효화를 설명하라.
- [ ] 58. `std::move(key)`에서 추론되는 템플릿 인자, `std::string` lvalue 인자, xvalue-reference 반환 사용, 상태 무변경, `noexcept`와 수명을 설명하라.
- [ ] 59. `std::any{std::move(value)}`의 converting constructor에서 `T=std::string`인 경우와 `T=int`인 경우의 인자·contained type·소유 결과·예외를 비교하라.
- [ ] 60. `values_.insert_or_assign(std::move(key), std::any{...})`가 새 key를 삽입하는 경로와 기존 key의 mapped any를 대입하는 경로를 구별하라.
- [ ] 61. 위 `insert_or_assign`의 반환 `std::pair<iterator,bool>`에서 bool 의미와 오늘 호출부가 반환을 버리는 이유, 평균/최악 복잡도, rehash·할당·예외·무효화를 설명하라.
- [ ] 62. `values_.find(key)`의 const 수신자, `const std::string&` 인자, const_iterator 반환 저장, 평균/최악 복잡도, 상태 무변경과 동시 읽기 조건을 설명하라.
- [ ] 63. `values_.end()` const overload의 무인자 호출, past-the-end iterator 반환, O(1), 역참조 금지와 컨테이너 변경 시 유효성을 설명하라.
- [ ] 64. `found == values_.end()` 또는 `!=`의 두 iterator 전제, bool 반환 사용, 무할당·무변경과 서로 다른 컨테이너 iterator 비교를 피해야 하는 이유를 설명하라.
- [ ] 65. `found->second` iterator `operator->`가 원소 주소를 통해 mapped any에 접근하는 과정과 iterator가 end가 아니어야 하는 전제를 설명하라.
- [ ] 66. `std::any_cast<T>(&found->second)`의 const-any pointer 인자와 `std::any_cast<std::string>(&copied)`의 mutable-any pointer 인자를 비교하라. 저장 타입과 `typeid(T)`의 일치 및 최상위 cv 처리, 각각의 `const T*`/`T*` 반환, mutable 결과를 const 포인터 변수로 바꾸는 qualification conversion, null 검사, 상태 무변경, 포인터 수명과 무동기 변경 금지를 설명하라.
- [ ] 67. `std::any copied{source}` copy constructor의 수신 전 상태, const any reference 입력, 반환 없음, contained object 깊은 복사, 예외 시 source 유지와 비용 의존성을 설명하라.
- [ ] 68. `std::any moved{std::move(source)}` move constructor의 xvalue 입력, 반환 없음, destination/source 사후 상태, `noexcept`, source가 빈다는 보장이 없다는 점과 관찰자 수명을 설명하라.
- [ ] 69. `source.reset()`의 정확한 non-const any lvalue 수신, 무인자·void 반환, contained object 파괴, 빈 후조건, `noexcept`, 기존 contained 포인터 무효화를 설명하라.
- [ ] 70. `source.has_value()`의 const 관찰 계약, 무인자, bool 반환이 `!`에 사용되는 과정, 별도 표준 Complexity 절 유무, `noexcept`, 상태·관찰자 무변경을 설명하라.
- [ ] 71. main/problem의 `std::cout <<` 연쇄에서 C 문자열, `std::string`, `int`, char overload를 구분하고 각 `std::ostream&` 반환의 연쇄/마지막 사용 여부를 적어라.
- [ ] 72. 출력 전후 cout 상태, 입력 값 소유권, 생성 문자 비용, buffer와 장치 오류, exception mask, 여러 스레드 출력 시 문자 섞임 가능성을 설명하라.
- [ ] 73. ICPC의 `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`에서 인자, 이전 상태 반환값 무시, 전역/stream 상태 변화와 C stdio·동시 접근 주의를 설명하라.
- [ ] 74. `std::cin >> vertex_count >> edge_count >> exact_edge_count`에서 `int&`/`Cost&` overload, stream-reference 연쇄, 성공·실패 후 변수와 상태, 범위 오류와 예외 mask를 설명하라.
- [ ] 75. `std::vector<Cost>(count, initial_value)` fill 생성자에서 count/value/allocator, 반환 없음, n² 원소 소유, O(n²), 할당·`length_error`·`bad_alloc`과 실패한 목적 객체를 설명하라.
- [ ] 76. const/non-const `vector::operator[]`와 `vector::swap` 각각의 인자·반환·복잡도·범위 UB·저장소 교환·iterator/reference 무효화·allocator 전제·스레드 조건을 비교하라.

## E. 기계 실행 관점

- [ ] 77. `unordered_map::find`가 개념상 hash, bucket 선택, key 비교, 조건 분기를 수행할 수 있지만 정확한 자료 배치와 probe 수를 표준이 고정하지 않는 이유를 말하라.
- [ ] 78. pointer `any_cast`가 타입 정보 비교와 주소/null 선택으로 구현될 수 있지만 특정 RTTI 포인터 비교나 명령 수를 보장하지 않는 이유를 설명하라.
- [ ] 79. any의 복사·이동이 관리 함수 간접 호출 또는 inline code가 될 수 있고, 이를 반드시 가상 호출이라고 부를 수 없는 이유를 말하라.
- [ ] 80. `std::move`가 cast 성격이라 source code의 함수 이름이 실제 call instruction이나 상태 store를 보장하지 않는 이유를 설명하라.
- [ ] 81. min-plus의 평탄 인덱스가 vector buffer load, INF compare, branch, add, min compare, store로 번역될 수 있으나 최적화 뒤 합쳐지거나 벡터화될 수 있음을 설명하라.
- [ ] 82. 실제 성능과 명령이 CPU, ABI, compiler, 표준 라이브러리 구현, 최적화 옵션과 입력의 도달 가능성 분포에 따라 달라지는 이유를 말하라.

## F. CSES 1724 Graph Paths II — min-plus 행렬 지수승

- [ ] 83. 문제의 path가 정확히 k개 간선을 쓰는 walk라는 뜻을 설명하고, 정점·간선 재방문을 금지하면 왜 다른 문제가 되는가?
- [ ] 84. `A[i][j]`를 “정확히 한 간선으로 i에서 j까지 가는 최소 비용”으로 정의하고 평행 간선 처리식을 쓰라.
- [ ] 85. 실제 self-loop가 없는 대각선을 0으로 초기화하면 정확한 간선 수가 어떻게 흐려지는지 작은 반례를 들어라.
- [ ] 86. 도달 불가능 칸을 `INF`로 표현하고, 두 피연산자 중 하나가 INF면 덧셈을 건너뛰는 이유를 설명하라.
- [ ] 87. `(X⊗Y)[i][j] = min_p(X[i][p]+Y[p][j])`에서 pivot `p`의 의미와 앞·뒤 walk의 간선 수가 더해지는 이유를 말하라.
- [ ] 88. 모든 pivot을 조사한 최소가 정확히 이어 붙일 수 있는 모든 walk를 포함한다는 완전성과, 선택한 두 최적 구간을 이으면 유효한 walk가 된다는 건전성을 증명하라.
- [ ] 89. min-plus 곱의 결합법칙이 walk를 세 구간으로 나누는 경계 선택과 `min`/`+`의 성질에서 나오는 이유를 설명하라.
- [ ] 90. min-plus 항등행렬이 대각선 0, 나머지 INF인 이유와 길이 0 walk의 의미를 연결하라.
- [ ] 91. `A^r[i][j]`가 정확히 r개 간선을 쓰는 최소 비용이라는 명제를 r에 대한 귀납으로 증명하라.
- [ ] 92. 이진 지수승의 `result ⊗ power^exponent = A^k` 불변식이 초기 상태에서 성립함을 보이라.
- [ ] 93. exponent의 최하위 비트가 1인 경우 result에 power를 곱한 뒤에도 불변식이 유지되는 식을 쓰라.
- [ ] 94. power를 제곱하고 exponent를 2로 나누는 단계가 남은 비트 의미를 보존하는 이유와 종료성을 증명하라.
- [ ] 95. 종료 시 exponent=0에서 result=A^k가 되고 `[0][n-1]`이 답이라는 결론을 도출하라.
- [ ] 96. 한 번의 곱이 세 중첩 루프로 O(n³), 반복 횟수가 O(log k)임을 세고 총 시간 복잡도를 증명하라.
- [ ] 97. adjacency/power, result, scratch의 행렬 수가 상수이고 각 n²칸이라는 사실로 O(n²) 공간을 증명하라.
- [ ] 98. 공식 최대 비용 `k*c <= 10^18`, `Cost=int64_t`, `INF=4*10^18`의 관계를 수치로 확인하라.
- [ ] 99. `left_cost > INF - right_cost`를 덧셈 전에 검사하는 이유와 signed overflow가 C++에서 미정의 동작인 점을 설명하라.
- [ ] 100. 공식 예제를 손으로 최소화해 27을 확인하고, 어느 8개 간선 walk를 택하는지 적어라.
- [ ] 101. 평행 간선 테스트에서 직접 1→3 비용 1이 있어도 정확히 2개 간선 답이 7인 이유를 말하라.
- [ ] 102. `1→2`, `1→3`만 있는 그래프에서 정확히 2개 간선으로 1→3이 불가능해 -1인 이유를 행렬의 INF와 연결하라.
- [ ] 103. 두 정점 cycle, `k=999999999`에서 비용 7 간선 수와 비용 4 간선 수를 세어 `5499999996`을 계산하라.
- [ ] 104. `n=2`에서 간선 `1→1(1)`, `1→2(5)`를 두고 `k=1`의 답 5와 `k=2`의 답 6을 계산해 self-loop도 정확한 간선 수 한 칸을 차지함을 설명하라.

## G. 손으로 고치고 실행하는 실기

- [ ] 105. `get<int>("trace")`, `get<std::string>("trace")`, `get<std::string>("missing")`을 한 번씩 호출해 저장 타입 일치·불일치·누락 결과를 표로 기록하라.
- [ ] 106. `std::any`에 `const char*`를 저장하는 임시 실험을 하고 `any_cast<std::string>`이 실패하는 것을 확인한 뒤, 저장 경계에서 소유 string으로 정규화하라.
- [ ] 107. copied의 문자열을 변경해 source와 독립임을 확인하되 포인터 cast의 null을 먼저 검사하라.
- [ ] 108. move 직후 source가 비었음을 요구하는 검사를 일부러 작성해 왜 이식 가능 계약이 아닌지 설명하고, `reset()` 뒤 검사로 되돌려라.
- [ ] 109. `get` 포인터를 얻은 뒤 같은 key를 덮어쓰는 코드를 만들고, 과거 포인터를 역참조하지 않은 채 무효화 시점을 주석으로 표시하라.
- [ ] 110. `std::variant<std::string,int>` 기반 map을 작은 대안으로 작성해 any와 compile-time 대안 집합·방문 처리 차이를 비교하라.
- [ ] 111. `n<=6`, `k<=8` 무작위 방향 그래프를 만들고 `dp[step][v]` oracle과 min-plus 결과를 수백 회 대조하라.
- [ ] 112. self-loop, 평행 간선, 정확 길이 불가, `k=1`, 큰 k cycle을 각각 독립 테스트로 실행하라.
- [ ] 113. min-plus 항등행렬의 비대각 INF 하나를 0으로 바꾸어 어떤 exact-length 테스트가 실패하는지 확인하고 원상 복구하라.
- [ ] 114. w64devkit g++, C++23, 높은 경고 설정으로 build한 뒤 CTest 6개가 아래 stdout 전체와 일치하는지 확인하라. 실행 전에는 체크하지 않는다.

```text
trace=req-42
retries=3
missing=true
wrong-type=true
```

```text
copied=trace
moved=trace
source-reset=true
```

- [ ] 115. `audit-standard-library-docs.ps1 -Scope all`, `git diff --check`, UTF-8, Markdown 상대 링크와 Mermaid 렌더링을 검사하고 build 산출물이 stage되지 않았는지 확인하라.

## 답 확인 기준

- 기초 문법 답은 키워드 번역을 넘어 접근 정책, 직접 초기화, 멤버 초기화 순서, 템플릿 인자와 수명까지 맞아야 한다.
- `std::any` 답은 **소유 값**, 저장 타입과 `typeid(T)`의 일치(최상위 cv 제외), pointer cast의 null 실패, 반환 포인터의 비소유 수명과 열린 타입 집합의 trade-off를 포함해야 한다.
- 값 범주 답은 이름 있는 객체가 lvalue라는 점, `std::move`가 xvalue cast일 뿐이라는 점, 뒤 생성자가 실제 복사/이동을 수행한다는 점을 구별해야 한다.
- 이동 뒤 source any가 자동으로 빈다는 주장은 오답이다. `source-reset=true`는 명시적 `reset()` 뒤 `has_value()`로 확인한 결과다.
- 호출 계약 답은 여섯 묶음과 함께 반환값 사용, iterator/contained pointer 무효화, 할당·예외·UB, 같은 객체의 무동기 동시 변경 위험을 빠뜨리지 않아야 한다.
- 알고리즘 답은 exact-edge 상태, min-plus 곱의 경계 정점, 항등행렬, 이진 지수승 불변식, overflow guard와 `O(n^3 log k)`/`O(n^2)` 증명을 포함해야 한다.
- 마지막으로 자료를 보지 않고 아래 문장을 완성한다.

> `std::any`는 구체 값을 ______하면서 타입을 ______한다. pointer `any_cast<T>`는 저장 타입이 T와 ______하면 contained object의 ______를, 아니면 ______를 반환한다. `std::move(source)`는 source를 ______ 식으로 보이게 하지만 source를 자동으로 ______지는 않는다. min-plus 곱은 경계 정점 ______에서 정확한 간선 수의 두 walk를 이어 붙이고, 이진 지수승 불변식은 ______이며 전체 시간은 ______, 공간은 ______이다.
