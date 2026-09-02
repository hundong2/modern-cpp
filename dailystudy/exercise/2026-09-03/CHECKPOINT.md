# 2026-09-03 CHECKPOINT

아래 질문은 “읽어 보았다”가 아니라 오늘 코드를 스스로 설명하고 바꿀 수 있음을 검증한다. 먼저 자료를 닫고 답한 뒤 코드·공용 문서와 대조한다.

## A. 기초 문법과 타입

- [ ] 1. `int revision{};` 기본 멤버 초기화자는 언제 0을 만들며, 생성자가 `revision{received_revision}`을 명시하면 왜 실행되지 않는가? 초기화 목록 없이 쓰인 `int revision;`과도 비교하라.
- [ ] 2. `struct ReportData`와 `class ReportSnapshot`의 기본 접근 지정자는 각각 무엇인가?
- [ ] 3. 생성자 선언 앞에 반환형을 쓰지 않는 이유를 설명할 수 있는가?
- [ ] 4. `explicit ReportData(...)`가 막는 copy-list-initialization과 허용하는 direct-list-initialization을 각각 써 보라.
- [ ] 5. `: title{...}, revision{...}`가 생성자 본문의 대입과 다른 이유는 무엇인가?
- [ ] 6. `const ReportData`에서 `title`을 바꿀 수 없는 경로와 `const std::string*`의 의미를 설명하라.
- [ ] 7. 포인터와 참조가 객체를 소유한다는 뜻이 아닌 이유를 각각 말하라.
- [ ] 8. `using Graph = std::vector<std::vector<int>>;`가 새 클래스 정의가 아닌 이유와 두 템플릿 인자를 풀어 써라.
- [ ] 9. `DfsFrame{start, std::size_t{0}}`에서 aggregate initialization과 각 중괄호 초기화의 역할을 말하라.
- [ ] 10. `if`, `for`, `while`, `continue`, `return`이 오늘 코드의 제어 흐름에서 맡은 역할을 예로 들어라.

## B. 값 범주·복사·이동·수명

- [ ] 11. `std::string{"release-notes"}`의 값 범주와 그 임시 객체의 수명 끝을 설명하라.
- [ ] 12. 값 매개변수 `title`은 string 객체인데 왜 식 `title`의 값 범주는 lvalue인가?
- [ ] 13. `std::move(title)` 자체가 객체를 이동하지 않는다는 말을 실제로 상태를 바꾸는 생성자와 연결해 설명하라.
- [ ] 14. `std::move(title)`의 반환형, 반환 대상, 복잡도, 예외 계약은 무엇인가?
- [ ] 15. 이동 뒤 string이 “반드시 비어 있다”가 아니라 “유효하지만 값이 미지정”인 이유는 무엇인가?
- [ ] 16. `return ReportSnapshot{...};`에서 C++17 이후 어떤 복사·이동이 생략되는가?
- [ ] 17. `title_handle = snapshot.title_handle()` 오른쪽 prvalue가 어떤 shared pointer 이동 대입 overload에 바인딩되는가?
- [ ] 18. aliasing 생성 시에는 strong count가 증가하지만 그 결과의 이동 대입에서는 증가하지 않는 과정을 수치로 추적하라.
- [ ] 19. 안쪽 scope 전·후 `use_count()`가 2와 1인 이유를 소유자 이름으로 나열하라.
- [ ] 20. 마지막 aliasing 핸들이 파괴될 때 `ReportData` 전체가 파괴되는 이유와, wrapper 이동 삭제·복사 허용이 non-null 불변식과 strong count에 미치는 영향을 설명하라.

## C. shared pointer 호출 계약

- [ ] 21. `std::shared_ptr<const std::string>{}`의 매개변수, 생성 후 상태, 반환값 유무, 복잡도, 할당·예외 계약을 말하라.
- [ ] 22. `std::make_shared<const ReportData>(std::move(title), revision)`의 명시 템플릿 인자와 두 함수 인자의 타입·값 범주를 말하라.
- [ ] 23. 위 `make_shared`가 반환하는 정확한 타입, 소유 대상, 실패 시 자원 상태를 설명하라.
- [ ] 24. `std::shared_ptr<const std::string>{owner_, &owner_->title}`의 수신 객체 유무와 두 매개변수를 정확히 설명하라.
- [ ] 25. aliasing 핸들의 stored pointer와 owned/control-block object가 각각 무엇인지 그려 보라.
- [ ] 26. 별칭 생성자가 O(1)·무할당·`noexcept`여도 나중의 `operator*`가 UB가 될 수 있는 잘못된 포인터 예를 들어라.
- [ ] 27. `owner_->revision`에서 `shared_ptr::operator->`의 반환형·전제조건·상태 변화·수명을 말하라.
- [ ] 28. `*title_handle`에서 `shared_ptr::operator*`가 반환한 참조는 언제까지 유효한가?
- [ ] 29. `use_count()` 반환값이 멀티스레드 수명/동기화 결정을 위한 확정 검사로 부적절한 이유는 무엇인가?
- [ ] 30. 서로 다른 shared pointer 객체의 참조 횟수 조작 안전성과 `ReportData` 필드의 동시 변경 안전성을 구분하라.

## D. vector·스트림 실제 식 계약

- [ ] 31. `Graph graph(static_cast<std::size_t>(node_count));`가 선택하는 count 생성자의 인자·결과·복잡도·오류를 말하라.
- [ ] 32. `visited(count, char{0})`와 `component(count, -1)`의 count/value 타입과 초기 원소 상태를 말하라.
- [ ] 33. `graph[index]`의 반환형, 범위 전제조건, 검사 여부, 참조 무효화 조건을 설명하라.
- [ ] 34. `graph[index].push_back(right)`의 수신 객체, 인자 값 범주, 반환형, 재할당 시 무효화를 설명하라.
- [ ] 35. `frames.push_back(DfsFrame{...})`가 위 int lvalue 호출과 선택하는 overload에서 어떻게 다른가?
- [ ] 36. `reserve(node_count)`가 size를 늘리지 않는 이유와 이번 코드에서 재할당을 예방하는 범위를 말하라.
- [ ] 37. `frames.empty()`가 `back()`과 `pop_back()`의 어떤 UB 전제조건을 증명하는가?
- [ ] 38. `DfsFrame& frame{frames.back()};` 참조를 `frames.pop_back()` 뒤 쓰면 왜 잘못이며 push 뒤에는 무엇을 확인해야 하는가?
- [ ] 39. `std::cin >> left_sign >> left_topping`에서 선택되는 char/int overload, 각 반환값의 연쇄 사용, 실패 상태를 말하라.
- [ ] 40. `std::cout << *title_handle << ' '`에서 string 삽입, char 삽입의 매개변수와 반환값, 상태 변화를 각각 말하라.

## E. 2-SAT와 SCC

- [ ] 41. `(a OR b)`가 `(NOT a -> b) AND (NOT b -> a)`와 동치임을 네 진리값으로 증명하라.
- [ ] 42. `positive=2*i`, `negative=2*i+1`일 때 `node ^ 1`이 부정을 두 방향 모두 올바르게 만드는 이유는 무엇인가?
- [ ] 43. `x`와 `NOT x`가 같은 SCC면 true/false 어느 선택도 모순인 이유를 경로로 설명하라.
- [ ] 44. 첫 DFS에서 발견 시점이 아니라 종료 시점에 `finish_order.push_back`해야 하는 이유는 무엇인가?
- [ ] 45. `DfsFrame::next_edge`가 유지하는 불변식과 재귀 함수의 어느 상태를 대신하는지 말하라.
- [ ] 46. 두 번째 DFS가 원래 그래프가 아니라 역그래프를 써야 하는 이유를 SCC 압축 DAG로 설명하라.
- [ ] 47. 오늘 구현에서 SCC 번호가 source에서 sink 방향으로 증가함을 단일 implication `NOT x -> x`로 추적하라.
- [ ] 48. 왜 `component[x] > component[NOT x]`인 리터럴을 참으로 선택하며, 부등호를 반대로 하면 어떤 테스트가 잡는가?
- [ ] 49. 변수 M개, 절 N개의 정점·간선 수와 전체 시간·공간 복잡도를 각각 계산하라.
- [ ] 50. 재귀 DFS와 명시적 vector 스택의 점근 공간은 같아도 온라인 저지 안정성이 다른 이유는 무엇인가?

## F. 직접 실행 검증

- [ ] 51. `daily_main`의 두 출력 줄을 실행 전에 예측하고 실제 결과와 일치시켰는가?
- [ ] 52. `problem.cpp`의 owner 수가 2가 되는 두 핸들을 이름으로 말할 수 있는가?
- [ ] 53. 공식 예제와 `IMPOSSIBLE`, 강제 `+ -`, implication chain CTest를 모두 통과했는가?
- [ ] 54. 변수 12개 이하 무작위 식을 brute force와 비교해 가능 여부와 출력 배정을 검증했는가?
- [ ] 55. 토핑 100,000개의 긴 chain에서 재귀 호출 없이 제한 시간·메모리 안에 끝남을 확인했는가?
- [ ] 56. 표준 라이브러리 전체 감사와 공용 알고리즘 문서의 C++ 예제 빌드를 통과했는가?

## 통과 기준

1. 체크 56개 중 설명 문제를 자료 없이 모두 답한다.
2. 각 표준 호출에서 여섯 항목(수신자 상태, overload/템플릿 인자, 모든 매개변수, 반환과 사용, 호출 후 상태, 복잡도·할당·무효화·수명·오류·스레드)을 빠짐없이 말한다.
3. CTest, 무작위 brute force, 최대 깊이 stress, 공용 문서 예제, 전체 표준 라이브러리 감사를 모두 통과한다.
4. `main.cpp`의 alias 포인터 수명과 `icpc_problem.cpp`의 SCC 배정 부등호를 실제 식으로 증명한다.
