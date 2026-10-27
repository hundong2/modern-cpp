# 2026-10-27 CHECKPOINT — 취소 callback과 함수형 그래프 거리 질의

답을 코드에서 그대로 복사하지 말고, 먼저 말과 그림으로 설명한 뒤 실제 식을 가리키며 검증한다.

## 1. 초보자 문법

- [ ] 1. `<stop_token>`, `<utility>`, `<iostream>`, `<vector>`, `<cstddef>`가 각각 어떤 선언을 제공하는가?
- [ ] 2. `int`, `bool`, `std::size_t`의 역할과 signed/unsigned 혼합 시 주의점을 설명하라.
- [ ] 3. `CleanupCount callback_runs{};`와 `bool resource_open{true};`의 초기값은 무엇인가?
- [ ] 4. 중괄호 초기화가 narrowing을 막는 예를 하나 만들라.
- [ ] 5. `using CleanupCount = int;`가 강한 새 타입이 아닌 이유는 무엇인가?
- [ ] 6. `std::stop_callback<decltype(cleanup_action)>`에서 template 인자는 무엇이며 `decltype`은 식을 실행하는가?
- [ ] 7. `struct`와 `class`의 기본 접근 차이, `public:`/`private:`의 목적을 설명하라.
- [ ] 8. `explicit CancellationCoordinator(std::stop_source)`가 막는 암시 변환을 예로 들어라.
- [ ] 9. 멤버 초기화 목록과 생성자 본문 중 실제 멤버 수명이 먼저 시작되는 곳은 어디인가?
- [ ] 10. 멤버 초기화 순서는 목록 순서인가 선언 순서인가?
- [ ] 11. `token() const noexcept`의 뒤 `const`와 `noexcept`가 각각 약속하는 것을 말하라.
- [ ] 12. reference와 pointer의 null·재바인딩 차이, 둘 다 자동 소유권이 아닌 이유를 설명하라.
- [ ] 13. `[&ledger]`가 무엇을 저장하고 무엇을 소유하지 않는가?
- [ ] 14. `if (resource_open)`와 callback의 `{}` block이 제어 흐름과 수명에 미치는 영향을 말하라.
- [ ] 15. `[[nodiscard]] bool request_stop()` 반환을 버리면 어떤 정보를 잃는가?

## 2. 값 범주·복사/이동·수명

- [ ] 16. `coordinator`, `token`, `cleanup_action`, `source` 매개변수는 식으로 사용할 때 각각 어떤 값 범주인가?
- [ ] 17. `std::stop_source{}`가 prvalue인 이유와 그 수명이 어디까지 이어지는지 말하라.
- [ ] 18. `std::move(source)` 결과는 어떤 타입·값 범주이며 실제 공유 상태를 누가 옮기는가?
- [ ] 19. 이름 있는 `std::stop_source&&` 매개변수를 식으로 쓰면 왜 lvalue인가?
- [ ] 20. closure를 이동할 때 reference capture 대상 ledger도 이동하는가?
- [ ] 21. moved-from source와 moved-from closure를 파괴해도 되는가? 다시 사용할 때 무엇을 가정하면 안 되는가?
- [ ] 22. `coordinator.token()` 반환 prvalue와 지역 token 초기화에 보장 복사 생략이 어떻게 적용되는가?
- [ ] 23. 보장 복사 생략과 이름 있는 지역을 반환할 때의 NRVO를 구별하라.
- [ ] 24. stop source/token/callback 중 누가 내부 stop state 수명을 공유하고 누가 stop 요청 권한을 갖는가?
- [ ] 25. callback의 `&ledger`가 안전하도록 선언·파괴 순서를 적어라.
- [ ] 26. callback을 owner보다 오래 보관하면 어떤 식이 댕글링되고 결과는 무엇인가?
- [ ] 27. `stop_callback`이 copy/move 불가인 사실이 등록 수명 모델을 어떻게 더 명확하게 하는가?
- [ ] 28. `return ExerciseResult{first, second, score};`에서 prvalue와 결과 객체의 관계를 설명하라.

## 3. 핵심 표준 호출의 여섯 항목 계약

- [ ] 29. `std::stop_source source{};`의 정확한 생성자, 인자, 사후 상태, 할당·예외 계약을 말하라.
- [ ] 30. `source_.get_token()`의 수신 타입/상태, 시그니처, 인자 수, 반환형과 사용을 말하라.
- [ ] 31. get_token 뒤 source와 token의 상태·수명 관계, 무효화와 thread 전제를 설명하라.
- [ ] 32. `CleanupCallback cleanup{token, std::move(cleanup_action)};`에서 `Callback`과 `Initializer` 타입을 채워라.
- [ ] 33. callback 생성자의 두 인자 각각의 타입·값 범주·소유권 의미를 설명하라.
- [ ] 34. 요청 전 callback 생성 뒤 등록 상태와 원본 closure 상태는 무엇인가?
- [ ] 35. 이미 요청된 token에 callback을 생성하면 어느 thread에서 언제 실행되는가?
- [ ] 36. stop 불가능한 disengaged token이라면 callback 생성은 어떤 효과를 갖는가?
- [ ] 37. callback callable 구성에서 가능한 예외와 callback 본문 예외가 빠져나올 때의 결과를 구별하라.
- [ ] 38. stop callback 소멸자가 미래 실행, 현재 다른 thread 실행, callable 수명에 미치는 영향을 말하라.
- [ ] 39. `source.request_stop()`의 정확한 시그니처와 위치 인자 수는 무엇인가?
- [ ] 40. 첫 요청과 반복 요청의 반환값·상태 변화·callback 실행 횟수를 말하라.
- [ ] 41. request_stop 복잡도가 callback 수와 본문 시간에 어떻게 의존하는가?
- [ ] 42. 동시 request_stop 호출 둘 중 몇 개가 true를 얻으며 왜 data race가 아닌가?
- [ ] 43. request_stop이 thread-safe라는 말이 captured `int`의 동시 접근도 자동으로 안전하게 한다는 뜻인가?
- [ ] 44. `std::move(cleanup_action)`의 template 인자 추론, 반환형, 반환값 사용과 상태 변화를 말하라.

## 4. 나머지 표준 호출·연산자

- [ ] 45. `std::cout << "first=" << first_request`의 각 선택 overload와 각 반환 `ostream&`의 사용을 말하라.
- [ ] 46. `boolalpha`를 켜지 않은 bool이 어떤 문자로 출력되는가?
- [ ] 47. `\n`과 `std::endl`의 flush 차이를 설명하라.
- [ ] 48. stream 삽입 실패가 상태 bit와 예외 mask에 따라 어떻게 보이는가?
- [ ] 49. `std::ios::sync_with_stdio(false)`의 bool 인자·반환값과 C stdio 혼용 전제를 말하라.
- [ ] 50. `std::cin.tie(nullptr)`의 pointer 인자, 반환값과 자동 flush 변화는 무엇인가?
- [ ] 51. `std::cin >> n >> query_count`에서 각 `int&` 인자와 반환 stream reference를 설명하라.
- [ ] 52. vector fill 생성자에서 count와 value 인자의 타입·허용 범위·할당 실패를 설명하라.
- [ ] 53. `vector::reserve(n)` 성공·실패 시 capacity와 기존 reference/iterator 무효화를 구별하라.
- [ ] 54. `vector::push_back(v)`가 재할당하는 경우와 하지 않는 경우의 무효화를 말하라.
- [ ] 55. `vector::operator[]`의 반환형, bounds check 부재와 UB 전제를 말하라.
- [ ] 56. `vector::size()`의 반환형·복잡도·상태 변화 여부를 말하라.

## 5. 함수형 그래프 알고리즘

- [ ] 57. 함수형 그래프의 정의와 각 성분에 사이클이 정확히 하나인 이유를 증명하라.
- [ ] 58. indegree 0 정점이 사이클에 있을 수 없는 이유는 무엇인가?
- [ ] 59. peeling 종료 뒤 남은 정점이 정확히 사이클 정점임을 양방향으로 증명하라.
- [ ] 60. reverse edge 목록을 만드는 이유와 원래 edge를 역으로 직접 찾을 때의 비용을 말하라.
- [ ] 61. 사이클을 순회하며 component id, position, length를 어떻게 붙이는가?
- [ ] 62. 여러 cycle vertex를 seed로 둔 reverse BFS에서 `depth == -1` 불변식의 역할은 무엇인가?
- [ ] 63. cycle vertex의 depth를 0으로 두면 tree/cycle 목적지를 어떻게 구별하는가?
- [ ] 64. `jump[k][v] = jump[k-1][jump[k-1][v]]`를 귀납적으로 증명하라.
- [ ] 65. `advance(v, steps)`가 steps의 set bit만 처리해 정확한 이유를 이진 표현으로 설명하라.
- [ ] 66. 서로 다른 component 사이 도달이 불가능한 이유는 무엇인가?
- [ ] 67. 목적지 `b`가 트리일 때 `depth[a] >= depth[b]`만으로 충분하지 않은 반례를 그려라.
- [ ] 68. tree 목적지의 필요충분 조건 `advance(a, depth[a]-depth[b]) == b`를 증명하라.
- [ ] 69. 출발점이 cycle이고 목적지가 tree이면 왜 항상 불가능한가?
- [ ] 70. 목적지가 cycle일 때 출발점의 cycle entry를 어떻게 구하는가?
- [ ] 71. `(pos[b]-pos[entry]+length)%length`가 음수 없이 순방향 최단 이동을 만드는 이유를 설명하라.
- [ ] 72. `a == b`인 tree/cycle 두 경우가 모두 0을 내는지 식으로 확인하라.
- [ ] 73. 전처리·질의·전체 시간 및 공간 복잡도를 각각 유도하라.
- [ ] 74. 출발점마다 직접 추적하는 풀이의 최악 시간 반례를 만들라.
- [ ] 75. 재귀를 쓰지 않은 구현이 긴 꼬리 입력에서 갖는 이점을 말하라.

## 6. 실제 식 해석과 실기

- [ ] 76. `node = jump[level][node]`에서 두 operator[]의 수신 타입·인자·반환 reference와 load/store를 추적하라.
- [ ] 77. `steps >>= 1`과 `(steps & 1) != 0`이 어떤 bit를 검사·소비하는가?
- [ ] 78. `peeled[head]` FIFO가 처리 중 `push_back`으로 늘어나도 모든 정점을 한 번씩 처리하는 이유를 말하라.
- [ ] 79. 공식 예제 `1→2`, `1→3`, `1→4`를 metadata와 jump로 직접 계산하라.
- [ ] 80. mapping `1→2→3→4→3`, `7→6→5→4`, `8→6`에서 `7→3`, `8→7`, `4→3`을 계산하라.
- [ ] 81. callback block을 request 전에 끝내 `cleanup=0`이 되는 변형을 작성하고 CTest를 추가하라.
- [ ] 82. 이미 요청된 token으로 만든 late callback 안에서 다시 `request_stop()`을 부르면 반환값이 무엇인지 예측하라.
- [ ] 83. 작은 무작위 그래프와 질의를 직접 따라가는 oracle을 작성해 제출 풀이와 비교하라.
- [ ] 84. `n=q=200000` 긴 꼬리+짧은 cycle stress를 만들어 runtime과 peak memory를 확인하라.
- [ ] 85. `powershell -ExecutionPolicy Bypass -File dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all`을 실행하고 모든 첫 호출 계약을 다시 읽어라.

## 최종 빈칸 요약

`stop_source`는 stop을 ______ 권한을 가진 값이고 `stop_token`은 같은 ______를 공유해 관찰·등록한다. `stop_callback` 객체의 ______이 callback 등록 수명이다. 첫 `request_stop()`은 ______를 반환하고 현재 callback을 ______ 실행하며, 뒤 요청은 ______를 반환한다. 이미 요청된 token의 callback은 ______ 안에서 즉시 실행된다. 함수형 그래프는 성분마다 유향 ______ 하나와 역방향 ______들로 나뉜다. tree 목적지는 depth 차만큼 올린 실제 정점 ______까지 확인하고, cycle 목적지는 꼬리 깊이와 cycle ______ 차를 더한다. 총 시간은 ______, 공간은 ______이다.
