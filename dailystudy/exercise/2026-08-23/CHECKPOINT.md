# 2026-08-23 이해 검증

자료를 닫고 먼저 답한다. 각 문항은 실제 코드 식, 입력·출력, 상태 변화를 근거로 설명해야 완료다.

## C++ 기초·객체 모델

- [ ] `Component`가 `struct`, `StartupCoordinator`가 `class`일 때 기본 접근 차이와 오늘 역할을 설명한다.
- [ ] `explicit StartupCoordinator(std::ptrdiff_t)`에서 반환형이 없는 이유, 매개변수 타입, `{3}` 직접 초기화가 올바른 이유를 설명한다.
- [ ] `ready_{component_count}, completed_units_{0}`의 초기화 순서가 목록 표기 순서가 아니라 멤버 선언 순서라는 점을 말한다.
- [ ] `coordinator`, `Component{...}`, `std::move(component)`, `launch(...)` 반환을 각각 lvalue·prvalue·xvalue로 분류한다.
- [ ] 람다의 `&coordinator`와 `component = std::move(component)`가 각각 비소유 참조와 소유 값인 이유 및 필요한 수명을 그린다.
- [ ] `std::jthread`가 복사되지 않고 이동 또는 복사 생략으로 소유권을 전달하는 이유를 설명한다.
- [ ] 함수 끝에서 지역 객체가 역순 소멸하고 jthread가 합류할 때 coordinator 참조가 아직 유효함을 증명한다.

## 표준 라이브러리 호출 계약

- [ ] `ready_{component_count}`의 수신 생성 타입, 입력 식·타입·허용 범위, 반환 객체, 호출 뒤 카운터, 위반 시 결과를 설명한다.
- [ ] `ready_.count_down()`의 정확한 수신 타입·호출 전 카운터, 생략한 기본 인자 1, `void` 반환, 호출 뒤 상태와 동기화를 설명한다.
- [ ] `ready_.wait()`가 데이터 인자를 받지 않고 `void`를 반환하며, latch를 바꾸지 않는다는 점과 반환 전후 happens-before를 설명한다.
- [ ] `release_.arrive_and_wait()`가 `count_down()+wait()`와 어떤 상태 변화를 한 호출로 묶는지, 재사용이 금지되는 이유를 설명한다.
- [ ] `completed_units_.fetch_add(units, std::memory_order_relaxed)`의 수신 `atomic<int>`, 두 입력의 타입·값 범주·의미, 증가 전 반환값을 버린다는 점, 호출 뒤 값을 설명한다.
- [ ] `completed_units_.load(std::memory_order_relaxed)`의 반환형·사용 위치·수신 객체 불변성과 latch 동기화가 담당하는 부분을 설명한다.
- [ ] `std::jthread{std::move(task)}`가 선택하는 생성자 템플릿의 F/Args, callable 소유권, 성공·실패, 소멸 계약을 설명한다.
- [ ] `edges.reserve(static_cast<std::size_t>(edge_count))`가 크기·용량·기존 반복자·할당 실패 상태를 어떻게 바꾸는지 설명한다.
- [ ] `edges.push_back(edge)`가 lvalue Edge를 복사하는 오버로드인 이유, 반환형, 상각 복잡도와 예약 용량 안 무효화를 설명한다.
- [ ] `std::cin >> edge.from >> edge.to >> edge.cost`의 각 입력 대상과 `istream&` 반환, 실패 뒤 변수·스트림 상태를 설명한다.
- [ ] `std::cout << value << '\n'`의 각 입력, `ostream&` 반환을 버리는 위치, flush 여부와 오류 상태를 설명한다.

## Bellman–Ford와 대회 필수 지식

- [ ] i번째 전체 간선 순회 뒤 “간선 수 최대 i인 경로” 불변식을 귀납법으로 증명한다.
- [ ] 음수 사이클이 없을 때 단순 최단 경로가 최대 N-1개 간선을 갖는 이유를 설명한다.
- [ ] N번째 완화가 시작점에서 도달 가능한 음수 사이클의 증거인 이유를 비둘기집 원리와 연결한다.
- [ ] `distance[from] == infinity` 분기가 도달 불가능한 음수 사이클 오판과 INF 덧셈을 함께 막는 이유를 설명한다.
- [ ] 다익스트라의 방문 확정 논리가 음수 간선에서 깨지는 세 정점 반례를 만든다.
- [ ] 시간 `O(NM)`, 공간 `O(N+M)`을 반복문 횟수와 저장 객체 수로 직접 유도한다.

## 실행으로 증명

- [ ] w64devkit g++로 세 실행 파일을 높은 경고 수준에서 빌드한다.
- [ ] `daily_main` 출력 `18`, `daily_problem` 출력 `10`과 종료 코드 0을 확인한다.
- [ ] CTest 8개가 모두 통과하고 공식 예제·두 종류 음수 사이클·평행 간선·음수 경로·단일 도시 음수 루프를 검증함을 설명한다.
- [ ] `../tools/audit-standard-library-docs.ps1 -Scope all`이 심볼·헤더·멤버 누락 없이 통과한다.
- [ ] 모든 C++ 파일을 다시 읽어 각 표준 호출의 인자 수, 반환값 사용/무시, 상태 변화 주석과 알고리즘 문서 링크를 대조한다.
