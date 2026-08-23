# 2026-08-24 이해 검증

자료를 닫고 먼저 답한다. 호출 이름을 번역하는 데 그치지 말고 수신 객체·각 입력·반환·호출 후 상태·수명·복잡도·실패 조건을 실제 코드 식과 연결한다.

## 1. 기초 문법과 아키텍처

1. `struct DeploymentRequest`와 `class TaskQueue`의 기본 접근 권한 차이와 오늘 역할에 각각 알맞은 이유를 설명한다.
2. `using Task = std::move_only_function<void()>;`가 새 객체나 상속 타입을 만드는지 답한다.
3. 생성자에는 왜 반환형이 없으며 `explicit TaskQueue(std::size_t)`가 막는 암시 변환은 무엇인가?
4. 멤버 초기화 목록 `total_replicas_{initial_replicas}`는 생성자 본문 대입과 무엇이 다른가?
5. `TaskQueue(const TaskQueue&) = delete`가 호출자에게 드러내는 소유권 계약을 설명한다.
6. `DeploymentLedger::apply(const DeploymentRequest&)`의 const 참조가 보장하는 것과 요청 수명에 보장하지 않는 것을 구분한다.

## 2. 값 범주·이동·수명

1. `request`, `std::move(request)`, `std::make_unique<DeploymentRequest>(...)`, 람다 식의 값 범주를 분류한다.
2. `std::move(request)`와 unique_ptr 이동 생성자 중 실제로 포인터 소유권을 옮기는 것은 무엇인가?
3. 첫 람다 캡처 뒤 `request`의 정확한 표준 보장 상태를 말한다.
4. `owned`와 `ledger` 캡처가 각각 소유하는 것, 빌리는 것, 수명을 연장하는 것을 구분한다.
5. 람다 prvalue→Task→enqueue 값 매개변수→vector 원소의 이동 경로를 그린다.
6. `run_all()` 반환 size_t prvalue가 `executed`를 초기화할 때 복사 생략·이동 가능성을 설명한다.
7. 이름 있는 `Task&&` 매개변수가 함수 본문 식에서는 lvalue라는 규칙을 설명한다.

## 3. 표준 라이브러리 호출 계약

각 식의 수신 객체 정확한 타입·호출 전 상태, 선택된 오버로드·템플릿 인자, 각 위치 인자의 타입·값 범주·복사/이동/참조/소유권, 반환형·사용 여부, 호출 후 상태, 복잡도·할당·무효화·수명·오류·스레드 보장을 자료 없이 설명한다.

1. `std::make_unique<DeploymentRequest>(DeploymentRequest{...})`
2. `TaskQueue::Task{[owned = std::move(request), &ledger] { ... }}`
3. `if (!task)`의 `move_only_function::operator bool()`
4. `tasks_.reserve(expected_count)`
5. `tasks_.push_back(std::move(task))`
6. `task()`
7. `tasks_.clear()`
8. `ledger.apply(*owned)`에서 `unique_ptr::operator*`와 일반 멤버 함수 호출
9. `grid.size()`와 `grid[0].size()`
10. `std::vector<int>(vertex_count, infinity)`
11. `frontier.empty()`와 `frontier.front()`
12. `frontier.pop_front()`
13. `frontier.push_front(next)`와 `frontier.push_back(next)`
14. `row_delta[direction]`의 `array::operator[]`
15. `std::numeric_limits<int>::max()`
16. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`
17. `std::cin >> start_row >> start_column >> destination_row >> destination_column`
18. `std::cout << minimum_energy(...) << '\n'`

추가 검증:

- 빈 `move_only_function`을 호출하면 어떤 전제조건을 어기며 `std::function`의 빈 호출과 무엇이 다른가?
- vector 예약 용량을 넘겨 재할당되면 Task 원소의 소유 호출 대상과 기존 반복자·참조는 어떻게 되는가?
- `deque::front()`가 반환한 참조를 `pop_front()` 뒤 사용하면 왜 잘못인가?
- 표준 컨테이너와 TaskQueue가 같은 객체에 대한 동시 enqueue/execute 안전을 자동 제공하는가?
- 스트림 추출 실패 때 대상 값과 뒤 연쇄 호출, 스트림 상태는 어떻게 되는가?

## 4. 0-1 BFS 대회 필수 지식

1. 일반 BFS가 모든 간선 비용이 같아야 하는 이유와 0/1 가중치에서 그대로 방문 확정하면 틀리는 반례를 든다.
2. 비용 0 간선을 덱 앞, 비용 1 간선을 뒤에 넣어야 하는 이유를 거리 층 불변식으로 설명한다.
3. 완화 조건 `candidate < distance[next]`가 방문 bool을 대신하는 이유를 말한다.
4. 덱 앞 후보의 거리가 최소임을 귀류법 또는 귀납법으로 증명한다.
5. 정점 V, 간선 E에서 시간 `O(V+E)`, 추가 공간 `O(V)`가 나오는 근거를 말한다.
6. Ocean Currents에서 `V=RC`, `E≤8RC`가 되는 이유와 간선을 즉석 생성하는 공간 이점을 설명한다.
7. 일반 다익스트라의 `O((V+E)logV)`와 비교해 0-1 BFS가 로그 인자를 없애는 조건을 말한다.
8. 가중치 2가 하나라도 들어오면 덱의 두 거리 층 불변식이 어떻게 깨지는가?
9. 시작과 목적지가 같은 질문의 답이 0인 이유를 빈 경로 관점에서 설명한다.
10. 방향 문자 `'0'..'7'`을 빼기로 int로 바꾸는 전제조건과 여덟 방향 배열의 대응을 손으로 검증한다.

## 5. 초보자 실기 검증

- [ ] `main.cpp`의 출력 `2 9 worker`를 실행 전에 예측하고 각 값의 근거를 적었다.
- [ ] 빈 Task 등록, 작업 0개 실행, 작업 3개로 예약 용량 초과를 각각 테스트했다.
- [ ] `problem.cpp`를 보지 않고 unique_ptr 이동 캡처 작업 두 개를 다시 작성했다.
- [ ] 공식 예제의 한 질문에서 덱 앞/뒤와 거리 갱신을 최소 다섯 단계 손으로 추적했다.
- [ ] 정방향 0비용과 역방향 1비용인 1×2 테스트를 직접 설명했다.
- [ ] 모든 C++의 표준 호출마다 인자 수와 주석 설명 수, 반환값 사용·무시 여부를 대조했다.
- [ ] 세 실행 파일을 높은 경고 수준으로 빌드하고 두 학습 실행 파일을 직접 실행했다.
- [ ] CTest 전체와 표준 라이브러리 문서 감사를 통과했다.

## 완료 기준

`move_only_function`은 “복사 불가 람다도 된다”에서 멈추지 않고 호출 대상 소유권·빈 상태·이동 뒤 상태·간접 호출·예외·스레드 비보장을 설명한다. 0-1 BFS는 코드를 보지 않고 완화식, 앞/뒤 삽입 규칙, 거리 순서 불변식, 정확성, 복잡도, 적용 불가 조건을 다시 구현할 수 있어야 한다.
