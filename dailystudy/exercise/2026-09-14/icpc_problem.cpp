/*
문제 ID·제목: CSES 2121 - Parcel Delivery
출처: https://cses.fi/problemset/task/2121/

문제 요약
- 1번 도시에서 n번 도시까지 소포 k개를 보내려 한다.
- 각 단방향 운송로에는 한 번에 보낼 수 있는 소포 수의 상한과, 소포 한 개당 비용이 있다.
- 모든 소포를 보낼 수 있다면 총비용의 최솟값을, 불가능하면 -1을 출력한다.

입력
- 첫 줄에 도시 수 n, 운송로 수 m, 보낼 소포 수 k가 주어진다.
- 다음 m줄의 a, b, r, c는 a에서 b로 가는 운송로의 용량이 r이고 단위 비용이 c라는 뜻이다.

출력
- k개를 모두 보낼 수 있을 때의 최소 총비용 하나를 출력한다.
- 잔여 네트워크에 1번에서 n번으로 가는 경로가 사라져 k개를 채우지 못하면 -1을 출력한다.

공식 제약
- 2 <= n <= 500, 1 <= m <= 1,000, 1 <= k <= 100
- 1 <= a,b <= n, 1 <= r,c <= 1,000
- 시간 제한 1초, 메모리 제한 512MB

공식 예제
- 입력:
  4 5 3
  1 2 5 100
  1 3 10 50
  1 4 7 500
  2 4 8 350
  3 4 2 100
- 출력: 750
- 1->2->4로 한 개를 보내면 450, 1->3->4로 두 개를 보내면 300이므로 합은 750이다.

대회에서 반드시 잡아야 할 핵심
- 원래 간선과 반대 방향의 잔여 간선을 한 쌍으로 둔다. 역간선의 음수 비용은 앞선 선택을 일부
  취소하고 더 좋은 경로로 재배치하는 비용을 표현하므로, 단순히 싼 원래 경로만 탐욕적으로 고르면 안 된다.
- 잠재치 potential을 사용한 감소 비용
      reduced(u,v) = cost(u,v) + potential[u] - potential[v]
  을 모든 시작점 도달 가능 잔여 간선에서 음이 아니게 유지하면, 음수 비용 역간선이 생긴 뒤에도
  이진 힙 Dijkstra를 안전하게 반복할 수 있다.
- Dijkstra 거리 d를 구한 뒤 도달한 v에 potential[v] += d[v]를 적용한다. 최단거리 삼각부등식으로
  새 감소 비용은 reduced(u,v)+d[u]-d[v] >= 0이고, 선택한 최단 경로의 새 역간선 감소 비용은 0이다.
- 한 번에 경로 병목만큼 보내므로 실제 반복 횟수 A는 k 이하이다. decrease-key 없이 오래된 후보를
  남기는 이 구현의 엄밀한 시간은 O(k (V + E log(E+1))), 저장 공간은 O(V+E)이다.
- 입력 비용이 양수이므로 자기 자신으로 돌아오는 간선은 흐름 보존에 아무 도움 없이 비용만 늘린다.
  이 구현은 그런 self-loop를 버린다. 덕분에 같은 adjacency vector에 정·역간선을 연속 삽입할 때 생기는
  reverse index 보정 함정도 피한다. 평행 간선은 서로 다른 edge index로 그대로 지원한다.
- 거리, 잠재치, 총비용은 std::int64_t로 계산한다. 공식 범위에서는 훨씬 작지만, 무한대 덧셈과
  일반화된 누적 비용에서 32비트 오버플로를 피하는 대회 습관이다.
*/

// <cstddef>는 vector의 크기와 0-based 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 정확히 64비트인 부호 있는 정수 std::int64_t를 선언한다.
#include <cstdint>
// <iostream>은 표준 입력 std::cin, 표준 출력 std::cout과 빠른 입출력 설정을 선언한다.
#include <iostream>
// <limits>는 덧셈 여유를 둔 무한대 표식을 만들 std::numeric_limits를 선언한다.
#include <limits>
// <queue>는 Dijkstra의 최소 거리 후보를 관리할 std::priority_queue를 선언한다.
#include <queue>
// <vector>는 인접 리스트와 거리·부모·잠재치 배열을 소유하는 std::vector를 선언한다.
#include <vector>

// using은 새 타입을 만드는 것이 아니라 긴 기존 타입에 별칭을 붙인다.
// Cost는 모든 경로 비용 계산을 정확히 64비트 부호 있는 정수로 통일한다.
using Cost = std::int64_t;

// struct와 class는 같은 사용자 정의 타입 문법을 쓰지만, struct의 기본 접근은 public이고
// class의 기본 접근은 private이다. Edge는 불변식을 숨길 필요 없는 단순 레코드라 struct가 알맞다.
struct Edge {
    int to;        // 잔여 간선이 향하는 0-based 정점 번호다.
    int reverse;   // graph[to] 안에서 짝 역간선이 놓인 index다.
    int capacity;  // 이 방향으로 지금 더 보낼 수 있는 정수 잔여 용량이다.
    Cost cost;     // 이 방향으로 소포 한 개를 더 보낼 때 총비용의 변화량이다.
};

// vector의 첫 템플릿 인자 Edge는 원소 타입을 정한다. 바깥 vector의 원소는 각 정점의 인접 리스트다.
using Adjacency = std::vector<Edge>;
using Graph = std::vector<Adjacency>;

// priority_queue에 저장하는 값이다. 이름 있는 QueueEntry 객체는 lvalue이고 QueueEntry{...}는 prvalue다.
struct QueueEntry {
    Cost distance;
    int vertex;
};

// priority_queue는 비교자가 true를 반환한 원소를 뒤로 보내므로, 더 큰 거리를 낮은 우선순위로 둔다.
// const lvalue 참조 매개변수는 두 QueueEntry를 복사하거나 소유하지 않고 함수 호출 동안만 읽는다.
struct QueueEntryGreater {
    [[nodiscard]] bool operator()(const QueueEntry& left, const QueueEntry& right) const noexcept {
        if (left.distance != right.distance) {
            return left.distance > right.distance;
        }
        // 같은 거리에서는 정점 번호로 순서를 고정한다. 정답에는 영향이 없지만 엄격 약순서를 이룬다.
        return left.vertex > right.vertex;
    }
};

// 세 템플릿 인자는 차례대로 저장 값, 내부 컨테이너, 우선순위 비교 정책을 뜻한다.
using MinQueue = std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryGreater>;

/*
값 범주·복사·이동·수명과 실행 관점
- graph_, potential, distance처럼 이름이 있는 객체는 lvalue다. Edge{...}, QueueEntry{...}, Cost{0}은
  prvalue이고 각 vector/priority_queue가 자기 저장소 안의 새 객체로 받아 소유한다.
- 이 코드는 std::move를 명시적으로 쓸 필요가 없다. Edge와 QueueEntry는 작은 스칼라 묶음이며,
  컨테이너가 재할당할 때 내부 원소를 복사 또는 이동해도 외부에는 edge index만 저장한다.
- Dijkstra 순회 중에는 graph_에 간선을 추가하지 않는다. 따라서 순회가 빌린 const Edge&와 갱신 단계의
  Edge&는 해당 반복 동안 유효하다. add_edge가 끝난 뒤에만 그런 참조를 얻는 수명 규칙이 중요하다.
- send의 Cost 반환식은 스칼라 prvalue다. 큰 객체 반환/RVO에 의존하지 않는다. 일반적인 class prvalue
  반환이라면 C++17 이후 결과 객체에 직접 생성될 수 있지만 이 함수에는 그런 복사 자체가 없다.
- 개념적으로 간선 탐색은 Edge 필드의 load, distance/potential의 비교와 load, 완화 성공 시 store,
  capacity==0·stale entry 판정의 조건 분기를 만든다. priority_queue 연산은 비교와 메모리 접근을
  수행하지만 실제 명령과 배치는 CPU, ABI, 컴파일러, 최적화 옵션에 따라 달라진다.
*/

// 구현 가까운 공용 알고리즘 문서: ../algorithm/min-cost-flow-potentials.md
class MinCostFlow {
public:
    // public은 main이 사용할 생성자·간선 추가·전송 API를 공개한다. graph_는 private라 외부에서
    // 정방향/역방향 짝 불변식을 깨뜨릴 수 없다.
    //
    // explicit은 int 하나가 실수로 MinCostFlow 객체로 암시 변환되는 것을 막는다.
    // 멤버 초기화 목록은 생성자 본문보다 먼저 graph_를 정확한 정점 수로 직접 구성한다.
    //
    // [생성 계약: std::vector<Adjacency>::vector(size_type)]
    // (1) 수신 객체 graph_는 아직 수명이 시작되지 않았고, vertex_count는 공식 범위 [2,500]의 const int다.
    // (2) explicit vector(size_type count, const Allocator& = Allocator()) overload를 선택하며
    //     바깥 value_type은 Adjacency=std::vector<Edge>, size_type은 std::size_t다.
    // (3) static_cast가 만든 std::size_t prvalue를 개수로 전달한다. 외부 버퍼나 원소 소유권은 받지 않는다.
    // (4) 생성자는 반환값이 없고, 성공하면 graph_가 vertex_count개의 독립적인 빈 Adjacency를 소유한다.
    // (5) 각 안쪽 vector는 default-insert되어 빈 상태가 되며 바깥 size만 vertex_count가 된다.
    // (6) O(V) 구성이고 바깥 저장소를 한 번 할당할 수 있다. 너무 크면 length_error, 메모리 부족이면
    //     bad_alloc이 가능하며 실패 시 이미 만든 원소는 정리된다. 기존 관찰자는 없고 객체는 단일 스레드 소유다.
    explicit MinCostFlow(const int vertex_count)
        : graph_(static_cast<std::size_t>(vertex_count)) {}

    // 반환형 void는 결과 객체가 없다는 뜻이다. 네 매개변수는 모두 작은 정수 값 복사이며 소유권 이동이 없다.
    void add_edge(const int from, const int to, const int capacity, const Cost cost) {
        // 공식 비용은 양수다. self-loop의 유입·유출은 같은 정점에서 상쇄되고 비용만 더하므로 최적해에
        // 필요하지 않다. from==to를 먼저 버려 아래 두 adjacency가 서로 다르다는 전제를 확립한다.
        if (from == to) {
            return;
        }

        // [호출 계약: Graph::operator[]와 Adjacency::size]
        // (1) 수신 graph_의 정확한 타입은 std::vector<std::vector<Edge>>인 비-const lvalue이고
        //     size는 V다. from/to는 입력 검증이 보장한 [0,V)이며 두 값은 서로 다르다.
        // (2) 바깥 호출은 reference operator[](size_type position), 이어지는 호출은
        //     size_type std::vector<Edge>::size() const noexcept overload다.
        // (3) to/from을 std::size_t prvalue로 변환해 위치 값으로 주며, 컨테이너 소유권은 옮기지 않는다.
        // (4) operator[]는 각 Adjacency lvalue 참조를 반환하고 size는 현재 edge 개수 prvalue를 반환한다.
        //     두 size 결과는 새 간선이 상대 목록에서 가리킬 reverse index로 사용한다.
        // (5) 두 호출은 graph_와 안쪽 vector를 변경하지 않으며 참조를 저장하지 않는다.
        // (6) 모두 O(1), 무할당·비무효화다. operator[]는 범위 검사를 하지 않아 잘못된 index면 UB지만
        //     공식 입력과 1-based 변환이 전제조건을 보장한다. edge 수는 int 범위이며 동시 접근은 없다.
        const int forward_reverse{
            static_cast<int>(graph_[static_cast<std::size_t>(to)].size())};
        const int backward_reverse{
            static_cast<int>(graph_[static_cast<std::size_t>(from)].size())};

        // [호출 계약: std::vector<Edge>::push_back(Edge&&)]
        // (1) 수신자는 서로 다른 from/to의 유효한 Adjacency lvalue 두 개이고 각 vector에는 기존 잔여
        //     간선들이 있다. Edge prvalue의 네 필드는 유효하며 capacity>0, cost>0이다.
        // (2) 각 호출은 void push_back(value_type&& value)를 선택하고 value_type은 Edge다.
        // (3) Edge{...} prvalue가 임시 객체로 구체화되어 Edge&& 매개변수에 바인딩되고,
        //     vector가 그 스칼라 필드를 새 원소로 이동한다.
        //     첫 원소는 주어진 용량/비용, 둘째는 용량 0/반대 비용이며 외부 자원 소유권은 없다.
        // (4) 반환형 void라 반환값은 없다. 계산해 둔 index로 두 새 원소가 서로를 가리키게 된다.
        // (5) 각 수신 vector의 size가 1 증가한다. 재할당이 있으면 그 vector의 기존 포인터·참조·반복자는
        //     무효화되지만 코드는 index만 보존하며, 두 adjacency가 달라 서로의 삽입 index에 영향이 없다.
        // (6) 각 호출은 상각 O(1), 재할당이 필요한 단일 호출은 해당 size에 선형이며
        //     bad_alloc/length_error가 가능하다. Edge 이동은 noexcept인 스칼라 복사다. 첫 호출 실패는
        //     vector의 강한 보장으로 변화가 없고, 둘째 호출 실패는 아래 catch가 첫 삽입을 롤백한다.
        //     따라서 add_edge도 예외 전 graph 내용과 짝 불변식을 복원하며 단일 스레드에서만 수정한다.
        graph_[static_cast<std::size_t>(from)].push_back(
            Edge{to, forward_reverse, capacity, cost});
        try {
            graph_[static_cast<std::size_t>(to)].push_back(
                Edge{from, backward_reverse, 0, -cost});
        } catch (...) {
            // [호출 계약: std::vector<Edge>::pop_back()]
            // (1) 수신 graph_[from]은 첫 push_back이 성공해 마지막에 방금 만든 forward Edge가 있는
            //     비어 있지 않은 Adjacency lvalue다. 둘째 adjacency 삽입은 실패해 짝 Edge가 없다.
            // (2) constexpr void pop_back() overload가 선택되고 명시 인자는 없다.
            // (3) 숨은 비-const this로 마지막 원소를 제거하며 외부 값이나 소유권 인자는 없다.
            // (4) 반환형 void라 제거한 Edge를 돌려주지 않으며, 목적은 예외 전 상태 복원이다.
            // (5) size가 1 감소하고 방금 forward 원소의 수명이 끝난다. 그 원소와 과거 past-the-end
            //     관찰자는 무효지만 기존 앞 원소와 capacity는 유지되어 정·역간선 짝 불변식이 돌아온다.
            // (6) O(1), 무할당이다. 빈 vector 호출은 UB지만 첫 push 성공이 전제를 보장한다.
            //     Edge 소멸자는 noexcept인 trivial 소멸자라 롤백 중 새 예외가 없고 원래 예외를 다시 던진다.
            graph_[static_cast<std::size_t>(from)].pop_back();
            throw;
        }
    }

    // source에서 sink로 required_flow만큼 보낸 최소비용을 반환하고 불가능하면 -1을 반환한다.
    // 세 int 매개변수는 값으로 받아 호출자의 변수를 변경하지 않는다.
    [[nodiscard]] Cost send(
        const int source,
        const int sink,
        const int required_flow) {
        // [호출 계약: std::numeric_limits<Cost>::max()]
        // (1) 수신 객체 없는 타입 특성 호출이며 Cost=std::int64_t는 유효한 정수 특수화다.
        // (2) static constexpr Cost max() noexcept가 선택되고 명시 인자와 템플릿 추론은 없다.
        // (3) 매개변수·소유권 입력이 없다.
        // (4) 표현 가능한 최대 Cost prvalue를 반환해 4로 나눈 값을 무한대 sentinel로 사용한다.
        // (5) 프로그램이나 타입의 상태는 변하지 않는다.
        // (6) 컴파일 시간 상수인 O(1), 무할당·비무효화·noexcept이며 오류 상태를 만들지 않는다.
        //     4로 나눈 여유는 유한 거리와 감소 비용을 더할 때 signed overflow를 피하도록 돕는다.
        constexpr Cost infinity{std::numeric_limits<Cost>::max() / 4};

        // [호출 계약: Graph::size()]
        // (1) 수신 graph_는 V개의 Adjacency와 모든 잔여 간선을 소유하는 유효한 const 관찰 상태다.
        // (2) size_type std::vector<Adjacency>::size() const noexcept가 선택되고 명시 인자는 없다.
        // (3) 숨은 const this만 읽으며 소유권을 빌리거나 옮기는 별도 인자는 없다.
        // (4) V를 std::size_t prvalue로 반환해 보조 배열 크기에 사용한다.
        // (5) graph_의 size/capacity/원소 및 모든 관찰자는 그대로다.
        // (6) O(1), 무할당·비무효화·noexcept다. 다른 스레드가 graph_를 동시에 수정하지 않는다.
        const std::size_t vertex_count{graph_.size()};

        // [생성 계약: std::vector<Cost>::vector(size_type, const Cost&)]
        // (1) potential은 아직 존재하지 않고 vertex_count는 graph_와 같은 V, Cost{0}은 유효한 prvalue다.
        // (2) vector(size_type count, const value_type& value, const Allocator& = Allocator()) overload이며
        //     value_type은 Cost=std::int64_t다.
        // (3) count는 std::size_t 값 복사, 0 prvalue는 const Cost&에 바인딩되어 V번 복사된다. 소유권 입력은 없다.
        // (4) 생성자는 반환값이 없고 성공 시 potential이 V개의 0을 소유한다.
        // (5) size==V이며 각 정점의 초기 잠재치가 0이다. 원래 양수 비용 때문에 이는 feasible potential이다.
        // (6) O(V), 한 번 할당할 수 있고 length_error/bad_alloc 시 구성이 실패하며 부분 자원은 정리된다.
        //     새 vector라 무효화할 관찰자는 없고 함수 호출 스레드만 접근한다.
        std::vector<Cost> potential(vertex_count, Cost{0});

        int sent_flow{0};
        Cost total_cost{0};

        // while은 아직 보내야 할 정수 흐름이 있을 때만 새 최단 증가 경로를 찾는다.
        while (sent_flow < required_flow) {
            // distance는 이번 잔여 그래프의 감소 비용 최단거리, 두 parent 배열은 그 최단경로 트리의
            // 직전 정점과 edge index다. 매 반복 새 vector로 만들면 이전 parent가 섞이지 않는다.
            std::vector<Cost> distance(vertex_count, infinity);

            // [생성 계약: std::vector<int>::vector(size_type, const int&)]
            // (1) parent_vertex/parent_edge는 아직 없고 vertex_count는 V, -1은 아직 부모 없음 sentinel이다.
            // (2) 두 호출 모두 vector(size_type, const value_type&, const Allocator&) fill 생성자이며
            //     value_type은 int다.
            // (3) V는 std::size_t 값 복사이고 -1 prvalue는 const int&에 바인딩돼 각 원소로 복사된다.
            // (4) 생성자는 값을 반환하지 않으며 두 vector가 각각 V개의 -1을 소유한다.
            // (5) 두 size는 V이고 이후 완화된 정점만 유효한 0-based 부모 값으로 바뀐다.
            // (6) 각각 O(V), 할당 가능하며 length_error/bad_alloc이면 예외가 전파되고 자동 정리된다.
            //     새 객체라 기존 관찰자 무효화는 없고 단일 스레드 지역 객체다.
            std::vector<int> parent_vertex(vertex_count, -1);
            std::vector<int> parent_edge(vertex_count, -1);

            // [생성 계약: std::priority_queue 기본 생성자]
            // (1) queue는 아직 없고 MinQueue의 Container=std::vector<QueueEntry>,
            //     Compare=QueueEntryGreater는 모두 기본 생성 가능하다.
            // (2) priority_queue() 기본 생성자가 빈 내부 Container와 Compare를 값 초기화한다.
            // (3) 명시 인자가 없고 외부 원소·버퍼·비교자 소유권을 받지 않는다.
            // (4) 생성자는 반환값이 없고 성공하면 queue.empty()==true인 최소 거리 어댑터가 된다.
            // (5) 내부 vector size는 0이며 comparator 상태도 비어 있다.
            // (6) 통상 O(1), 빈 vector는 할당하지 않는다. 하위 생성자의 예외가 전파될 수 있으나 여기의
            //     comparator는 trivial하다. 공개 반복자가 없고 지역 queue는 단일 스레드에서만 쓴다.
            MinQueue queue{};

            // [호출 계약: std::vector<Cost>::operator[] 쓰기]
            // (1) 수신 distance는 V개의 infinity를 소유하는 비-const vector<Cost> lvalue이고
            //     source는 [0,V)의 유효 index다.
            // (2) reference operator[](size_type position) overload가 선택된다.
            // (3) source를 std::size_t prvalue로 바꾸어 전달하며 소유권은 없다.
            // (4) Cost&를 반환해 0을 저장하는 데 사용한다.
            // (5) source 원소만 0이 되고 size/capacity 및 다른 관찰자는 그대로다.
            // (6) O(1), 무할당·비무효화다. 범위 밖이면 UB지만 공개 함수 전제가 막는다.
            //     같은 vector에 동시 접근하는 다른 스레드는 없다.
            distance[static_cast<std::size_t>(source)] = Cost{0};

            // [호출 계약: std::priority_queue<QueueEntry,...>::push(value_type&&)]
            // (1) 수신 queue는 유효한 빈 MinQueue lvalue이고 QueueEntry{0,source}는 완전한 prvalue다.
            // (2) void push(value_type&& value) overload가 선택되며 value_type은 QueueEntry다.
            // (3) prvalue가 임시 객체로 구체화되어 QueueEntry&& 매개변수에 바인딩되고,
            //     내부 vector가 그 값을 소유한다. 외부 자원은 없다.
            // (4) 반환형 void라 값은 없고 이후 top/empty로 결과를 관찰한다.
            // (5) size가 1이 되고 top은 시작점 거리 0이다. 내부 재할당은 공개 참조가 없어 외부 영향이 없다.
            // (6) heap 재정렬은 O(log size)지만 내부 vector 재할당이 필요한 단일 호출은 O(size)일 수
            //     있고, 연속 push 전체에 대해서는 상각 O(log size)다. bad_alloc/length_error가 가능하다.
            //     comparator는 noexcept이며 어댑터를 여러 스레드가 공유하지 않는다.
            queue.push(QueueEntry{Cost{0}, source});

            // [호출 계약: std::priority_queue::empty()]
            // (1) 수신 queue는 0개 이상의 유효 QueueEntry를 가진 const 관찰 가능한 MinQueue lvalue다.
            // (2) [[nodiscard]] bool empty() const overload이며 명시 인자는 없다.
            // (3) 숨은 const this만 읽고 원소나 컨테이너 소유권을 건드리지 않는다.
            // (4) 원소가 없으면 true인 bool prvalue를 반환해 while 조건 분기에 사용한다.
            // (5) queue의 원소·size·우선순위 순서는 변하지 않는다.
            // (6) O(1), 무할당·비무효화이며 표준 vector의 empty 관찰은 예외를 던지지 않는다.
            //     검사 직후 다른 스레드가 pop하지 않는다는 단일 스레드 전제가 있다.
            while (!queue.empty()) {
                // [호출 계약: std::priority_queue::top()]
                // (1) 수신 queue는 직전 empty 검사가 false인 비어 있지 않은 MinQueue lvalue다.
                // (2) const_reference top() const overload가 선택되고 명시 인자는 없다.
                // (3) 숨은 const this만 읽고 QueueEntry 소유권을 옮기지 않는다.
                // (4) 최소 distance 원소의 const QueueEntry&를 반환하고 current에 즉시 값 복사한다.
                // (5) queue는 그대로이며 반환 참조는 다음 pop 전에만 사용된다.
                // (6) O(1), 무할당·비무효화다. 빈 queue에서 호출하면 전제 위반/UB지만 조건이 막는다.
                const QueueEntry current{queue.top()};

                // [호출 계약: std::priority_queue::pop()]
                // (1) 수신 queue는 방금 top을 읽은 비어 있지 않은 MinQueue lvalue다.
                // (2) void pop() overload가 선택되고 명시 인자는 없다.
                // (3) 숨은 비-const this를 통해 최상위 원소를 제거하며 외부 소유권 입력은 없다.
                // (4) 반환형 void라 제거값은 돌려주지 않아 current 복사를 먼저 만들었다.
                // (5) size가 1 감소하고 남은 원소 중 최소 거리가 새 top이 된다. 옛 top 참조는 제거된
                //     객체가 하나뿐이면 무효가 되고, 그 밖에도 같은 최상위 값을 계속 가리킨다고 의존할 수 없다.
                // (6) O(log size), 무할당이다. 빈 상태 호출은 UB이나 위 조건이 보장한다.
                //     comparator는 noexcept이고 QueueEntry 파괴도 예외가 없으며 동시 접근은 없다.
                queue.pop();

                const std::size_t vertex{static_cast<std::size_t>(current.vertex)};
                // 더 짧은 후보가 나중에 들어오면 이전 heap entry는 stale이다. decrease-key 대신 버린다.
                if (current.distance != distance[vertex]) {
                    continue;
                }

                // [호출 계약: std::vector<Edge>::operator[] 읽기]
                // (1) 수신 graph_[vertex]는 Dijkstra 동안 변경되지 않는 유효한 Adjacency lvalue이고
                //     edge_index는 아래 size 조건으로 [0,size)다.
                // (2) graph_가 비-const이므로 reference operator[](size_type position) overload가 선택된다.
                //     반환된 Edge&를 const Edge&에 바인딩해 이 순회에서는 읽기만 한다.
                // (3) edge_index std::size_t lvalue의 값을 복사해 위치 인자로 주며 소유권은 없다.
                // (4) Edge&를 반환한 뒤 const Edge&로 바인딩하고 이번 loop body 동안 필드만 읽는다.
                // (5) adjacency와 edge는 바뀌지 않고 참조는 graph_에 삽입이 없어 끝까지 유효하다.
                // (6) O(1), 무할당·비무효화다. 범위 밖이면 UB지만 loop 조건이 막고 동시 수정은 없다.
                for (std::size_t edge_index{0};
                     edge_index < graph_[vertex].size();
                     ++edge_index) {
                    const Edge& edge{graph_[vertex][edge_index]};
                    if (edge.capacity == 0) {
                        continue;
                    }

                    const std::size_t next{static_cast<std::size_t>(edge.to)};
                    // 잠재치 불변식 때문에 source에서 도달 가능한 잔여 간선의 reduced_cost는 0 이상이다.
                    const Cost reduced_cost{
                        edge.cost + potential[vertex] - potential[next]};

                    // infinity를 실제 숫자로 썼으므로 덧셈 전에 여유를 검사한다. 공식 범위와
                    // reduced_cost>=0에서는 항상 통과하지만 일반화 시 signed overflow를 막는다.
                    if (current.distance > infinity - reduced_cost) {
                        continue;
                    }
                    const Cost candidate{current.distance + reduced_cost};
                    if (candidate >= distance[next]) {
                        continue;
                    }

                    distance[next] = candidate;

                    // [호출 계약: std::vector<int>::operator[] 쓰기]
                    // (1) 두 수신자는 V개의 int를 소유하는 비-const vector<int> lvalue이고 next는 [0,V)다.
                    // (2) reference operator[](size_type position) overload를 각각 선택한다.
                    // (3) next 위치 값을 복사하고 current.vertex/edge_index의 좁혀진 int 값을 저장한다.
                    //     컨테이너나 edge의 소유권은 이동하지 않는다.
                    // (4) 각 호출은 int&를 반환해 대입의 왼쪽 피연산자로 사용한다.
                    // (5) next 원소만 최단경로 부모로 바뀌고 size/capacity/다른 원소는 유지된다.
                    // (6) 각 O(1), 무할당·비무효화다. 범위 밖이면 UB지만 Edge.to와 loop 불변식이 막는다.
                    //     m<=1000이라 edge_index의 int 변환도 안전하고 동시 접근은 없다.
                    parent_vertex[next] = current.vertex;
                    parent_edge[next] = static_cast<int>(edge_index);
                    queue.push(QueueEntry{candidate, edge.to});
                }
            }

            const std::size_t sink_index{static_cast<std::size_t>(sink)};
            if (distance[sink_index] == infinity) {
                // 더 이상 증가 경로가 없으므로 현재 흐름보다 큰 어떤 흐름도 만들 수 없다.
                break;
            }

            for (std::size_t vertex{0}; vertex < vertex_count; ++vertex) {
                if (distance[vertex] != infinity) {
                    potential[vertex] += distance[vertex];
                }
            }

            int pushed{required_flow - sent_flow};
            // 첫 역추적은 경로의 최소 잔여 용량, 즉 병목을 찾는다. parent는 유효 경로를 이룬다.
            for (int vertex{sink}; vertex != source;
                 vertex = parent_vertex[static_cast<std::size_t>(vertex)]) {
                const int previous{parent_vertex[static_cast<std::size_t>(vertex)]};
                const int edge_index{parent_edge[static_cast<std::size_t>(vertex)]};
                const Edge& edge{
                    graph_[static_cast<std::size_t>(previous)]
                          [static_cast<std::size_t>(edge_index)]};
                if (edge.capacity < pushed) {
                    pushed = edge.capacity;
                }
            }

            // 두 번째 역추적은 정방향 잔여 용량을 줄이고 짝 역간선을 늘린다.
            // [호출 계약: 비-const std::vector<Edge>::operator[] 갱신]
            // (1) 수신 adjacency들은 간선 추가가 끝난 graph_ 안의 유효한 vector<Edge> lvalue이고,
            //     previous/edge_index 및 edge.to/edge.reverse는 짝 간선 불변식이 보장한 범위 안 index다.
            // (2) reference operator[](size_type) overload들이 선택된다.
            // (3) 각 정수 index를 std::size_t 값으로 전달하고 원소 소유권은 옮기지 않는다.
            // (4) Edge&를 반환해 capacity를 제자리 수정하며 반환 참조는 이번 반복 밖에 저장하지 않는다.
            // (5) forward capacity는 pushed 감소, reverse capacity는 같은 양 증가해 유량 보존과 취소 가능성을
            //     유지한다. vector size/capacity는 변하지 않아 다른 원소 참조도 유효하다.
            // (6) 각 접근 O(1), 무할당·비무효화다. 범위 밖이면 UB이나 add_edge의 reverse index와
            //     parent 불변식이 막는다. 하나의 실행 스레드만 graph_를 수정한다.
            for (int vertex{sink}; vertex != source;
                 vertex = parent_vertex[static_cast<std::size_t>(vertex)]) {
                const int previous{parent_vertex[static_cast<std::size_t>(vertex)]};
                const int edge_index{parent_edge[static_cast<std::size_t>(vertex)]};
                Edge& edge{
                    graph_[static_cast<std::size_t>(previous)]
                          [static_cast<std::size_t>(edge_index)]};

                // edge 참조로 forward capacity를 바꿔도 to/reverse/cost는 바뀌지 않는다.
                const int next{edge.to};
                const int reverse_index{edge.reverse};
                const Cost edge_cost{edge.cost};
                edge.capacity -= pushed;
                graph_[static_cast<std::size_t>(next)]
                      [static_cast<std::size_t>(reverse_index)]
                          .capacity += pushed;
                total_cost += static_cast<Cost>(pushed) * edge_cost;
            }

            sent_flow += pushed;
        }

        return sent_flow == required_flow ? total_cost : Cost{-1};
    }

private:
    // private 접근 지정자는 정·역간선 쌍을 외부 수정에서 보호한다. MinCostFlow 객체가 graph_를
    // 독점 소유하고, 함수가 끝나면 vector 소멸자가 모든 내부 저장소를 자동 해제하는 RAII 구조다.
    Graph graph_;
};

int main() {
    // [호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 수신 객체 없는 static 설정 함수이며 어떤 표준 스트림에도 아직 입출력하지 않은 시작 상태다.
    // (2) static bool sync_with_stdio(bool sync = true)에 bool false prvalue 하나를 준다.
    // (3) false는 C와 C++ 표준 스트림의 동기화를 끄는 설정값이며 버퍼 소유권을 전달하지 않는다.
    // (4) 이전 동기화 상태 bool을 반환하지만 복구할 필요가 없어 의도적으로 사용하지 않는다.
    // (5) 이후 C stdio와 iostream을 섞을 때 상대 순서가 자동 보장되지 않을 수 있다.
    // (6) 표준은 점근 복잡도를 정하지 않으며 첫 I/O 전 호출이 전제다. 할당/반복자 무효화는 없고
    //     구현별 내부 오류 외 예외를 기대하지 않는다. 설정을 다른 스레드에서 동시에 바꾸지 않는다.
    std::ios::sync_with_stdio(false);

    // [호출 계약: std::cin.tie(nullptr)]
    // (1) 수신자는 프로그램 전체 수명의 유효한 std::istream lvalue std::cin이고 cout에 묶였을 수 있다.
    // (2) std::ostream* basic_ios<char>::tie(std::ostream* tied_stream) overload를 선택한다.
    // (3) nullptr prvalue는 자동 flush 대상을 없애라는 값이며 stream 소유권을 주고받지 않는다.
    // (4) 이전 tied ostream 포인터를 반환하지만 이 비대화형 프로그램은 복원하지 않아 버린다.
    // (5) 이후 입력 전에 cout이 자동 flush되지 않으며 cin/cout 객체의 수명·상태는 유지된다.
    // (6) 통상 O(1), 무할당·비무효화다. nullptr은 허용되고 설정을 동시에 변경하지 않는다.
    std::cin.tie(nullptr);

    int city_count{0};
    int route_count{0};
    int parcel_count{0};

    // [호출 계약: std::istream 정수 operator>> 연쇄 호출]
    // (1) 왼쪽 수신 std::cin은 입력 가능한 std::istream lvalue이고 세 int 대상은 0으로 초기화됐다.
    // (2) 각 단계에서 basic_istream<char>& operator>>(int&) 산술 overload가 선택된다.
    // (3) 대상 int lvalue를 비-const 참조로 빌려 공백 구분 정수를 저장하며 소유권 이동은 없다.
    // (4) 매 호출은 같은 istream&를 반환해 다음 >>의 왼쪽 피연산자로 사용하고 최종 반환은 버린다.
    // (5) 성공하면 입력 위치가 소비한 문자 뒤로 이동하고 세 변수에 값이 저장된다. 실패하면 failbit가
    //     설정되고 해당 단계 이후 대상은 문제 입력 전제 밖 상태지만 온라인 저지 입력은 항상 유효하다.
    // (6) 소비 문자 수에 선형이고 locale/버퍼가 내부 작업을 할 수 있다. ios_base::failure는 예외 mask가
    //     설정된 경우 가능하다. 변수 수명은 main 끝까지이며 다른 스레드가 cin을 읽지 않는다.
    std::cin >> city_count >> route_count >> parcel_count;

    MinCostFlow network{city_count};
    for (int route{0}; route < route_count; ++route) {
        int from{0};
        int to{0};
        int capacity{0};
        int unit_cost{0};
        std::cin >> from >> to >> capacity >> unit_cost;

        // 입력 경계에서 한 번만 1-based를 0-based로 바꾼다. unit_cost는 양수 int라 Cost 변환이 정확하다.
        network.add_edge(from - 1, to - 1, capacity, static_cast<Cost>(unit_cost));
    }

    const Cost answer{network.send(0, city_count - 1, parcel_count)};

    // [호출 계약: std::ostream의 int64 정수 및 char 삽입 operator<<]
    // (1) 왼쪽 수신 std::cout은 출력 가능한 std::ostream lvalue이고 answer는 초기화된 Cost lvalue다.
    // (2) 이 Windows GCC에서 Cost의 기반형에 맞는 basic_ostream<char>::operator<<(long long)과,
    //     이어서 비멤버 operator<<(basic_ostream<char>&, char) overload가 선택된다.
    // (3) answer 값과 '\n' char prvalue를 복사해 서식화하며 어떤 객체의 소유권도 이전하지 않는다.
    // (4) 각 호출은 같은 ostream&를 반환해 두 번째 <<에 사용하거나 최종적으로 버린다.
    // (5) 성공하면 숫자와 줄바꿈 문자가 출력 버퍼에 추가되고 answer는 변하지 않는다. 실패하면
    //     badbit/failbit가 설정될 수 있다.
    // (6) 정수 자릿수에 선형이고 버퍼 내부 할당 가능성은 구현에 달렸다. 예외 mask가 설정되면
    //     ios_base::failure가 가능하다. 참조/반복자 무효화는 없고 cout 동시 쓰기는 하지 않는다.
    std::cout << answer << '\n';
    return 0;
}
