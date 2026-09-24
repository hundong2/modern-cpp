# 2026-09-25 CHECKPOINT — `std::generator`와 Meet-in-the-middle

각 질문에 코드나 문서를 보지 않고 답한 뒤 실제 식을 손으로 추적한다. “대충 지연 실행”처럼 이름만 말하지 말고 타입·값 범주·소유권·수명·복잡도를 함께 설명해야 통과다.

## 1. 기초 문법

- [ ] 1. `int latency_ms{};`에서 기본 타입, 중괄호 초기화, 결과 값을 말한다.
- [ ] 2. 부분집합 합과 답에 `int` 대신 `long long`을 쓰는 수치 근거를 각각 계산한다.
- [ ] 3. `struct MetricSample`과 `class MetricBatch`의 기본 접근 차이를 설명한다.
- [ ] 4. `public:`과 `private:`가 오늘 owner 타입의 불변식을 어떻게 보호하는가?
- [ ] 5. 생성자에 반환형이 없는 이유와 `explicit`이 거부하는 초기화 한 가지를 적는다.
- [ ] 6. `: samples_{std::move(samples)}`가 생성자 본문 대입과 다른 시점을 설명한다.
- [ ] 7. 멤버 초기화 실제 순서가 목록 순서가 아니라 선언 순서인 이유를 말한다.
- [ ] 8. `using SlowSampleStream = ...`이 새 강한 타입을 만드는지 답한다.
- [ ] 9. `std::generator<const MetricSample&>`의 템플릿 인자가 뜻하는 산출 타입을 말한다.
- [ ] 10. `select_slow_samples(MetricBatch batch, const int threshold_ms)`의 반환형과 두 매개변수를 구분한다.
- [ ] 11. `samples() const &`의 함수 `const`와 참조 한정자 `&`를 각각 설명한다.
- [ ] 12. `samples() const && = delete`가 막는 수명 버그를 최소 식으로 보인다.
- [ ] 13. 포인터와 참조의 null 가능성·재바인딩·소유권 차이를 말한다.
- [ ] 14. `if (sample.latency_ms >= threshold_ms)`가 경계값 100을 포함하는지 답한다.
- [ ] 15. range-`for`가 개념적으로 호출하는 다섯 반복 연산을 적는다.

## 2. 값 범주·이동·복사 생략·수명

- [ ] 16. 이름 있는 `samples`, `batch`, `stream`의 값 범주를 말한다.
- [ ] 17. `MetricSample{std::string{"gateway"},135}`와 coroutine 반환 결과의 값 범주를 말한다.
- [ ] 18. `std::move(samples)`가 실제 byte 이동을 수행하는지, 누가 실제 이동을 수행하는지 구분한다.
- [ ] 19. xvalue와 prvalue의 공통점과 차이를 오늘 식으로 설명한다.
- [ ] 20. vector 이동 뒤 원본에 허용되는 두 연산과 의존하면 안 되는 상태를 말한다.
- [ ] 21. coroutine 호출 때 값 매개변수 batch가 어디에 저장되고 언제 파괴되는가?
- [ ] 22. initial suspend 때문에 함수 호출 직후 어떤 본문이 아직 실행되지 않았는가?
- [ ] 23. `SlowSampleStream stream{select_slow_samples(...)}`에서 보장된 복사 생략과 이동 가능성을 설명한다.
- [ ] 24. 이름 있는 `sums` 반환에서 NRVO가 허용되는 이유와 실패 시 vector 이동 경로를 말한다.
- [ ] 25. `co_yield sample`이 복사하는가, 어떤 객체를 빌리는가?
- [ ] 26. generator iterator 증가 전/후와 generator 파괴 뒤 sample 참조 수명을 구분한다.
- [ ] 27. `copied_services.push_back(sample.service)` 결과가 frame보다 오래 살아도 되는 이유는?
- [ ] 28. string 대신 `string_view`를 저장했다면 어떤 owner가 먼저 죽을 때 댕글링하는가?
- [ ] 29. generator가 move-only인 것이 frame 소유권 중복을 어떻게 막는가?
- [ ] 30. 같은 generator에 `begin()`을 두 번 호출하는 동작을 정확히 분류한다.

## 3. STL 호출 계약을 식으로 설명하기

다음 식마다 (1) 수신 객체 타입/상태, (2) 선택 오버로드/템플릿 인자, (3) 각 인자의 타입·값 범주·소유권·허용값, (4) 반환형·의미·사용 여부, (5) 호출 뒤 상태, (6) 전제조건·복잡도·할당·무효화·수명·오류·스레드 보장을 말한다.

- [ ] 31. `std::move(samples)`
- [ ] 32. `samples.reserve(4)`
- [ ] 33. `samples.push_back(MetricSample{std::string{"gateway"},135})`
- [ ] 34. `select_slow_samples(std::move(batch), 100)`
- [ ] 35. generator range-`for`의 숨은 `stream.begin()`과 `stream.end()`
- [ ] 36. generator iterator의 `operator*`, `operator++`, sentinel `operator==`
- [ ] 37. `co_yield sample`이 선택하는 `promise_type::yield_value`
- [ ] 38. `copied_services.push_back(sample.service)`
- [ ] 39. `copied_services.size()`
- [ ] 40. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`
- [ ] 41. `std::cin >> number_count >> target_sum`
- [ ] 42. `std::vector<long long> values(static_cast<std::size_t>(number_count))`
- [ ] 43. `values[position]`
- [ ] 44. `sums.push_back(sums[index] + value)`
- [ ] 45. `std::ranges::sort(right_sums)`
- [ ] 46. `std::ranges::equal_range(right_sums, needed)`
- [ ] 47. `matches.size()`의 반환 타입과 long long 변환 안전 근거
- [ ] 48. `std::cout << answer << '\n'`

## 4. 실제 Expression 해석

- [ ] 49. `std::size_t{1} << width`에서 왼쪽 피연산자를 먼저 size_t로 만드는 이유를 말한다.
- [ ] 50. `last - first`의 전제조건과 `first>last`일 때 생기는 unsigned 문제를 설명한다.
- [ ] 51. `const std::size_t previous_size{sums.size()};`가 매 단계의 0/1 선택을 어떻게 고정하는가?
- [ ] 52. `sums[index] + value`에서 두 피연산자의 값 범주와 결과 값 범주를 말한다.
- [ ] 53. `target_sum - left_sum`이 음수가 될 수 있어도 long long 검색 키가 안전한 이유는?
- [ ] 54. `answer += static_cast<long long>(matches.size())`의 변환 상한을 수치로 증명한다.
- [ ] 55. `std::ranges::equal_range` 반환 subrange가 원소를 소유하는지 iterator만 소유하는지 답한다.
- [ ] 56. right_sums를 재할당하면 기존 matches에 무슨 일이 생기는가?
- [ ] 57. 정렬 전 equal_range 호출은 어떤 partition 전제조건을 깨는가?
- [ ] 58. `const long long left_sum : left_sums`가 참조가 아니라 값 복사여도 적절한 이유는?
- [ ] 59. `std::cout` 삽입 연쇄에서 각 호출의 반환 참조가 어떻게 다음 호출의 수신자가 되는가?

## 5. MITM 알고리즘 증명

- [ ] 60. 초기 목록 `[0]`이 빈 절반에서도 필요한 이유를 말한다.
- [ ] 61. k개 처리 후 부분합 열거 불변식을 정확히 한 문장으로 쓴다.
- [ ] 62. 다음 값 v를 고른 집합과 고르지 않은 집합이 서로소이며 완전한 이유를 증명한다.
- [ ] 63. 같은 합 값이 목록에 여러 번 남아야 하는 이유를 위치 선택 관점에서 설명한다.
- [ ] 64. 전체 부분집합과 `(왼쪽 부분집합, 오른쪽 부분집합)` 쌍의 전단사(bijection)를 보인다.
- [ ] 65. 왼쪽 합 a에 대해 오른쪽 조건이 `target-a` 하나로 충분한 이유는?
- [ ] 66. equal_range 크기를 모든 왼쪽 원소에 더하면 multiplicity 곱이 계산되는 과정을 보인다.
- [ ] 67. 열거, 정렬, 질의의 시간을 L/R 기호로 각각 구한 뒤 균등 분할 식으로 바꾼다.
- [ ] 68. 두 목록의 최대 원소 수와 long long raw 저장소 대략 크기를 계산한다.
- [ ] 69. 네 개의 1, 목표 2에서 답 6을 실제 왼쪽/오른쪽 합 목록으로 계산한다.
- [ ] 70. 40개의 1, 목표 20이 32비트 답을 깨는 이유를 조합식으로 말한다.
- [ ] 71. `sort` 뒤 `unique`를 넣으면 틀리는 가장 작은 반례를 만든다.
- [ ] 72. lower_bound 존재 여부만 더하는 구현이 틀리는 반례를 만든다.
- [ ] 73. 양쪽 정렬+투 포인터 빈도 묶기 변형의 불변식을 적는다.
- [ ] 74. target 범위가 작을 때 0/1 knapsack DP가 더 나을 수 있는 기준을 비교한다.

## 6. 기계 실행 관점

- [ ] 75. generator 생성·최초 begin·iterator 증가·co_yield마다 가능한 load/store/간접 분기를 구분한다.
- [ ] 76. 오늘 class에 virtual 함수가 없으므로 어떤 가상 간접 호출이 필수적이지 않은가?
- [ ] 77. 문자열 깊은 복사가 allocation과 문자 store를 포함할 수 있는 조건을 말한다.
- [ ] 78. 이진 탐색 한 번이 중앙 원소 load·비교·조건 분기를 몇 단계 반복하는가?
- [ ] 79. 템플릿 인라인·frame allocation elision을 특정 어셈블리로 단정하면 안 되는 이유를 열거한다.

## 7. 직접 실행하는 초보자 검증

- [ ] A. `gateway`를 100으로 바꾸기 전 출력 변화부터 적고 CTest를 실행한다.
- [ ] B. `copied_services` 대신 `const MetricSample*`를 저장한 뒤 stream 파괴 후 왜 읽으면 안 되는지 수명 그림을 그린다. 실제 UB 실행은 하지 않는다.
- [ ] C. `previous_size`를 제거한 잘못된 열거를 `[1]`에서 손으로 세 단계만 추적한다.
- [ ] D. 공식 예제의 왼쪽/오른쪽 합 목록을 정렬 전후로 출력하는 임시 진단을 넣어 예측과 비교한다.
- [ ] E. `[1,1,1,1]`, 목표 2와 단일 원소 일치/불일치 테스트를 직접 추가한다.
- [ ] F. 40개의 1, 목표 20 결과가 `137846528820`인지 확인한다.
- [ ] G. strict warning 빌드, CTest, 표준 라이브러리 전체 감사를 모두 통과시킨다.

## 통과 기준

- `std::generator`가 frame을 소유한다는 말과 산출 참조가 가리키는 실제 객체의 수명을 구분한다.
- 모든 최초 STL 호출에서 6개 계약 항목을 인자 수만큼 빠짐없이 설명한다.
- 절반 합 열거 불변식과 전체 부분집합 전단사를 사용해 정확성을 증명한다.
- 시간 `O(n*2^(n/2))`, 공간 `O(2^(n/2))`, 64비트 경계를 수치로 설명한다.
- 세 실행 파일 빌드, CTest, 무작위 oracle 비교, 전체 문서 감사가 모두 통과한다.
