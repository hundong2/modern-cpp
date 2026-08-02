/*
문제 ID/제목: Kattis shortestpath1 - Single source shortest path, non-negative weights
출처: https://open.kattis.com/problems/shortestpath1

문제 요약: 여러 테스트 케이스마다 음이 아닌 가중치를 가진 방향 그래프와 시작 정점이 주어진다.
각 질의 정점까지의 최단 거리를 답하고, 도달할 수 없으면 Impossible을 출력한다.
입력: 각 케이스는 정점 수 n, 간선 수 m, 질의 수 q, 시작점 s로 시작한다. 이어서 m개의
방향 간선(u, v, w)과 q개의 질의 정점이 온다. 0 0 0 0이면 입력이 끝난다.
출력: 각 질의마다 최단 거리 또는 Impossible을 한 줄에 출력하고 케이스 사이에는 빈 줄을 둔다.
제약: 1<=n<=10,000, 0<=m<=30,000, 1<=q<=100, 0<=w<=1,000이며 정점은 0부터 n-1이다.
예제: 0에서 간선 0->1(2), 1->2(2)가 있으면 질의 0,1,2,3의 답은 0,2,4,Impossible이다.
*/

// <functional>은 최소 힙 비교 함수 객체 std::greater를 제공한다.
#include <functional>
// <iostream>은 빠른 표준 입력·출력 객체를 제공한다.
#include <iostream>
// <limits>는 정수형이 표현 가능한 최댓값을 안전하게 조회한다.
#include <limits>
// <queue>는 우선순위 큐 컨테이너 어댑터를 제공한다.
#include <queue>
// <utility>는 두 값을 묶는 std::pair를 제공한다.
#include <utility>
// <vector>는 크기가 실행 중 정해지는 연속 저장 컨테이너를 제공한다.
#include <vector>

struct Edge { // struct의 기본 접근 public으로 목적지와 가중치를 단순 데이터로 공개한다.
    int to{};
    int weight{};
};

using Distance = long long; // 누적 합의 int 오버플로 여유를 확보하는 별칭이다.
using State = std::pair<Distance, int>; // (현재 거리, 정점)을 사전식 비교한다.

// 구현 근처 알고리즘 문서: ../algorithm/dijkstra.md
[[nodiscard]] std::vector<Distance> dijkstra(
    int start, const std::vector<std::vector<Edge>>& graph) { // const 참조는 큰 인접 리스트를 복사하지 않는다.
    constexpr Distance infinity{std::numeric_limits<Distance>::max() / 4}; // 덧셈 여유가 있는 센티널이다.
    std::vector<Distance> distance(graph.size(), infinity); // 모든 정점을 아직 미도달로 초기화한다.
    std::priority_queue<State, std::vector<State>, std::greater<State>> heap; // 가장 작은 거리 후보가 top이다.
    distance[static_cast<std::size_t>(start)] = 0;
    heap.push(State{0, start}); // 시작 상태 prvalue를 힙에 저장한다.

    while (!heap.empty()) { // 불변식: distance[v]는 지금까지 발견한 경로 중 최소 길이이다.
        const auto [current, vertex]{heap.top()}; // 구조적 바인딩으로 pair의 두 값을 복사한다.
        heap.pop();
        if (current != distance[static_cast<std::size_t>(vertex)]) {
            continue; // 더 좋은 경로가 이미 발견된 오래된 후보는 버린다.
        }
        for (const Edge& edge : graph[static_cast<std::size_t>(vertex)]) { // 각 간선을 최대 한 번 유효 완화한다.
            const Distance candidate{current + edge.weight};
            Distance& best{distance[static_cast<std::size_t>(edge.to)]}; // lvalue 참조로 원소를 직접 갱신한다.
            if (candidate < best) { // 음이 아닌 간선에서 더 짧은 경로만 채택한다.
                best = candidate;
                heap.push(State{best, edge.to});
            }
        }
    }
    return distance; // 반환 prvalue는 복사 생략 또는 이동으로 호출자에게 소유권이 넘어간다.
}

int main() {
    std::ios::sync_with_stdio(false); // C 입출력 동기화를 끄며 실제 호출·로드는 구현과 최적화에 따라 달라진다.
    std::cin.tie(nullptr); // 입력 전 자동 flush 연결을 끊는다.
    constexpr Distance infinity{std::numeric_limits<Distance>::max() / 4};

    while (true) {
        int vertex_count{}; int edge_count{}; int query_count{}; int start{};
        std::cin >> vertex_count >> edge_count >> query_count >> start;
        if (!std::cin || (vertex_count == 0 && edge_count == 0 && query_count == 0 && start == 0)) {
            break; // 입력 실패 또는 종료 센티널이면 반복문을 끝낸다.
        }
        std::vector<std::vector<Edge>> graph(static_cast<std::size_t>(vertex_count));
        for (int i{0}; i < edge_count; ++i) {
            int from{}; int to{}; int weight{};
            std::cin >> from >> to >> weight;
            graph[static_cast<std::size_t>(from)].push_back(Edge{to, weight}); // 방향 간선만 추가한다.
        }
        const std::vector<Distance> distance{dijkstra(start, graph)}; // 한 번 계산해 모든 질의가 공유한다.
        for (int i{0}; i < query_count; ++i) {
            int target{}; std::cin >> target;
            const Distance answer{distance[static_cast<std::size_t>(target)]};
            if (answer == infinity) { std::cout << "Impossible\n"; }
            else { std::cout << answer << '\n'; }
        }
        std::cout << '\n';
    }
    // 시간 O((n+m) log n + q), 공간 O(n+m). 구체적 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return 0;
}
