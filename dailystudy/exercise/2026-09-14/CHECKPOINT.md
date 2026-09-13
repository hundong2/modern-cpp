# 2026-09-14 CHECKPOINT — `std::ospanstream`과 potential 기반 최소 비용 유량

체크 표시는 답을 소리 내어 설명하고 실제 식의 호출 전후 상태와 소유 관계를 종이에 그린 뒤에만 한다. 표준 라이브러리 호출 문제는 매번 다음 여섯 묶음을 빠짐없이 답해야 통과다.

1. 수신 객체의 정확한 타입·값 범주와 호출 전 size/capacity/stream/소유 상태
2. 선택된 signature·overload·템플릿 인자와 숨은 `this`
3. 각 인자 식의 타입·값 범주·허용 범위, 복사/이동/차용과 소유권 의미
4. 반환형·반환값 의미와 호출부가 저장·연쇄·검사·무시하는지
5. 호출 뒤 수신 객체·인자·backing storage의 상태 변화
6. 전제·후조건, 복잡도, 할당, 무효화, 수명, 오류·예외·UB와 스레드 보장

## A. 기초 문법과 타입

- [ ] 1. `<array>`, `<cstddef>`, `<iostream>`, `<optional>`, `<span>`, `<spanstream>`, `<string_view>`, `<utility>`가 `main.cpp`에서 각각 어떤 선언을 직접 제공하는가?
- [ ] 2. `<cstdint>`, `<limits>`, `<queue>`, `<vector>`가 `icpc_problem.cpp`에서 각각 어떤 선언을 직접 제공하는가?
- [ ] 3. `int`, `bool`, `std::size_t`, `std::int64_t`가 오늘 코드의 어떤 값에 쓰이며 signed/unsigned·폭 차이는 무엇인가?
- [ ] 4. `int sequence{};`, `bool allowed{};`, `std::string_view actor{};`, `std::array<char, Capacity> buffer_{};`의 초기값을 말하라.
- [ ] 5. 중괄호 초기화가 narrowing을 막는 예를 들고, 초기화하지 않은 지역 기본 타입을 읽을 때의 문제를 설명하라.
- [ ] 6. `const AuditEvent event`, `const AuditEvent& event`, `const std::span<char> storage`, `const char* const first`에서 각각 무엇이 const인지 구별하라.
- [ ] 7. `const std::span<char>`인데도 `ospanstream`이 가리키는 char를 쓸 수 있는 이유를 span 객체의 const와 element type으로 설명하라.
- [ ] 8. `const AuditEvent&`가 복사·소유·수명 연장 중 무엇을 하는지, null 참조 상태가 가능한지 말하라.
- [ ] 9. `const char* first`가 `delete` 책임을 받는지, `[first, first+count)`의 유효성은 누가 보장하는지 설명하라.
- [ ] 10. `struct AuditEvent`/`MetricSample`/`Edge`와 `class AuditLineEncoder`/`MetricLineFormatter`/`MinCostFlow`의 기본 접근 차이를 말하라.
- [ ] 11. `public encode`와 `private buffer_` 분리가 어떤 불변식과 수명 규칙을 보호하는가?
- [ ] 12. `using Result = std::optional<std::string_view>;`, `using Cost = std::int64_t;`, `using Graph = std::vector<Adjacency>;`가 새 타입인지 별칭인지 답하라.
- [ ] 13. `template <std::size_t Capacity>`의 인자 종류와 `AuditLineEncoder<48>`·`MetricLineFormatter<32>`가 서로 다른 specialization인 이유를 설명하라.
- [ ] 14. `std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryGreater>`의 세 템플릿 인자 역할을 말하라.
- [ ] 15. `explicit AuditLineEncoder(std::string_view channel) noexcept : channel_{channel} {}`에서 반환형, `explicit`, `noexcept`, 멤버 초기화 목록을 차례로 설명하라.
- [ ] 16. `explicit MinCostFlow(const int vertex_count) : graph_(size_t(vertex_count)) {}`가 본문 대입보다 직접 구성인 이유를 설명하라.
- [ ] 17. `[[nodiscard]] Result encode(const AuditEvent& event)`와 `[[nodiscard]] Cost send(int,int,int)`에서 attribute, 반환형, 매개변수를 읽어라.
- [ ] 18. `if`, `for`, `while`, `continue`, `break`, `return`이 오늘 코드에서 각각 어느 분기·반복 종료를 표현하는지 한 사례씩 찾아라.

## B. 값 범주, 참조 바인딩, 복사·이동·소유권과 수명

- [ ] 19. 이름 있는 `encoder`, `event`, `encoded`, `storage`, `output`, `written` 식의 값 범주를 말하라.
- [ ] 20. `encoder.encode(event)`, `output.span()`, `Result{...}`, `QueueEntry{...}`의 값 범주를 말하라.
- [ ] 21. `std::move(encoded)`의 정확한 역할과 반환 값 범주를 말하고, 이 함수 자체가 buffer 문자를 옮기지 않는 이유를 설명하라.
- [ ] 22. `std::move(encoded).value()`가 `value() &&`를 고르는 이유와 반환형 `std::string_view&&`의 의미를 설명하라.
- [ ] 23. 위 식 뒤 `encoded`가 engaged이고 내부 view의 주소·길이가 그대로인 이유를 `string_view`의 defaulted 복사 생성자로 설명하라.
- [ ] 24. `published`로 복사되는 것이 문자 소유권이 아니라 주소·길이 view 값이라는 것을 owner 그림으로 보이라.
- [ ] 25. `return Result{std::string_view{first,count}};`에서 Result prvalue가 호출자의 `encoded`를 초기화할 때 보장 복사 생략과 NRVO를 구별하라.
- [ ] 26. `const auto line{formatter.format(sample)};`의 `auto`가 reference가 아니라 optional 값인 이유를 설명하라.
- [ ] 27. `std::array<char,48> -> std::span<char> -> std::ospanstream -> std::span<char> -> std::string_view`의 소유/차용 화살표를 그려라.
- [ ] 28. `output`이 함수 끝에서 파괴되어도 반환 string_view가 즉시 dangling이 아닌 이유와 실제 owner를 말하라.
- [ ] 29. encoder/formatter가 파괴된 뒤 view를 읽으면 왜 UB인지 설명하라.
- [ ] 30. 같은 encoder의 두 번째 `encode` 뒤 첫 view의 주소와 내용에 각각 어떤 일이 생길 수 있는가?
- [ ] 31. `channel_`에 문자열 리터럴 `"gateway"`를 저장하는 경우와 지역 `std::string`에서 만든 view를 저장하는 경우의 수명을 비교하라.
- [ ] 32. `event.actor`의 `"kim"`이 안전한 근거와, 함수 인자로 받은 임시 소유 문자열의 문자를 장기 보관하면 안 되는 이유를 말하라.
- [ ] 33. 반환형을 `std::string`으로 바꿀 때 얻게 되는 독립 수명과 잃게 되는 고정 버퍼/할당 특성을 비교하라.
- [ ] 34. `const Edge& edge{graph_[vertex][edge_index]};`가 Dijkstra 순회 동안 유효한 이유를 간선 추가 시점과 vector 재할당으로 설명하라.
- [ ] 35. `Edge& edge`를 얻은 뒤 vector에 `push_back`한다면 어떤 참조 무효화 위험이 생기는가?
- [ ] 36. `Edge{...}`와 `QueueEntry{...}` prvalue를 vector/heap이 어떤 저장소에 받아 소유하는지 말하라.
- [ ] 37. `Cost` 반환은 스칼라 prvalue라 큰 객체 RVO 논의와 어떤 차이가 있는가?
- [ ] 38. `MinCostFlow`가 `Graph graph_`를 값으로 소유해 소멸 시 모든 중첩 vector를 자동 해제하는 RAII 흐름을 설명하라.

## C. 실제 코드 식 해석

- [ ] 39. `AuditLineEncoder<48> encoder{"gateway"};`에서 literal 배열, string_view 변환, encoder 생성과 channel_ 초기화를 순서대로 적어라.
- [ ] 40. `const AuditEvent event{17, "kim", true};`가 세 public 멤버를 어느 선언 순서로 초기화하는가?
- [ ] 41. `const std::span<char> storage{buffer_};`가 선택하는 array 변환 생성자와 static/dynamic extent를 설명하라.
- [ ] 42. `std::ospanstream output{storage};`가 span 값을 복사하지만 문자를 복사하지 않는다는 뜻을 말하라.
- [ ] 43. `output << channel_ << '|' << event.sequence << ...`의 각 피연산자 타입·값 범주와 반환 stream reference 연쇄를 적어라.
- [ ] 44. `event.allowed ? "ALLOW" : "DENY"`의 조건 평가와 결과 타입·수명을 설명하라.
- [ ] 45. `if (!output) return std::nullopt;`에서 explicit bool 변환, 논리 부정, 반환 Result 구성을 해석하라.
- [ ] 46. `const std::span<char> written{output.span()};`에서 전체 capacity와 기록 길이가 다른 이유를 말하라.
- [ ] 47. `const char* const first{written.data()};`와 `const std::size_t count{written.size()};`의 반환값과 용도를 설명하라.
- [ ] 48. `std::string_view{first,count}`가 null terminator를 찾지 않는 이유와 유효 범위 전제를 말하라.
- [ ] 49. `if (!encoded) return 1;`이 뒤의 `value()` 예외 조건을 어떻게 제거하는가?
- [ ] 50. `const std::string_view published{std::move(encoded).value()};`의 lvalue→xvalue→rvalue reference→defaulted 복사 생성 흐름을 그려라.
- [ ] 51. `std::cout << published << '\n';`이 `published.data()`를 C string처럼 읽는지, 길이를 사용하는지 답하라.
- [ ] 52. `return published == "gateway|17|kim|ALLOW" ? 0 : 2;`의 비교, 조건 연산자와 종료 코드 의미를 설명하라.
- [ ] 53. `MetricLineFormatter<8> tiny{};`와 `tiny.format(sample)`가 실패할 때 부분 buffer와 `rejected` 상태를 말하라.
- [ ] 54. `return *line == "latency_ms=37" && !rejected ? 0 : 2;`의 단락 평가와 `*line` 전제조건을 설명하라.
- [ ] 55. `graph_[size_t(from)].push_back(Edge{to,...})`에서 두 `operator[]`, prvalue 생성과 push 소유를 순서대로 읽어라.
- [ ] 56. `network.add_edge(from-1,to-1,...)`에서 1-based→0-based 변환이 한 번만 일어나는 이유와 범위 전제를 말하라.

## D. 표준 라이브러리 호출 계약

각 항목에 대해 문서 첫머리의 여섯 묶음을 모두 답한다.

- [ ] 57. `std::array<char, Capacity> buffer_{};`의 수신 전 상태, 생성, 반환 없음, 모든 원소 초기값, 주소 안정성과 객체 수명 종료를 설명하라.
- [ ] 58. `std::span<char>{buffer_}`의 선택 template 생성자, array lvalue 인자, 반환 없음, pointer/size 상태, O(1)·무소유·수명 계약을 설명하라.
- [ ] 59. `std::ospanstream output{storage}`의 정확한 생성 signature, 기본 openmode, put position, backing span 수명, 버퍼 비확장과 오류 가능성을 설명하라.
- [ ] 60. `ospanstream`에 대한 string_view/char/int/const-char-pointer `operator<<` overload를 구별하고 각 반환 `ostream&`의 사용 여부를 말하라.
- [ ] 61. 고정 용량을 넘긴 삽입 뒤 stream 상태, 부분 기록, 예외 mask에 따른 `ios_base::failure`, 외부 공개 여부를 설명하라.
- [ ] 62. `basic_ios::operator bool`의 반환 의미, `!output` 결과, 객체 변화·복잡도·동시 접근 조건을 말하라.
- [ ] 63. `output.span()`의 `const noexcept` signature, 반환 범위 의미, 수신/버퍼 사후 상태, 무효화·수명 조건을 말하라.
- [ ] 64. `written.data()`와 `written.size()`의 반환형, 빈 범위, 사용 여부, O(1)·noexcept와 owner 의존성을 설명하라.
- [ ] 65. `std::string_view(first,count)`의 두 인자 타입·값 범주, 유효 범위 전제, 반환 없음, O(1)·무할당과 dangling 조건을 말하라.
- [ ] 66. `string_view operator==`의 두 범위, bool 반환 사용, 최악 O(n), 무변경과 dangling 시 UB를 설명하라.
- [ ] 67. `Result{view}`와 `return std::nullopt`의 engaged/disengaged 상태, contained value 소유와 문자 비소유를 비교하라.
- [ ] 68. `optional::operator bool`, `operator*() const&`, `value() &&`의 반환·빈 상태 동작을 비교하라. 특히 `*`는 UB, `value`는 `bad_optional_access`임을 구별하라.
- [ ] 69. `std::move(encoded)`의 template 인자 추론, 인자 값 범주, 반환형, 사후 상태와 무할당 계약을 설명하라.
- [ ] 70. `std::vector<Adjacency>(vertex_count)`, `vector<Cost>(V,0)`, `vector<int>(V,-1)` 생성자의 서로 다른 overload와 O(V)·할당·예외를 설명하라.
- [ ] 71. `Adjacency::push_back(Edge&&)`의 수신 size/capacity, prvalue 인자, void 반환, 상각 O(1), 재할당·예외·참조 무효화를 설명하라.
- [ ] 72. `graph_.size()`, `graph_[v].size()`와 중첩 `operator[]`의 const/non-const 반환, 범위 밖 UB, 수정 중 수명을 설명하라.
- [ ] 73. `MinQueue queue{}`, `queue.push(QueueEntry{...})`, `queue.empty()`, `queue.top()`, `queue.pop()` 각각의 인자·반환·전제·복잡도·무효화를 설명하라.
- [ ] 74. `std::numeric_limits<Cost>::max()`의 template specialization, 반환값 사용, `noexcept`와 `max()/4` sentinel 이유를 설명하라.
- [ ] 75. `std::ios::sync_with_stdio(false)`, `std::cin.tie(nullptr)`, 정수 `operator>>`, Cost/char `operator<<`의 반환값 사용·상태 변화·오류·동시성 계약을 설명하라.

## E. 최소 비용 유량 불변식과 증명

- [ ] 76. 정방향 `Edge{to,reverse,capacity,cost}`와 역방향 `Edge{from,reverse,0,-cost}`가 서로를 가리키는 index 불변식을 적어라.
- [ ] 77. 정방향으로 `pushed`를 보낼 때 두 capacity 갱신이 유량 취소 가능성과 보존을 유지함을 보이라.
- [ ] 78. self-loop를 버려도 양수 비용 최소해가 바뀌지 않는 이유와, 같은 adjacency에 두 간선을 넣을 때 피하는 index 함정을 설명하라.
- [ ] 79. 평행 간선을 합치지 않고 독립 edge index로 보존해야 하는 경우를 용량·단가로 설명하라.
- [ ] 80. 최초 potential을 모두 0으로 둘 수 있는 근거를 원래 간선 비용 제약과 reduced cost로 설명하라.
- [ ] 81. `reduced(u,v)=cost(u,v)+p[u]-p[v]`를 쓰고 한 source-sink 경로 합에서 potential 항이 어떻게 상쇄되는지 전개하라.
- [ ] 82. Dijkstra 뒤 `p'[v]=p[v]+dist[v]`일 때 새 reduced cost가 `old_reduced+dist[u]-dist[v] >= 0`임을 삼각부등식으로 증명하라.
- [ ] 83. 선택한 최단경로 edge의 새 역간선 reduced cost가 0이 되는 이유를 계산하라.
- [ ] 84. reduced cost 비음수 불변식이 음수 원래 비용 역간선이 있어도 Dijkstra를 가능하게 하는 이유를 말하라.
- [ ] 85. heap에 같은 정점 후보를 여러 번 넣고 `current.distance != distance[v]`로 stale entry를 버리는 방식이 decrease-key를 대체함을 설명하라.
- [ ] 86. sink가 infinity일 때 더 큰 flow도 만들 수 없다는 것을 잔여 경로와 max-flow 관점에서 설명하라.
- [ ] 87. 부모 정점·edge 배열로 경로를 복원하고 `pushed=min(k-sent, bottleneck)`을 정하는 과정을 적어라.
- [ ] 88. 최소 비용인 f-flow에 최소 비용 잔여 증대 경로를 더하면 새 flow도 최소 비용이라는 교환/잔여망 논거를 설명하라.
- [ ] 89. 각 증대가 적어도 1을 보내 최대 k번이고, lazy heap의 크기가 `O(E)`까지 늘 수 있음을 이용해 전체 `O(k(V + E log(E+1)))`를 유도하라.
- [ ] 90. graph 정·역간선, 거리·potential·부모 배열, heap을 합쳐 `O(V+E)` 공간임을 유도하라.
- [ ] 91. `infinity=max()/4`, `current.distance > infinity-reduced_cost` 검사, Cost 곱셈이 signed overflow UB를 막는 방식을 설명하라.

## F. 손 추적과 실행 가능한 실기 검증

- [ ] 92. 공식 예제에서 `1->2->4` 한 개 비용 450과 `1->3->4` 두 개 비용 300을 계산해 출력 750을 검산하라.
- [ ] 93. direct-capacity 입력에서 단가 3인 간선으로 2개, 단가 5인 간선으로 3개를 보내 `2*3+3*5=21`을 검산하라.
- [ ] 94. impossible 입력에서 총 source-sink 용량이 1인데 k가 2여서 두 번째 Dijkstra가 sink에 도달하지 못하고 `-1`이 되는 시점을 추적하라.
- [ ] 95. reverse-residual 입력에서 첫 비용 3 경로, 두 번째 역간선 포함 증대 경로, 최종 두 실제 경로와 총비용 8을 그림으로 검산하라.
- [ ] 96. `AuditLineEncoder<48>`과 `<12>`, `MetricLineFormatter<32>`와 `<8>`을 실행해 성공 문자열·실패 optional·부분 buffer 비공개를 확인하라.
- [ ] 97. 같은 formatter를 두 번 호출해 첫 view 내용이 덮이는 것을 확인하고, 반환을 소유 string으로 바꾼 버전과 비교하라.
- [ ] 98. 작은 무작위 방향 multigraph를 만들어 Bellman-Ford 기반 successive shortest path 또는 모든 정수 유량 열거 oracle과 답을 대조하라. 평행 간선·cycle·불가능·역간선 재배치를 포함하라.
- [ ] 99. w64devkit GCC 16.1.0 C++23 높은 경고 빌드와 CTest 6/6을 통과하고 `n=500,m=1000,k=100` stress 및 알고리즘 문서 예제를 실행하라.
- [ ] 100. `-Scope latest`/`-Scope all` 감사, README Mermaid, 모든 로컬 링크, strict UTF-8, `git diff --check`를 통과하고 build 산출물이 stage되지 않았음을 확인하라.

## 답 확인 기준

- 기초·값 범주 답은 “복사한다/참조한다”에서 멈추지 않고 **누가 실제 문자를 소유하는지, 어떤 식이 lvalue/prvalue/xvalue인지, 언제 view가 dangling 또는 내용 변경 상태가 되는지**까지 맞아야 한다.
- 호출 계약 답은 각 문항마다 맨 위의 여섯 묶음을 모두 포함하고, `optional::operator*`의 빈 상태 UB와 `value()`의 예외, `priority_queue::top/pop`의 비어 있지 않음 전제를 구별해야 한다.
- 알고리즘 답은 역간선의 “취소” 의미, reduced-cost 비음수 증명, potential 망원경 상쇄, `sent==k`/불가능 판정을 모두 설명해야 한다.
- 실행 답은 stdout이 정확히 `gateway|17|kim|ALLOW`, `latency_ms=37`, `750`, `21`, `-1`, `8`이고 CTest가 **6/6**이어야 한다.
- 마지막으로 자료를 보지 않고 아래 문장을 완성한다.

> 실제 문자는 ______가 소유한다. `span`, `ospanstream`, `string_view`는 이를 ______하며 원본 수명을 ______ 않는다. 이름 있는 `encoded`는 ______이고 `std::move(encoded)`는 ______, `value() &&` 결과는 ______다. 최소 비용 유량은 ______ 간선으로 과거 선택을 취소하고, ______가 모든 잔여 reduced cost를 비음수로 유지해 ______를 반복하므로 시간 ______, 공간 ______이다.
