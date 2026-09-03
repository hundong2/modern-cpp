# 2026-09-04 CHECKPOINT

아래 질문은 “읽었다”가 아니라 오늘 코드를 스스로 설명하고 고칠 수 있음을 검증한다. 먼저 자료를 닫고 답한 뒤 코드와 공용 문서로 대조한다.

## A. 기초 문법과 타입

- [ ] 1. `long long total{};`과 `std::size_t item_count{};`가 만드는 초기값과 두 타입을 선택한 이유를 말하라.
- [ ] 2. 중괄호 초기화가 narrowing을 거부하는 예를 하나 만들고 괄호 초기화와 비교하라.
- [ ] 3. `struct BatchReport`와 `class PendingBatch`의 기본 접근 지정자는 각각 무엇인가?
- [ ] 4. 생성자 앞에 반환형이 없는 이유와 `explicit`이 막는 초기화 문법을 실제 식으로 쓰라.
- [ ] 5. `: name{...}, total{...}`가 본문의 대입과 다른 객체 수명 단계를 설명하라.
- [ ] 6. `const std::vector<int>& values`의 `const`, `&`, 비소유 의미와 호출 동안 필요한 수명 조건을 각각 말하라.
- [ ] 7. pointer/reference가 객체를 소유하거나 수명을 자동 연장하지 않는다는 반례를 하나 들어라.
- [ ] 8. `template <class T>`에서 T의 역할과 `OneShotChannel<StatusMessage>`가 만드는 구체 타입을 풀어 쓰라.
- [ ] 9. `using Graph = std::vector<std::vector<IncidentEdge>>;`가 새 타입 정의가 아닌 이유와 두 vector의 원소 타입을 말하라.
- [ ] 10. `if`, `for`, `while`, `continue`, `return`이 오늘 코드에서 만드는 제어 흐름을 각각 한 줄씩 찾으라.

## B. 값 범주·복사·이동·수명

- [ ] 11. 이름 있는 `scores`, `producer`, 값 매개변수 `name`의 식 값 범주는 무엇인가?
- [ ] 12. `std::string{"orders-42"}`와 `BatchReport{...}`의 값 범주와 임시 객체 수명을 설명하라.
- [ ] 13. `std::move(scores)` 자체가 이동이 아니라는 말을 실제 상태를 바꾸는 vector 생성자와 연결하라.
- [ ] 14. `std::move(producer)`의 반환 타입, 반환 대상, 복잡도, 예외 계약을 말하라.
- [ ] 15. 이동 뒤 string/vector가 “반드시 비었다”가 아니라 “유효하지만 값 미지정”인 이유는 무엇인가?
- [ ] 16. lambda init-capture의 왼쪽 `producer`와 오른쪽 `producer`가 각각 어느 객체인지 구분하라.
- [ ] 17. 참조 캡처 `[&values]`가 현재 생성자 구조에서 위험한 수명선을 그려라.
- [ ] 18. `channel.publish(source)`에서 StatusMessage와 내부 string의 복사가 언제 발생하며 source가 유지되는 이유를 말하라.
- [ ] 19. `BatchReport report{pending.take()}`의 shared-state 이동과 C++17 prvalue 직접 구성을 구분하라.
- [ ] 20. 멤버 선언 역순 파괴가 `worker_`와 `result_`에 왜 중요한지 설명하라.

## C. promise/future/jthread 호출 계약

- [ ] 21. `std::promise<BatchReport> producer{}`의 인자, 생성 후 상태, 소유권, 할당·오류 가능성을 말하라.
- [ ] 22. `producer.get_future()` 수신자의 호출 전 상태, 반환 정확 타입, 반환값 사용, 두 번째 호출 오류를 설명하라.
- [ ] 23. `result_ = producer.get_future()`가 선택하는 future 이동 대입과 양쪽 handle의 호출 후 상태를 추적하라.
- [ ] 24. `producer.set_value(BatchReport{...})`의 수신자와 유일 인자의 타입·값 범주·소유권을 말하라.
- [ ] 25. 위 `set_value`의 반환형, ready 상태 변화, 중복 호출 오류, 값 이동 예외 가능성을 설명하라.
- [ ] 26. `future::valid()`의 bool이 ready가 아니라 shared-state 연관 여부인 이유는 무엇인가?
- [ ] 27. `future::get()`의 대기, 반환값, 성공 뒤 invalid를 설명하고, 두 번째 호출/no-state가 왜 `future_error` 보장이 아니라 전제조건 위반인 표준상 UB인지 말하라.
- [ ] 28. `set_value` 이전 생산자 쓰기와 성공한 `get` 뒤 소비자 읽기 사이의 동기화 관계를 설명하라.
- [ ] 29. promise가 값을 게시하지 않은 채 마지막 생산자 끝점을 잃으면 future가 무엇을 관찰하는가?
- [ ] 30. `std::jthread{lambda}`가 callable을 어떻게 소유하고 어떤 실패가 가능하며 소멸자는 무엇을 하는가?
- [ ] 31. `worker_ = std::jthread{...}`에서 수신자가 이미 joinable일 때와 오늘의 non-joinable일 때를 비교하라.
- [ ] 32. promise/future가 결과 내부의 임의 공유 데이터까지 자동으로 thread-safe하게 만들지 않는 이유를 말하라.

## D. 알고리즘·vector·스트림 실제 식 계약

- [ ] 33. `std::accumulate(values.begin(), values.end(), 0LL)`의 세 인자 타입·값 범주와 반환형을 말하라.
- [ ] 34. 초기값을 `0`으로 바꾸면 누적 계산 타입이 왜 달라질 수 있으며 long long 결과에 어떤 위험이 있는가?
- [ ] 35. `begin()`/`end()` 반환 반복자의 수명과 어떤 구조 변경이 이를 무효화하는지 설명하라.
- [ ] 36. `Graph adjacency(n+1)`의 count 생성자가 만드는 원소, 복잡도, 할당 오류를 말하라.
- [ ] 37. `used(m, char{0})`의 fill 생성자 두 인자와 성공 뒤 원소 상태를 말하라.
- [ ] 38. `std::vector<int> vertex_stack;` 기본 생성 뒤 size/capacity/원소 수명에 대해 보장되는 것과 구현 세부를 구분하라.
- [ ] 39. `adjacency[index]`의 반환형, 범위 전제조건, 검사 여부, 범위 위반 결과를 설명하라.
- [ ] 40. `adjacency[from].push_back(IncidentEdge{to,id})`의 두 수신 단계, rvalue overload, 반환형, 재할당 무효화를 설명하라.
- [ ] 41. `reserve(M+1)`이 size를 늘리지 않는 이유와 성공·실패 시 관찰자 계약을 말하라.
- [ ] 42. `empty()`가 `back()`과 `pop_back()`의 어떤 UB 전제조건을 증명하는가?
- [ ] 43. `const int city{vertex_stack.back()};`로 참조 대신 값을 복사한 이유를 다음 pop과 연결하라.
- [ ] 44. `pop_back()`의 반환형, 제거 원소 수명, size/capacity, 무효화 범위를 설명하라.
- [ ] 45. `std::cin >> crossing_count >> street_count`의 각 int& 인자, 반환 istream&, 실패 상태를 말하라.
- [ ] 46. `std::cout << report.name << ' '`에서 string 삽입과 char 삽입의 overload·피연산자·반환 사용을 구분하라.

## E. 오일러 회로와 Hierholzer

- [ ] 47. 무방향 그래프가 시작점 1의 오일러 회로를 가지는 차수·연결 조건을 정확히 말하라.
- [ ] 48. 닫힌 보행에서 각 정점 차수가 짝수여야 하는 이유를 들어오는 간선과 나가는 간선의 짝으로 증명하라.
- [ ] 49. 정점 방문 배열이 아닌 `edge_id`별 `used`가 필요한 삼각형 또는 평행 간선 반례를 그려라.
- [ ] 50. 무방향 간선의 두 인접 항목이 같은 id를 공유해야 하는 이유를 설명하라.
- [ ] 51. `next_incident[v]`보다 앞선 항목에 대해 항상 참인 불변식은 무엇인가?
- [ ] 52. 미사용 간선이 있을 때 stack에 push하고 없을 때만 route에 넣는 이유를 설명하라.
- [ ] 53. pop 순서가 왜 정방향 답이 아니라 역순이며 작은 회로 splice와 어떻게 같은가?
- [ ] 54. 모든 차수가 짝수지만 정점 1과 분리된 삼각형이 있을 때 어느 검사가 실패하는가?
- [ ] 55. `reversed_route.size()==M+1`이 모든 서로 다른 간선을 사용했음을 증명하라.
- [ ] 56. 각 인접 항목을 한 번만 검사한다는 사실로 시간 `O(N+M)`을 도출하라.
- [ ] 57. 재귀 대신 명시적 vector stack을 쓰는 이유를 `M=200,000` 최악 깊이와 연결하라.
- [ ] 58. 시작·끝이 다른 무방향 오일러 경로와 방향 오일러 회로에서는 존재 조건이 어떻게 바뀌는가?

## F. 직접 실행 검증

- [ ] 59. `daily_main` 출력 `orders-42 20 3 1 0`의 다섯 필드를 실행 전에 설명했는가?
- [ ] 60. `daily_problem`에서 원본과 수신 문자열이 모두 같은 이유와 future 상태 `1 -> 0`을 설명했는가?
- [ ] 61. 공식 예제, 삼각형, 홀수 차수, 분리된 두 삼각형, 두 회로 splice CTest를 모두 통과했는가?
- [ ] 62. 작은 무작위 그래프에서 존재 조건과 프로그램 판정을 대조하고 출력 간선 multiset을 검증했는가?
- [ ] 63. 최대 `N=100,000`, `M=200,000` 입력에서 정점 토큰 `M+1`개와 시작/끝 1을 확인했는가?
- [ ] 64. 공용 알고리즘 예제, Mermaid, 로컬 링크, UTF-8, `-Scope latest/all` 감사를 모두 통과했는가?

## 통과 기준

1. 체크 64개 중 설명 문제를 자료 없이 모두 답한다.
2. 각 표준 호출에서 여섯 항목(수신자 상태, overload/템플릿 인자, 모든 매개변수, 반환과 사용, 호출 후 상태, 복잡도·할당·무효화·수명·오류·스레드)을 빠짐없이 말한다.
3. CTest, 무작위 독립 검증, 최대 크기 stress, 공용 문서 예제, 전체 표준 라이브러리 감사를 모두 통과한다.
4. `main.cpp`의 one-shot shared-state 수명과 `icpc_problem.cpp`의 pop 역순·답 길이 불변식을 실제 식으로 증명한다.
