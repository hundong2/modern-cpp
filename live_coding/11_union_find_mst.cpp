/*
Chapter 11. Union-Find, Kruskal MST

문제 1: 동적 연결성 질의를 처리하라.
접근: Disjoint Set Union. find는 대표자를 찾고, unite는 두 집합을 합친다.
정답 해설:
- path compression과 union by size/rank를 같이 쓰면 거의 O(1)에 가깝다.

문제 2: 모든 정점을 연결하는 최소 비용을 구하라.
접근: Kruskal. 간선을 비용 오름차순으로 보며 서로 다른 컴포넌트만 연결한다.
정답 해설:
- 가장 싼 간선부터 골라도 cycle만 피하면 MST가 된다. Cut property 때문이다.
복잡도: O(E log E)

문제 3: 무방향 그래프에서 cycle을 만드는 첫 간선을 찾아라.
접근: 이미 같은 집합인 두 정점을 잇는 간선은 cycle을 만든다.
*/

#include <algorithm>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
using namespace std;

class DSU {
public:
    explicit DSU(int n) : parent(n), size(n, 1) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]);
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (size[a] < size[b]) swap(a, b);
        parent[b] = a;
        size[a] += size[b];
        return true;
    }

    bool same(int a, int b) {
        return find(a) == find(b);
    }

private:
    vector<int> parent;
    vector<int> size;
};

struct Edge {
    int u;
    int v;
    int w;
};

pair<bool, long long> kruskalMst(int n, vector<Edge> edges) {
    sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.w < b.w;
    });

    DSU dsu(n);
    long long cost = 0;
    int used = 0;

    for (const Edge& e : edges) {
        if (!dsu.unite(e.u, e.v)) continue;
        cost += e.w;
        ++used;
        if (used == n - 1) break;
    }
    return {used == n - 1, cost};
}

pair<int, int> firstRedundantEdge(int n, const vector<pair<int, int>>& edges) {
    DSU dsu(n);
    for (auto [u, v] : edges) {
        if (!dsu.unite(u, v)) return {u, v};
    }
    return {-1, -1};
}

int countComponentsAfterUnions(int n, const vector<pair<int, int>>& unions) {
    DSU dsu(n);
    int components = n;
    for (auto [u, v] : unions) {
        if (dsu.unite(u, v)) --components;
    }
    return components;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<Edge> edges = {
        {0, 1, 1},
        {0, 2, 5},
        {1, 2, 2},
        {1, 3, 4},
        {2, 3, 1}
    };
    auto [connected, cost] = kruskalMst(4, edges);
    cout << "[mst] connected=" << boolalpha << connected << " cost=" << cost << '\n';

    auto redundant = firstRedundantEdge(4, {{0, 1}, {1, 2}, {2, 0}, {2, 3}});
    cout << "[redundant] " << redundant.first << ' ' << redundant.second << '\n';
    cout << "[components] " << countComponentsAfterUnions(5, {{0, 1}, {3, 4}}) << '\n';
    return 0;
}
