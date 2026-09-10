# 2026-09-11 CHECKPOINT — `std::ranges::to`와 영속 세그먼트 트리

체크 표시는 답을 소리 내어 설명하고 실제 식의 호출 전후 상태를 종이에 그린 뒤에만 한다. 표준 라이브러리 호출 문제는 매번 다음 여섯 묶음을 빠짐없이 답해야 통과다.

1. 수신 객체의 정확한 타입, 값 범주, 호출 전 size/capacity/소유 상태
2. 선택된 signature·overload, 템플릿 인자와 숨은 `this`
3. 각 인자 식의 타입·값 범주·허용값, 복사/이동/차용과 소유권 의미
4. 반환형·반환값 의미, 호출부가 저장·연쇄·검사·무시하는지
5. 호출 뒤 수신 객체와 각 인자의 값·크기·용량·소유권·수명 변화
6. 전제/후조건, 시간·공간 복잡도, 할당, 무효화, 수명, 오류·예외·UB, 스레드 보장

## A. 기초 문법과 타입

- [ ] 1. `<ranges>`, `<vector>`, `<string>`, `<utility>`, `<cstddef>`, `<iostream>`이 오늘 코드에서 각각 어떤 선언을 직접 제공하는가?
- [ ] 2. `int`, `bool`, `long long`, `std::size_t`를 오늘 코드가 각각 어떤 값에 쓰며 signed/unsigned 차이는 무엇인가?
- [ ] 3. `int n{0};`, `Node{sum,left,right}`, 초기화하지 않은 지역 `int n;`의 초기 상태와 narrowing 차이를 설명하라.
- [ ] 4. `const int minimum_score`, `const UserRow& row`, 함수 뒤의 `const`가 서로 무엇을 const로 만드는가?
- [ ] 5. `struct UserRow`와 `class UserRepository`의 기본 접근 차이와, 접근 지정자 외 언어 수준 차이가 있는지 말하라.
- [ ] 6. `public` query/command와 `private rows_` 분리가 어떤 불변식을 보호하는가?
- [ ] 7. `using UserCards = std::vector<UserCard>;`가 새 타입인지 별칭인지 말하고 템플릿 인자 `UserCard`의 역할을 설명하라.
- [ ] 8. `using Values = std::vector<long long>;`에서 `Values`, vector 특수화, 원소 타입을 각각 구별하라.
- [ ] 9. `explicit UserRepository(UserRows rows)`의 반환형·값 매개변수·`explicit` 의미를 설명하라.
- [ ] 10. `: rows_{std::move(rows)}` 멤버 초기화 목록과 생성자 본문에서 대입하는 코드의 객체 수명 차이를 설명하라.
- [ ] 11. `const UserRepository& repository_`가 왜 생성자 목록에서 반드시 바인딩되어야 하며 무엇을 소유하지 않는가?
- [ ] 12. `[[nodiscard]] UserCards load_cards(const int minimum_score) const`의 attribute, 반환형, 매개변수, 마지막 `const`를 차례로 읽어라.
- [ ] 13. `[minimum_score](const UserRow& row)`에서 capture, 매개변수 참조, 반환 `bool`의 역할을 설명하라.
- [ ] 14. `for (UserRow& row : rows_)`가 원소를 복사하는지 빌리는지, `return`을 만나면 제어가 어디로 가는지 말하라.
- [ ] 15. `if (query_left <= middle)`과 뒤의 독립된 `if (middle < query_right)`를 `else if`로 바꾸면 왜 구간 질의가 틀릴 수 있는가?
- [ ] 16. 입력의 1-based position/version과 내부 0-based index를 어느 경계에서 변환하는지 세 식을 찾아라.

## B. lvalue/prvalue/xvalue, 참조 바인딩, 복사·이동·수명

- [ ] 17. 이름 있는 `seed`와 생성자 매개변수 `rows`의 값 범주가 둘 다 lvalue인 이유를 설명하라.
- [ ] 18. `std::move(seed)`의 정확한 반환형·값 범주와 move 자체가 vector 버퍼를 옮기지 않는 이유를 말하라.
- [ ] 19. seed → 값 매개변수 rows → 멤버 rows_로 소유권이 이동하는 두 단계를 그려라.
- [ ] 20. 각 vector 이동 뒤 원본은 어떤 상태이며, 파괴·대입·size 가정 중 무엇이 허용되는가?
- [ ] 21. `std::views::filter(rows_, predicate)` 반환 식의 값 범주와, 이름 `eligible`로 초기화한 뒤 `eligible` 식의 값 범주를 구별하라.
- [ ] 22. filter view가 보관하는 원본 범위 표현과 predicate의 소유/차용 상태, forward range에서 첫 통과 반복자를 cache할 수 있어 변경 뒤 같은 view 대신 새 view를 만들어야 하는 이유를 설명하라.
- [ ] 23. `std::views::transform(eligible, projector)`가 만들어질 때 `UserCard`가 아직 생성되지 않는 이유는?
- [ ] 24. `UserCard{row.id,row.name}`의 값 범주와 두 멤버가 각각 값 복사·문자열 복사를 하는 과정을 말하라.
- [ ] 25. `row.name`을 xvalue로 바꾸지 않은 이유와, const 행에서 이동을 시도할 때 보통 복사가 선택되는 이유를 설명하라.
- [ ] 26. `std::ranges::to<UserCards>(projected)`가 반환하는 값의 타입·값 범주·소유 저장소를 말하라.
- [ ] 27. `return std::ranges::to<UserCards>(projected);`과 `UserCards first{service.load_cards(80)};` 사이에 필수 vector 복사/이동이 있는가? 보장 복사 생략과 NRVO를 구별해 답하라.
- [ ] 28. repository → service reference → filter view → transform view의 수명 의존 방향을 화살표로 그려라.
- [ ] 29. `first` snapshot이 repository 파괴 뒤에도 문자열을 안전하게 읽을 수 있는 이유를 원소 수준 깊은 복사로 설명하라.
- [ ] 30. DTO가 `string_view` 또는 raw pointer를 저장했다면 `ranges::to<vector>`만으로 대상 문자까지 소유하게 되는지 반례를 들어라.

## C. 실제 식 해석

- [ ] 31. `UserRows seed{UserRow{...}, ...};`에서 문자열 literal, 임시 string, UserRow, initializer_list 원소, 최종 vector 원소의 생성·파괴 순서를 적어라.
- [ ] 32. `return row.active && row.score >= minimum_score;`의 단락 평가, lvalue-to-rvalue 변환, 비교와 bool 결과를 해석하라.
- [ ] 33. `auto eligible = repository_.active_with_minimum(minimum_score);`에서 `auto`가 reference인지 view 값인지 설명하라.
- [ ] 34. `auto projected = std::views::transform(eligible, projector);`가 기반 행 소유권을 옮기지 않는 근거를 타입 관계로 말하라.
- [ ] 35. `return std::ranges::to<UserCards>(projected);`의 `C`, `R`, `Args...`, 인자 값 범주를 정확히 채워라.
- [ ] 36. `repository.deactivate(1);` 뒤에도 `first[0].name`이 `Ada`인 반면 새 `second`에는 Ada가 없는 이유를 시간 순서로 설명하라.
- [ ] 37. `const std::string& first_other_name{first[1].name};`의 표준 vector `operator[]`, 멤버 접근, reference binding과 수명을 구분하라.
- [ ] 38. `std::cout << first_count << ' ' << first_name ...`의 각 피연산자 타입·값 범주와 반환 stream reference 사용을 적어라.
- [ ] 39. `Values values(static_cast<std::size_t>(n));`가 iterator-range가 아니라 count 생성자를 선택하는 이유는?
- [ ] 40. `const std::size_t expected_nodes{2U * size_t(n) + 24U * size_t(q) + 1U};`가 int 곱셈 overflow를 피하는 원리를 usual arithmetic conversions로 설명하라.
- [ ] 41. `Node next{nodes_[size_t(old_index)]};`가 참조가 아니라 지역 값 복사인 이유와 재귀 append 중 재할당 안전성을 설명하라.
- [ ] 42. `next.left_child = assign_impl(...)` 전후에 반대 자식 index가 공유되고 목표 자식만 새 index로 바뀌는 과정을 그려라.
- [ ] 43. `return append_node(next);`가 자식과 합을 모두 완성한 뒤 한 번만 실행되어 공개된 arena node를 다시 수정하지 않음을 증명하라.
- [ ] 44. `roots[root_slot] = tree.assign(old_root, n, position - 1, value);`의 오른쪽 평가 결과와 type 1의 version 의미를 설명하라.
- [ ] 45. `const int copied_root{roots[size_t(version-1)]};` 뒤 `roots.push_back(copied_root);`가 배열 전체 복사가 아니라 root 정수 복사인 이유와 지역 복사·reserve의 역할을 말하라.

## D. 표준 라이브러리 호출 계약

각 항목에 대해 문서 첫머리의 여섯 묶음을 모두 답한다.

- [ ] 46. `std::string{"Ada"}`와 `UserCard{row.id,row.name}` 안 문자열 복사 생성자의 포인터/참조 인자, 반환 없음, 독립 버퍼, 선형 비용·할당 실패를 비교하라.
- [ ] 47. UserRows initializer-list 생성자의 const 원소 복사와 `Values values(count)`의 value-initialized 0 원소를 비교하라.
- [ ] 48. NodePool/vector<int> 기본 생성자의 인자·반환·초기 size와 allocator/예외/스레드 계약을 설명하라.
- [ ] 49. vector 이동 생성과 `std::move(rows)`의 역할을 분리하고, 반환 reference·moved-from 상태·복잡도·noexcept 조건을 설명하라.
- [ ] 50. `std::views::filter(rows_, predicate)`의 두 인자 바인딩, 반환 view, lazy 실행, 원본 상태, 수명·무효화·스레드 전제를 설명하라.
- [ ] 51. `std::views::transform(eligible, projector)`의 기반 view/함수 객체 보관, 반환형, 호출 시점, 예외 전파를 설명하라.
- [ ] 52. `std::ranges::to<UserCards>(projected)`의 정확한 template 인자·overload, forwarding reference, 반환 prvalue, 결과 소유권을 설명하라.
- [ ] 53. 위 파이프가 `sized_range`는 아니지만 `forward_range`인 이유와, 거리·구성 순회에서 술어가 반복 평가될 수 있어도 vector 재할당은 없는 이유를 설명하고 n, k, L에 대한 시간·공간을 쓰라.
- [ ] 54. `nodes_.reserve(expected_nodes)`와 `roots.reserve(q+1)`의 수신 전후 size/capacity, void 반환, 할당·예외·전체 관찰자 무효화를 설명하라.
- [ ] 55. `nodes_.push_back(node)`의 선택 overload, node lvalue 소유 의미, 반환형, 상각 비용과 재할당 예외/무효화를 설명하라.
- [ ] 56. 실제 `roots.push_back(initial_root)`와 `roots.push_back(copied_root)`가 모두 외부 int lvalue를 빌리는 이유를 말하고, 가상의 `roots.push_back(roots[index])`에서 내부 별칭을 먼저 지역 값으로 분리하면 추론이 쉬워지는 이유를 설명하라.
- [ ] 57. `nodes_.size()`와 snapshot `first.size()`의 반환형·사용처, O(1), noexcept·비무효화 계약을 설명하라.
- [ ] 58. `nodes_[index]`, `roots[root_slot]`, `first[0]`의 const/non-const overload, 반환 참조, 범위 전제·UB·재할당 수명을 설명하라.
- [ ] 59. `std::ios::sync_with_stdio(false)`의 bool 인자·이전 상태 반환/무시, 첫 I/O 전 호출 이유와 C stdio 혼용 영향을 설명하라.
- [ ] 60. `std::cin.tie(nullptr)`와 `std::cin >> n >> query_count`의 pointer/정수 참조 인자, 반환값 사용, 상태 비트·범위 오류·설정 예외를 설명하라.
- [ ] 61. string/정수 삽입과 `operator<<(std::ostream&, char)`의 overload, 반환 연쇄, 출력 후 상태, 문자 수 비용·오류·동시성 보장을 설명하라.
- [ ] 62. vector/string/ranges materialization 중 `std::bad_alloc`이 날 때 완성된 부분 객체와 원본 부수 효과에 어떤 보장이 있는가?

## E. 경로 복사 영속 세그먼트 트리 불변식과 증명

- [ ] 63. node index `v`와 담당 구간 `[l,r]`에 대한 `nodes_[v].sum` 불변식을 정확히 진술하라.
- [ ] 64. “공개된 node는 immutable”에서 공개 시점을 설명하고 새 node가 공개 전 조립되는 것은 왜 허용되는가?
- [ ] 65. build의 leaf와 internal node 경우를 나눠 구간 길이에 대한 귀납 증명을 하라.
- [ ] 66. point assign이 높이마다 정확히 한 node만 복사하고 반대 자식을 공유함을 재귀 호출로 보이라.
- [ ] 67. 갱신 leaf부터 root까지 sum이 다시 정확해지는 것을 귀납적으로 증명하라.
- [ ] 68. old root에서 도달 가능한 node가 수정되지 않으므로 다른 version이 보존된다는 공유 안전 정리를 증명하라.
- [ ] 69. type 1이 새 version을 append하지 않고 선택한 `roots[k-1]`을 교체해야 하는 문제 의미를 설명하라.
- [ ] 70. type 3이 root index 하나만 append해도 그 순간의 배열 전체를 나타내는 이유는?
- [ ] 71. 구간 질의의 완전 포함·왼쪽 겹침·오른쪽 겹침 분기가 query 구간을 중복 없이 분할함을 보이라.
- [ ] 72. 합의 항등원 0이 언제 필요하며 현재 `range_sum_impl`은 비겹침 호출 자체를 어떻게 피하는가?
- [ ] 73. 배열 전체 깊은 복사, mutable segment tree 하나, path-copying tree의 version 격리와 비용을 비교하라.
- [ ] 74. raw/shared pointer node와 vector arena+int index의 할당 수, 제어 블록, cache locality, 재할당 안전성을 비교하라.
- [ ] 75. `n=200,000`일 때 트리 높이와 한 update의 새 node 최대 수를 계산하라.
- [ ] 76. build `O(n)`, assign/query `O(log n)`, copy `O(1)`을 각각 재귀 트리에서 증명하라.
- [ ] 77. update 수 u와 copy 수 c를 써 총 node/roots 공간 `O(n + u log n + c)`를 유도하라.
- [ ] 78. 합 최댓값 `2*10^14`를 계산하고 32-bit signed `int`가 실패하는 이유를 말하라.
- [ ] 79. 최대 reserve 5,200,001 node와 실제 `sizeof(Node)`로 byte/MiB를 계산하고 512MB 제한과 비교하라.
- [ ] 80. arena reference를 재귀 호출 너머 보관하지 않는 규칙이 reserve가 충분해 보여도 필요한 이유는?

## F. 손으로 추적하기

- [ ] 81. `[2,3,1,2,5]`의 초기 root에서 각 leaf와 부모 sum을 구간별로 그려라.
- [ ] 82. 공식 예제의 type 3 뒤 version 1·2 root 값과 공유 node를 표시하라.
- [ ] 83. version 2의 position 2를 5로 바꿀 때 복사되는 구간 경로와 새 sum을 아래에서 위로 적어라.
- [ ] 84. 위 갱신 뒤 version 1 전체 합 13, version 2 전체 합 15가 되는 root DAG를 그려라.
- [ ] 85. `n=1`에서 build·copy·assign이 각각 만드는 node/root 수를 세라.
- [ ] 86. branching CTest에서 version 1, 2, 3의 최종 배열과 합 `19,46,37`을 직접 계산하라.
- [ ] 87. large-sum CTest의 네 출력 `5000000000, 4000000001, 2000000000, 3000000003`을 손으로 검산하라.
- [ ] 88. `[1,4]` tree에서 query `[2,3]`이 방문하는 node와 더하는 완전 포함 조각을 표시하라.

## G. 실행 가능한 실기 검증

- [ ] 89. C++23 Release와 모든 경고 옵션으로 세 실행 파일을 빌드했고 경고가 없다.
- [ ] 90. `daily_main`이 정확히 `2 Ada Linus 1 Linus`를 출력한다.
- [ ] 91. `problem.cpp`를 가리고 다시 작성해 정확히 `2 101 103 1 103`을 출력한다.
- [ ] 92. CTest 6개가 모두 통과하고 출력 helper가 공백·누락 줄 차이도 잡는지 확인했다.
- [ ] 93. filter view를 만든 뒤 첫 `begin` 전에 repository를 변경해 lazy 관찰을 확인하고, 이미 순회한 view는 cache 때문에 재사용하지 않고 변경 뒤 새 view를 만들었다.
- [ ] 94. 반환 DTO를 `string_view`로 바꾼 작은 실패 예제에 sanitizer 또는 명시 수명 추적으로 dangling을 확인했다.
- [ ] 95. 작은 무작위 n/q에서 각 version을 실제 vector로 깊은 복사하는 oracle과 모든 type 2 답을 대조했다.
- [ ] 96. type 1이 version 수를 늘리는 잘못된 구현을 branching 입력이 검출하는지 확인했다.
- [ ] 97. `n=q=200,000` update 집중 stress에서 실행 시간, node 수, capacity, `sizeof(Node)`, 최대 RSS를 기록했다.
- [ ] 98. 알고리즘 대표 문서의 독립 예제를 컴파일·실행했다.
- [ ] 99. `-Scope latest`와 `-Scope all` 표준 라이브러리 감사를 모두 통과했다.
- [ ] 100. README Mermaid, 모든 로컬 Markdown 링크, strict UTF-8, `git diff --check`를 확인했다.

## 최종 자기 설명

자료를 보지 않고 아래 문장을 완성한다.

> `filter_view`는 ______를 소유하지 않고 ______ 시점에 predicate를 실행한다. `transform_view`의 `UserCard` 결과는 ______ 값 범주이고, `std::ranges::to<UserCards>`는 이를 ______ 저장소에 넣은 ______ 값으로 반환한다. `seed`는 이름 있는 ______이고 `std::move(seed)` 결과는 ______이며 실제 버퍼 이전은 ______가 수행한다. 영속 세그먼트 트리는 공개 node를 ______ 않고 type 1마다 ______ 경로만 복사하므로 갱신·질의 시간 ______, 버전 복사 시간 ______, 전체 공간 ______이다.
