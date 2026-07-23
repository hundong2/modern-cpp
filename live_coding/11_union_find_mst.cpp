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
        iota(parent.begin(), parent.end(), 0); // <numeric>의 iota는 0,1,2... 연속 값을 채운다. 초기 parent 배열 생성에 딱 맞다.
    }

    int find(int x) {
        if (parent[x] == x) return x;
        return parent[x] = find(parent[x]); // 경로 압축으로 다음 find 비용을 줄인다.
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (size[a] < size[b]) swap(a, b); // 작은 트리를 큰 트리 밑에 붙여 높이 증가를 억제한다.
        parent[b] = a;
        size[a] += size[b];
        return true;
    }

    bool same(int a, int b) {
        return find(a) == find(b); // 연결성 질의는 대표자가 같은지만 보면 된다.
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
    }); // Kruskal은 가장 싼 간선부터 cycle 없이 선택한다.

    DSU dsu(n);       // DSU 내부 vector는 parent/size 배열을 연속 메모리로 보관해 find 접근이 빠르다.
    long long cost = 0; // 간선 비용 합은 int를 넘을 수 있어 long long을 쓴다.
    int used = 0;     // MST는 정확히 n-1개의 간선을 사용해야 한다.

    for (const Edge& e : edges) {
        if (!dsu.unite(e.u, e.v)) continue; // 이미 연결된 두 정점을 잇는 간선은 cycle을 만든다.
        cost += e.w;
        ++used;
        if (used == n - 1) break;
    }
    return {used == n - 1, cost}; // n-1개를 못 골랐다면 그래프가 연결되어 있지 않다.
}

pair<int, int> firstRedundantEdge(int n, const vector<pair<int, int>>& edges) {
    DSU dsu(n);
    for (auto [u, v] : edges) {
        if (!dsu.unite(u, v)) return {u, v}; // unite 실패는 두 정점이 이미 같은 컴포넌트였다는 뜻이다.
    }
    return {-1, -1};
}

int countComponentsAfterUnions(int n, const vector<pair<int, int>>& unions) {
    DSU dsu(n);
    int components = n; // 처음에는 모든 정점이 독립 컴포넌트다.
    for (auto [u, v] : unions) {
        if (dsu.unite(u, v)) --components; // 실제로 합쳐졌을 때만 컴포넌트 수가 줄어든다.
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
