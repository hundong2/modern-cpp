# 2026-09-07 CHECKPOINT

아래 문제는 “읽었다”가 아니라 오늘 코드를 스스로 설명하고 고칠 수 있음을 검증한다. 먼저 자료를 닫고 답한 뒤 코드·공용 문서와 대조한다.

## A. 기초 문법과 타입

- [ ] 1. `int replicas_{}`와 `bool canary_{}`가 만드는 초기값을 각각 말하라.
- [ ] 2. 중괄호 초기화가 narrowing을 거부하는 짧은 식을 만들고 괄호 초기화와 비교하라.
- [ ] 3. `struct HealthSnapshot`과 `class DeploymentPlan`의 기본 접근 지정자는 각각 무엇인가?
- [ ] 4. `public:`/`private:`가 이름 접근과 객체 메모리 배치에 미치는 영향을 구분하라.
- [ ] 5. 생성자에 반환형이 없는 이유와 `explicit`이 거부하는 copy-list-initialization 식을 쓰라.
- [ ] 6. `: service_{...}, replicas_{...}`가 본문 대입과 다른 객체 수명 단계를 설명하라.
- [ ] 7. `using RawDeploymentRow = std::tuple<std::string,int,bool>`이 새 타입 정의가 아닌 이유를 말하라.
- [ ] 8. 위 tuple의 템플릿 인자와 각 위치의 업무 의미를 순서대로 말하라.
- [ ] 9. `map(const HealthRow& raw) const`에서 앞 `const`, `&`, 뒤 `const`의 뜻을 각각 설명하라.
- [ ] 10. reference와 pointer가 소유권이나 수명을 자동으로 늘리지 않는 반례를 하나 들어라.
- [ ] 11. `const char* rollout_lane`가 댕글링되지 않는 이유를 문자열 리터럴의 저장 기간과 연결하라.
- [ ] 12. 오늘 코드의 `if`, `for`, `while`, `return`이 만드는 제어 흐름을 각각 한 줄씩 찾으라.

## B. 값 범주·복사·이동·수명

- [ ] 13. 이름 있는 `raw`, 값 매개변수 `row`, lambda 매개변수 `service`의 식 값 범주는 무엇인가?
- [ ] 14. `std::string{"billing"}`, lambda 식, `DeploymentPlan{...}`의 값 범주를 말하라.
- [ ] 15. `std::move(raw)` 자체가 이동하지 않는다는 말을 실제 tuple 이동 생성과 연결하라.
- [ ] 16. `std::move(row)`의 정확한 반환형, 반환 대상, 복잡도와 예외 계약을 말하라.
- [ ] 17. `map(std::move(raw))` 호출 뒤 raw의 string이 반드시 비었다고 단정할 수 없는 이유는 무엇인가?
- [ ] 18. 이동 뒤에도 raw 객체의 수명과 타입 불변식은 언제까지 유지되는가?
- [ ] 19. `problem.cpp`의 const lvalue tuple 적용에서 원본 string과 결과 string의 소유권을 추적하라.
- [ ] 20. lambda의 `const std::string& service`가 원본 수명을 늘리지 않아도 안전한 범위를 설명하라.
- [ ] 21. `DeploymentPlan plan{mapper.map(...)}`에서 same-type prvalue 직접 구성과 선택적 NRVO를 구분하라.
- [ ] 22. 복사 생략이 tuple/string의 모든 소유권 이동까지 없앤다는 주장이 왜 틀렸는가?
- [ ] 23. 지역 `raw`, `mapper`, `plan`, `rollout_lane`의 파괴 순서를 역순 규칙으로 말하라.
- [ ] 24. `service()`가 돌려준 `const std::string&`를 plan 파괴 뒤 보관하면 왜 댕글링인가?

## C. `std::tuple`과 `std::apply` 호출 계약

- [ ] 25. `RawDeploymentRow raw{std::string{"billing"}, 4, true}`가 선택하는 string/tuple 생성자와 세 인자 값 범주를 말하라.
- [ ] 26. 위 string 생성자의 null 포인터 전제, 시간 복잡도, 할당·예외 가능성을 설명하라.
- [ ] 27. tuple 구성 성공 뒤 각 원소의 소유권·수명을 말하고, 특정 인덱스 파괴 순서는 왜 가정하면 안 되는가?
- [ ] 28. `std::apply`의 대표 함수 템플릿 서명에서 `F`, `Tuple`, `decltype(auto)`의 역할을 말하라.
- [ ] 29. `std::apply(lambda, std::move(row))`의 두 인자 타입·값 범주·소유권 의미를 정확히 말하라.
- [ ] 30. 위 호출이 lambda의 `std::string`, `int`, `bool` 값 매개변수를 어떻게 구성하는지 추적하라.
- [ ] 31. apply 반환형·반환값 사용처와 호출 뒤 row의 각 원소 상태를 말하라.
- [ ] 32. apply 자체와 callable 실행 비용·할당·예외를 구분하라.
- [ ] 33. tuple-like 원소 수와 lambda 인자 수가 다르면 런타임 오류가 아니라 컴파일 오류인 이유를 말하라.
- [ ] 34. apply가 컨테이너 반복자·참조를 무효화하지 않지만 전달된 callable은 그럴 수 있다는 예를 들어라.
- [ ] 35. `std::apply(lambda, raw)`에서 `Tuple=const HealthRow&`일 때 세 원소 식의 cv/ref 범주를 말하라.
- [ ] 36. lambda가 string을 `const&`로 받고 결과 생성자가 값으로 받을 때 정확히 어디서 복사되는가?
- [ ] 37. 같은 tuple 객체를 다른 스레드가 수정하면서 apply하면 왜 apply 자체가 데이터 경쟁을 해결하지 못하는가?
- [ ] 38. 위치 기반 tuple을 mapper 밖까지 노출할 때 열 순서 변경이 만드는 유지보수 위험을 설명하라.

## D. vector·정렬·스트림 실제 식 계약

- [ ] 39. `std::vector<Point> points(count)`의 count 인자 타입·허용 범위와 성공 뒤 size를 말하라.
- [ ] 40. vector count 생성의 시간·공간 복잡도, `length_error`/`bad_alloc`, 새 객체의 관찰자 상태를 설명하라.
- [ ] 41. `points.begin()`/`points.end()`의 수신자, 반환형, 반환값 사용, 무효화 조건을 말하라.
- [ ] 42. `std::sort(points.begin(), points.end(), point_less)`의 반복자·비교자 타입, 세 인자, strict weak ordering과 반환형을 설명하라.
- [ ] 43. sort 뒤 반복자 주소가 유효해도 그 위치의 Point 값이 바뀔 수 있는 이유는 무엇인가?
- [ ] 44. `hull.reserve(point_count * std::size_t{2})`의 인자·반환·size/capacity 상태 변화와 실패 보장을 말하라.
- [ ] 45. `hull.size()`와 `sorted_points.size()`의 반환형, 복잡도, 예외·할당·상태 변화 계약을 말하라.
- [ ] 46. `hull[hull.size()-2]`와 `sorted_points[index]`의 반환형·const 차이와 범위를 증명하는 while/for 전제를 쓰라.
- [ ] 47. vector `operator[]`가 범위를 검사하지 않을 때 위 전제가 깨지면 어떤 결과인가?
- [ ] 48. `hull.back()`의 비어 있지 않음 전제, 반환 참조 수명과 다음 `pop_back()`의 영향을 설명하라.
- [ ] 49. `hull.pop_back()`의 반환형, 제거 원소 수명, size/capacity, 무효화 범위를 말하라.
- [ ] 50. `hull.push_back(sorted_points[index])`의 const lvalue overload, 반환형, 분할 상환 복잡도와 재할당 무효화를 설명하라.
- [ ] 51. `std::ios::sync_with_stdio(false)`의 bool 인자·bool 반환값과 첫 I/O 전 호출 조건을 말하라.
- [ ] 52. `std::cin.tie(nullptr)`의 수신자, 포인터 인자, 반환 포인터와 소유권 변화를 말하라.
- [ ] 53. `std::cin >> points[index].x >> points[index].y`의 두 `operator[]` 호출, 두 수정 인자, 각 반환 istream& 사용과 실패 상태를 말하라.
- [ ] 54. 실제 네 문장 `std::cout << point.x;`, `std::cout << ' ';`, `std::cout << point.y;`, `std::cout << '\n';`에서 정수/char overload와 매번 버리는 반환 ostream&를 구분하라.
- [ ] 55. 스트림 삽입의 비용을 무조건 O(1)이라고 할 수 없는 이유와 오류·스레드 계약을 말하라.

## E. 볼록 껍질과 Andrew monotone chain

- [ ] 56. 볼록 집합과 주어진 점 집합의 convex hull을 자신의 말로 정의하라.
- [ ] 57. `cross(a,b,c)`의 부호가 반시계/공선/시계 방향을 어떻게 구분하는가?
- [ ] 58. `(x,y)` 사전순 정렬 뒤 lower chain을 왼쪽에서 오른쪽으로 만들 수 있는 이유를 설명하라.
- [ ] 59. lower/upper chain에 새 점을 넣기 직전 유지해야 하는 불변식을 쓰라.
- [ ] 60. 시계 방향 회전에서 가운데 점을 제거해도 hull 경계를 잃지 않는 이유를 증명하라.
- [ ] 61. CSES 2195에서 pop 조건이 `cross < 0`이어야 하고 `<= 0`이면 안 되는 이유를 공선 경계 예로 설명하라.
- [ ] 62. `cross == 0`인 세 점을 유지해도 각 점이 stack에서 push/pop되는 총횟수가 O(N)인 이유는 무엇인가?
- [ ] 63. lower와 upper가 공유하는 양 끝점을 제거하지 않으면 출력에 어떤 중복이 생기는가?
- [ ] 64. 껍질 넓이가 양수라는 공식 조건이 모든 점 공선 특수 처리를 어떻게 없애는가?
- [ ] 65. 좌표 차의 최대 절댓값 `2*10^9`에서 외적 최대 절댓값 `8*10^18`을 유도하라.
- [ ] 66. 32-bit `int` 곱셈 overflow가 단순 wrap 보장이 아니라 signed overflow UB인 이유를 말하라.
- [ ] 67. 정렬과 두 번의 선형 scan으로 전체 시간 `O(N log N)`, 추가 공간 `O(N)`을 도출하라.
- [ ] 68. 내부점, 꼭짓점, 변 위 공선점을 각각 알고리즘이 어떻게 처리하는지 손으로 추적하라.
- [ ] 69. 경계 꼭짓점만 요구하는 문제로 바뀌면 pop 조건과 전처리를 어떻게 바꿀 것인가?
- [ ] 70. 회전하는 캘리퍼스나 점-다각형 판정 전에 hull의 순서·공선 정책을 명확히 해야 하는 이유는 무엇인가?

## F. 직접 실행 검증

- [ ] 71. `daily_main` 출력 `billing 4 1 canary`의 네 값과 이동 경계를 실행 전에 예측했는가?
- [ ] 72. `daily_problem` 출력 `search 2 3 0`과 원본 row 보존 이유를 설명했는가?
- [ ] 73. 공식 예제, 공선 경계 직사각형, 삼각형, 음수 좌표 CTest를 모두 통과했는가?
- [ ] 74. 작은 무작위 점 집합에서 독립 지지선 판정과 프로그램의 경계점 집합을 비교했는가?
- [ ] 75. 최대 `N=200,000` 입력에서 실행 시간, 출력 중복, 64-bit 외적 범위를 확인했는가?
- [ ] 76. 공용 알고리즘 예제, Mermaid, 로컬 링크, UTF-8, `-Scope latest/all` 감사를 모두 통과했는가?

## 통과 기준

1. 체크 76개 중 설명 문제를 자료 없이 모두 답한다.
2. 각 표준 호출에서 여섯 항목(수신자 상태, overload/템플릿 인자, 모든 매개변수, 반환과 사용, 호출 뒤 상태, 복잡도·할당·무효화·수명·오류·스레드)을 빠짐없이 말한다.
3. CTest, 무작위 독립 검증, 최대 크기 stress, 공용 문서 예제와 전체 표준 라이브러리 감사를 모두 통과한다.
4. `main.cpp`의 tuple 소유권 이동과 `icpc_problem.cpp`의 공선점 보존·chain 불변식을 실제 식으로 증명한다.
