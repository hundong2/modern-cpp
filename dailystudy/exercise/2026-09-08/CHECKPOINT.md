# 2026-09-08 CHECKPOINT

아래 문제는 “읽었다”가 아니라 오늘 코드를 자료 없이 설명하고 안전하게 바꿀 수 있음을 검증한다. 먼저 답을 말하거나 적은 뒤 코드와 공용 문서에서 근거를 확인한다.

## A. 기초 문법과 타입

- [ ] 1. `int attempts_{}`와 `const Config* first{}`의 초기값을 각각 말하라.
- [ ] 2. 중괄호 초기화가 narrowing을 거부하는 기본 타입 식을 하나 만들라.
- [ ] 3. `struct Report`와 `class Config`의 기본 접근 지정자는 무엇인가?
- [ ] 4. `public:`/`private:`가 이름 접근과 객체 배치·실행 속도에 미치는 영향을 구분하라.
- [ ] 5. 생성자에는 왜 반환형이 없으며 `explicit Config(...)`는 어떤 암시 변환을 막는가?
- [ ] 6. `: endpoint_{...}, generation_{...}`가 본문 대입과 다른 수명 단계를 설명하라.
- [ ] 7. 멤버 초기화 순서가 목록에 쓴 순서가 아니라 선언 순서인 이유와 오늘 선언 순서를 말하라.
- [ ] 8. `using Mask = unsigned long long`가 새 강한 타입을 만드는 문장이 아닌 이유는 무엇이며, 이 기본 타입의 최소 폭은 오늘 shift에 어떤 보장을 주는가?
- [ ] 9. `std::optional<Config>`와 `std::vector<std::vector<int>>`의 각 템플릿 인자를 풀어 말하라.
- [ ] 10. `const Config& get() const`에서 반환형의 `const`/`&`와 함수 뒤 `const`의 뜻을 구분하라.
- [ ] 11. `ConfigSource& source_`와 `const Config* first`가 각각 무엇을 빌리며 왜 소유하지 않는가?
- [ ] 12. `source`, `provider`, reader 블록의 파괴 순서가 비소유 참조를 안전하게 만드는 과정을 말하라.
- [ ] 13. `if`, `for`, `continue`, `return`이 오늘 세 C++ 파일에서 만드는 제어 흐름을 한 예씩 찾으라.
- [ ] 14. 가상 소멸자가 필요한 일반 조건과 `ConfigSource`/`ReportSource`에 둔 이유를 말하라.

## B. 값 범주·복사·이동·수명

- [ ] 15. 이름 있는 `source`, `snapshot`, `first` 식의 값 범주는 무엇인가?
- [ ] 16. `std::string{"config.internal"}`, lambda 식, `Config{...}`, `Report{73}`의 값 범주는 무엇인가?
- [ ] 17. `std::move(endpoint)`의 정확한 인자 타입·값 범주와 반환 reference 타입을 설명하라.
- [ ] 18. `std::move` 자체가 문자열을 옮기지 않는다는 말을 뒤의 string 이동 생성과 연결하라.
- [ ] 19. `Config snapshot{*first}`에서 어떤 복사가 일어나며 cache의 원본 소유권은 어떻게 되는가?
- [ ] 20. `Config transferred{std::move(snapshot)}` 뒤 transferred와 snapshot의 string 상태를 각각 말하라.
- [ ] 21. 이동된 `snapshot.endpoint()`가 반드시 빈 문자열이라고 가정할 수 없는 이유는 무엇인가?
- [ ] 22. `Report`는 int 하나뿐인데도 `std::move(snapshot)` 결과가 xvalue라는 사실과 실제 자원 이전 비용을 구분하라.
- [ ] 23. `return Config{...}`의 same-type prvalue 직접 구성과 이름 있는 지역의 선택적 NRVO를 구분하라.
- [ ] 24. `optional::emplace(source_.load())`에서 load 결과 객체와 contained Config 사이의 이동 가능 지점을 추적하라.
- [ ] 25. `first`/`second`가 가리키는 Config의 소유자는 누구이며 언제 두 pointer가 댕글링되는가?
- [ ] 26. lambda `[&]`가 캡처 대상 수명을 연장하지 않는데 오늘은 왜 안전한가?

## C. `std::once_flag`와 `std::call_once` 호출 계약

- [ ] 27. `std::once_flag init_flag_{}`가 선택하는 생성자 시그니처, 인자, 반환, 초기 상태를 말하라.
- [ ] 28. once_flag 생성 자체가 동기화가 아니라는 말이 객체 게시 순서에 어떤 요구를 만드는가?
- [ ] 29. once_flag의 복사 생성/대입이 삭제되고 reset API가 없는 이유를 일회 초기화 의미와 연결하라.
- [ ] 30. once_flag 멤버 때문에 `LazyConfigProvider`의 암시적 복사·이동 가능성이 어떻게 되는가?
- [ ] 31. `std::call_once(init_flag_, [this]{...})`의 두 인자 타입·값 범주·소유권 의미를 정확히 말하라.
- [ ] 32. 위 호출에서 추론되는 `Callable`, 빈 `Args...`, 반환형과 반환값 사용 여부를 말하라.
- [ ] 33. active, exceptional, returning, passive 실행을 각각 정의하라.
- [ ] 34. 같은 flag에서 returning 실행이 최대 하나이고 존재한다면 마지막 active인 이유를 설명하라.
- [ ] 35. callable이 예외를 던지면 어느 호출자가 무엇을 받고 다음 호출은 어떤 상태에서 시작하는가?
- [ ] 36. 예외 전 source가 파일을 썼다면 call_once가 그 외부 쓰기를 rollback하지 않는 이유는 무엇인가?
- [ ] 37. active 실행들의 total order와 returning 실행→passive 반환 동기화 관계를 그려라.
- [ ] 38. 위 동기화가 비원자 optional/Config의 게시를 데이터 경쟁 없이 읽게 하는 근거를 말하라.
- [ ] 39. 서로 다른 callable을 같은 flag에 넘기면 최초 성공 뒤 나머지 callable은 어떻게 되는가?
- [ ] 40. call_once의 점근 복잡도, 할당, lock-free, 공정성, 대기 상한을 왜 단정할 수 없는가?
- [ ] 41. callable이 같은 flag로 재귀 call_once를 시도하면 왜 설계상 피해야 하는가?
- [ ] 42. `mutable`인 flag/cache와 `get() const`가 물리적 변경과 논리적 const를 어떻게 구분하는가?

## D. `std::optional`, `std::jthread`, string 실제 식 계약

- [ ] 43. `std::optional<Config> cache_{}`의 기본 생성 뒤 상태·할당·예외·contained 수명을 말하라.
- [ ] 44. `cache_.emplace(source_.load())`의 수신자, 선택 템플릿 인자 팩, 반환형과 반환값 사용을 말하라.
- [ ] 45. load가 인자 평가 중 던지는 경우와 Config contained 생성이 던지는 경우 optional 상태를 구분하라.
- [ ] 46. emplace 전에 값이 이미 있었다면 그 값의 수명과 과거 reference는 어떻게 되는가?
- [ ] 47. `mutable cache_`를 `const std::optional<T>& published_cache`에 먼저 바인딩하는 이유, `published_cache.value()`가 선택하는 cv/ref overload와 반환 reference 수명을 말하라.
- [ ] 48. 빈 optional에서 `value()`를 부르면 어떤 오류 방식이고 오늘 불변식은 이를 어떻게 배제하는가?
- [ ] 49. `std::jthread first_reader{lambda}`의 수신/목적 객체, callable 인자, 반환형과 생성 뒤 상태를 말하라.
- [ ] 50. jthread 생성 실패 가능성과 첫 reader 생성 뒤 둘째 reader 생성이 실패할 때 RAII 정리를 설명하라.
- [ ] 51. jthread 소멸자가 joinable thread에 수행하는 stop 요청과 join이 pointer 수명에 주는 보장을 말하라.
- [ ] 52. 두 lambda가 서로 다른 pointer 객체에 쓰고 같은 Config는 읽기만 하는 점을 데이터 경쟁 분석에 포함하라.
- [ ] 53. `std::string{"config.internal"}` 생성자의 pointer 전제, 시간 복잡도, 할당·예외 가능성을 설명하라.
- [ ] 54. string 복사 생성과 기본 allocator 이동 생성의 복잡도·원본 상태·관찰자 귀속을 비교하라.

## E. vector·스트림 호출 계약

- [ ] 55. `incoming(n)` count 생성자의 정확한 count 타입과 성공 뒤 바깥/안쪽 vector size를 말하라.
- [ ] 56. `dp(cell_count, 0)` fill 생성자의 두 인자·반환·시간/공간·예외 계약을 말하라.
- [ ] 57. `incoming[to]`의 반환형, 범위 전제와 범위 밖 접근 결과를 말하라.
- [ ] 58. `.push_back(from)`이 고르는 const lvalue overload, 반환형, size 변화와 분할 상환 복잡도를 말하라.
- [ ] 59. 안쪽 vector가 재할당될 때 어떤 pointer/reference/iterator가 무효화되고 바깥 vector 관찰자는 어떻게 되는가?
- [ ] 60. `predecessors.size()`의 반환형·복잡도·예외·상태 변화 계약을 말하라.
- [ ] 61. `predecessors[index]`와 `dp[flat_index(...)]`의 범위를 각각 루프·산술로 증명하라.
- [ ] 62. `std::ios::sync_with_stdio(false)`의 bool 인자·bool 반환값과 첫 I/O 전 호출 이유를 말하라.
- [ ] 63. `std::cin.tie(nullptr)`의 수신 타입, pointer 인자, 반환 pointer 사용과 소유권 변화를 말하라.
- [ ] 64. `std::cin >> city_count >> flight_count`의 각 수정 인자와 중간/마지막 `istream&` 사용을 말하라.
- [ ] 65. 입력 추출 실패가 상태 비트와 예외 mask에 따라 어떻게 표현되고 온라인 저지 코드는 무엇을 전제로 하는가?
- [ ] 66. `std::cout << dp[...] << '\n'`에서 int/char overload, 각 반환 `ostream&`의 연쇄 사용을 말하라.
- [ ] 67. 출력 비용을 무조건 O(1)이라 할 수 없는 이유와 실패·다중 스레드 레코드 계약을 설명하라.

## F. Hamiltonian 경로 부분집합 DP

- [ ] 68. Hamiltonian 경로와 단순 경로의 관계를 자신의 말로 정의하라.
- [ ] 69. `dp[mask][v]`가 세는 대상을 시작점·방문 집합·마지막 정점까지 빠짐없이 말하라.
- [ ] 70. `dp[1][0] = 1`이 왜 경로 한 개를 뜻하는가?
- [ ] 71. `previous_mask = mask ^ city_bit`가 city 비트를 확실히 제거하는 전제는 무엇인가?
- [ ] 72. 마지막 간선 `u -> v`를 제거한 이전 상태 점화식을 수식으로 쓰라.
- [ ] 73. 모든 이전 상태에 `u -> v`를 붙이면 유효 경로가 되는 이유를 증명하라.
- [ ] 74. 모든 유효 경로가 정확히 하나의 이전 상태와 마지막 입력 간선 occurrence로 분해됨을 증명하라.
- [ ] 75. 병렬 `1 -> 2` 항공편 두 개를 predecessor 목록에 둘 다 두면 답이 2가 되는 의미를 설명하라.
- [ ] 76. 시작점 비트가 없는 mask를 건너뛰어도 답이 사라지지 않는 이유는 무엇인가?
- [ ] 77. full이 아닌데 도착점 비트가 있는 mask를 건너뛰어도 되는 이유를 중복 방문 금지로 증명하라.
- [ ] 78. full mask에서 마지막 도시가 n번이 아닌 상태를 계산하지 않아도 되는 이유는 무엇인가?
- [ ] 79. mask를 오름차순으로 순회할 때 previous가 이미 계산됐음을 정수 크기로 증명하라.
- [ ] 80. 들어오는 간선 목록을 쓰면 전이 시간이 왜 `O(2^n(n+m))` 상한이 되는가?
- [ ] 81. 조밀 그래프에서 위 시간이 `O(n^2 2^n)`이 되는 이유를 말하라.
- [ ] 82. `n*2^n`개의 int가 n=20에서 약 80MiB임을 직접 계산하라.
- [ ] 83. `Mask{1} << city_count`에서 왼쪽 타입과 shift count 전제를 말하라.
- [ ] 84. `ways`와 addend가 `[0,MOD)`일 때 한 번 더한 값이 signed int 범위 안임을 수치로 증명하라.
- [ ] 85. modulus를 매 predecessor 뒤 적용하지 않으면 어떤 종류의 미정의 동작 위험이 생기는가?
- [ ] 86. TSP 최소 비용 문제로 바꿀 때 상태의 값, 기저, 결합 연산과 도달 불가 표식을 어떻게 바꿀 것인가?

## G. 직접 실행 검증

- [ ] 87. `daily_main` 출력 `config.internal 42 1 1`의 네 값과 두 reader의 수명 순서를 실행 전에 예측했는가?
- [ ] 88. `daily_problem` 출력 `1 73 1`이 source 1회와 같은 Report reference를 뜻함을 설명했는가?
- [ ] 89. 공식·최소·도달 불가·병렬 간선·완전 방향 그래프 CTest를 모두 통과했는가?
- [ ] 90. 작은 무작위 multigraph에서 순열 완전탐색과 프로그램 답을 대조했는가?
- [ ] 91. `n=20` 조밀 graph stress에서 실행 시간·메모리·modulus 범위를 확인했는가?
- [ ] 92. 공용 알고리즘 예제를 높은 경고로 컴파일하고 공식 예제 출력 2를 확인했는가?
- [ ] 93. Mermaid, 로컬 링크, strict UTF-8, `-Scope latest/all` 표준 문서 감사를 모두 통과했는가?

## 통과 기준

1. 93개 항목을 자료 없이 설명한다.
2. 모든 실제 표준 호출에서 여섯 요소(수신 객체 상태, 시그니처/overload/템플릿 인자, 각 매개변수, 반환값과 사용, 호출 뒤 상태, 복잡도·할당·무효화·수명·오류·스레드)를 빠짐없이 말한다.
3. CTest, 무작위 독립 oracle, 최대 크기 stress, 공용 문서 예제와 전체 표준 라이브러리 감사를 통과한다.
4. `main.cpp`의 happens-before·비소유 수명과 `icpc_problem.cpp`의 마지막 간선 일대일 대응을 실제 식으로 증명한다.
