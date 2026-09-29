# 2026-09-30 CHECKPOINT — `chunk_by`와 Salary Queries

먼저 코드와 README를 가리고 답한다. 그다음 실제 식을 가리키며 타입·값 범주·소유권·수명·전제조건·후조건·복잡도까지 말한다. “view라서 빠르다”, “Fenwick이라 로그 시간이다”처럼 이름만 답하면 통과가 아니다.

## 1. 초보자 기초 문법과 객체 설계

- [ ] 1. `int salary{};`, `std::size_t employee_count{};`, `long long salary_sum{};`의 초기값과 각 타입을 선택한 이유를 말한다.
- [ ] 2. `enum class Department`가 일반 `enum`이나 정수 상수보다 강하게 막는 암시 변환은 무엇인가?
- [ ] 3. `Department::engineering`에서 `::`가 나타내는 범위는 무엇인가?
- [ ] 4. `struct Employee`와 `class PayrollSnapshot`의 기본 접근 수준 차이는 무엇인가?
- [ ] 5. `public:` API와 `private:` vector owner가 비소유 view의 수명 불변식을 어떻게 보호하는가?
- [ ] 6. `department_name`의 반환형과 매개변수 타입을 구분하고 `[[nodiscard]]`, `constexpr`, `noexcept`의 뜻을 각각 말한다.
- [ ] 7. `switch (department)`가 정상 열거값 세 개를 어떻게 조건 분기하는가? 마지막 `return "";`는 왜 필요한가?
- [ ] 8. 생성자에는 왜 반환형이 없으며 `explicit PayrollSnapshot(Records records)`가 막는 암시 변환 식은 무엇인가?
- [ ] 9. `: records_{std::move(records)}`는 생성자 본문의 대입과 언제, 어떻게 다른가?
- [ ] 10. 멤버 초기화 목록에 적힌 순서와 실제 멤버 초기화 순서 중 무엇이 우선하는가?
- [ ] 11. `using Records = std::vector<Employee>;`가 새 강한 타입인지 별칭인지 답한다.
- [ ] 12. `std::vector<Employee>`와 `std::vector<DepartmentSummary>`에서 template 인자 `T`는 각각 무엇인가?
- [ ] 13. `auto department_groups() const &`의 반환형 추론, 빈 매개변수 목록, 함수 `const`, 참조 한정자 `&`를 구분한다.
- [ ] 14. `void department_groups() const && = delete;`가 막는 위험한 최소 식을 작성한다.
- [ ] 15. `const Employee&`와 `const Employee*`의 null 가능성, 재바인딩, 접근 문법, 소유권을 비교한다.
- [ ] 16. range-`for`가 개념적으로 사용하는 `begin`, `end`, 비교, 역참조, 증가 연산을 순서대로 적는다.
- [ ] 17. `if (is_first)`가 첫 원소에서만 부서를 기록하게 하는 상태 변화를 설명한다.
- [ ] 18. `static_cast<long long>(employee.salary)`가 어떤 변환을 명시하며 왜 합계 전에 수행하는가?
- [ ] 19. `DepartmentSummary{.department=..., .employee_count=..., .salary_sum=...}`의 지정 초기화가 지켜야 할 멤버 순서를 설명한다.
- [ ] 20. ICPC 연산을 표현하는 `struct`의 문자 종류, 두 정수, 1-based 직원 번호가 각각 어떤 의미인지 실제 선언과 연결한다.

## 2. 값 범주·참조 바인딩·복사/이동·수명

- [ ] 21. 이름 있는 `seed`, `snapshot`, `groups`, `summaries`, 생성자 매개변수 `records` 식의 값 범주는 무엇인가?
- [ ] 22. `PayrollSnapshot::Records{...}`, `snapshot.summarize()`, `snapshot.department_groups()` 결과 식의 값 범주는 무엇인가?
- [ ] 23. `std::move(seed)`의 값 범주와 그 식이 여전히 가리키는 객체를 말한다.
- [ ] 24. lvalue, prvalue, xvalue를 오늘 코드의 식 하나씩으로 구분한다.
- [ ] 25. `std::move`가 실제 원소를 옮기는지, 실제 소유권 이전은 어느 선택된 연산이 수행하는지 설명한다.
- [ ] 26. `PayrollSnapshot snapshot{std::move(seed)}`에서 값 매개변수와 `records_`까지 소유권이 이동하는 경계를 추적한다.
- [ ] 27. 이동 뒤 `seed`와 생성자 매개변수 `records`에 허용되는 연산 두 개와 의존하면 안 되는 상태를 말한다.
- [ ] 28. rvalue reference로 전달되었더라도 이름 있는 `records` 식이 본문에서 lvalue인 이유는?
- [ ] 29. 술어의 `const Employee& left/right`가 어느 lvalue에 바인딩되고 복사가 생기는지 답한다.
- [ ] 30. `auto&& group`이 무엇에 바인딩되는지, 그 subrange가 Employee를 소유하는지 답한다.
- [ ] 31. `const Employee& employee`의 수명과 유효성이 기반 `records_` 및 vector 구조 변경에 어떻게 묶이는가?
- [ ] 32. range-`for`가 view prvalue의 수명을 늘릴 수 있어도 기반 owner 수명은 왜 늘리지 못하는가?
- [ ] 33. `return summaries;`에서 NRVO가 가능한 이유와 적용되지 않을 때 허용되는 이동 경로를 말한다.
- [ ] 34. `const Summaries summaries = snapshot.summarize();`의 보장된 복사 생략과 `copied_summaries{summaries}`의 독립 vector 복사 생성을 대조한다.
- [ ] 35. 이름 있는 지역 반환이면 항상 복사/이동 0회라고 단정할 수 없는 이유는?
- [ ] 36. `department_name`이 반환한 `string_view`가 안전한 기간과 `department_groups()`의 view가 안전한 기간이 다른 이유는?
- [ ] 37. `summarize()` 결과가 snapshot 파괴 뒤에도 유효하지만 `group`은 그렇지 않은 이유를 소유권으로 설명한다.

## 3. `chunk_by` 인접 의미와 owner/view 계약

- [ ] 38. `std::views::chunk_by(range, pred)`가 술어를 적용하는 두 인자의 위치 관계를 정확히 말한다.
- [ ] 39. `engineering, sales, engineering`을 같은 부서 술어로 묶으면 몇 chunk가 되는가? 이유는?
- [ ] 40. `engineering, engineering, sales, support, support`의 모든 경계에서 술어 결과를 `true/false`로 적는다.
- [ ] 41. 같은 키 전체를 하나로 모으려면 `chunk_by` 전에 어떤 전제가 필요하거나 어떤 다른 접근이 필요한가?
- [ ] 42. 각 chunk가 “최대 인접 구간”이라는 말을 술어 결과로 정의한다.
- [ ] 43. 빈 기반 범위와 원소 하나인 기반 범위는 각각 몇 chunk를 만드는가?
- [ ] 44. view 구성 시 원소 순회·복사·할당 여부와 첫 경계 탐색 시점을 구분한다.
- [ ] 45. 길이 `n`, 결과 chunk 수 `g`일 때 전체 요약의 시간과 결과 저장 공간을 계산한다.
- [ ] 46. `ref_view<const Records>`가 소유하는 것과 소유하지 않는 것을 말한다.
- [ ] 47. snapshot 파괴, vector 재할당, 현재 원소 값만 읽기, 동시 쓰기가 view/iterator/reference에 주는 영향을 각각 말한다.
- [ ] 48. 끝 iterator 역참조와 서로 다른 범위 iterator 비교가 왜 전제조건 위반인지 설명한다.
- [ ] 49. view가 snapshot, 잠금, 원자성, owner 수명 연장 중 어느 것도 제공하지 않는다는 말의 실무 결과는?
- [ ] 50. `department_groups() const && = delete`와 `compress() const && = delete`가 컴파일 단계에서 제거하는 댕글링 경로를 그린다.
- [ ] 51. `main.cpp`의 8개 직원을 손으로 chunk로 나누고 각 `employee_count`, `salary_sum`을 계산한다.
- [ ] 52. `problem.cpp`의 7개 표본을 상태 chunk로 나누고 첫/마지막 분, 개수를 계산한다.

## 4. STL 호출 계약을 여섯 항목으로 설명하기

아래 모든 식에 대해 반드시 다음 여섯 부분을 빠짐없이 답한다.

1. 수신 객체의 **정확한 타입**과 호출 직전 상태
2. 선택된 시그니처·overload·template 인자
3. 각 매개변수 식의 타입·값 범주·소유권 의미·허용값
4. 반환형·반환값 의미·실제 사용 또는 폐기 여부
5. 호출 뒤 수신 객체와 각 인자의 상태 변화
6. 전제조건·후조건·시간 복잡도·할당·iterator/reference 무효화·수명·오류/예외·스레드 보장

- [ ] 53. 문자열 리터럴에서 `std::string_view`를 만드는 `return "engineering"`
- [ ] 54. `PayrollSnapshot::Records seed{...}`의 `std::vector<Employee>` initializer-list 생성자
- [ ] 55. `std::move(seed)`와 이어지는 값 매개변수 vector 이동 생성
- [ ] 56. `std::move(records)`와 `records_`의 vector 이동 생성
- [ ] 57. `std::views::chunk_by(records_, same_department)`
- [ ] 58. 바깥 chunk-by-view range-`for`의 숨은 `begin()`, `end()`, 비교, 역참조, 증가
- [ ] 59. 안쪽 subrange range-`for`의 숨은 다섯 반복 연산
- [ ] 60. `Summaries summaries{}`의 vector 기본 생성자
- [ ] 61. `summaries.push_back(DepartmentSummary{...})`
- [ ] 62. `std::cout << department_name(...) << ':' << ... << '\n'`의 각 삽입과 연쇄 반환
- [ ] 63. ICPC 코드의 `std::ios_base::sync_with_stdio(false)` 또는 실제 별칭을 통한 호출
- [ ] 64. `std::cin.tie(nullptr)`에서 수신 stream, 포인터 인자, 이전 tied stream 반환값을 설명한다.
- [ ] 65. `std::cin >> employee_count >> query_count`처럼 이어지는 정수 추출
- [ ] 66. 압축 후보 `std::vector`의 `push_back` 또는 `reserve`가 실제 코드에 있다면 각각의 정확한 계약
- [ ] 67. `std::sort(coordinates.begin(), coordinates.end())`
- [ ] 68. `std::unique(coordinates.begin(), coordinates.end())`와 이어지는 `vector::erase`
- [ ] 69. `std::lower_bound(coordinates.begin(), coordinates.end(), value)`
- [ ] 70. `std::upper_bound(coordinates.begin(), coordinates.end(), upper)`
- [ ] 71. Fenwick 저장소 `std::vector<int>`의 count 생성자와 `operator[]`
- [ ] 72. ICPC 출력의 `std::cout << answer << '\n'`

## 5. 표준 라이브러리 계약의 정확한 세부 질문

- [ ] 73. vector initializer-list 생성자는 목록 원소를 복사하는가, 목록 backing array를 빌리는가? 성공 뒤 수명 관계는?
- [ ] 74. 기본 allocator를 쓰는 `vector(vector&&)`의 대표 시간·할당·예외 계약과 이동 전 원소 참조의 대상을 말한다.
- [ ] 75. `vector::push_back(T&&)`가 재할당할 때와 하지 않을 때 기존 원소 참조 및 과거 `end()`가 어떻게 되는가?
- [ ] 76. `push_back`이 `bad_alloc` 또는 원소 이동 예외를 낼 수 있는 조건과 일반적인 강한 예외 보장의 예외를 말한다.
- [ ] 77. `vector::operator[](index)`의 반환형, 복잡도, 범위 검사 여부와 `index>=size()`일 때 동작을 말한다.
- [ ] 78. `vector::begin/end`의 반환값과 복잡도, `end()` 역참조 가능 여부, 구조 변경 후 유효성을 말한다.
- [ ] 79. `std::sort(first,last)`가 요구하는 반열린 유효 범위, iterator 범주, 평균/최악 비교 복잡도와 예외 전파를 말한다.
- [ ] 80. `std::unique(first,last)`가 실제 vector 크기를 줄이는지, 반환 iterator 뒤 값의 상태와 기존 iterator 무효화를 말한다.
- [ ] 81. `vector::erase(new_end,end)`가 파괴하는 원소, 반환 iterator, 복잡도와 무효화 범위를 말한다.
- [ ] 82. `lower_bound`가 요구하는 partition/정렬 전제, 반환 iterator 의미, 비교 복잡도와 찾지 못한 경우를 말한다.
- [ ] 83. `upper_bound`와 `lower_bound`가 동등 값의 어느 쪽 경계를 반환하는지 비교한다.
- [ ] 84. stream 추출 실패 때 대상 값과 `failbit/eofbit`가 어떻게 될 수 있으며 반환된 `istream&`는 어떻게 쓰이는가?
- [ ] 85. stream 삽입 실패 때 상태 비트와 `exceptions()` mask에 따른 예외를 설명한다.
- [ ] 86. `sync_with_stdio(false)`의 반환형과 호출 시점 제약, C stdio와 섞을 때의 의미를 말한다.
- [ ] 87. `cin.tie(nullptr)`가 반환하는 이전 포인터의 사용 여부와 자동 flush 관계를 말한다.
- [ ] 88. 같은 vector를 한 스레드가 변경하는 동안 다른 스레드가 읽을 때 왜 표준 컨테이너가 자동 동기화하지 않는가?

## 6. 실제 Expression 해석

- [ ] 89. `salary_sum += static_cast<long long>(employee.salary)`에서 읽기, 변환, 덧셈, 저장 순서를 언어 의미 수준에서 설명한다.
- [ ] 90. `++employee_count`의 피연산자 타입, lvalue 수정, 전위 증가 반환 의미를 말한다.
- [ ] 91. `left.department == right.department`에서 두 load와 비교 결과 타입을 말한다.
- [ ] 92. `auto groups = department_groups();`에서 `auto`가 참조를 추론하는지 view 값을 추론하는지 답한다.
- [ ] 93. `for (auto&& group : groups)`의 `auto&&`가 forwarding reference 형태로 역참조 결과에 어떻게 바인딩되는가?
- [ ] 94. `for (const Employee& employee : group)`에서 역참조 결과가 왜 복사되지 않는가?
- [ ] 95. `std::move(seed)`가 const를 붙이거나 제거하는지, 원래 타입과 결과 참조 타입을 적는다.
- [ ] 96. ICPC 코드의 `index += index & -index`를 `index=6`의 이진수와 다음 인덱스로 계산한다.
- [ ] 97. `index -= index & -index`를 `index=12`에서 0까지 손으로 추적하고 각 책임 구간을 적는다.
- [ ] 98. `upper_bound(...) - coordinates.begin()`의 결과 타입과 이를 Fenwick prefix 길이로 바꿀 때 필요한 변환을 설명한다.
- [ ] 99. 갱신 `! 3 6`에서 입력의 직원 번호 3을 vector 인덱스와 Fenwick 인덱스로 각각 어떻게 바꾸는가?
- [ ] 100. `prefix(right_count) - prefix(left_count)`가 `[a,b]` 양끝을 포함하는지 `a=b=2`로 검증한다.

## 7. 좌표 압축과 Fenwick 불변식·정확성

- [ ] 101. 최초 급여와 모든 갱신 목표 급여를 미리 수집해야 하는 이유는? 질의 경계는 왜 필수가 아닌가?
- [ ] 102. `[7,2,2,5]`를 정렬·중복 제거한 좌표와 각 값의 0-based/1-based rank를 적는다.
- [ ] 103. 좌표 압축이 보존해야 하는 순서와 동등성 성질을 수식으로 쓴다.
- [ ] 104. Fenwick의 `tree[i]`가 담당하는 구간을 `lowbit(i)`로 정의한다.
- [ ] 105. `i=4,6,8`의 `lowbit`와 각 책임 구간을 계산한다.
- [ ] 106. `add(position,delta)`가 `i += lowbit(i)`로 방문하는 모든 노드가 position을 포함함을 설명한다.
- [ ] 107. `prefix(r)`가 `i -= lowbit(i)`로 고르는 구간들이 겹치지 않고 `[1,r]`을 덮음을 설명한다.
- [ ] 108. 모든 직원 초기 삽입 뒤 “현재 급여 빈도” 불변식을 정확히 한 문장으로 쓴다.
- [ ] 109. 갱신에서 `-1`, `+1`, 현재 급여 대입의 순서를 적고 빈도 불변식 보존을 증명한다.
- [ ] 110. `[a,b]` 답이 `count(<=b)-count(<a)`인 이유를 집합 차이로 증명한다.
- [ ] 111. `lower_bound(a)`와 `upper_bound(b)`가 위 두 prefix 길이를 정확히 만드는 이유를 말한다.
- [ ] 112. 공식 예제의 초기 압축 좌표와 Fenwick 빈도를 만들고 첫 `? 2 3` 답 3을 계산한다.
- [ ] 113. `! 3 6` 뒤 이전/새 rank에서 바뀌는 값과 두 번째 답 2를 계산한다.
- [ ] 114. 같은 급여로 갱신할 때 `-1`과 `+1`이 상쇄되어도 불변식이 유지되는 이유는?
- [ ] 115. 질의 경계가 좌표 사이에 있을 때, 예를 들어 급여 `{10,30}`에서 `[11,29]`가 0이 되는 과정을 보인다.
- [ ] 116. 모든 급여보다 작은/큰 질의 경계에서 `begin/end` 위치와 prefix 인자를 적는다.
- [ ] 117. `M`개 좌표에서 add/prefix 반복 횟수가 `O(log M)`인 이유를 lowbit 이동으로 설명한다.
- [ ] 118. 전처리, 초기화, 각 갱신, 각 질의의 비용을 합쳐 `O((n+q) log(n+q))`를 유도한다.
- [ ] 119. 입력 급여, 저장 연산, 압축 좌표, Fenwick 배열을 모두 포함한 공간 `O(n+q)`를 유도한다.
- [ ] 120. Fenwick tree와 segment tree, 정렬 multiset을 이 문제의 갱신·범위 개수 질의 기준으로 비교한다.

## 8. 경계 사례와 대회 실수 찾기

- [ ] 121. `n=1`, 급여 5에서 `? 5 5`, `? 1 4`, `! 1 1`, `? 1 1`의 답을 추적한다.
- [ ] 122. 모든 급여가 같은 경우 압축 좌표 수, Fenwick 크기, `[x,x]` 답은 무엇인가?
- [ ] 123. 갱신 목표 급여를 압축 후보에서 빠뜨리면 어느 호출의 전제조건 또는 인덱스 계산이 깨지는가?
- [ ] 124. `lower_bound(b)`를 오른쪽 경계에 쓰면 급여가 정확히 `b`인 직원을 왜 잃는가?
- [ ] 125. `upper_bound(a)`를 왼쪽 경계에 쓰면 급여가 정확히 `a`인 직원을 어떻게 잘못 제외하는가?
- [ ] 126. 0-based 압축 위치를 Fenwick add에 그대로 넘겼을 때 위치 0에서 루프가 멈추지 않을 수 있는 이유는?
- [ ] 127. 직원 번호를 0-based로 바꾸지 않으면 마지막 직원 접근에서 어떤 UB가 가능한가?
- [ ] 128. 현재 급여 배열을 먼저 덮어쓴 뒤 이전 빈도를 빼면 어떤 잘못된 상태가 생기는가?
- [ ] 129. q가 200000일 때 매 질의마다 모든 직원을 세는 `O(nq)`가 왜 시간 제한에 부적합한가?
- [ ] 130. 급여 값 `10^9` 때문에 크기 `10^9+1` 직접 빈도 배열을 쓰는 선택의 메모리 문제를 계산한다.

## 9. 기계 실행 관점

- [ ] 131. chunk 순회에서 가능한 department load, compare, 조건 분기, salary load/add, summary store를 소스 식과 연결한다.
- [ ] 132. Fenwick `add`에서 배열 load/store와 lowbit 계산, 루프 분기가 반복되는 과정을 한 갱신으로 설명한다.
- [ ] 133. 이진 탐색에서 비교 결과에 따른 좌우 범위 축소가 조건 분기나 조건 이동으로 구현될 수 있음을 설명한다.
- [ ] 134. vector의 연속 배치가 Fenwick과 좌표 탐색의 cache locality에 줄 수 있는 이점을 말한다.
- [ ] 135. 오늘 도메인 class에 virtual 함수가 없다는 사실과 표준 stream 내부에서 간접 호출이 가능하다는 사실을 구분한다.
- [ ] 136. CPU·ABI·표준 라이브러리·컴파일러·최적화 옵션 없이 특정 명령, 분기, 할당 횟수를 단정할 수 없는 이유는?
- [ ] 137. signed integer overflow와 unsigned wraparound의 언어 규칙 차이를 말하고 lowbit 구현 타입의 안전 조건을 확인한다.

## 10. 직접 실행하는 검증

- [ ] A. `main.cpp`의 세 줄을 손으로 예측하고 실제 stdout 전체와 비교한다.
- [ ] B. `problem.cpp`의 세 상태 구간을 손으로 예측하고 실제 stdout 전체와 비교한다.
- [ ] C. 떨어진 같은 부서가 별도 chunk가 되는 수정 입력을 만들어 adjacency 의미를 확인한다.
- [ ] D. 임시 `PayrollSnapshot{...}.department_groups()`가 삭제된 overload 때문에 컴파일되지 않는 최소 검사를 작성한다.
- [ ] E. CSES 공식 예제의 두 출력 `3`, `2`를 CTest로 확인한다.
- [ ] F. 같은 급여 반복, 같은 값 갱신, `n=1`, 경계가 좌표에 없는 질의를 각각 실행한다.
- [ ] G. 고정 seed 작은 무작위 연산을 단순 `O(n)` 카운트 구현과 대조한다.
- [ ] H. 최대 제약 성능을 Release 빌드에서 측정하고 장비·컴파일러·옵션과 함께 기록한다.
- [ ] I. CMake C++23 빌드와 모든 CTest가 통과했는지 실제 종료 코드와 테스트 개수로 확인한다.
- [ ] J. 전체 표준 라이브러리 문서 감사를 실행해 미문서화 심볼과 헤더가 0개인지 확인한다.

## 통과 기준

- `chunk_by`가 같은 키 전체가 아니라 인접 쌍 술어로 최대 구간을 만든다고 설명한다.
- owner, view, subrange, iterator, reference, 소유 결과의 수명과 vector 무효화 규칙을 구분한다.
- lvalue/prvalue/xvalue, 참조 바인딩, 복사·이동, 이동 후 상태, NRVO와 보장된 복사 생략을 실제 식으로 설명한다.
- 모든 최초 표준 라이브러리 호출을 여섯 계약 항목과 실제 인자 수에 맞춰 설명한다.
- 좌표 압축의 순서 보존, Fenwick 책임 구간·빈도 불변식, 닫힌 구간 질의의 정확성을 증명한다.
- 전체 시간 `O((n+q) log(n+q))`, 공간 `O(n+q)`와 경계 변환을 스스로 유도한다.
- 세 실행 파일 빌드, CTest, 무작위 브루트포스 대조, 전체 문서 감사를 실제로 통과한 뒤에만 검증 완료로 표시한다.
