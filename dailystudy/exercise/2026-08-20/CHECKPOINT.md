# 2026-08-20 CHECKPOINT

자료를 닫고 답한 뒤 코드와 공용 문서를 다시 확인한다. 이름 번역이 아니라 각 식의 입력, 반환, 상태 변화와 수명을 말해야 통과다.

## Modern C++ 문법·아키텍처

1. `struct RawDeployment`와 `class DeploymentPlan`의 기본 접근 차이, `public`/`private`가 바꾸는 접근 범위를 설명한다.
2. DTO를 집합체로 두고 도메인 객체를 `private` 멤버로 감싼 이유를 “생성 편의”와 “불변식” 관점에서 설명한다.
3. `RawDeployment{.service = "billing", .replicas = 3, .canary = true}`의 지정자 순서, 생략 멤버, 중괄호 초기화 규칙을 설명한다.
4. `explicit ReplicaCount(int value) : value_{value} {}`에서 생성자에 반환형이 없는 이유, 매개변수, 멤버 초기화 목록과 생성자 본문의 실행 순서를 말한다.
5. `ReplicaCount count = 3;`과 `ReplicaCount count{3};` 중 어떤 식이 허용되는지 `explicit`의 암시 변환 차단과 연결한다.
6. `request`, `std::move(request)`, `DeploymentPlan{...}`, `result.value()`를 lvalue·xvalue·prvalue로 분류한다.
7. `make(request)`와 `make(std::move(request))`가 값 매개변수를 만들 때 복사·이동 중 무엇을 선택할 수 있는지, 호출 뒤 원본 상태를 비교한다.
8. `const DeploymentPlan& plan{result.value()}`과 `plan.service()`가 빌리는 객체, 참조가 유효한 기간, 어떤 변경이 참조를 무효로 만들 수 있는지 설명한다.
9. 반환 `DeploymentPlan` prvalue가 `expected` 성공값과 `result`를 초기화할 때 복사 생략·이동이 일어날 수 있는 지점을 그린다.
10. 문자열 접근과 조건 검사가 로드·비교·분기·호출로 번역될 수 있지만 특정 명령으로 단정할 수 없는 이유를 CPU·ABI·컴파일러·최적화와 연결한다.

## 표준 라이브러리 호출 계약

11. `request.service.empty()`의 정확한 수신 타입·호출 전 상태, 인자 수, 대표 시그니처, 반환형/사용처, 호출 뒤 상태, 복잡도·할당·예외를 설명한다.
12. `std::move(request.service)`의 템플릿 입력 식 타입·값 범주, 반환 값 범주, 소유권을 실제로 옮기는 주체, 이동 후 문자열 계약을 설명한다.
13. `std::unexpected<PlanError>{PlanError::empty_service}`가 소유하는 입력, 생성 결과의 값 범주, `expected`가 선택하는 대안과 할당·예외 가능성을 설명한다.
14. `if (!result)`의 `expected::operator bool()`이 받는 명시 인자, 반환값, 상태 변화와 복잡도를 말한다.
15. `result.value()`와 `result.error()` 각각의 호출 전제조건, 반환 참조 타입, 잘못 호출했을 때의 계약, 반환 참조 수명을 비교한다.
16. `graph[from].push_back(to)`에서 수신 객체의 정확한 타입과 호출 전 크기, 선택 오버로드, `to`의 타입·값 범주·복사, 반환형, 크기/용량 변화, 재할당과 무효화, 복잡도·예외 보장을 설명한다.
17. `pending.front()`와 `pending.pop()`을 순서대로 호출하는 이유를 각 반환형, 빈 큐 전제조건, 호출 뒤 큐와 참조 상태로 설명한다.
18. `pending.push(next)`와 `pending.empty()`가 받는 각 입력, 반환값 사용 여부, 큐 크기 변화, 복잡도·할당·예외를 비교한다.
19. `std::min(bottleneck, residual)`의 두 매개변수 타입·값 범주, 반환 참조, 대입으로 값을 즉시 복사하는 이유와 수명 위험을 설명한다.
20. `std::numeric_limits<int>::max()`는 수신 객체나 데이터 인자가 있는지, 반환형·상태 변화·복잡도·오버플로 방지 역할을 말한다.
21. `std::cin >> from_label >> to_label >> amount`의 각 입력 대상과 반환 `istream&`, 성공 뒤 대상/입력 위치, 실패 상태와 최종 반환 참조 사용 여부를 설명한다.
22. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`의 각 인자, 반환값을 버리는 이유, 호출 뒤 전역/수신 상태, 보장하지 않는 출력 순서를 설명한다.

## 최대 유량·Edmonds–Karp

23. 용량, 유량, 잔여 용량, 증가 경로, 병목을 오늘 코드의 `capacity`, `flow`, `parent`, `bottleneck`과 연결해 정의한다.
24. `flow[vertex][previous] -= bottleneck`이 만드는 역방향 잔여 용량이 과거 경로 선택을 어떻게 취소하는지 작은 그래프로 보인다.
25. BFS를 쓰면 증가 경로가 간선 수 기준 최단이 되는 이유와 이것이 증가 횟수를 `O(VE)`로 제한하는 핵심을 설명한다.
26. 각 BFS `O(E)`, 증가 횟수 `O(VE)`에서 전체 `O(VE^2)`가 되는 과정을 적고 공간 `O(V^2+E)`의 구성 요소를 말한다.
27. 평행 파이프에서 `capacity[from][to] = amount`가 아니라 `+=`여야 하는 이유와 양방향 입력을 두 용량에 더하는 이유를 설명한다.
28. 증가 경로가 없을 때 잔여 그래프에서 source로부터 도달 가능한 집합이 만드는 컷과 현재 유량 값이 같은 이유를 말해 최대성을 증명한다.
29. 최대 유량을 이분 매칭으로 바꾸는 전형적 네트워크 구성(source→왼쪽→오른쪽→sink)과 각 용량을 설명한다.
30. 공개 예제, 평행 간선, 소문자 정점, 두 갈래 경로 테스트가 각각 어떤 오류를 잡는지 설명하고 직접 예상 출력을 적는다.

## 실기 완료 기준

- `RawDeployment`에 region을 추가하고 도메인 객체까지 안전하게 전달한다.
- ICPC 풀이의 네 테스트를 손으로 추적한 뒤 새 테스트 하나를 추가한다.
- 모든 C++ 파일을 높은 경고 수준으로 빌드하고 실행한다.
- CTest와 `audit-standard-library-docs.ps1 -Scope all`이 모두 통과한다.
