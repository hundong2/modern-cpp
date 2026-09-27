# 2026-09-28 CHECKPOINT — `std::views::slide`와 Sliding Window Cost

코드나 README를 보지 않고 먼저 답한 뒤, 반드시 실제 식과 작은 입력을 손으로 추적한다. “view라서 빠르다”, “multiset은 정렬된다”처럼 이름만 말하지 말고 타입·값 범주·소유권·수명·전제조건·복잡도를 함께 설명해야 통과다.

## 1. 초보자 기초 문법과 객체 설계

- [ ] 1. `int budget_breaches{};`에서 기본 타입과 빈 중괄호 초기화 결과를 말한다.
- [ ] 2. ICPC 코드가 `Entry::value`, 두 합, 비용에 `long long`을 쓰는 수치 근거를 계산한다.
- [ ] 3. `struct LatencyReport`와 `class ServiceLatencyHistory`의 기본 접근 차이는 무엇인가?
- [ ] 4. `public:`과 `private:`가 vector owner와 비소유 view의 수명 불변식을 어떻게 보호하는가?
- [ ] 5. `final`이 오늘 owner class의 수명 정책에 어떤 의미를 더하는가?
- [ ] 6. 생성자에 반환형이 없는 이유와 `explicit`이 막는 의도치 않은 초기화 한 가지를 적는다.
- [ ] 7. `std::vector<int>&& latency_samples`가 바인딩할 수 있는 식과 바인딩할 수 없는 식을 하나씩 든다.
- [ ] 8. `: latency_ms_{std::move(latency_samples)}`가 본문 대입과 다른 시점에 실행되는 이유는?
- [ ] 9. 멤버 초기화 목록 순서와 실제 초기화 순서가 다를 때 무엇이 기준인가?
- [ ] 10. `ServiceLatencyHistory(const ServiceLatencyHistory&) = delete;`가 금지하는 연산을 실제 식으로 보인다.
- [ ] 11. `using Windows = std::ranges::slide_view<BaseView>;`가 새 강한 타입을 만드는지 답한다.
- [ ] 12. `slide_view<BaseView>`와 `ref_view<const std::vector<int>>`의 템플릿 인자가 각각 뜻하는 바를 말한다.
- [ ] 13. `Windows windows() const &`의 반환형, 매개변수 목록, 함수 `const`, 참조 한정자 `&`를 구분한다.
- [ ] 14. `windows() const && = delete`가 막는 최소 댕글링 식을 적는다.
- [ ] 15. `analyze_latency(const ServiceLatencyHistory& history, const int budget_ms)`에서 참조 전달과 값 전달의 차이는?
- [ ] 16. 참조와 포인터의 null 가능성, 재바인딩, 소유권, 접근 문법 차이를 말한다.
- [ ] 17. range-`for`가 개념적으로 사용하는 `begin`, `end`, 비교, 역참조, 증가를 실행 순서대로 적는다.
- [ ] 18. `if (total_ms > budget_ms)`에서 경계값이 포함되는지 말하고 `>=`로 바꿀 때 출력을 예측한다.
- [ ] 19. `struct Entry { long long value{}; int index{}; };`를 aggregate 중괄호 초기화할 때 어느 순서로 대응되는가?
- [ ] 20. 사용자 정의 `operator<`가 strict weak ordering을 만족하려면 같은 `(value,index)`에 어떤 결과를 내야 하는가?

## 2. 값 범주·참조 바인딩·복사/이동·수명

- [ ] 21. 이름 있는 `latency_samples`, `history`, `report`, `window` 식의 값 범주를 말한다.
- [ ] 22. `std::vector<int>{1,2,3}`, `history.windows()`, `analyze_latency(history, 150)`의 값 범주는?
- [ ] 23. `std::move(latency_samples)`의 값 범주와 가리키는 실제 객체를 말한다.
- [ ] 24. `std::move`가 실제 원소 byte를 옮기는지, 실제 이동은 어느 함수가 수행하는지 구분한다.
- [ ] 25. prvalue와 xvalue의 공통점과 차이를 오늘 식 두 개로 설명한다.
- [ ] 26. vector 이동 뒤 원본에 허용되는 연산 두 개와 의존하면 안 되는 상태를 말한다.
- [ ] 27. `std::vector<int>&&` 매개변수 이름 `latency_samples`가 함수 본문에서는 왜 lvalue인가?
- [ ] 28. `const ServiceLatencyHistory&`가 lvalue owner에 바인딩될 때 객체 수명을 연장하는가?
- [ ] 29. `history.windows()` prvalue가 range-`for`의 숨은 `auto&& __range`에 바인딩되면 무엇의 수명만 연장되는가?
- [ ] 30. view 수명이 늘어도 기반 `history` 수명이 늘지 않는 이유를 owner/view 관계로 설명한다.
- [ ] 31. `const Window window`는 view 값인가 원소 복사본인가? 세 int의 소유자는 누구인가?
- [ ] 32. 함수의 이름 있는 `report` 반환에서 NRVO가 가능한 이유와 적용되지 않을 때 경로를 말한다.
- [ ] 33. `const LatencyReport report{analyze_latency(...)}`에서 반환 prvalue와 목적 객체 관계를 설명한다.
- [ ] 34. history 복사·이동 삭제가 이미 발급된 ref-view의 안전성에 어떤 도움을 주는가?
- [ ] 35. view 또는 창을 history 파괴 뒤 역참조하면 동작을 어떻게 분류해야 하는가?

## 3. `slide_view` 폭·수명·무효화

- [ ] 36. `std::views::slide(range, width)`의 기반 범위가 최소한 만족해야 할 range 능력은?
- [ ] 37. `width`에 관한 표준 전제조건을 정확히 말하고 0을 넘길 때의 동작을 분류한다.
- [ ] 38. 길이 `N`과 폭 `W>0`일 때 창 개수를 `N>=W`, `N<W`로 나누어 식으로 쓴다.
- [ ] 39. `[42,55,61,38,77]`에서 폭 3의 세 창과 각 합을 직접 적는다.
- [ ] 40. slide view 구성의 시간·추가 할당과 모든 창 원소를 합산하는 분석의 시간을 구분한다.
- [ ] 41. `ref_view<const vector<int>>`에서 `const`가 막는 것과 막지 못하는 것을 말한다.
- [ ] 42. 기반 vector 재할당·파괴가 기존 view/창/반복자에 미치는 영향을 각각 말한다.
- [ ] 43. 구조 변경 없이 기존 `int` 값을 바꾸는 경우 const view와 동시 접근 계약은 어떻게 다른가?
- [ ] 44. 끝 반복자를 역참조하거나 증가시키는 것이 왜 전제조건 위반인가?
- [ ] 45. 서로 다른 slide view에서 얻은 반복자를 비교해도 되는지 설명한다.
- [ ] 46. `slide_view`가 snapshot, mutex, 원자성 중 무엇도 제공하지 않는다는 말의 실무 결과는?
- [ ] 47. 폭이 큰 모든 창의 합만 필요할 때 slide 이중 순회보다 prefix sum/rolling sum이 나은 이유는?
- [ ] 48. `problem.cpp`의 `[100,108,115,102,130]` 폭 3 창에서 span 세 개와 불안정 판정을 계산한다.

## 4. STL 호출 계약을 실제 식으로 설명하기

아래 각 식마다 반드시 여섯 부분으로 답한다.

1. 수신 객체의 정확한 타입과 호출 전 상태
2. 선택된 시그니처·overload·템플릿 인자
3. **각 매개변수** 식의 타입·값 범주·소유권 의미·허용값
4. 반환형·반환값 의미·실제 사용/폐기 여부
5. 호출 뒤 수신 객체와 각 인자의 상태 변화
6. 전제조건·후조건·복잡도·할당·무효화·수명·오류/예외·스레드 보장

- [ ] 49. `std::vector<int> latency_samples{42, 55, 61, 38, 77}`
- [ ] 50. `std::move(latency_samples)`와 이어지는 `std::vector<int>` 이동 생성자
- [ ] 51. `std::views::slide(latency_ms_, window_width)`
- [ ] 52. 바깥 slide-view range-`for`의 숨은 `begin()`, `end()`, 비교, `operator*`, `operator++`
- [ ] 53. 안쪽 Window range-`for`의 숨은 다섯 반복 연산
- [ ] 54. `std::cout << "breaches=" << report.budget_breaches << ... << '\n'`
- [ ] 55. `std::multiset<Entry>` 두 개의 기본 생성자
- [ ] 56. `lower_.size()`와 `upper_.size()`
- [ ] 57. `lower_.insert(entry)`
- [ ] 58. `auto pivot{lower_.end()}; --pivot; *pivot`
- [ ] 59. `const auto lower_position{lower_.find(entry)}`와 `lower_position != lower_.end()`
- [ ] 60. `lower_.erase(lower_position)`
- [ ] 61. `auto source{upper_.begin()}; const Entry moving{*source}`
- [ ] 62. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`
- [ ] 63. `std::cin >> number_count >> window_size`
- [ ] 64. `std::vector<long long> values(number_count)`
- [ ] 65. `std::cin >> values[index]`
- [ ] 66. `std::cout << ' ' << window.cost()`

## 5. 정확한 Expression 해석

- [ ] 67. `if (lower_.size() == 0U)`에서 `0U`의 타입과 signed/unsigned 비교 경고 회피 이유는?
- [ ] 68. `if (!(*pivot < entry))`를 `(value,index)`의 사전식 순서 문장으로 번역한다.
- [ ] 69. `const auto total_size{lower_.size() + upper_.size()};`의 추론 타입은 무엇인가?
- [ ] 70. `(total_size + 1U) / 2U`가 홀수·짝수에서 각각 `ceil(total/2)`가 되는지 대입해 본다.
- [ ] 71. `while (lower_.size() > desired_lower_size)`가 공개 갱신 한 번 뒤 몇 회 실행될 수 있는가?
- [ ] 72. `const Entry moving{*source}`가 참조 보관이 아니라 값 복사여야 erase 전에 안전한 이유는?
- [ ] 73. `const long long lower_count{static_cast<long long>(lower_.size())};`의 변환이 공식 제약에서 정확한 이유는?
- [ ] 74. `median * lower_count - lower_sum_ + upper_sum_ - median * upper_count`의 각 곱셈 타입을 말한다.
- [ ] 75. `Entry{values[leaving_index], leaving_index}`의 두 `operator[]` 전제조건을 인덱스 식으로 증명한다.
- [ ] 76. `operator<<(char)`가 반환한 `std::ostream&`가 다음 long long 삽입의 수신자가 되는 과정을 설명한다.

## 6. 두 multiset 불변식과 정확성 증명

- [ ] 77. 현재 창과 `lower ∪ upper` 사이의 완전 분할 불변식을 정확히 한 문장으로 쓴다.
- [ ] 78. `lower`와 `upper`의 목표 크기를 홀수 `k`와 짝수 `k`로 나누어 적는다.
- [ ] 79. 순서 불변식 `max(lower) <= min(upper)`가 중앙값 선택에 주는 결론은?
- [ ] 80. 합 불변식이 add/remove/파티션 이동 각각에서 보존되는지 갱신식을 적는다.
- [ ] 81. 왜 키를 값 하나가 아니라 `(value,index)`로 두어야 하는가? 가장 작은 중복 반례를 만든다.
- [ ] 82. `m=max(lower)`가 홀수 창의 유일 중앙값, 짝수 창의 아래쪽 중앙값인 이유는?
- [ ] 83. 절댓값 합이 중앙값에서 최소라는 사실을 목표값을 1 증가시킬 때 기울기 변화로 증명한다.
- [ ] 84. lower에서 `Σ(m-x)`가 `m*|lower|-lower_sum`으로 바뀌는 전개를 보인다.
- [ ] 85. upper에서 `Σ(x-m)`가 `upper_sum-m*|upper|`로 바뀌는 전개를 보인다.
- [ ] 86. 짝수 창에서 두 중앙값 사이 어느 정수도 같은 최소 비용을 내는 이유는?
- [ ] 87. 삽입 위치 선택 뒤 크기 재균형만으로 순서 불변식도 유지되는 이유를 증명한다.
- [ ] 88. 삭제 후 부족한 쪽으로 경계 원소를 옮기면 네 불변식이 모두 복구됨을 증명한다.
- [ ] 89. 공식 첫 창 `[2,4,3]`을 두 set과 합으로 나타내고 비용 2를 식으로 계산한다.
- [ ] 90. `[3,3,3,9]`에서 인덱스를 포함한 Entry 네 개와 비용 6을 계산한다.
- [ ] 91. 초기화, 각 add/remove, cost query의 복잡도를 분리한 뒤 전체 `O(n log k)`를 유도한다.
- [ ] 92. 창 자료구조 `O(k)`와 입력 vector까지 포함한 이 구현 전체 저장 `O(n)`을 구분한다.
- [ ] 93. `7`개 값 `[1,1,1,1,10^9,10^9,10^9]` 비용 `2,999,999,997`을 계산해 int overflow를 증명한다.
- [ ] 94. 두 heap + lazy deletion, 좌표 압축 Fenwick tree와 두 multiset 선택의 장단점을 비교한다.

## 7. 기계 실행 관점

- [ ] 95. slide 순회에서 가능한 원소 load, 합 add, 비교, 조건 분기, 결과 store를 소스 식과 연결한다.
- [ ] 96. multiset add/remove에서 연속 배열과 다른 pointer chasing·노드 할당/해제가 가능한 이유는?
- [ ] 97. 오늘 class에 virtual 함수가 없으므로 어떤 가상 간접 호출이 언어상 필요하지 않은가?
- [ ] 98. 컴파일러가 조건 분기를 조건 이동이나 다른 형태로 바꿀 수 있는 이유는?
- [ ] 99. CPU·ABI·표준 라이브러리·컴파일러·최적화 옵션 없이 특정 명령/할당 횟수를 단정하면 안 되는 이유는?

## 8. 직접 실행하는 초보자 검증

- [ ] A. `main.cpp`의 세 창과 출력 `breaches=3,worst_total_ms=176`을 손으로 먼저 예측한 뒤 실행한다.
- [ ] B. `problem.cpp`의 세 span과 출력 `unstable=2,widest_span_mg=28`을 손으로 먼저 예측한 뒤 실행한다.
- [ ] C. 공식 예제 결과 `2 2 5 7 7 1`을 작은 정렬 창으로 독립 계산해 CTest와 비교한다.
- [ ] D. `k=1`이면 왜 모든 비용이 0인지 설명하고 `9 1 9 5` 테스트를 실행한다.
- [ ] E. `k=n`인 `[5,1,9,2,7]`의 단일 비용 13을 계산한다.
- [ ] F. 중복 입력 `[3,3,3,3,9,3]`, `k=4`에서 답 `0 6 6`과 정확히 삭제되는 index를 추적한다.
- [ ] G. 64비트 경계 입력의 기대값 `2999999997`을 확인하고 모든 중간 산술 타입을 점검한다.
- [ ] H. 원소 2개인 history에서 폭 3 slide가 창 0개를 만드는지 작은 임시 테스트로 확인한다.
- [ ] I. `windows() const && = delete`가 임시 owner 호출을 컴파일 오류로 막는지 별도 최소 코드로 확인한다.
- [ ] J. CMake C++23 빌드, CTest 7건, 전체 표준 라이브러리 문서 감사를 차례로 통과시킨다.

## 통과 기준

- `slide_view`가 소유하지 않는 대상을 정확히 말하고 owner·view·창·반복자 수명 및 재할당 무효화를 구분한다.
- `width > 0`, 창 개수, view 구성 비용과 창 분석 비용을 스스로 유도한다.
- 모든 최초 STL 호출에 여섯 계약 항목을 실제 인자 수만큼 빠짐없이 설명한다.
- 두 multiset의 네 불변식과 중앙값 비용 공식을 사용해 정확성을 증명한다.
- 전체 시간 `O(n log k)`, 창 상태 `O(k)`, 구현 전체 저장 `O(n)`, 64비트 필요성을 수치로 설명한다.
- 세 실행 파일 빌드, exact-output CTest 7건, 전체 문서 감사를 모두 통과한다.
