/*
문제 ID/제목: BOJ 1753 - 최단경로
출처: https://www.acmicpc.net/problem/1753

문제 요약: 방향 그래프와 시작 정점이 주어질 때 시작점에서 모든 정점까지의 최단 거리를 구한다.
입력: 첫 줄에 정점 수 V와 간선 수 E, 다음 줄에 시작 정점 K가 온다. 이어지는 E줄은 출발 u, 도착 v, 양의 가중치 w이다.
출력: 1번부터 V번까지 최단 거리를 한 줄씩 출력하고 도달할 수 없으면 INF를 출력한다.
제약: 1 <= V <= 20,000, 1 <= E <= 300,000, 가중치는 10 이하의 자연수이다. 서로 다른 두 정점 사이에 여러 간선이 있을 수 있다.
예제 입력: 5 6 / 시작 1 / 간선 (5,1,1), (1,2,2), (1,3,3), (2,3,4), (2,4,5), (3,4,6)
예제 출력: 0, 2, 3, 7, INF가 각각 한 줄에 나온다.
*/

// <functional>은 priority_queue를 최소 힙으로 만드는 std::greater 비교자를 제공한다.
#include <functional>
// <iostream>은 빠른 표준 입력·출력 객체 cin과 cout을 제공한다.
#include <iostream>
// <limits>는 정수형이 표현할 수 있는 최댓값을 안전하게 얻는다.
#include <limits>
// <queue>는 우선순위 큐 std::priority_queue를 제공한다.
#include <queue>
// <utility>는 두 값을 묶는 std::pair를 제공한다.
#include <utility>
// <vector>는 정점별 인접 리스트와 거리 배열을 동적으로 저장한다.
#include <vector>

// 간선 하나의 도착 정점과 가중치를 공개 데이터로 묶는다. struct의 기본 접근은 public이다.
struct Edge {
    int to{};     // 정점 번호는 int로 충분하며 중괄호가 기본값 0을 보장한다.
    int weight{}; // 문제의 양의 가중치를 저장한다.
};

// 다익스트라의 정의·불변식·정확성·복잡도는 공용 문서 ../algorithm/dijkstra.md를 함께 본다.
using Distance = long long;              // using 별칭으로 누적 거리 타입의 의도를 드러낸다.
using State = std::pair<Distance, int>;  // (현재까지 거리, 정점)인 힙 원소 타입이다.

int main() {
    // 동기화를 끄고 묶음을 풀어 많은 입력에서 표준 스트림 비용을 줄인다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int vertex_count{};
    int edge_count{};
    std::cin >> vertex_count >> edge_count; // >>가 공백으로 구분된 두 정수를 읽어 변수에 저장한다.
    int start{};
    std::cin >> start;

    // vector<Edge>를 정점 수+1개 만든 인접 리스트이며 1번 인덱스부터 써서 입력 번호와 맞춘다.
    std::vector<std::vector<Edge>> graph(static_cast<std::size_t>(vertex_count + 1));
    for (int i{0}; i < edge_count; ++i) { // E개의 간선을 정확히 한 번 읽으므로 O(E)이다.
        int from{};
        int to{};
        int weight{};
        std::cin >> from >> to >> weight;
        graph[static_cast<std::size_t>(from)].push_back(Edge{to, weight}); // 방향 간선만 출발 정점 목록 끝에 추가한다.
    }

    // 덧셈 여유를 남긴 큰 값을 무한대로 사용해 실제 최단 거리와 구분한다.
    constexpr Distance infinity{std::numeric_limits<Distance>::max() / 4};
    std::vector<Distance> distance(static_cast<std::size_t>(vertex_count + 1), infinity);
    distance[static_cast<std::size_t>(start)] = 0; // 시작점까지 빈 경로의 거리는 0이다.

    // 기본 priority_queue는 최대 힙이므로 greater<State> 템플릿 인자로 가장 작은 pair가 먼저 나오게 한다.
    std::priority_queue<State, std::vector<State>, std::greater<State>> frontier;
    frontier.push(State{0, start}); // 시작 상태 prvalue를 힙에 넣는다.

    while (!frontier.empty()) { // 발견했지만 아직 처리하지 않은 후보가 있는 동안 반복한다.
        const auto [current_distance, vertex]{frontier.top()}; // 구조적 바인딩으로 최소 pair의 두 값을 복사한다.
        frontier.pop(); // top()으로 읽은 원소를 힙에서 제거한다.

        // 불변식: distance[v]는 지금까지 발견한 최솟값이다. 더 긴 오래된 힙 항목은 확장하지 않는다.
        if (current_distance != distance[static_cast<std::size_t>(vertex)]) {
            continue; // 아래 간선 반복을 건너뛰고 다음 최소 후보로 간다.
        }

        // 모든 가중치가 음수가 아니므로 최소 거리로 꺼낸 정점의 값은 이후 더 작아질 수 없다.
        for (const Edge& edge : graph[static_cast<std::size_t>(vertex)]) {
            const Distance candidate{current_distance + edge.weight}; // 현재 경로 뒤에 간선 하나를 붙인 완화 후보이다.
            Distance& best{distance[static_cast<std::size_t>(edge.to)]}; // 참조로 배열 원소를 직접 갱신한다.
            if (candidate < best) { // 더 짧은 경로를 찾은 경우에만 거리와 힙을 바꾼다.
                best = candidate;
                frontier.push(State{candidate, edge.to}); // 감소 키 대신 새 상태를 추가하고 오래된 상태는 위에서 거른다.
            }
        }
    }

    for (int vertex{1}; vertex <= vertex_count; ++vertex) { // 모든 정점의 답을 번호 순서로 출력한다.
        const Distance answer{distance[static_cast<std::size_t>(vertex)]};
        if (answer == infinity) {
            std::cout << "INF\n";
        } else {
            std::cout << answer << '\n';
        }
    }
    // 시간 O((V+E) log V), 공간 O(V+E). 실제 로드·비교·분기·호출 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return 0;
}

