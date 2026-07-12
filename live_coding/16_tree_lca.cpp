/*
Chapter 16. 트리, 지름, LCA

문제 1: 트리에서 각 노드의 부모와 깊이를 구하라.
접근: root에서 DFS/BFS를 한 번 수행한다.
복잡도: O(N)

문제 2: 트리의 지름 길이를 구하라.
접근: 임의의 점에서 가장 먼 A를 찾고, A에서 가장 먼 B까지의 거리가 지름이다.
정답 해설:
- 트리에서는 두 점 사이 경로가 유일하므로 가장 먼 점을 두 번 찾는 방식이 성립한다.

문제 3: 두 노드의 LCA를 빠르게 구하라.
접근: binary lifting. up[k][v] = v의 2^k번째 조상.
복잡도: 전처리 O(N log N), 질의 O(log N)
*/

#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

class LCA {
public:
    explicit LCA(const vector<vector<int>>& tree, int root = 0)
        : n((int)tree.size()), graph(tree), depth(n, 0) {
        log = 1;
        while ((1 << log) <= n) ++log;
        up.assign(log, vector<int>(n, root));
        dfs(root, root);
    }

    int query(int a, int b) const {
        if (depth[a] < depth[b]) swap(a, b);

        int diff = depth[a] - depth[b];
        for (int k = 0; k < log; ++k) {
            if (diff & (1 << k)) a = up[k][a];
        }

        if (a == b) return a;
        for (int k = log - 1; k >= 0; --k) {
            if (up[k][a] != up[k][b]) {
                a = up[k][a];
                b = up[k][b];
            }
        }
        return up[0][a];
    }

    int getDepth(int node) const {
        return depth[node];
    }

private:
    int n;
    int log;
    const vector<vector<int>>& graph;
    vector<int> depth;
    vector<vector<int>> up;

    void dfs(int u, int parent) {
        up[0][u] = parent;
        for (int k = 1; k < log; ++k) {
            up[k][u] = up[k - 1][up[k - 1][u]];
        }

        for (int v : graph[u]) {
            if (v == parent) continue;
            depth[v] = depth[u] + 1;
            dfs(v, u);
        }
    }
};

pair<int, int> farthestFrom(int start, const vector<vector<int>>& tree) {
    vector<int> dist(tree.size(), -1);
    queue<int> q;
    q.push(start);
    dist[start] = 0;

    int best = start;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (dist[u] > dist[best]) best = u;
        for (int v : tree[u]) {
            if (dist[v] != -1) continue;
            dist[v] = dist[u] + 1;
            q.push(v);
        }
    }
    return {best, dist[best]};
}

int treeDiameter(const vector<vector<int>>& tree) {
    auto first = farthestFrom(0, tree);
    int a = first.first;
    auto [b, diameter] = farthestFrom(a, tree);
    (void)b;
    return diameter;
}

long long subtreeSumDfs(int u, int parent, const vector<vector<int>>& tree, const vector<int>& value, vector<long long>& sub) {
    sub[u] = value[u];
    for (int v : tree[u]) {
        if (v == parent) continue;
        sub[u] += subtreeSumDfs(v, u, tree, value, sub);
    }
    return sub[u];
}

vector<long long> subtreeSums(const vector<vector<int>>& tree, const vector<int>& value, int root = 0) {
    vector<long long> sub(tree.size(), 0);
    subtreeSumDfs(root, root, tree, value, sub);
    return sub;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n = 7;
    vector<vector<int>> tree(n);
    auto add = [&](int u, int v) {
        tree[u].push_back(v);
        tree[v].push_back(u);
    };
    add(0, 1);
    add(0, 2);
    add(1, 3);
    add(1, 4);
    add(2, 5);
    add(5, 6);

    LCA lca(tree, 0);
    cout << "[lca 3 4] " << lca.query(3, 4) << '\n';
    cout << "[lca 4 6] " << lca.query(4, 6) << '\n';
    cout << "[diameter] " << treeDiameter(tree) << '\n';

    auto sums = subtreeSums(tree, {5, 3, 4, 1, 2, 7, 6});
    cout << "[subtree sums]";
    for (long long x : sums) cout << ' ' << x;
    cout << '\n';
    return 0;
}
