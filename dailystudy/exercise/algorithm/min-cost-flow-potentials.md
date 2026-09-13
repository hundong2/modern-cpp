# 잠재치와 최소비용 유량

## 정의

최소비용 유량(min-cost flow)은 방향 간선마다 용량과 단위 비용이 있을 때, 출발점에서 도착점으로
요구한 양의 흐름을 보내면서 전체 비용을 최소화하는 문제다. 간선 e에 보낸 흐름을 f(e), 단위 비용을
c(e)라 하면 목적값은 다음 합이다.

~~~text
total cost = sum over every original edge e of f(e) * c(e)
~~~

각 흐름은 0 이상 capacity(e) 이하여야 하고, 출발점과 도착점을 제외한 정점에서는 들어온 양과 나간
양이 같아야 한다. 오늘 구현은 한 번에 하나의 최단 증가 경로를 찾아 병목만큼 흘리는
successive shortest augmenting path(SSAP) 방식이다.

역간선 때문에 잔여 그래프에는 음수 비용이 생긴다. 매 반복 Bellman–Ford를 쓰면 느리므로,
Johnson 잠재치로 간선 비용을 음이 아닌 감소 비용으로 바꾸고 이진 힙 Dijkstra를 사용한다.

## 적용 조건

다음 상황에 알맞다.

- 보내야 하는 흐름 F가 비교적 작거나, 한 증가 경로가 큰 병목을 보내 반복 수가 적다.
- 용량과 비용이 정수이고, 최적 흐름의 실제 비용이 64비트 정수 범위 안이다.
- 각 간선을 몇 번 사용할지 선택하면서 총비용을 최소화해야 한다.
- 운송, 배정, 최소비용 이분 매칭, 여러 개의 간선 서로소 경로처럼 네트워크로 모델링할 수 있다.

다음 경우에는 다른 선택도 검토한다.

- 비용이 모두 같고 최대 유량만 필요하면 Dinic 같은 최대 유량 알고리즘이 더 단순하다.
- F가 매우 크면 cost scaling, capacity scaling, primal-dual의 더 강한 구현이 필요할 수 있다.
- 원래 잔여 간선에 음수 비용이 있다면 잠재치 0으로 Dijkstra를 시작할 수 없다. Bellman–Ford,
  SPFA에 의존하지 않는 다른 최단거리 초기화, 또는 DAG 최단거리로 feasible initial potential을 만든다.
- 음수 비용 사이클의 흐름을 별도로 최적화해야 하는 모델은 cycle canceling이나 더 일반적인 이론이 필요하다.

오늘 CSES 문제는 모든 원래 비용이 1 이상이고 요구 흐름 k가 최대 100이다. 따라서 초기 잠재치 0이
유효하고 SSAP 반복 수도 최대 100이라 이 방식에 잘 맞는다.

## 잔여 네트워크와 역간선

원래 간선 u -> v의 용량이 cap, 단위 비용이 cost라면 다음 두 Edge를 함께 저장한다.

| 방향 | 초기 잔여 용량 | 단위 비용 | 의미 |
|---|---:|---:|---|
| u -> v | cap | cost | 아직 이 원래 간선으로 더 보낼 수 있는 양 |
| v -> u | 0 | -cost | 이미 보낸 흐름을 취소할 수 있는 양 |

u -> v로 x를 새로 보내면 정방향 잔여 용량은 x만큼 줄고 역방향 잔여 용량은 x만큼 늘어난다.
나중 증가 경로가 v -> u 역간선을 사용하면 이전 흐름 x 중 일부를 취소한다. 즉, SSAP는 앞서 한
선택에 영원히 묶이지 않고 더 싼 전체 조합으로 rerouting할 수 있다.

각 Edge는 상대 adjacency vector 안의 짝 간선 index인 reverse를 저장한다. 평행 간선도 서로 다른
index를 가져 안전하다. 다만 from==to인 self-loop에 보통 쓰는 두 번의 push_back 공식을 그대로
적용하면 두 간선이 같은 vector에 들어가므로 첫 삽입 뒤 index가 바뀐다는 함정이 있다.

오늘 입력에서는 모든 비용이 양수다. self-loop에 f를 흘려도 한 정점의 유입과 유출에 f가 동시에
더해져 흐름 보존에는 변화가 없고 비용 f*cost만 늘어난다. 따라서 어떤 최적해도 양수 비용 self-loop를
사용할 필요가 없으며, 구현은 입력은 읽되 그 간선을 저장하지 않는다. 비용 0 또는 음수 self-loop까지
지원해야 하는 일반 라이브러리라면 같은 vector에 넣는 경우의 reverse index를 따로 보정해야 한다.

## 핵심 아이디어와 불변식

### 1. 정방향·역방향 짝 불변식

graph[u][i]가 v로 가고 reverse가 j라면 graph[v][j]는 반드시 u로 돌아오며 다시 i를 가리킨다.
두 용량의 합은 원래 용량이고 두 비용의 합은 0이다.

~~~text
forward.to = v
graph[v][forward.reverse].to = u
graph[v][forward.reverse].reverse = i
forward.cost + reverse.cost = 0
~~~

증가량 x를 보낼 때 두 capacity를 반대로 같은 양만큼 바꾸므로 이 불변식이 유지된다.

### 2. 유량 실행 가능성 불변식

매 반복이 시작될 때 현재 흐름은 모든 원래 용량을 지키고 중간 정점의 유량 보존을 만족한다.
source-to-sink 경로를 따라 같은 x를 더하면 내부 정점에는 x가 하나 들어오고 하나 나가므로 보존식이
그대로다. 경로의 병목 이하만 보내므로 어느 잔여 용량도 음수가 되지 않는다.

### 3. 감소 비용과 feasible potential

정점 v의 잠재치를 pi[v]라 하고 잔여 간선 u -> v의 감소 비용을 다음처럼 정의한다.

~~~text
reduced(u,v) = cost(u,v) + pi[u] - pi[v]
~~~

source에서 도달 가능한 양의 잔여 용량 간선에 대해 reduced가 항상 0 이상이라는 불변식을 유지한다.
처음에는 pi가 모두 0이고 원래 양수 비용 간선만 잔여 용량을 가지므로 참이다.

Dijkstra가 감소 비용 최단거리 d를 구하면 도달한 정점에 다음 갱신을 한다.

~~~text
pi[v] = pi[v] + d[v]
~~~

최단거리 삼각부등식 d[v] <= d[u] + reduced(u,v)를 정리하면 새 감소 비용은 다음과 같이 음이 아니다.

~~~text
new reduced(u,v)
  = reduced(u,v) + d[u] - d[v]
  >= 0
~~~

선택한 최단경로의 각 간선에서는 등호가 성립한다. 그 간선을 뒤집어 새로 활성화한 역간선의 새 감소
비용도 정확히 0이므로 다음 Dijkstra가 음수 간선을 만나지 않는다.

도달하지 못한 정점의 잠재치는 갱신하지 않는다. 한 augmentation이 새로 활성화하는 역간선의 양 끝은
모두 이번에 도달한 경로 위 정점이다. 따라서 지금 source에서 도달하지 못한 정점이 다음 반복에 갑자기
도달 가능해지지 않으며, Dijkstra가 실제로 방문할 잔여 부분에서는 불변식이 충분히 유지된다.

### 4. 최단거리 부모 불변식

distance[v]가 무한대가 아니면 parent_vertex[v]와 parent_edge[v]는 source에서 v로 가는 현재 최선
경로의 마지막 잔여 간선을 나타낸다. 더 작은 candidate만 받아들이고, priority_queue에서 꺼낸 거리가
현재 distance와 다르면 stale entry로 버린다. decrease-key 없이도 각 확정 거리가 올바르다.

### 5. 비용 변환은 경로 순서를 바꾸지 않는다

source s에서 sink t까지 어떤 경로 P의 감소 비용 합을 전개하면 중간 잠재치가 망원경처럼 상쇄된다.

~~~text
sum reduced(e in P) = sum cost(e in P) + pi[s] - pi[t]
~~~

pi[s]-pi[t]는 모든 s-to-t 경로에 같은 상수다. 따라서 감소 비용이 가장 작은 경로는 실제 비용도 가장
작다. 잠재치는 최적 경로를 바꾸지 않고 Dijkstra가 사용할 수 있는 음이 아닌 간선 값으로 표현만 바꾼다.

### 6. 정수성과 반복 횟수

용량과 요구 흐름이 정수이므로 경로 병목도 양의 정수다. 매 성공 반복마다 적어도 한 단위 이상 보내므로
augmentation 횟수 A는 required flow F 이하이다. 오늘 문제에서는 A <= k <= 100이다.

## 단계별 절차

1. 정점마다 Edge vector를 만들고 각 원래 간선에 정방향 Edge와 용량 0의 역방향 Edge를 추가한다.
2. 양수 비용 self-loop는 최적해에 필요 없으므로 건너뛴다.
3. 모든 잠재치를 0으로 초기화한다. 이는 원래 비용이 음이 아닐 때만 바로 가능하다.
4. 감소 비용을 가중치로 Dijkstra를 실행한다.
5. sink가 도달 불가능하면 더 보낼 수 없으므로 실패한다.
6. Dijkstra에서 도달한 정점의 잠재치에 그 정점의 거리를 더한다.
7. parent를 sink에서 source까지 따라가며 남은 요구량과 모든 경로 간선 용량의 최솟값을 병목으로 정한다.
8. 같은 경로를 다시 따라 정방향 capacity를 줄이고 역방향 capacity를 늘린다.
9. 각 경로 간선의 실제 cost 곱하기 병목을 total cost에 더한다.
10. 요구 흐름을 모두 보낼 때까지 4~9를 반복한다.

## 의사코드

~~~text
add_edge(u, v, cap, cost):
    if u == v and cost > 0:
        return
    forward.reverse = graph[v].size
    reverse.reverse = graph[u].size
    graph[u].append(Edge(v, forward.reverse, cap, cost))
    graph[v].append(Edge(u, reverse.reverse, 0, -cost))

min_cost_flow(source, sink, need):
    potential[v] = 0 for every v
    flow = 0
    answer = 0

    while flow < need:
        distance[v] = infinity
        parent[v] = none
        distance[source] = 0
        run Dijkstra using:
            reduced = edge.cost + potential[u] - potential[edge.to]

        if distance[sink] is infinity:
            return -1

        for every reached v:
            potential[v] += distance[v]

        pushed = need - flow
        walk parent from sink to source:
            pushed = min(pushed, edge.capacity)

        walk the same path again:
            edge.capacity -= pushed
            reverse.capacity += pushed
            answer += pushed * edge.cost

        flow += pushed

    return answer
~~~

## 컴파일 가능한 C++ 뼈대

다음 코드는 양수 원래 비용, 정수 용량, 0-based 내부 정점을 전제로 한 완전한 C++20 프로그램이다.
재귀를 사용하지 않으므로 그래프 깊이와 호출 스택 크기가 무관하다.

~~~cpp
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <queue>
#include <vector>

using Cost = std::int64_t;

struct Edge {
    int to;
    int reverse;
    int capacity;
    Cost cost;
};

using Graph = std::vector<std::vector<Edge>>;

struct State {
    Cost distance;
    int vertex;
};

struct Greater {
    bool operator()(const State& left, const State& right) const noexcept {
        if (left.distance != right.distance) {
            return left.distance > right.distance;
        }
        return left.vertex > right.vertex;
    }
};

using Queue = std::priority_queue<State, std::vector<State>, Greater>;

class MinCostFlow {
public:
    explicit MinCostFlow(const int vertex_count)
        : graph_(static_cast<std::size_t>(vertex_count)) {}

    void add_edge(const int from, const int to, const int capacity, const Cost cost) {
        // 양수 비용 self-loop는 흐름 보존에 기여하지 않고 비용만 늘리므로 버린다.
        // 이를 허용하려면 같은 adjacency에 두 번 넣을 때 forward reverse를 size+1로 보정해야 한다.
        if (from == to) {
            return;
        }

        const int forward_reverse{
            static_cast<int>(graph_[static_cast<std::size_t>(to)].size())};
        const int backward_reverse{
            static_cast<int>(graph_[static_cast<std::size_t>(from)].size())};

        graph_[static_cast<std::size_t>(from)].push_back(
            Edge{to, forward_reverse, capacity, cost});
        graph_[static_cast<std::size_t>(to)].push_back(
            Edge{from, backward_reverse, 0, -cost});
    }

    Cost run(const int source, const int sink, const int need) {
        constexpr Cost inf{std::numeric_limits<Cost>::max() / 4};
        const std::size_t n{graph_.size()};
        std::vector<Cost> potential(n, 0);
        int flow{0};
        Cost total{0};

        while (flow < need) {
            std::vector<Cost> distance(n, inf);
            std::vector<int> parent_vertex(n, -1);
            std::vector<int> parent_edge(n, -1);
            Queue queue;

            distance[static_cast<std::size_t>(source)] = 0;
            queue.push(State{0, source});

            while (!queue.empty()) {
                const State current{queue.top()};
                queue.pop();
                const std::size_t u{static_cast<std::size_t>(current.vertex)};
                if (current.distance != distance[u]) {
                    continue; // decrease-key 대신 오래된 heap entry를 버린다.
                }

                for (std::size_t i{0}; i < graph_[u].size(); ++i) {
                    const Edge& edge{graph_[u][i]};
                    if (edge.capacity == 0) {
                        continue;
                    }

                    const std::size_t v{static_cast<std::size_t>(edge.to)};
                    // 잠재치 불변식 때문에 reduced는 방문 가능한 잔여 간선에서 음이 아니다.
                    const Cost reduced{
                        edge.cost + potential[u] - potential[v]};
                    if (current.distance > inf - reduced) {
                        continue; // 무한대 sentinel과의 덧셈 overflow를 피한다.
                    }
                    const Cost candidate{current.distance + reduced};
                    if (candidate >= distance[v]) {
                        continue;
                    }

                    distance[v] = candidate;
                    parent_vertex[v] = current.vertex;
                    parent_edge[v] = static_cast<int>(i);
                    queue.push(State{candidate, edge.to});
                }
            }

            const std::size_t target{static_cast<std::size_t>(sink)};
            if (distance[target] == inf) {
                return -1;
            }

            for (std::size_t v{0}; v < n; ++v) {
                if (distance[v] != inf) {
                    potential[v] += distance[v];
                }
            }

            int pushed{need - flow};
            for (int v{sink}; v != source;
                 v = parent_vertex[static_cast<std::size_t>(v)]) {
                const int u{parent_vertex[static_cast<std::size_t>(v)]};
                const int i{parent_edge[static_cast<std::size_t>(v)]};
                const Edge& edge{
                    graph_[static_cast<std::size_t>(u)][static_cast<std::size_t>(i)]};
                if (edge.capacity < pushed) {
                    pushed = edge.capacity;
                }
            }

            for (int v{sink}; v != source;
                 v = parent_vertex[static_cast<std::size_t>(v)]) {
                const int u{parent_vertex[static_cast<std::size_t>(v)]};
                const int i{parent_edge[static_cast<std::size_t>(v)]};
                Edge& edge{
                    graph_[static_cast<std::size_t>(u)][static_cast<std::size_t>(i)]};
                const int to{edge.to};
                const int reverse{edge.reverse};
                const Cost cost{edge.cost};

                edge.capacity -= pushed;
                graph_[static_cast<std::size_t>(to)]
                      [static_cast<std::size_t>(reverse)]
                          .capacity += pushed;
                total += static_cast<Cost>(pushed) * cost;
            }
            flow += pushed;
        }

        return total;
    }

private:
    Graph graph_;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n{0};
    int m{0};
    int k{0};
    std::cin >> n >> m >> k;

    MinCostFlow network{n};
    for (int i{0}; i < m; ++i) {
        int a{0};
        int b{0};
        int capacity{0};
        int cost{0};
        std::cin >> a >> b >> capacity >> cost;
        network.add_edge(a - 1, b - 1, capacity, static_cast<Cost>(cost));
    }

    std::cout << network.run(0, n - 1, k) << '\n';
}
~~~

## 정확성 근거

### 보조정리 1: 잔여 간선 갱신은 실행 가능한 흐름을 보존한다

증가 경로의 모든 정방향 잔여 용량에서 같은 pushed를 빼고 짝 역방향 용량에 더한다. pushed는 경로
병목 이하이므로 용량이 음수가 되지 않는다. 경로의 내부 정점은 들어오는 간선과 나가는 간선에서 같은
양만큼 변하므로 유량 보존을 유지한다. source의 순유출과 sink의 순유입만 pushed 증가한다.

### 보조정리 2: 매 Dijkstra 시작 시 방문 가능한 감소 비용은 음이 아니다

첫 반복은 잠재치가 0이고 양수 원래 간선만 활성화되어 자명하다. 어느 반복 시작에 명제가 참이라고
가정한다. Dijkstra 최단거리의 삼각부등식으로 잠재치 갱신 뒤 기존 잔여 간선의 새 감소 비용은
reduced+d[u]-d[v] >= 0이다. 선택 경로 간선은 등호가 성립하므로 새 역간선도 감소 비용 0이다.
새로 활성화되는 간선은 이 역간선들뿐이므로 다음 반복에도 명제가 유지된다.

### 보조정리 3: Dijkstra가 고른 경로는 실제 비용 최소 증가 경로다

보조정리 2로 Dijkstra가 요구하는 음이 아닌 가중치 전제가 성립한다. 따라서 계산한 감소 비용 경로가
최소다. 모든 source-to-sink 경로의 감소 비용은 실제 비용에 같은 상수 pi[source]-pi[sink]를 더한
값이므로 경로 순서는 바뀌지 않는다. 따라서 선택 경로는 실제 잔여 비용도 최소다.

### 보조정리 4: 병목 augmentation 뒤 새 잔여 그래프에는 음수 비용 사이클이 없다

흐름 0의 잔여 그래프에는 양수 원래 간선만 활성화되어 음수 사이클이 없다. 현재 잔여 그래프에도 음수
비용 사이클이 없다고 귀납 가정하고, Dijkstra 직전 source에서 도달 가능한 정점 집합을 S라 하자.

- S의 정의상 양의 잔여 용량을 가진 S -> V-S 간선은 없다. 그런 간선이 있으면 끝점도 도달 가능해 S에
  들어가야 하기 때문이다.
- S 안에서 계속 남는 기존 잔여 간선은 잠재치 갱신 뒤 감소 비용이 음이 아니다.
- augmentation이 새로 활성화하는 간선은 선택 경로의 역간선뿐이다. 경로 전체가 S 안에 있고 이 새
  역간선의 감소 비용은 0이다. 정방향 간선 일부가 포화되어 사라지는 것은 새 사이클을 만들 수 없다.
- V-S 내부는 augmentation으로 전혀 바뀌지 않으므로 귀납 가정상 그 안에도 음수 사이클이 없다.
- augmentation 뒤에도 S -> V-S 잔여 간선은 새로 생기지 않는다. 따라서 방향 사이클은 S와 V-S를
  가로지를 수 없다. V-S -> S 간선이 있더라도 돌아나갈 S -> V-S 간선이 없기 때문이다.

결국 새 잔여 그래프의 모든 사이클은 S 내부 또는 V-S 내부에만 있고 어느 쪽에도 음수 사이클이 없다.
이 논리는 0보다 크고 경로 병목 이하인 어떤 pushed에도 성립한다. 따라서 한 단위씩 멈추지 않고 병목
전체를 보내도 최적성 조건을 건너뛰지 않는다.

### 보조정리 5: 각 augmentation 뒤 현재 유량은 그 유량값에서 최소비용이다

고정된 유량값의 실행 가능한 흐름이 최소비용일 필요충분조건은 그 잔여 그래프에 음수 비용 사이클이
없는 것이다. 더 싼 같은 값의 흐름이 있다면 두 흐름의 차이를 잔여 간선의 순환들로 분해했을 때 총비용이
음수인 사이클이 적어도 하나 생긴다. 반대로 음수 사이클이 있으면 그 사이클 병목만큼 순환시켜 유량값을
바꾸지 않고 비용을 낮출 수 있다.

흐름 0은 최소비용이고 보조정리 4가 매 병목 augmentation 뒤 음수 사이클 부재를 보존한다. 따라서
귀납적으로 매 성공 단계의 현재 흐름은 그 흐름값에서 최소비용이다. 역간선은 이 차이 분해에서 이전
선택을 취소하는 방향까지 빠짐없이 표현한다.

### 정리: 반환값은 요구 흐름의 최소비용이거나 -1이다

required flow에 도달하면 보조정리 5에 따라 total이 그 흐름값의 최소비용이다. 도달 전에 sink가
잔여 그래프에서 보이지 않으면 source-to-sink 증가 경로가 없다. 최대 유량의 증가 경로 정리에 따라
현재보다 더 큰 실행 가능 흐름도 없으므로 요구량은 불가능하고 -1이 정확하다.

### 양수 비용 self-loop 제거의 정확성

self-loop 흐름 f는 같은 정점의 유입과 유출에 동시에 포함되어 흐름 보존과 source-to-sink 유량값을
바꾸지 않는다. cost>0이므로 f를 0으로 낮추면 실행 가능성은 그대로이고 총비용은 f*cost만큼 감소한다.
따라서 최소비용 해 중에는 모든 양수 비용 self-loop 흐름이 0인 해가 반드시 존재하며 입력에서 버려도 된다.

## 시간·공간 복잡도

원래 저장 간선 수를 E라 하면 잔여 그래프에는 최대 2E개의 Edge가 있다. 이 구현은 decrease-key 대신
오래된 후보를 heap에 남기므로 한 번의 Dijkstra에서 O(E)개 후보가 쌓일 수 있다. 간선 완화는 O(E),
각 heap push/pop은 상각 또는 heap 재정렬 관점에서 O(log(E+1))이며, 거리 배열 초기화는 O(V)다.
augmentation 횟수 A는 매번 정수 흐름을 최소 1 이상 보내므로 A <= F다.

- 시간: O(A (V + E log(E+1)))
- 오늘 문제: F=k<=100이므로 O(k (V + E log(E+1)))
- 공간: graph O(V+E), 거리·잠재치·부모 O(V), heap의 stale entry까지 O(E), 합계 O(V+E)

공식 범위의 단순 원래 경로 비용은 최대 (V-1)*1000이고 k개 최종 비용도 64비트보다 훨씬 작다.
그래도 감소 비용, 음수 역간선, 잠재치, 곱셈이 한 식에 섞이므로 Cost를 64비트로 통일한다.
infinity는 numeric limit의 1/4을 쓰고 실제 덧셈 전에 current_distance <= infinity-reduced를 확인해
부호 있는 정수 overflow를 피한다.

## 흔한 실수

1. 역간선 비용을 -cost가 아니라 0으로 둔다. 이전 선택 취소 비용을 잃어 최솟값이 깨진다.
2. 정방향 capacity만 줄이고 역방향 capacity를 늘리지 않는다. rerouting이 불가능해진다.
3. 음수 역간선이 생겼는데 원래 cost로 Dijkstra를 반복한다. Dijkstra의 전제가 깨진다.
4. 잠재치를 모든 정점에 infinity까지 더한다. overflow가 나므로 도달한 정점만 갱신한다.
5. 감소 비용으로 total cost를 더한다. 출력은 원래 edge.cost의 합이어야 한다.
6. parent에 Edge 포인터나 참조를 저장한 뒤 add_edge로 vector를 재할당한다. index 쌍을 저장하거나,
   모든 간선 추가가 끝난 뒤에만 참조를 빌린다.
7. 평행 간선을 합쳐 버린다. 용량과 비용이 다를 수 있으므로 각각 독립 Edge로 두어야 한다.
8. self-loop에서 서로 다른 adjacency라고 가정한 reverse index 공식을 쓴다. 오늘처럼 양수 비용이면
   안전하게 제거하고, 일반 구현이면 첫 push 뒤 위치 변화를 보정한다.
9. source/sink의 1-based 입력을 내부 index와 섞는다. 입력 경계에서 한 번만 0-based로 바꾼다.
10. total, potential, distance를 int로 둔다. 제약이 커지면 곱셈과 누적에서 overflow한다.
11. heap에서 stale entry를 검사하지 않는다. 정답은 맞을 수 있어도 불필요한 간선 순회가 크게 늘어난다.
12. 초기 음수 비용 간선이 있는데 잠재치 0으로 시작한다. Bellman–Ford 등의 초기화가 필요하다.
13. path bottleneck 대신 항상 한 단위만 보낸다. 정답은 같지만 큰 용량 문제에서 지나치게 느려진다.
14. sink가 도달 불가능한데 parent를 역추적한다. 먼저 distance[sink]를 검사해야 한다.

## 변형

- **최소비용 최대유량**: required flow를 두지 않고 sink가 도달 불가능할 때까지 반복해 flow와 cost를 함께 반환한다.
- **최소비용 이분 매칭**: source->왼쪽, 가능한 짝, 오른쪽->sink를 모두 용량 1로 만들고 짝 비용을 넣는다.
- **최대 이익 선택**: 이익을 음수 비용으로 바꾼다. 이때 초기 잠재치 계산 또는 Bellman–Ford가 필요하다.
- **하한이 있는 흐름**: 각 간선 lower bound를 미리 흘린 것으로 처리하고 정점 demand를 조정한 뒤
  super source/sink로 실행 가능성을 검사한다.
- **여러 source/sink**: super source와 super sink를 추가한다.
- **경로 자체 복원**: 각 augmentation의 parent 경로와 pushed를 기록한 뒤 최종 흐름을 원래 간선별로 읽는다.
- **cost scaling / capacity scaling**: F가 큰 문제에서 단순 SSAP의 반복 횟수를 줄인다.
- **DAG의 음수 비용 초기화**: 그래프가 DAG라면 위상 순서 최단거리로 초기 잠재치를 선형 시간에 만들 수 있다.

## 오늘 문제와의 연결

2026-09-14의 [CSES 2121 제출 코드](../2026-09-14/icpc_problem.cpp)는 도시를 정점, 운송로를 용량 r과
단위 비용 c를 가진 방향 간선으로 모델링한다. source는 0, sink는 n-1, required flow는 k다.

공식 비용이 양수라 잠재치 0으로 시작하고, 양수 self-loop를 건너뛴다. 평행 운송로는 각각 정·역간선
쌍을 가진다. 한 경로가 여러 소포를 실을 수 있으므로 남은 k와 경로 병목의 최솟값을 한 번에 보내며,
도중에 역간선을 쓰는 경로가 발견되면 이전 운송 배치를 취소하고 더 싼 전체 배치로 바꿀 수 있다.

공식 예제에서는 1->3->4의 용량이 2라 두 소포를 단위 비용 150에 보내고, 남은 하나를
1->2->4의 단위 비용 450에 보낸다. 총비용은 2*150+1*450=750이다.

## 초보자 직접 해보기와 이해 검증

### 손으로 확인

1. 간선 1->2의 capacity=3, cost=7인 정·역간선을 표로 그린다.
2. 그 간선으로 두 단위를 보낸 뒤 두 capacity가 각각 1과 2, 비용이 7과 -7인지 확인한다.
3. 세 정점 경로의 감소 비용을 더했을 때 중간 정점 잠재치가 왜 상쇄되는지 식을 전개한다.
4. 공식 예제에서 첫 경로가 1->3->4이고 병목이 2인 이유를 적는다.
5. 한 단위씩이 아니라 병목만큼 보내도 최단성 조건이 유지되는 이유를 설명한다.

### 코드를 바꿔 보기

1. 경로 병목을 항상 1로 바꾸고 공식 예제의 답은 같지만 반복 수가 늘어나는지 측정한다.
2. 역간선 capacity 증가 한 줄을 지우고, 앞선 선택 취소가 필요한 작은 반례를 찾아 본다.
3. priority_queue의 stale 검사를 지우고 같은 정점 거리가 여러 번 줄어드는 그래프에서 pop 횟수를 센다.
4. cost가 음수인 원래 간선 하나를 추가하고 잠재치 0 Dijkstra가 왜 안전하지 않은지 설명한다.
5. self-loop를 무시하지 않는 add_edge를 작성하되, 같은 vector 안에서 forward.reverse가
   삽입 전 size+1이어야 함을 작은 vector index로 검증한다.

### 이해를 증명하는 질문

1. capacity와 flow의 차이를 자신의 말로 설명할 수 있는가?
2. 역간선 cost가 왜 원래 cost의 음수여야 하는가?
3. reduced(u,v)가 음이 아니면 Dijkstra를 쓸 수 있는 이유는 무엇인가?
4. pi 갱신 뒤 선택 경로 역간선의 reduced cost가 0임을 식으로 보일 수 있는가?
5. parent_vertex와 parent_edge 두 배열이 모두 필요한 이유는 무엇인가?
6. 평행 간선이 있을 때 정점 부모만 저장하면 어떤 정보가 사라지는가?
7. distance가 infinity인 정점의 잠재치를 갱신하면 어떤 overflow 위험이 있는가?
8. sink가 보이지 않을 때 단순히 이번 경로만 없는 것이 아니라 요구 흐름 전체가 불가능한 이유는 무엇인가?
9. 시간복잡도 O(k (V + E log(E+1)))에서 k가 반복 횟수의 상한인 이유를 정수성으로 설명할 수 있는가?
10. total_cost에 reduced_cost가 아니라 edge.cost를 더해야 하는 이유를 말할 수 있는가?

위 질문에 코드 없이 답하고, 네 정점 이하의 작은 그래프에서 정·역간선 표와 잠재치 한 회를 손으로
계산한 뒤 뼈대를 다시 작성해 컴파일하면 이 알고리즘의 핵심을 이해했다고 볼 수 있다.
