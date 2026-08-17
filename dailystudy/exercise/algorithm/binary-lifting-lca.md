# 이진 리프팅 최소 공통 조상(Binary Lifting LCA)

## 정의

루트가 정해진 트리에서 두 정점의 최소 공통 조상(Lowest Common Ancestor, LCA)은 두 정점의 공통 조상 중 깊이가 가장 큰 정점이다. 이진 리프팅은 각 정점의 1, 2, 4, 8, ...번째 조상을 미리 저장해 한 번에 2의 거듭제곱만큼 위로 이동하는 전처리 기법이다.

## 적용 조건

- 입력 그래프가 연결되고 사이클이 없는 트리이며 루트가 정해져 있어야 한다.
- 정점이나 간선 변경 없이 LCA 질의를 많이 처리하는 정적 트리에 특히 적합하다.
- 정점 수 N과 질의 수 M이 커서 질의마다 부모를 한 칸씩 따라가는 `O(N)` 풀이가 느릴 때 사용한다.
- 동적 트리의 링크·컷이 필요하면 Link-Cut Tree 같은 다른 자료구조를 검토한다.

## 핵심 아이디어와 불변식

`up[k][v]`를 정점 v의 `2^k`번째 조상으로 정의한다. 직계 부모 `up[0][v]`를 알면 다음 점화식이 성립한다.

```text
up[k][v] = up[k-1][ up[k-1][v] ]
```

`2^k = 2^(k-1) + 2^(k-1)`이므로 절반 거리 조상에서 다시 절반 거리 조상으로 이동한 결과다.

질의 중 유지할 불변식은 다음과 같다.

1. 깊이를 맞춘 뒤 두 후보 정점의 깊이는 항상 같다.
2. 동시 점프는 두 `2^k` 조상이 서로 다를 때만 하므로 LCA 자체로 올라가거나 그 위로 건너뛰지 않는다.
3. 큰 k부터 검사한 반복이 끝나면 두 후보는 서로 다르지만 직계 부모는 같다. 그 부모가 LCA다.

## 단계별 절차

1. 인접 리스트로 무방향 트리를 저장한다.
2. 루트에서 BFS 또는 DFS를 하며 `depth[v]`와 `up[0][v]`를 정한다.
3. 점화식으로 `k=1..floor(log2 N)`의 조상 표를 채운다.
4. 질의 두 정점 중 깊은 쪽을 2의 거듭제곱 점프로 올려 깊이를 맞춘다.
5. 두 정점이 같으면 즉시 반환한다.
6. 큰 k부터 두 `up[k]`가 다를 때 두 정점을 함께 올린다.
7. 마지막 후보의 직계 부모 `up[0]`를 반환한다.

## 의사 코드

```text
BFS(root):
    depth[root] = 0
    queue.push(root)
    while queue not empty:
        v = queue.front(); queue.pop()
        for next in graph[v]:
            if depth[next] is unvisited:
                depth[next] = depth[v] + 1
                up[0][next] = v
                queue.push(next)

for k = 1 .. LOG-1:
    for v = 1 .. N:
        up[k][v] = up[k-1][up[k-1][v]]

LCA(a, b):
    make a the deeper vertex
    lift a until depth[a] == depth[b]
    if a == b: return a
    for k from LOG-1 down to 0:
        if up[k][a] != up[k][b]:
            a = up[k][a]
            b = up[k][b]
    return up[0][a]
```

## 컴파일 가능한 C++ 뼈대

```cpp
// <iostream>은 예제 결과를 출력하는 std::cout을 제공한다.
#include <iostream>
// <queue>는 BFS 방문 순서를 보존하는 std::queue를 제공한다.
#include <queue>
// <vector>는 그래프와 조상 표의 동적 저장소를 소유한다.
#include <vector>

// TreeLca는 정적 트리 전처리 결과와 질의 동작을 한 객체에 묶는다.
class TreeLca {
public:
    // 생성자는 반환형이 없고 정점 수에 맞춰 컨테이너를 초기화한다.
    explicit TreeLca(int vertex_count)
        // vector(count)는 vertex_count+1개의 빈 인접 리스트를 만든다.
        : graph_(static_cast<unsigned int>(vertex_count + 1)),
          // 깊이 -1은 아직 방문하지 않았다는 뜻이다.
          depth_(static_cast<unsigned int>(vertex_count + 1), -1),
          // 이 작은 예제는 N<16이므로 네 조상 행이면 충분하다.
          up_(4U, std::vector<int>(static_cast<unsigned int>(vertex_count + 1), 0)) {}

    // 무방향 트리 간선을 양쪽 인접 리스트에 추가한다.
    void add_edge(int from, int to) {
        // push_back은 int를 끝에 복사하며 재할당 시 기존 관찰자를 무효화할 수 있다.
        graph_[from].push_back(to);
        graph_[to].push_back(from); // 반대 방향도 저장한다.
    }

    // 루트에서 깊이와 직계 부모를 구하고 조상 점화식을 채운다.
    void build(int root) {
        // queue<int>가 처리 대기 정점을 FIFO로 소유한다.
        std::queue<int> pending{};
        depth_[root] = 0; // 루트 깊이는 0이다.
        pending.push(root); // 루트를 BFS 시작점으로 뒤에 복사한다.
        // empty는 상태를 바꾸지 않고 빈 여부 bool을 반환한다.
        while (!pending.empty()) {
            const int current{pending.front()}; // 비어 있지 않을 때 첫 값을 복사한다.
            pending.pop(); // 첫 원소를 제거하며 값을 반환하지 않는다.
            // 범위 for는 인접 vector의 모든 int를 선형 순회한다.
            for (const int next : graph_[current]) {
                if (depth_[next] != -1) {
                    continue; // 이미 방문했다면 부모 방향 간선을 건너뛴다.
                }
                depth_[next] = depth_[current] + 1;
                up_[0][next] = current; // 직계 부모를 기록한다.
                pending.push(next);
            }
        }
        // 네 행의 점화식을 작은 level부터 채운다.
        for (int level{1}; level < 4; ++level) {
            // size()는 인자 없이 부호 없는 원소 수를 O(1)에 반환하고 graph_를 바꾸지 않는다.
            for (int vertex{1}; vertex < static_cast<int>(graph_.size()); ++vertex) {
                up_[level][vertex] = up_[level - 1][up_[level - 1][vertex]];
            }
        }
    }

    // 두 정점의 최소 공통 조상을 로그 시간에 반환한다.
    [[nodiscard]] int query(int first, int second) const {
        if (depth_[first] < depth_[second]) {
            const int temporary{first};
            first = second;
            second = temporary;
        }
        // 깊은 정점의 깊이를 먼저 맞춘다.
        for (int level{3}; level >= 0; --level) {
            if (depth_[first] - (1 << level) >= depth_[second]) {
                first = up_[level][first];
            }
        }
        if (first == second) {
            return first;
        }
        // 두 조상이 다를 때만 함께 점프해 LCA 바로 아래까지 간다.
        for (int level{3}; level >= 0; --level) {
            if (up_[level][first] != up_[level][second]) {
                first = up_[level][first];
                second = up_[level][second];
            }
        }
        return up_[0][first]; // 같은 직계 부모가 LCA다.
    }

private:
    // class의 기본 접근은 private이며 각 vector가 동적 메모리를 직접 소유한다.
    std::vector<std::vector<int>> graph_{};
    std::vector<int> depth_{};
    std::vector<std::vector<int>> up_{};
};

int main() {
    TreeLca tree{5};
    tree.add_edge(1, 2);
    tree.add_edge(1, 3);
    tree.add_edge(2, 4);
    tree.add_edge(2, 5);
    tree.build(1);
    // ostream 삽입은 정답 2와 개행을 쓰고 반환 스트림 참조는 버린다.
    std::cout << tree.query(4, 5) << '\n';
    return tree.query(4, 3) == 1 ? 0 : 1;
}
```

## 정확성 근거

### 전처리의 정확성

BFS는 트리의 루트에서 각 정점까지 유일한 경로를 따라 처음 방문한다. 따라서 처음 발견한 이전 정점이 직계 부모이고, 부모 깊이에 1을 더한 값이 정확한 깊이다. `up[0]`이 정확하다고 하자. `up[k-1][v]`는 귀납 가정으로 v의 `2^(k-1)`번째 조상이고, 거기서 다시 `2^(k-1)`번째 조상으로 가면 v의 `2^k`번째 조상이다. 귀납법으로 모든 표가 정확하다.

### 질의의 정확성

깊은 정점만 조상으로 올리는 동안 두 정점의 조상 관계는 바뀌지 않으므로 LCA는 유지된다. 깊이를 맞춘 뒤 둘이 같으면 그 정점이 공통 조상 중 가장 깊다. 서로 다르면 큰 k부터 `up[k]`가 다를 때만 함께 올린다. 같은 조상으로 점프하면 그 조상이 LCA이거나 그 위일 수 있어 건너뛰므로 적용하지 않는다. 반복 종료 후 더 이상 서로 다른 조상으로 올릴 수 없으므로 두 후보의 직계 부모는 같고, 후보 자신은 다르다. 따라서 공통 직계 부모가 최소 공통 조상이다.

## 시간·공간 복잡도

- BFS: 시간 `O(N)`, 인접 리스트 공간 `O(N)`
- 조상 표: 시간 `O(N log N)`, 공간 `O(N log N)`
- 질의 하나: 시간 `O(log N)`, 추가 공간 `O(1)`
- M개 질의 전체: 시간 `O((N+M) log N)`

## 흔한 실수

- 그래프가 무방향인데 간선을 한 방향만 넣는다.
- 방문 표시 없이 부모 간선을 다시 따라가 무한 순회한다.
- 필요한 `LOG` 행을 하나 작게 잡아 가장 큰 깊이 차이를 못 올린다.
- 깊이를 맞추지 않고 두 정점을 동시에 올린다.
- `up[k][a] == up[k][b]`일 때도 점프해 LCA를 지나친다.
- 정점 번호가 1부터인데 크기를 N으로만 잡아 `operator[]` 범위를 벗어난다.

## 변형

- 두 정점 거리: `depth[a] + depth[b] - 2 * depth[lca(a,b)]`
- k번째 조상: k의 켜진 비트마다 대응하는 `up[level]`로 점프한다.
- 간선 가중치 합/최솟값/최댓값: 조상 표와 함께 각 2^k 구간의 집계값을 저장한다.
- 오일러 투어 진입·종료 시각과 결합하면 한 정점이 다른 정점의 조상인지 상수 시간에 판정할 수 있다.

## 오늘 문제와의 연결

2026-08-18의 [BOJ 11438 LCA 2 풀이](../2026-08-18/icpc_problem.cpp)는 N, M이 각각 100,000이므로 질의마다 부모를 따라가는 방식이 위험하다. BFS로 깊이와 직계 부모를 정한 뒤 동적 `maximum_log` 크기의 조상 표를 만들고 각 질의를 두 번의 역방향 level 반복으로 처리한다.

## 직접 해보기와 초보자 검증

1. 1-2-3-4-5 편향 트리의 모든 `up[k][v]` 표를 손으로 채운다.
2. 4와 3, 4와 5, 1과 5처럼 조상-자손·형제·루트 질의를 각각 추적한다.
3. `up[k][a] != up[k][b]`를 `==`로 바꾸면 어떤 예제에서 LCA를 지나치는지 찾는다.
4. 같은 정점을 두 번 질의할 때 깊이 맞추기 직후 반환되는 이유를 설명한다.
5. 위 C++ 뼈대를 빌드해 첫 출력이 2이고 종료 코드가 0인지 확인한 뒤 거리 질의를 추가한다.
