# 2026-08-30 CHECKPOINT

자료를 닫고 실제 코드 식을 가리키며 답한다.

## Modern C++ 문법·아키텍처

1. `struct Job`의 기본 접근과 `class BlockingJobQueue`의 기본 접근이 다른 이유를 설명하라.
2. `explicit BlockingJobQueue(std::size_t capacity)`에서 생성자에 반환형이 없는 이유, 매개변수, 멤버 초기화 목록, 중괄호 초기화 결과를 말하라.
3. `explicit`이 막는 암시 변환과 올바른 `BlockingJobQueue queue{3U}` 직접 초기화를 설명하라.
4. `push(Job job)` 값 매개변수의 소유권을 lvalue 호출과 `Job{...}` prvalue 호출로 나눠 복사·이동·복사 생략과 연결하라.
5. `std::move(job)`의 값 범주는 무엇이며 실제 string 버퍼 이동은 어느 생성자/대입이 수행하는가? 이동 뒤 job의 수명과 허용 연산은 무엇인가?
6. `wait` lambda의 `[this]`가 소유하지 않는 것은 무엇인가? 큐가 대기 스레드보다 먼저 파괴되면 왜 위험한가?
7. `const std::size_t capacity_`와 private deque/mutex가 어떤 불변식을 지키는가?
8. `optional<Job>` 반환이 잠금 해제 뒤에도 Job 수명을 유지하는 이유와 nullopt가 뜻하는 종료 상태를 설명하라.

## 표준 라이브러리 호출 계약

1. `not_full_.wait(lock, [this]{...})`의 수신 객체 정확한 타입·상태, 첫 인자 타입/잠금 소유, 둘째 인자 타입/캡처/반환, 함수 반환형을 설명하라.
2. 위 `wait` 호출 전후 mutex 소유 상태, 원자적 unlock-and-wait가 막는 lost wakeup, 허위 깨움 처리, 예외·수명·공정성 계약을 설명하라.
3. `not_empty_.notify_one()`과 `notify_all()`은 각각 누구를 깨울 수 있고 무엇을 반환하며 queue/closed_를 직접 바꾸는가? 즉시 실행과 순서를 보장하는가?
4. `std::unique_lock<std::mutex> lock{mutex_}`와 `std::scoped_lock lock{mutex_}`의 수신 mutex, 생성자 입력, 소유 상태, 소멸 후 상태, condition_variable 적합성 차이를 말하라.
5. `jobs_.push_back(std::move(job))`의 수신 deque 호출 전 상태, 실제 오버로드, 인자 타입·값 범주·소유권, 반환, 크기/용량, 할당/예외/무효화를 설명하라.
6. `jobs_.front()`와 `jobs_.pop_front()`의 전제조건, 반환형, 상태 변화, 복잡도, 제거 원소와 다른 원소의 참조 수명을 설명하라.
7. `result_.emplace(std::move(value))`와 `result_.value()`의 각 입력·반환, 기존/빈 상태 변화, 예외, 반환 참조 수명을 설명하라.
8. `std::jthread worker{lambda}`의 callable 소유권·참조 캡처 수명·생성 오류와 `worker.join()`의 반환·후조건·동기화·오류를 설명하라.
9. `next.fill(-1)`, `nodes_.emplace_back()`, `pending.push/front/pop` 각각의 수신 객체, 인자, 반환값 사용 여부, 복잡도, 할당·무효화를 설명하라.
10. `std::cin >> pattern`과 `std::cout << (...)`가 반환하는 참조, 바꾸는 객체/상태, 실패 시 동작, 인자 소유 여부를 설명하라.

## Aho–Corasick 증명

1. 패턴마다 text를 독립 검색할 때 최악 복잡도와 Aho–Corasick의 복잡도를 입력 기호 `N`, `P`, `K`로 비교하라.
2. trie 상태가 나타내는 문자열과 `fail[v]`의 정의를 말하라.
3. 실패 링크를 BFS로 계산해야 하는 이유와 `fail[parent]` 전이가 먼저 완성된다는 불변식을 증명하라.
4. 없는 전이를 실패 상태의 전이로 채워도 인식하는 suffix 후보를 놓치지 않는 이유를 설명하라.
5. text 문자 하나마다 정확히 한 전이만 하는데도 모든 suffix 패턴을 찾을 수 있는 이유는 무엇인가?
6. 검색 중 도착 상태만 `matched=true`로 표시한 뒤 BFS 역순으로 `matched[fail[v]] |= matched[v]`하는 정확성을 증명하라.
7. 중복 패턴이 같은 terminal 상태를 공유해도 입력 행마다 올바른 답을 내는 이유를 설명하라.
8. `array<int,26>` 완성 전이의 공간/속도 trade-off를 map 또는 sparse edge 저장과 비교하라.

## 초보자 실기 검증

- `wait(lock,predicate)`를 `if`와 술어 없는 wait로 바꾼 뒤 왜 while 재검사가 필요한지 실행 순서로 증명한다.
- capacity 1, 생산자 2개, 소비자 1개에서 close까지 교착 없이 끝나는 테스트를 추가한다.
- 패턴 suffix 전파를 정방향 BFS로 바꾸고 `he/she` 같은 반례를 찾는다.
- 무작위 소문자 입력에서 각 결과를 `std::string::find`와 최소 1,000회 대조한다.
- CTest, 경고 빌드, 전체 표준 라이브러리 감사가 모두 통과해야 완료로 판정한다.
