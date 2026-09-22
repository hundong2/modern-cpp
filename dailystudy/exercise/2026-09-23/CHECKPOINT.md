# 2026-09-23 CHECKPOINT

먼저 코드를 보지 않고 말로 답한다. 막힌 항목만 [`main.cpp`](main.cpp), [`problem.cpp`](problem.cpp), [`icpc_problem.cpp`](icpc_problem.cpp), [`../algorithm/weighted-interval-scheduling.md`](../algorithm/weighted-interval-scheduling.md)을 다시 확인한다. 통과하려면 **타입·값 범주·소유자·수명·반환값·복잡도·오류 방식**을 실제 식과 연결해야 한다.

## 1. 기초 문법과 객체 모델

1. `int over_budget_ms{};`의 기본 타입과 중괄호 초기값을 말하라. 중괄호가 막는 축소 변환 예도 하나 들어라.
2. `std::size_t row_count() const noexcept`의 반환형, 빈 매개변수 목록, 뒤쪽 `const`, `noexcept`를 각각 설명하라.
3. `struct LatencyColumns`와 `class LatencyTable`의 기본 접근 지정자는 각각 무엇인가?
4. 공개 열 묶음에는 `struct`, 길이 불변식을 지키는 소유자에는 `class`를 쓴 이유를 말하라.
5. `public:` 팩터리와 `private:` 생성자/멤버가 임의의 길이 불일치 객체 생성을 어떻게 제한하는가?
6. `explicit LatencyTable(LatencyColumns&& columns)`에서 `explicit`이 제한하는 초기화 형태와 설계 의도를 설명하라.
7. `: service_names_{std::move(columns.service_names)}, ...` 멤버 초기화 목록과 생성자 본문 대입의 차이를 객체 수명 관점에서 말하라.
8. `const LatencyTable& table`에서 `const`, `&`, 이름의 역할을 각각 설명하라. 포인터와 달리 어떤 상태를 표현하지 않는가?
9. `using ServiceNames = std::vector<std::string>`은 새 타입을 만드는가? 두 템플릿 인자를 안쪽부터 지목하라.
10. `if`와 range-`for`가 오늘 코드에서 각각 어떤 비교·분기와 반복을 표현하는지 실제 줄로 설명하라.
11. `[[nodiscard]]` 결과를 버리면 반드시 컴파일 오류인가? 구현이 보통 무엇을 진단하는가?
12. `rows() const &`의 `&`가 참조 반환형이나 매개변수 `&`와 어떻게 다른지 설명하라.
13. `void rows() && = delete;`와 `void rows() const && = delete;`가 런타임 오류가 아니라 컴파일 오류를 만들며 두 선언이 각각 어떤 rvalue를 막는지 말하라.
14. ICPC 코드의 `Project{start, finish, reward}` aggregate 초기화가 어느 멤버에 어떤 순서로 값을 넣는지 말하라.

## 2. lvalue·prvalue·xvalue, 복사·이동·수명

15. 이름 있는 `columns`, `table`, `table_result`와 식 `LatencyColumns{...}`, `Breach{...}`, `std::move(columns)`의 값 범주를 분류하라.
16. 선언 타입이 `T&&`인 이름 있는 변수/매개변수도 이름을 식으로 쓰면 왜 lvalue인가?
17. `std::move(columns)`가 하는 일과 실제 vector 저장소를 옮기는 연산을 구분하라.
18. 이동 뒤 원본이 “유효하지만 값 미지정”이라는 말은 파괴·대입·내용 관찰에 관해 무엇을 뜻하는가?
19. `LatencyTable::create`의 값 매개변수는 호출자의 lvalue/rvalue 인자에서 각각 어떤 복사·이동 가능성을 만드는가?
20. `return RestockReport{...};` 같은 반환형과 동일한 prvalue의 직접 반환과 `return local;`의 선택적 NRVO를 구분하라. 한편 `LatencyTable::create`의 `return LatencyTable{...};`은 왜 `std::optional<LatencyTable>` 변환 생성 단계를 거치는지도 설명하라.
21. `std::views::zip` 결과가 세 vector 원소를 소유하는가, 반복/접근 경계를 소유하는가?
22. `auto&& [service, observed, budget]`의 세 이름이 원소 복사본인지 참조인지 설명하라.
23. zip view를 함수 밖에 보관한 뒤 `LatencyTable`이 파괴되면 무엇이 dangling이 되는가?
24. table이 살아 있어도 기반 vector가 재할당되면 기존 zip iterator/행 참조가 왜 무효가 될 수 있는가?
25. `Breach{service, ...}`가 문자열을 깊게 복사하기 때문에 결과 목록이 table보다 오래 살아도 안전한 이유를 말하라.
26. 두 rvalue `rows()` 삭제가 막는 `make_table().rows()` 및 const xvalue 호출의 수명 버그를 시간 순서로 설명하라.

## 3. 표준 라이브러리 호출 계약

27. `std::views::zip(service_names_, observed_ms_, budget_ms_)`의 수신 객체 유무와 세 인자의 정확한 타입·값 범주·소유권을 말하라.
28. 같은 호출이 선택하는 adaptor overload의 반환 타입 성격과 호출부의 사용 방법을 설명하라.
29. zip 입력 길이가 `3, 3, 2`라면 몇 행을 만들며, 이 동작이 왜 예외나 오류 상태를 만들지 않는가?
30. 세 입력이 모두 sized range일 때 zip `size()`의 의미·복잡도와 원본 상태 변화를 말하라.
31. zip 생성 자체와 전체 순회의 시간 복잡도를 열 수 `K`, 최단 길이 `N`으로 표현하라.
32. zip 생성/순회가 원소 저장소를 동적 할당하는가? 기반 vector의 기존 관찰자를 무효화하는가?
33. range-`for`가 숨겨 호출하는 `begin`, `end`, iterator 역참조·증가·비교의 반환/상태 변화를 설명하라.
34. zip iterator 역참조 결과가 `tuple<T&...>` 성격이라는 사실과 structured binding 수명을 설명하라.
35. 다른 실행 흐름이 순회 중 같은 vector를 구조 변경하면 어떤 데이터 경쟁·무효화·UB 위험이 있는가?
36. `std::optional<LatencyTable>`의 빈 상태와 값 보유 상태가 각각 어떤 객체 수명을 관리하는가?
37. `has_value()`의 수신 상태, 시그니처, 인자 수, 반환형·의미, 상태 변화, 복잡도·예외를 말하라.
38. `value()`의 반환 참조와 빈 상태 전제, `bad_optional_access`, 참조 수명을 설명하라.
39. `std::nullopt`는 함수인가 객체인가? 빈 optional을 만들 때 어떤 소유 자원이 남는가?
40. 세 vector의 initializer-list 생성에서 목록 원소 타입, 복사/이동, 반환값 유무, 복잡도·할당·예외를 설명하라.
41. `vector::size()`가 돌려주는 타입과 값, 상태 변화, 복잡도, 스레드 조건을 설명하라.
42. `vector::reserve(table.row_count())`의 인자, 반환형, 성공 뒤 capacity/size, 재할당 무효화, 예외 보장을 말하라.
43. `vector::push_back(Breach{...})` 또는 `emplace_back`의 선택 overload, rvalue 소유권, 반환형, 재할당·예외를 설명하라.
44. `std::move`의 템플릿 인자 추론, 매개변수·반환형, 상태 변화 여부, 복잡도와 예외를 말하라.
45. `std::ranges::sort(projects, comparator)`의 두 명시 인자와 기본 projection, 반환형, 재배치, 엄격 약순서, 복잡도·예외를 설명하라.
46. `std::ios::sync_with_stdio(false)`가 반환하는 값과 호출부 사용 여부, 호출 뒤 I/O 혼용·스레드 주의를 설명하라.
47. `std::cin.tie(nullptr)`의 인자 허용값, 반환 포인터, 호출 뒤 상태와 pointee 수명을 말하라.
48. `std::cin >> n`의 선택 연산자와 반환 참조, 실패 표현, 대상 변경, 복잡도를 설명하라.
49. `std::cout << answer << '\n'`의 정수/문자 삽입 overload 각각의 피연산자·반환 참조·오류 표현을 말하라.
50. ICPC의 vector count/fill 생성자와 `operator[]`에서 크기·초깃값·유효 첨자·할당/UB 조건을 실제 식으로 설명하라.

## 4. 가중 구간 스케줄링

51. 왜 종료일 순서가 DP 접두사를 닫힌 부분 문제로 만드는가?
52. `best[i]`를 정확한 한 문장으로 정의하고 `best[0]`의 값을 설명하라.
53. 현재 시작일 `start`에 호환되는 이전 프로젝트 종료일의 조건이 `finish < start`인 이유를 닫힌 날짜 구간으로 설명하라.
54. 이분 탐색에서 유지하는 반열린 후보 구간과 `finish >= start`일 때 어느 경계를 움직이는지 말하라.
55. 이분 탐색 반환값이 “호환 가능한 마지막 인덱스”가 아니라 “호환 가능한 접두사 크기”일 때 DP 첨자에 어떤 장점이 있는가?
56. `skip = best[i]`, `take = reward[i] + best[k]`가 가능한 모든 최적해를 두 경우로 완전히 나누는 이유를 증명하라.
57. 귀납 가정 `best[0..i]`가 최적이라는 데서 `best[i+1]` 최적성을 도출하라.
58. 종료일이 같은 프로젝트가 여러 개여도 비교자의 tie-break와 DP가 정답을 보존하는 이유를 말하라.
59. 보상만 큰 프로젝트를 먼저 고르는 탐욕 반례를 세 프로젝트로 만들어라.
60. 전체 `O(n log n)`을 정렬과 `n`번의 이분 탐색으로, `O(n)` 공간을 세 배열로 나눠 증명하라.
61. 최대 답이 `2 * 10^14`까지 갈 수 있는 입력을 설명하고 32비트 `int`가 부족한 이유를 말하라.
62. 이분 탐색 범위에 현재 프로젝트까지 넣으면 어떤 자기 참조 또는 잘못된 호환 판정이 생길 수 있는가?

## 5. 실기 검증

63. `main.cpp`에서 첫 관측값을 예산과 같게 바꾸고 `breaches` 수와 출력 순서를 예측·실행하라.
64. 길이 `3,3,2`인 열과 길이는 같지만 값 `-1`을 가진 열을 각각 만들고 두 팩터리가 빈 optional인지 검증하라.
65. non-const/const rvalue `rows()`를 각각 호출하려는 최소 코드를 별도 파일에 쓰고 두 컴파일 진단을 해석하라.
66. `problem.cpp`의 재고 열 순서를 바꾸되 세 열의 같은 인덱스 관계를 유지하고 결과를 예측하라.
67. CSES 공식 예제에서 종료일 정렬 뒤 각 단계의 `k`, `skip`, `take`, `best`를 표로 써라.
68. `(1,2,5)`, `(2,3,7)`에서 두 프로젝트를 함께 고를 수 없고 답이 `7`인지 실행하라.
69. 프로젝트 20개 이하를 무작위 생성하고 모든 부분집합을 검사하는 oracle과 실행 파일을 500회 이상 대조하라.
70. `n=200,000`의 서로 겹치지 않는 프로젝트를 만들어 답 `200,000 * reward`와 실행 시간·메모리를 확인하라.

## 통과 기준

- 1~62번을 코드 없이 실제 타입과 식으로 설명한다.
- 27~50번은 여섯 요소(수신 상태, 시그니처/overload, 인자, 반환, 사후 상태, 전제·복잡도·할당/무효화/수명/오류/스레드)를 빠뜨리지 않는다.
- 56~57번은 “DP라서 된다”가 아니라 최적해의 두 경우 분할과 귀납 불변식으로 증명한다.
- 63~70번 중 적어도 네 개를 실제로 수행하고, 69번 무작위 oracle 대조는 반드시 통과한다.
