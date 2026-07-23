/*
Chapter 09. 위상 정렬, DAG DP

문제 1: 선수 과목 관계가 주어질 때 가능한 수강 순서를 구하라.
접근: indegree가 0인 정점을 큐에 넣는 Kahn 알고리즘.
정답 해설:
- 간선 u -> v는 u를 먼저 처리해야 v의 indegree가 줄어든다는 뜻이다.
- 처리한 정점 수가 N보다 작으면 cycle이 있어 위상 정렬이 불가능하다.
복잡도: O(V + E)

문제 2: 작업 시간과 선행 관계가 있는 DAG에서 전체 완료 최소 시간을 구하라.
접근: 위상 순서대로 earliest[v] = max(earliest[v], earliest[u] + time[v])
*/

#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

vector<int> topologicalOrder(int n, const vector<pair<int, int>>& edges) {
    vector<vector<int>> graph(n); // u -> v 간선을 저장하는 방향 그래프다.
    vector<int> indegree(n, 0);   // 아직 처리되지 않은 선행 정점 수다.
    for (auto [u, v] : edges) {
        graph[u].push_back(v);
        ++indegree[v]; // v보다 먼저 끝나야 하는 선행 작업 수다.
    }

    queue<int> q; // queue는 먼저 들어온 indegree 0 정점부터 꺼내는 FIFO 작업 목록이다.
    for (int i = 0; i < n; ++i) {
        if (indegree[i] == 0) q.push(i); // 지금 바로 처리 가능한 작업부터 시작한다.
    }

    vector<int> order; // 큐에서 꺼낸 순서가 가능한 위상 정렬 결과다.
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u); // indegree 0인 정점만 들어오므로 지금 처리해도 안전하다.

        for (int v : graph[u]) {
            if (--indegree[v] == 0) q.push(v); // 모든 선행 작업이 끝난 순간 큐에 넣는다.
        }
    }

    if ((int)order.size() != n) return {}; // 처리하지 못한 정점이 있으면 cycle이 있다는 뜻이다.
    return order;
}

int minimumProjectTime(const vector<int>& duration, const vector<pair<int, int>>& edges) {
    int n = (int)duration.size();
    vector<vector<int>> graph(n);
    vector<int> indegree(n, 0);
    for (auto [u, v] : edges) {
        graph[u].push_back(v);
        ++indegree[v];
    }

    queue<int> q; // 위상 정렬 DP에서도 처리 가능한 작업을 FIFO queue에 넣는다.
    vector<int> earliest = duration; // 선행 작업이 없는 작업은 자기 duration만큼 시간이 걸린다.
    for (int i = 0; i < n; ++i) {
        if (indegree[i] == 0) q.push(i);
    }

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : graph[u]) {
            earliest[v] = max(earliest[v], earliest[u] + duration[v]); // 가장 늦게 끝나는 선행 작업이 시작 가능 시간을 결정한다.
            if (--indegree[v] == 0) q.push(v);
        }
    }
    return *max_element(earliest.begin(), earliest.end());
}

bool hasCycleDirectedDfs(int u, const vector<vector<int>>& graph, vector<int>& color) {
    color[u] = 1; // 1은 현재 DFS call stack 안에 있다는 뜻이다.
    for (int v : graph[u]) {
        if (color[v] == 1) return true; // call stack 안의 정점을 다시 만나면 cycle이다.
        if (color[v] == 0 && hasCycleDirectedDfs(v, graph, color)) return true;
    }
    color[u] = 2; // 탐색이 끝난 정점은 다시 들어가도 cycle을 만들지 않는다.
    return false;
}

bool hasDirectedCycle(int n, const vector<pair<int, int>>& edges) {
    vector<vector<int>> graph(n);
    for (auto [u, v] : edges) graph[u].push_back(v);

    vector<int> color(n, 0);
    for (int i = 0; i < n; ++i) {
        if (color[i] == 0 && hasCycleDirectedDfs(i, graph, color)) return true;
    }
    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<pair<int, int>> edges = {{0, 2}, {1, 2}, {2, 3}, {2, 4}};
    auto order = topologicalOrder(5, edges);
    cout << "[topological order]";
    for (int x : order) cout << ' ' << x;
    cout << '\n';

    vector<int> duration = {3, 2, 5, 4, 6};
    cout << "[project time] " << minimumProjectTime(duration, edges) << '\n';
    cout << "[has cycle] " << boolalpha << hasDirectedCycle(3, {{0, 1}, {1, 2}, {2, 0}}) << '\n';
    return 0;
}
