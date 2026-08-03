# 크루스칼 최소 스패닝 트리

## 정의와 적용 조건

연결된 가중치 무방향 그래프의 **최소 스패닝 트리(MST)** 는 모든 정점을 연결하고 사이클이 없으며 가중치 합이 최소인 부분 그래프다. 크루스칼 알고리즘은 간선을 가벼운 순서로 보면서 사이클을 만들지 않는 간선을 고른다. 방향 그래프의 도달 문제나 모든 정점 연결이 불가능한 그래프에는 그대로 적용하지 않는다. 음수 가중치는 허용된다.

## 핵심 아이디어와 불변식

선택한 간선은 항상 사이클 없는 숲이다. 현재 서로 다른 두 컴포넌트를 잇는 가장 가벼운 간선은 절단 속성에 의해 어떤 MST에도 안전하게 추가할 수 있다. 서로소 집합(DSU)은 두 정점이 같은 컴포넌트인지 빠르게 판별한다.

## 단계별 절차

1. 모든 정점을 서로 다른 집합으로 초기화한다.
2. 모든 간선을 가중치 오름차순으로 정렬한다.
3. 간선 `(u,v,w)`를 순서대로 검사한다.
4. `find(u) != find(v)`이면 간선을 선택하고 두 집합을 합친다.
5. 간선을 `V-1`개 선택하면 종료한다.

## 의사 코드

```text
sort edges by weight
answer = 0
for (u, v, w) in edges:
    if find(u) != find(v):
        union(u, v)
        answer += w
        if selected == V - 1: break
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <algorithm> // std::sort
#include <numeric>   // std::iota
#include <vector>    // std::vector

struct Edge { int u{}; int v{}; int w{}; };

class Dsu {
public:
    explicit Dsu(int n) : parent_(n), size_(n, 1) { std::iota(parent_.begin(), parent_.end(), 0); }
    int find(int x) { return parent_[x] == x ? x : parent_[x] = find(parent_[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (size_[a] < size_[b]) std::swap(a, b);
        parent_[b] = a; size_[a] += size_[b]; return true;
    }
private:
    std::vector<int> parent_;
    std::vector<int> size_;
};

long long kruskal(int n, std::vector<Edge> edges) {
    std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) { return a.w < b.w; });
    Dsu dsu{n}; long long answer{}; int selected{};
    for (const Edge& edge : edges) {
        if (dsu.unite(edge.u, edge.v)) { answer += edge.w; if (++selected == n - 1) break; }
    }
    return answer;
}

int main() { return kruskal(3, {{0, 1, 1}, {1, 2, 2}, {0, 2, 3}}) == 3 ? 0 : 1; }
```

## 정확성 근거

선택 숲의 두 컴포넌트를 나누는 절단을 생각하자. 아직 보지 않은 간선 중 이 절단을 가로지르는 가장 가벼운 간선은 절단 속성에 의해 안전하다. 크루스칼은 전체 오름차순에서 바로 그 안전 간선을 선택한다. 같은 컴포넌트의 간선은 사이클만 만들므로 제외한다. 이 과정을 `V-1`번 반복하면 연결된 사이클 없는 트리가 되고, 매 단계의 안전성으로 최소 가중치다.

## 시간·공간 복잡도

정렬은 `O(E log E)`, 경로 압축과 크기 기준 합치기를 쓴 DSU 연산 전체는 `O(E α(V))`다. 따라서 총 시간은 `O(E log E)`, 간선과 DSU 저장은 `O(V+E)`다.

## 흔한 실수

- 방향 간선처럼 한쪽만 연결된다고 해석한다.
- `find` 없이 부모 한 단계만 비교한다.
- 같은 집합의 간선도 합계에 더해 사이클을 만든다.
- 합계를 `int`로 두어 오버플로한다.
- 그래프가 비연결일 수 있는데 `V-1`개 선택 여부를 확인하지 않는다.

## 변형

- 비연결 그래프에서는 같은 절차가 최소 스패닝 포리스트를 만든다.
- 간선이 매우 많고 인접 리스트가 자연스러우면 프림 알고리즘과 비교한다.
- 최대 스패닝 트리는 간선을 내림차순으로 정렬한다.
- DSU는 동적 연결성, 사이클 판별, 오프라인 질의에도 재사용한다.

## 오늘 문제와의 연결

2026-08-04 BOJ 1197 풀이 [`../2026-08-04/icpc_problem.cpp`](../2026-08-04/icpc_problem.cpp)는 간선 정렬, 경로 압축, 크기 기준 합치기를 그대로 사용한다. `selected_edges == V-1`이면 모든 정점이 연결된 트리가 완성되어 남은 무거운 간선을 볼 필요가 없다.
