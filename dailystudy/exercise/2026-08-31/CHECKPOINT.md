# 2026-08-31 CHECKPOINT

자료를 닫고 답한다. “호출한다”에서 멈추지 말고 수신 객체, 각 인자, 반환값, 호출 뒤 상태, 복잡도, 수명·무효화·오류를 실제 식에 맞춰 설명한다.

## Modern C++ 문법과 아키텍처

1. `struct DeploymentEvent`와 `class DeploymentFormatter`에서 접근 지정자를 생략했을 때 기본 접근이 각각 무엇인가?
2. 생성자에 반환형이 없는 이유와 `explicit DeploymentFormatter(std::string)`이 막는 암시 변환 예를 적는다.
3. `DeploymentFormatter{std::string{"prod"}}`에서 안쪽 식과 바깥쪽 식의 값 범주, 각 객체가 소유하는 메모리를 설명한다.
4. 생성자 본문 대입 대신 `: environment_{std::move(environment)}` 멤버 초기화 목록을 쓰는 이유는 무엇인가?
5. 이름 있는 `environment`의 선언 타입이 값 타입인데 식으로는 lvalue인 이유, `std::move(environment)`가 xvalue인 이유를 말한다.
6. `render()`의 `const DeploymentEvent&`가 복사·소유권·수명에 주는 의미는 무엇인가? 함수가 참조를 저장했다면 어떤 추가 계약이 필요한가?
7. `return std::format(...)`에서 복사 생략/이동이 가능한 지점과 반환 `string`의 수명을 설명한다.
8. DTO와 formatter/service를 분리한 구조가 실무에서 테스트와 출력 정책 교체에 주는 이점을 두 가지 적는다.
9. `string_view`가 문자열 리터럴을 볼 때는 안전하지만 지역 `string`을 볼 때 dangling이 될 수 있는 예를 만든다.
10. load·store·비교·조건 분기·함수 호출 관점의 설명이 특정 어셈블리 명령을 보장하지 않는 이유를 CPU·ABI·최적화와 연결한다.

## 표준 라이브러리 호출 계약

### `std::format("[{}] service={} id={} latency={}ms", environment_, event.service_name, event.service_id, event.latency_ms)`

1. 첫 인자를 포함해 총 다섯 인자의 타입, 값 범주, 읽기/복사/이동/소유권 의미를 순서대로 말한다.
2. 선택되는 대표 함수 템플릿 형태, 반환형, 반환 문자열의 소유자, 반환값이 저장되는 위치를 말한다.
3. 호출 뒤 `environment_`와 `event`가 왜 유지되는가? 결과 길이에 따른 시간·공간·할당과 가능한 오류를 말한다.
4. 런타임 형식 문자열을 다뤄야 할 때 컴파일 시간 `format_string` 계약과 무엇이 달라지는지 조사한다.

### `lines.reserve(events.size())`

5. 수신 객체의 정확한 타입과 호출 전 size, 안쪽 `size()`의 반환형·상태 변화, 바깥 `reserve` 인자를 말한다.
6. `reserve` 뒤 size와 capacity는 각각 어떻게 되는가? 재할당 시 어떤 포인터·참조·반복자가 무효화되는가?
7. 반환형과 반환값 사용 여부, 복잡도, 할당 실패 시 보장을 말한다.

### `tree_.assign(static_cast<std::size_t>(base_ * 2), 0)`

- 수신 `vector<int>`의 호출 전 상태와 두 인자의 타입·값·값 범주·허용 범위를 순서대로 말한다.
- 반환형과 반환값 사용 여부, 호출 뒤 size/capacity/원소 값, 기존 원소와 관찰자 무효화를 설명한다.
- O(count) 시간·공간, 할당 가능성과 예외가 발생했을 때 확인해야 할 보장을 말한다.

### `lines.push_back(formatter_.render(event))`

8. 수신 객체 타입과 호출 전 상태, 인자 식의 타입과 값 범주, 선택되는 `push_back` 오버로드를 말한다.
9. 반환형은 무엇이며 저장/무시되는가? 호출 뒤 size/capacity와 인자 임시 객체의 수명은 어떻게 변하는가?
10. 분할 상환 복잡도, 재할당과 무효화, 이동 생성이 예외를 던질 때의 보장을 설명한다.

### `stack.back()` / `stack.pop_back()` / `stack.empty()`

11. 각 호출의 수신 타입, 인자 수, 반환형과 반환값 사용 여부를 말한다.
12. `back()`과 `pop_back()`의 전제조건은 무엇이며 코드의 어떤 조건문이 이를 증명하는가?
13. `pop_back()` 뒤 size, capacity, 마지막 원소 수명, 다른 원소 참조의 유효성을 설명한다.

### 스트림과 알고리즘

14. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`의 각 입력·출력·상태 변화를 설명한다. 반환값은 어떻게 처리하는가?
15. `std::cin >> type >> first >> second`의 각 연산자 호출이 받는 두 피연산자와 반환 `istream&`가 연쇄에 쓰이는 과정을 말한다.
16. `std::max(answer, segment_tree.query(...))`에서 두 인자의 값 범주, 반환 참조의 수명, 대입 뒤 각 값의 상태를 말한다.
17. `tree_[index]`가 경계 검사를 하지 않는다는 뜻과 잘못된 index가 미정의 동작이 되는 이유를 설명한다.

## HLD와 세그먼트 트리

18. `parent`, `depth`, `subtree_size`, `heavy_child`, `head`, `position` 배열을 한 문장씩 정의한다.
19. light 간선의 자식 서브트리 크기가 부모의 절반 이하임을 증명하고, 경로가 O(log N)개 체인으로 나뉘는 결론을 낸다.
20. 간선 가중치를 더 깊은 자식 위치에 저장하면 간선 번호 갱신이 왜 한 점 갱신이 되는가?
21. 마지막 같은 체인 질의가 `position[to] + 1`에서 시작하는 이유를 LCA와 연결한다.
22. head가 다를 때 정점 자체의 depth가 아니라 `depth[head[from]]`과 `depth[head[to]]`를 비교하는 이유는 무엇인가?
23. 반복형 세그먼트 트리의 `left_node`가 홀수, `right_node`가 짝수일 때 그 노드를 답에 포함하는 이유를 구간 그림으로 설명한다.
24. 전처리, 한 점 갱신, 한 구간 질의, 한 경로 질의, 전체 공간 복잡도를 각각 적는다.
25. 가중치가 음수도 가능하다면 최댓값 항등값 0이 왜 틀릴 수 있으며 무엇으로 바꿔야 하는가?
26. 경로 연산이 문자열 연결처럼 비가환이면 현재 누산 순서가 왜 부족한지 설명한다.

## 검증으로 이해 증명하기

27. 같은 정점 경로가 0, 단일 간선이 현재 가중치, 여러 체인이 올바른 최대를 내는 CTest의 목적을 각각 말한다.
28. 작은 무작위 트리에서 갱신/질의를 생성하고 BFS brute force와 비교하는 테스트 절차를 의사 코드로 적는다.
29. N=100,000 일자 트리에서 재귀 대신 명시적 vector 스택을 사용한 이유와 heap/stack 객체 수명을 설명한다.
30. `edge_child[edge_index]`가 입력 방향과 무관하게 더 깊은 정점을 가리키는지 확인할 최소 반례를 만든다.

## 완료 기준

- 위 30문항을 자료 없이 설명한다.
- `std::format`, `reserve`, `assign`, `push_back`, `back/pop_back`, 스트림 호출의 인자 수만큼 계약을 빠짐없이 말한다.
- HLD의 절반 감소 불변식과 간선-LCA 제외 규칙을 직접 증명한다.
- 경고 포함 빌드, 두 학습 실행, 모든 CTest, 표준 라이브러리 전체 감사가 통과한다.
