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

#include <bits/stdc++.h>
using namespace std;

vector<int> topologicalOrder(int n, const vector<pair<int, int>>& edges) {
    vector<vector<int>> graph(n);
    vector<int> indegree(n, 0);
    for (auto [u, v] : edges) {
        graph[u].push_back(v);
        ++indegree[v];
    }

    queue<int> q;
    for (int i = 0; i < n; ++i) {
        if (indegree[i] == 0) q.push(i);
    }

    vector<int> order;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);

        for (int v : graph[u]) {
            if (--indegree[v] == 0) q.push(v);
        }
    }

    if ((int)order.size() != n) return {};
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

    queue<int> q;
    vector<int> earliest = duration;
    for (int i = 0; i < n; ++i) {
        if (indegree[i] == 0) q.push(i);
    }

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : graph[u]) {
            earliest[v] = max(earliest[v], earliest[u] + duration[v]);
            if (--indegree[v] == 0) q.push(v);
        }
    }
    return *max_element(earliest.begin(), earliest.end());
}

bool hasCycleDirectedDfs(int u, const vector<vector<int>>& graph, vector<int>& color) {
    color[u] = 1;
    for (int v : graph[u]) {
        if (color[v] == 1) return true;
        if (color[v] == 0 && hasCycleDirectedDfs(v, graph, color)) return true;
    }
    color[u] = 2;
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
