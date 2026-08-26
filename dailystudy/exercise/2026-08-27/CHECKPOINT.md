# 2026-08-27 이해 검증

자료를 닫고 먼저 답한다. 표준 항목 이름을 번역하는 데 그치지 말고 수신 객체·각 위치 인자·반환·호출 후 상태·수명·복잡도·실패 조건을 실제 식과 연결한다.

## 1. 기초 문법과 아키텍처

1. `struct ReleaseReport`와 `class ReleaseCoordinator`의 기본 접근 권한 차이와 오늘 역할에 각각 알맞은 이유를 설명한다.
2. `run(const std::vector<int>& weights) const`의 반환형, 매개변수 타입, 두 `const`의 서로 다른 의미를 말한다.
3. 생성자에는 왜 반환형이 없으며 `std::barrier<decltype(on_phase_complete)>`의 템플릿 인자는 무엇인가?
4. `int prepared{}`와 `std::atomic<int> prepared{}`의 초기값, 복사 가능성, 동시 접근 계약을 비교한다.
5. `ReleaseCoordinator`, jthread, lambda, barrier, atomic 가운데 실행·단계·합계·수명을 각각 누가 책임지는가?
6. worker vector를 별도 블록에 둔 이유를 파괴 역순과 참조 캡처 수명으로 설명한다.

## 2. 값 범주·복사·이동·수명

1. `weights`, `on_phase_complete`, `std::move(on_phase_complete)`, worker lambda 식, `ReleaseReport{...}`의 값 범주를 분류한다.
2. `std::move`와 barrier 생성자 중 실제 callable 상태를 옮기는 것은 무엇인가?
3. `weight` 값 캡처와 `&prepared` 참조 캡처가 소유권·수명·동시 접근 면에서 어떻게 다른가?
4. 이름 있는 rvalue-reference 매개변수도 식으로 쓰면 lvalue라는 규칙을 오늘 `std::move`와 연결한다.
5. `ReleaseReport` 반환에서 prvalue 직접 초기화와 복사 생략을 설명한다.
6. workers가 barrier보다 오래 살면 어떤 댕글링 참조가 생길 수 있는가?

## 3. 표준 라이브러리 호출 계약

각 식의 정확한 수신 타입·호출 전 상태, 선택된 오버로드·템플릿 인자, 모든 위치 인자의 타입·값 범주·복사/이동/참조/소유권, 반환형·사용 여부, 호출 후 상태, 복잡도·할당·무효화·수명·오류·스레드 보장을 자료 없이 설명한다.

1. `std::barrier<decltype(on_phase_complete)>{static_cast<std::ptrdiff_t>(weights.size()), std::move(on_phase_complete)}`
2. `phase_gate.arrive_and_wait()`
3. `completed_phases.fetch_add(1, std::memory_order_relaxed)`
4. `prepared.load(std::memory_order_relaxed)`
5. `workers.reserve(weights.size())`
6. `workers.emplace_back(lambda)`
7. `std::jthread{lambda}`와 jthread 소멸자
8. `workers[0].joinable()`
9. `workers[0].join()`
10. `std::array<std::jthread, 2>{...}`와 `workers[1]`
11. `jobs_by_worker[worker].push_back(one_based_job - 1)`
12. `std::vector<int>(job_count, -1)`
13. `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)`
14. `std::cin >> worker_count >> job_count`
15. `std::cout << matching_size << '\n'`

추가 검증:

- barrier 생성자의 expected가 0이면 파괴 외 멤버 호출을 할 수 없는 점과 구현 최대치를 넘기면 전제조건 위반인 점을 설명한다.
- 한 worker가 예외·조기 return으로 도착하지 않으면 다른 참가자에게 어떤 진행 보장 문제가 생기는가?
- `arrive_and_drop()`이 `arrive_and_wait()`와 다음 단계 예상 참가자 수를 어떻게 다르게 바꾸는가?
- relaxed atomic이 왜 합계 데이터 경쟁은 막지만 “모든 준비 뒤 게시” 순서를 혼자 만들지는 못하는가?
- vector 재할당 시 기존 jthread 객체 참조는 무효화되지만 스레드 실행 자체는 왜 이동된 소유자와 계속 연결되는가?
- `join()`을 joinable하지 않은 jthread에 호출할 때 오류 계약을 설명한다.

## 4. 이분 매칭 대회 필수 지식

1. 이분 그래프와 매칭, 최대 매칭, 자유 정점, 교대 경로, 증강 경로를 각각 정의한다.
2. 현재 `owner_by_job[job]`가 저장하는 값과 `-1` 센티널의 의미를 말한다.
3. 빈 일을 찾거나 현재 담당자를 다른 일로 옮길 수 있을 때 간선을 뒤집어도 매칭인 이유를 증명한다.
4. 증강 경로 하나가 매칭 크기를 정확히 1 늘리는 이유를 경로의 선택/비선택 간선 개수로 보인다.
5. 재귀 호출에 현재 worker가 아니라 `owner_by_job[job]`를 넘기는 이유를 설명한다.
6. 같은 증강 시도에서 일을 다시 방문하지 않아도 해를 놓치지 않는 이유와 사이클 방지 효과를 말한다.
7. stamp를 worker+1로 두면 매 시도마다 O(M) 초기화하지 않아도 되는 이유와 정수 범위 안전성을 설명한다.
8. 모든 왼쪽 정점에 대해 증강 경로가 더 없을 때 매칭이 최대라는 증강 경로 정리(Berge)를 연결한다.
9. `직원1={1,2}, 직원2={1}`에서 owner 배열과 재귀 호출을 손으로 추적해 답 2를 얻는다.
10. Kuhn `O(N*E)`와 Hopcroft–Karp `O(E√V)`의 선택 기준을 말한다.
11. 이분 매칭을 최대 유량으로 바꾸는 source→왼쪽→오른쪽→sink 구성과 모든 용량 1의 의미를 설명한다.

## 5. 초보자 실기 검증

- [ ] `main.cpp` 출력 `9 18 2`와 `problem.cpp` 출력 `14`를 실행 전에 계산했다.
- [ ] 두 단계마다 각 worker의 도착 수와 완료 함수 호출 시점을 표로 그렸다.
- [ ] 참조 캡처 대상을 worker보다 먼저 파괴하는 잘못된 예를 작성하고 실행 없이 위험을 설명했다.
- [ ] 공개 예제의 `owner_by_job`을 직원마다 손 추적해 최대 배정 4를 얻었다.
- [ ] 재배정 최소 반례와 간선 없는 경계 입력을 코드 없이 계산했다.
- [ ] 모든 C++ 표준 호출마다 인자 수와 주석 설명 수, 반환값 저장·검사·무시 여부를 대조했다.
- [ ] 세 실행 파일을 높은 경고 수준으로 빌드하고 두 학습 실행 파일을 직접 실행했다.
- [ ] CTest 전체와 표준 라이브러리 문서 감사를 통과했다.

## 완료 기준

barrier를 “스레드를 기다린다”에서 멈추지 않고 expected 카운트, 단계, 완료 함수, 재사용, happens-before, drop, 수명과 교착 조건까지 설명한다. 이분 매칭은 단순 탐욕이 아니라 교대·증강 경로 불변식, 재배정, 방문 제어, 정확성과 복잡도를 코드 없이 다시 구현할 수 있어야 한다.
