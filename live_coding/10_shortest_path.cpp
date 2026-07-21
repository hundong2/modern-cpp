/*
Chapter 10. 최단 경로

문제 1: 양의 가중치 그래프에서 시작점으로부터 최단 거리를 구하라.
접근: Dijkstra. priority_queue에는 현재까지 발견한 후보 거리를 넣는다.
복잡도: O((V + E) log V)

문제 2: 음수 간선이 있을 수 있는 그래프에서 최단 거리와 음수 사이클 여부를 구하라.
접근: Bellman-Ford. V-1번 완화 후 한 번 더 완화되면 음수 사이클.
복잡도: O(VE)

문제 3: 모든 쌍 최단 경로를 구하라.
접근: Floyd-Warshall. k를 중간 정점으로 허용할 때의 최단 거리 DP.
복잡도: O(V^3)

문제 4: 간선 가중치가 0 또는 1이면 0-1 BFS를 사용한다.
접근: 0 비용은 deque 앞, 1 비용은 deque 뒤에 넣는다.
*/

#include <algorithm>
#include <climits>
#include <deque>
#include <functional>
#include <iostream>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>
using namespace std;

const long long INF = (1LL << 60);

vector<long long> dijkstra(int n, const vector<vector<pair<int, int>>>& graph, int start) {
    vector<long long> dist(n, INF);
    priority_queue<pair<long long, int>, vector<pair<long long, int>>, greater<pair<long long, int>>> pq;

    dist[start] = 0;
    pq.push({0, start});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d != dist[u]) continue; // 더 짧은 거리로 갱신된 뒤 남은 오래된 후보는 무시한다.

        for (auto [v, w] : graph[u]) {
            if (dist[v] > d + w) {
                dist[v] = d + w; // u를 거쳐 가는 경로가 더 짧으면 완화한다.
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

pair<bool, vector<long long>> bellmanFord(int n, const vector<tuple<int, int, int>>& edges, int start) {
    vector<long long> dist(n, INF);
    dist[start] = 0;

    for (int iter = 0; iter < n - 1; ++iter) {
        bool changed = false;
        for (auto [u, v, w] : edges) {
            if (dist[u] == INF) continue;
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w; // 모든 간선을 반복 완화해 음수 간선도 처리한다.
                changed = true;
            }
        }
        if (!changed) break; // 더 이상 갱신이 없으면 조기 종료한다.
    }

    for (auto [u, v, w] : edges) {
        if (dist[u] != INF && dist[v] > dist[u] + w) {
            return {true, dist};
        }
    }
    return {false, dist};
}

vector<vector<long long>> floydWarshall(vector<vector<long long>> dist) {
    int n = (int)dist.size();
    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < n; ++j) {
                if (dist[k][j] == INF) continue;
                dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]); // k를 중간 정점으로 쓰는 경우를 반영한다.
            }
        }
    }
    return dist;
}

vector<int> zeroOneBfs(int n, const vector<vector<pair<int, int>>>& graph, int start) {
    vector<int> dist(n, INT_MAX);
    deque<int> dq;
    dist[start] = 0;
    dq.push_front(start);

    while (!dq.empty()) {
        int u = dq.front();
        dq.pop_front();

        for (auto [v, w] : graph[u]) {
            if (dist[v] <= dist[u] + w) continue;
            dist[v] = dist[u] + w;
            if (w == 0) dq.push_front(v); // 0 비용 간선은 같은 거리 레벨이라 앞에 넣는다.
            else dq.push_back(v);         // 1 비용 간선은 다음 거리 레벨이라 뒤에 넣는다.
        }
    }
    return dist;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<vector<pair<int, int>>> graph(5);
    auto add = [&](int u, int v, int w) {
        graph[u].push_back({v, w});
        graph[v].push_back({u, w});
    };
    add(0, 1, 2);
    add(0, 2, 5);
    add(1, 2, 1);
    add(1, 3, 2);
    add(2, 4, 3);

    auto dist = dijkstra(5, graph, 0);
    cout << "[dijkstra]";
    for (long long d : dist) cout << ' ' << d;
    cout << '\n';

    auto [hasNegativeCycle, bf] = bellmanFord(3, {{0, 1, 4}, {1, 2, -2}, {0, 2, 5}}, 0);
    cout << "[bellman negative cycle] " << boolalpha << hasNegativeCycle << " dist2=" << bf[2] << '\n';

    vector<vector<long long>> matrix(3, vector<long long>(3, INF));
    for (int i = 0; i < 3; ++i) matrix[i][i] = 0;
    matrix[0][1] = 3;
    matrix[1][2] = 4;
    matrix[0][2] = 10;
    auto all = floydWarshall(matrix);
    cout << "[floyd 0->2] " << all[0][2] << '\n';
    return 0;
}
