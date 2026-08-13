# 분리 집합(Disjoint Set Union, Union-Find)

## 정의와 적용 조건

서로 겹치지 않는 여러 집합을 관리하면서 두 원소가 같은 집합인지 묻고 두 집합을 합치는 자료구조다. 간선이 추가되기만 하는 연결성 질의, 크루스칼 알고리즘의 사이클 검사에 알맞다. 원소 삭제나 간선 삭제가 필요한 동적 연결성에는 그대로 적용하기 어렵다.

## 핵심 아이디어와 불변식

각 원소는 부모 번호를 저장해 루트가 대표인 숲을 만든다. 핵심 불변식은 `parent[root] == root`이고, 두 원소의 대표가 같을 때와 오직 그때만 같은 집합이다. `find`의 경로 압축은 방문한 노드를 대표에 바로 연결하고, `unite`의 랭크/크기 기준 합치기는 작은 트리를 큰 트리 아래 붙여 높이를 억제한다.

## 단계별 절차

1. 모든 `parent[i]`를 `i`로 초기화한다.
2. `find(x)`는 부모를 따라 루트를 찾고 돌아오며 경로를 루트로 압축한다.
3. `unite(a,b)`는 두 대표를 구한다.
4. 대표가 같으면 아무것도 하지 않는다.
5. 다르면 랭크가 낮은 루트를 높은 루트 아래 붙이고, 랭크가 같았으면 새 루트 랭크를 늘린다.

## 의사 코드

```text
find(x):
  if parent[x] != x: parent[x] = find(parent[x])
  return parent[x]

unite(a, b):
  ra = find(a), rb = find(b)
  if ra == rb: return false
  낮은 랭크 루트를 높은 랭크 루트 아래에 연결
  return true
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <numeric> // std::iota
#include <vector>  // std::vector

class Dsu {
public:
    explicit Dsu(int n) : parent_(n), size_(n, 1) {
        std::iota(parent_.begin(), parent_.end(), 0);
    }
    int find(int x) {
        if (parent_[x] != x) parent_[x] = find(parent_[x]);
        return parent_[x];
    }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (size_[a] < size_[b]) { int temp{a}; a = b; b = temp; }
        parent_[b] = a;
        size_[a] += size_[b];
        return true;
    }
private:
    std::vector<int> parent_{};
    std::vector<int> size_{};
};

int main() {
    Dsu dsu{4};
    dsu.unite(0, 1);
    return dsu.find(0) == dsu.find(1) ? 0 : 1;
}
```

## 정확성 근거

초기에는 각 원소가 자신의 대표라 집합 표현이 정확하다. 합치기는 서로 다른 두 대표 중 하나의 부모만 다른 대표로 바꾸므로 두 집합의 모든 원소가 하나의 대표를 공유하고 다른 집합에는 영향을 주지 않는다. 경로 압축은 노드를 이미 속한 집합의 동일 대표로 연결하므로 소속 관계를 바꾸지 않는다. 따라서 연산 뒤에도 불변식과 질의 결과가 유지된다.

## 복잡도

초기화는 O(n), 공간은 O(n)이다. 경로 압축과 랭크/크기 기준 합치기를 함께 쓰면 연산당 상각 O(alpha(n))이며, 역 아커만 함수는 현실적인 입력에서 매우 작다. 둘 다 쓰지 않으면 트리가 길어져 한 연산이 O(n)에 가까워질 수 있다.

## 흔한 실수

- `parent[a] = b`처럼 원소를 바로 연결하고 대표를 찾지 않는다.
- 랭크와 실제 집합 크기를 혼동한다.
- 크기 기반 합치기에서 새 루트의 크기만 갱신하지 않는다.
- 0부터 n까지 쓰는 문제에서 벡터를 n칸만 만든다.
- 재귀 `find`가 부담인 환경의 매우 깊은 초기 트리를 고려하지 않는다.

## 변형

집합 크기 저장, 연결 시점 기록, 가중치/퍼텐셜 DSU, 롤백 DSU가 있다. 삭제가 섞인 오프라인 동적 연결성에는 시간 구간 분할과 롤백 DSU를 결합할 수 있다.

## 오늘 문제와의 연결

2026-08-13의 BOJ 1717은 합집합과 동일 집합 질의를 그대로 요구한다. `icpc_problem.cpp`는 경로 압축과 랭크 합치기를 사용해 최대 입력에서도 빠르게 처리한다.

직접 해보기: 원소 0~5에서 `(0,1)`, `(2,3)`, `(1,3)`을 합치며 부모 배열을 적고, 모든 원소에 `find`를 호출한 뒤 배열이 어떻게 평평해지는지 확인하라.
