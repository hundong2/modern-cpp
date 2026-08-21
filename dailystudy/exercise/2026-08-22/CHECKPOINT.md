# 2026-08-22 이해 검증

자료를 닫고 먼저 답한다. 설명만 외우지 말고 코드 식의 입력·출력·상태·수명을 정확히 말하고, 마지막 실기 검증을 통과해야 오늘 학습을 완료한다.

## 1. 기초 문법과 아키텍처

1. `struct Session`과 `class SessionRegistry`의 기본 접근 권한 차이를 말하고 오늘 역할에 각각 알맞은 이유를 설명한다.
2. `using Table = std::unordered_map<std::string, Session>;`가 새 런타임 객체나 새 타입을 만드는지 답한다.
3. 생성자에는 왜 반환형이 없으며 `explicit SessionRegistry(Table)`가 막는 암시 변환은 무엇인가?
4. `Table sessions` 값 매개변수와 `const Table&` 매개변수의 소유권·복사/이동 가능성 차이를 설명한다.
5. 멤버 초기화 목록 `sessions_{std::move(sessions)}`가 생성자 본문 대입과 다른 점을 말한다.
6. `enum class RenameResult`가 범위 없는 enum과 비교해 막는 두 가지 실수를 말한다.

## 2. 값 범주·이동·수명

1. `initial`, `std::string{"alpha"}`, `std::move(initial)`, `sessions_.extract(old_id)`의 값 범주를 각각 분류한다.
2. 이름 있는 `Table&& parameter`가 식으로 쓰이면 lvalue인 이유를 설명한다.
3. `std::move(node)`가 실제로 node를 비우는 함수가 아닌 이유와 실제 소유권 이전을 수행하는 호출을 말한다.
4. 이동 뒤 `initial`과 `new_id`에서 보장되는 것과 보장되지 않는 것을 구분한다.
5. `const Session*` 반환 포인터가 소유권과 수명을 연장하지 않는다는 뜻을 설명한다.
6. `extract`한 원소가 node에 있는 동안, map의 크기와 원소 소유자는 어떻게 바뀌는가?

## 3. 표준 라이브러리 호출 계약

각 식마다 수신 객체의 정확한 타입·호출 전 상태, 선택 오버로드, 모든 입력의 타입·값 범주·소유권, 반환형·사용 여부, 호출 후 상태, 복잡도·할당·무효화·수명·오류·스레드 보장을 자료 없이 설명한다.

1. `sessions_.contains(new_id)`
2. `Table::node_type node{sessions_.extract(old_id)}`
3. `node.empty()`
4. `node.key() = std::move(new_id)`
5. `++node.mapped().generation`
6. `Table::insert_return_type inserted{sessions_.insert(std::move(node))}`
7. `initial.emplace(std::string{"alpha"}, Session{std::string{"kim"}, 0})`
8. `const auto iterator{sessions_.find(id)}`와 `sessions_.end()`
9. `std::vector<std::vector<long long>>(count, row)`
10. `direct = std::min(direct, cost)`
11. `std::numeric_limits<long long>::max()`
12. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`
13. `std::cin >> from >> to >> cost`
14. `std::cout << printable << separator`

추가 질문:

- `contains` 뒤 다른 스레드가 같은 map을 변경하면 왜 검사-행동 원자성이 생기지 않는가?
- node insert에서 재해시가 발생하면 반복자와 원소 포인터·참조 중 무엇이 무효화되는가?
- `operator[]`로 거리 행렬을 읽을 때 전제조건은 무엇이며 범위를 벗어나면 왜 예외가 아니라 미정의 동작인가?
- 스트림 추출 실패 때 대상 값, 스트림 상태, 뒤 연쇄 추출은 어떻게 되는가?

## 4. 플로이드–워셜 필수 지식

1. `k` 반복이 끝난 직후 거리 행렬의 불변식을 정확히 한 문장으로 쓴다.
2. `D[k][i][j]` 점화식을 쓰고 `k`를 쓰는 경로와 쓰지 않는 경로로 정확성을 증명한다.
3. 왜 `via` 반복이 가장 바깥이어야 하는가?
4. 같은 `from → to` 버스가 여러 개일 때 입력 초기화에 `min`이 필요한 반례를 든다.
5. `INF + 거리`를 계산하지 않는 이유를 정수 오버플로와 잘못된 비교 관점에서 설명한다.
6. 시간 `O(N³)`, 공간 `O(N²)`가 나오는 반복 횟수와 저장 원소 수를 계산한다.
7. 음수 간선은 허용하면서 음수 사이클은 별도 처리가 필요한 이유를 말한다.
8. 다익스트라를 N번 실행하는 방법과 플로이드–워셜을 밀집/희소 그래프 관점에서 비교한다.
9. 실제 경로를 복원하려면 어떤 추가 행렬을 어떤 때 갱신해야 하는가?
10. 도달 가능성만 필요할 때 점화식의 min/plus를 어떤 논리 연산으로 바꿀 수 있는가?

## 5. 초보자 실기 검증

- [ ] `main.cpp`의 출력 `1 1 kim 1`을 실행 전에 예측하고 각 값의 근거를 적었다.
- [ ] 누락 원본, 같은 키, 목적지 충돌, 정상 변경 네 경우를 각각 실행했다.
- [ ] `problem.cpp`를 보지 않고 `extract-key-insert` 흐름을 다시 작성했다.
- [ ] ICPC 풀이에서 `via=1`, `via=2` 뒤 작은 3×3 행렬을 손으로 갱신했다.
- [ ] 병렬 간선과 도달 불가 테스트를 직접 추가했다.
- [ ] 세 실행 파일을 높은 경고 수준으로 빌드하고 직접 실행했다.
- [ ] CTest 전체와 표준 라이브러리 문서 감사를 통과했다.
- [ ] 모든 C++의 표준 호출마다 인자 수와 주석 설명 수, 반환값 사용/무시 여부를 다시 대조했다.

## 완료 기준

호출 이름을 한글로 번역하는 데 그치지 않고 `누가 무엇을 소유하는가`, `어떤 값이 반환되는가`, `호출 뒤 어떤 상태가 바뀌는가`, `무엇이 무효화되는가`, `어떤 복잡도와 실패 가능성이 있는가`를 답한다. 플로이드–워셜은 코드를 보지 않고 점화식·불변식·복잡도·병렬 간선·INF 주의를 설명하고 다시 구현할 수 있어야 한다.
