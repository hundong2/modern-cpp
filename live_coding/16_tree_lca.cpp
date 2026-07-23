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
        while ((1 << log) <= n) ++log; // 가장 높은 점프 크기가 n 이상이 되도록 log를 잡는다.
        up.assign(log, vector<int>(n, root)); // root의 조상은 root로 채워 경계 처리를 단순화한다.
        dfs(root, root);
    }

    int query(int a, int b) const {
        if (depth[a] < depth[b]) swap(a, b);

        int diff = depth[a] - depth[b];
        for (int k = 0; k < log; ++k) {
            if (diff & (1 << k)) a = up[k][a]; // 깊이 차이만큼 a를 위로 올려 두 노드의 깊이를 맞춘다.
        }

        if (a == b) return a; // 깊이를 맞춘 뒤 같아졌다면 그 노드가 LCA다.
        for (int k = log - 1; k >= 0; --k) {
            if (up[k][a] != up[k][b]) {
                a = up[k][a]; // 조상이 달라지는 가장 큰 점프부터 동시에 올린다.
                b = up[k][b];
            }
        }
        return up[0][a]; // 서로 다른 자식까지 올렸으므로 바로 위 부모가 LCA다.
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
            up[k][u] = up[k - 1][up[k - 1][u]]; // 2^k 조상은 2^(k-1) 조상의 2^(k-1) 조상이다.
        }

        for (int v : graph[u]) {
            if (v == parent) continue;
            depth[v] = depth[u] + 1;
            dfs(v, u);
        }
    }
};

pair<int, int> farthestFrom(int start, const vector<vector<int>>& tree) {
    vector<int> dist(tree.size(), -1); // -1은 아직 방문하지 않은 정점이다.
    queue<int> q;
    q.push(start);
    dist[start] = 0;

    int best = start;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (dist[u] > dist[best]) best = u; // BFS 중 가장 먼 정점을 계속 갱신한다.
        for (int v : tree[u]) {
            if (dist[v] != -1) continue;
            dist[v] = dist[u] + 1; // 트리 간선 수 기준 거리라 부모 거리 + 1이다.
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
        sub[u] += subtreeSumDfs(v, u, tree, value, sub); // 자식 서브트리 합을 현재 노드 합에 누적한다.
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
