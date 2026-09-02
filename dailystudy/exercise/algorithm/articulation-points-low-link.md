# 단절점과 DFS Low-Link

## 정의

무방향 그래프에서 정점 하나와 그 정점에 닿은 간선을 모두 제거했을 때 연결 요소 수가 증가하면 그 정점을 **단절점(articulation point, cut vertex)** 이라 한다. 네트워크의 단일 장애점, 도로망의 필수 교차점, 의존성 그래프의 취약 지점을 찾는 모델이다.

DFS가 정점 `v`를 처음 방문한 순서를 `discovered[v]`라 하자. `low[v]`는 DFS 트리에서 `v`의 서브트리 정점들이 **0개 이상의 트리 간선과 최대 한 개의 조상 방향 back edge**를 사용해 도달할 수 있는 가장 작은 방문 순서다. 무방향 DFS에서는 이미 방문했고 부모가 아닌 이웃으로 가는 간선이 조상으로 향하는 back edge 역할을 한다.

## 적용 조건

- 그래프가 무방향이고, 정점 하나를 제거했을 때 연결성이 깨지는지 모두 알고 싶다.
- 한 정점마다 BFS/DFS를 다시 돌리는 `O(V(V+E))` 대신 전체 선형 시간이 필요하다.
- 입력이 여러 연결 요소를 가질 수 있으면 각 미방문 정점을 DFS 루트로 시작한다.
- 방향 그래프의 강결합 취약성은 같은 공식을 그대로 쓰지 않는다. SCC, dominator tree 등 목표에 맞는 도구가 필요하다.
- 중복 간선이 있으면 “부모 정점 번호”가 아니라 “부모 간선 ID” 하나만 건너뛰어야 한다. 오늘 CSES 문제는 같은 두 정점 사이 도로가 최대 하나라 정점 비교가 충분하다.

## 핵심 아이디어와 불변식

DFS 트리의 비루트 정점 `v`에 자식 `u`가 있다고 하자.

- `low[u] < discovered[v]`이면 `u` 서브트리에서 `v`보다 먼저 방문한 조상으로 돌아가는 길이 있다. `v`가 사라져도 그 back edge를 통해 위쪽과 연결될 가능성이 있다.
- `low[u] >= discovered[v]`이면 `u` 서브트리는 `v`보다 위 조상으로 직접 갈 수 없다. `v`를 제거하면 `u` 서브트리와 DFS 트리 위쪽 사이의 모든 길이 끊기므로 `v`는 단절점이다.

DFS 루트는 부모 쪽 영역이 없으므로 별도 규칙이 필요하다. 루트의 서로 다른 DFS 자식 서브트리 사이에는 루트를 거치지 않는 간선이 있을 수 없다. 있었다면 뒤에 발견된 쪽은 독립 자식이 아니라 앞선 자식 DFS 안에서 방문됐을 것이다. 따라서 루트의 DFS 트리 자식이 둘 이상일 때만 단절점이다.

항상 유지할 불변식은 다음과 같다.

1. `discovered[v] == 0`이면 아직 DFS 트리에 들어오지 않았다.
2. 방문 직후 `low[v] == discovered[v]`다.
3. 이미 방문한 부모 아닌 이웃 `w`를 보면 `low[v] = min(low[v], discovered[w])`로 back edge를 반영한다.
4. 자식 `u`의 탐색이 완전히 끝나면 `low[v] = min(low[v], low[u])`로 자식 서브트리 도달성을 합친다.
5. 단절점 판정은 자식 탐색이 끝나 `low[u]`가 확정된 뒤에만 한다.

## 단계별 절차

1. 그래프를 양방향 인접 목록으로 만든다.
2. `discovered`, `low`, `parent`, 루트별 자식 수, 단절점 표시를 초기화한다.
3. 아직 방문하지 않은 정점을 DFS 루트로 고른다.
4. 정점을 처음 방문하면 증가하는 timer 값을 `discovered`와 `low`에 넣는다.
5. 미방문 이웃이면 부모를 기록하고 DFS 트리 자식으로 내려간다.
6. 이미 방문한 부모 아닌 이웃이면 그 방문 순서로 현재 `low`를 낮춘다.
7. 자식 탐색이 끝나 돌아오면 자식 low를 부모 low에 합치고 비루트 단절점 조건을 검사한다.
8. 루트 탐색이 끝나면 DFS 트리 자식 수가 둘 이상인지 검사한다.
9. 여러 연결 요소가 있을 수 있으면 3~8을 반복한다.

## 의사 코드

```text
timer <- 0

function dfs(v, is_root):
    discovered[v] <- low[v] <- timer + 1
    timer <- timer + 1
    child_count <- 0

    for each neighbor u of v:
        if discovered[u] == 0:
            child_count <- child_count + 1
            parent[u] <- v
            dfs(u, false)
            low[v] <- min(low[v], low[u])

            if not is_root and low[u] >= discovered[v]:
                articulation[v] <- true
        else if u is not the parent edge:
            low[v] <- min(low[v], discovered[u])

    if is_root and child_count >= 2:
        articulation[v] <- true
```

반복 구현에서는 각 정점의 “다음에 볼 인접 인덱스”를 저장한다. 인접 목록을 모두 본 정점을 명시적 스택에서 꺼내는 순간이 재귀 함수의 반환 시점과 같다. 바로 그때 자식 low를 부모에 합치고 단절점 조건을 검사한다.

## 컴파일 가능한 C++ 뼈대

아래 예제는 `1-2-3` 일자 그래프에서 정점 2만 단절점인지 검증한다. 교육용으로 재귀 관계를 가장 짧게 보이고, 입력 한계가 커 호출 스택이 위험하면 오늘 풀이처럼 명시적 스택으로 바꾼다.

```cpp
#include <algorithm> // std::min으로 low 값을 합친다.
#include <iostream>  // 검증 결과를 출력한다.
#include <vector>    // 인접 목록과 상태 배열을 소유한다.

// graph는 호출자가 소유하고 const lvalue 참조로 빌리며, 나머지 배열은 비const 참조로 갱신한다.
void find_points(int vertex,
                 bool root,
                 const std::vector<std::vector<int>>& graph,
                 std::vector<int>& discovered,
                 std::vector<int>& low,
                 std::vector<int>& parent,
                 std::vector<char>& articulation,
                 int& timer) {
    discovered[vertex] = ++timer; // int timer를 먼저 증가시킨 뒤 최초 방문 순서를 저장한다.
    low[vertex] = discovered[vertex]; // 아직 back edge를 모르므로 자기 순서로 시작한다.
    int children{}; // int{}는 0 값 초기화이며 DFS 트리 자식 수다.

    // const int는 인접 목록 원소 값을 복사하며 graph를 바꾸지 않는다.
    for (const int neighbor : graph[vertex]) {
        if (discovered[neighbor] == 0) { // 미방문 이웃은 DFS 트리 자식이 된다.
            parent[neighbor] = vertex;
            ++children;
            find_points(neighbor, false, graph, discovered, low, parent, articulation, timer);

            // std::min은 두 int를 읽고 작은 값의 참조를 반환하며 즉시 int에 복사 저장한다.
            low[vertex] = std::min(low[vertex], low[neighbor]);
            if (!root && low[neighbor] >= discovered[vertex]) {
                articulation[vertex] = 1; // 자식이 위 조상으로 돌아가지 못하므로 비루트 vertex가 단절점이다.
            }
        } else if (neighbor != parent[vertex]) {
            low[vertex] = std::min(low[vertex], discovered[neighbor]); // 부모 아닌 방문 이웃은 back edge다.
        }
    }

    if (root && children >= 2) {
        articulation[vertex] = 1; // 루트는 독립 DFS 자식이 둘 이상일 때만 단절점이다.
    }
}

int main() {
    // vector<vector<int>>가 0~3 인접 목록을 소유하고 0번은 사용하지 않는다.
    const std::vector<std::vector<int>> graph{{}, {2}, {1, 3}, {2}};
    std::vector<int> discovered(4, 0); // count=4개의 0을 복사 초기화한다.
    std::vector<int> low(4, 0);
    std::vector<int> parent(4, -1);
    std::vector<char> articulation(4, 0);
    int timer{};

    find_points(1, true, graph, discovered, low, parent, articulation, timer);
    // operator<<는 bool 비교 결과와 개행을 cout에 쓰고 같은 ostream 참조를 반환한다.
    std::cout << (articulation[2] != 0) << '\n';
    return articulation[2] != 0 && articulation[1] == 0 && articulation[3] == 0 ? 0 : 1;
}
```

재귀 호출은 함수 프레임을 실행 스레드의 호출 스택에 둔다. 깊이 N의 일자 그래프는 환경별 스택 한도를 넘을 수 있다. `vector<int> stack`과 `next_edge[v]`를 쓰면 프레임에 해당하는 상태를 동적 저장소에 두어 이 위험을 통제할 수 있다. 어떤 경우든 객체의 실제 load·store·비교·분기·호출 명령은 CPU·ABI·컴파일러·최적화에 따라 달라지며 특정 어셈블리 명령을 보장하지 않는다.

## 정확성 근거

### 보조정리 1: DFS 종료 시 low 정의가 성립한다

정점 `v`를 처음 방문할 때 자기 자신까지는 도달 가능하므로 `low[v]=discovered[v]`가 맞다. 부모가 아닌 방문 이웃으로 가는 back edge가 있으면 그 이웃 방문 순서를 후보로 넣는다. 각 DFS 자식 `u`의 탐색이 끝났을 때 귀납 가정으로 `low[u]`는 u 서브트리의 최소 도달 순서다. 이 값을 v의 후보에 합치면 v 서브트리 전체에서 가능한 모든 경우를 포함한다. 다른 종류의 간선은 무방향 DFS에서 존재하지 않으므로 종료 시 정의와 일치한다.

### 보조정리 2: 비루트 조건은 필요충분하다

비루트 `v`의 자식 `u`가 `low[u] >= discovered[v]`라면 u 서브트리에서 v보다 먼저 방문한 조상으로 가는 back edge가 없다. DFS 트리 성질상 서브트리 밖으로 가는 다른 미처리 간선도 없으므로 v를 제거하면 u 쪽이 위쪽과 분리된다. 반대로 모든 자식이 `low[u] < discovered[v]`이면 각 자식 서브트리는 v보다 위 조상으로 가는 길을 가지므로 v를 제거해도 위쪽을 통해 서로 연결될 수 있다. 따라서 어떤 자식에 조건이 성립하는 것과 v가 단절점인 것이 동치다.

### 보조정리 3: 루트 조건은 필요충분하다

루트의 DFS 트리 자식이 둘 이상이면 서로 다른 자식 서브트리 사이를 잇는 루트 없는 간선이 없다. 그런 간선이 있었다면 한 자식 DFS가 다른 쪽까지 방문해 별도 자식이 되지 않았을 것이다. 루트를 제거하면 자식 서브트리가 분리된다. 자식이 0개나 1개면 루트를 제거해도 남은 현재 연결 요소가 둘 이상으로 갈라지지 않는다.

### 정리

알고리즘은 각 비루트 정점에 보조정리 2의 조건을, 각 DFS 루트에 보조정리 3의 조건을 정확히 적용한다. 보조정리 1로 사용한 low 값이 정확하므로 표시된 정점은 모두 단절점이고 모든 단절점이 표시된다.

## 시간·공간 복잡도

- 각 정점을 한 번 발견하고 각 무방향 간선을 양쪽 인접 목록에서 한 번씩 검사하므로 시간은 `O(V+E)`다.
- 인접 목록은 `O(V+E)`, 방문·low·부모·자식·표시 배열은 `O(V)`다.
- 재귀 구현의 호출 스택 또는 반복 구현의 명시적 스택은 최악 `O(V)`다.
- 따라서 전체 보조 공간을 그래프 저장까지 포함하면 `O(V+E)`다.

## 자료구조 선택

- `vector<vector<int>>`: 실제 간선만 저장해 희소 그래프에서 인접 행렬의 `O(V²)` 낭비를 피한다.
- `vector<int> discovered/low/parent`: 정점 번호로 O(1) 접근하며 각각 한 의미만 맡긴다.
- `vector<char> articulation`: `vector<bool>`의 proxy reference 특수화를 피하고 정점당 한 바이트 표시를 쓴다.
- `vector<size_t> next_edge`: 반복 DFS에서 각 함수 프레임의 반복자 진행 위치를 보존한다.
- `vector<int> stack`: 최대 깊이를 heap 기반 저장소로 옮기고 `reserve(V)`로 재할당을 피한다.

## 흔한 실수

1. **루트에도 일반 조건 적용**: 루트의 한 자식이 `low[child] >= discovered[root]`인 것은 흔하며, 자식이 하나면 루트는 단절점이 아니다.
2. **back edge에 `low[neighbor]` 사용**: 이미 방문한 조상 이웃은 `discovered[neighbor]`를 써야 한다. 아직 처리 중인 서브트리 low를 섞으면 정의를 벗어난다.
3. **부모 간선을 back edge로 계산**: 모든 자식 low가 부모 순서까지 내려가 판정 의미가 흐려진다.
4. **자식 종료 전 판정**: low가 아직 확정되지 않아 뒤에서 발견할 back edge를 놓친다.
5. **DFS 트리 자식 수와 그래프 차수 혼동**: 루트 조건은 인접 정점 수가 아니라 처음 발견한 DFS 자식 수다.
6. **연결 그래프만 가정**: 일반 입력에서는 모든 미방문 정점을 새 루트로 시작해야 한다.
7. **중복 간선 무시**: 부모 정점으로 가는 평행 간선 둘 중 하나는 back edge다. 간선 ID로 정확히 한 간선만 제외한다.
8. **재귀 깊이 무시**: 정점 100,000 일자 그래프는 환경에 따라 호출 스택을 넘길 수 있다.
9. **vector 참조 무효화**: 명시적 stack 원소 참조를 보관한 채 push_back하면 재할당으로 댕글링될 수 있다. 값을 복사하거나 충분히 reserve한다.

## 변형과 확장

- **단절선(bridge)**: DFS 트리 간선 `(v,u)`에 대해 `low[u] > discovered[v]`면 그 간선을 제거할 때 연결성이 깨진다. 단절점의 `>=`와 다름을 주의한다.
- **2-edge-connected components**: bridge를 제거해 남는 연결 요소를 묶는다.
- **vertex-biconnected components**: DFS 간선 스택을 유지하고 `low[u] >= discovered[v]` 시점에 간선을 꺼내 block을 만든다.
- **block-cut tree**: 원래 단절점과 biconnected block을 이분 트리로 연결해 정점 제거 질의를 구조화한다.
- **온라인 장애 질의**: 단절점만 나열하는 것보다 LCA, block-cut tree, offline query가 추가로 필요할 수 있다.
- **SCC low-link와 구분**: Tarjan SCC도 `low`라는 이름을 쓰지만 방향 그래프·활성 스택 규칙과 의미가 다르다.

## 오늘 문제와의 연결

[`../2026-09-02/icpc_problem.cpp`](../2026-09-02/icpc_problem.cpp)는 [CSES 2077 Necessary Cities](https://cses.fi/problemset/task/2077)를 반복 DFS로 푼다. `next_edge[vertex]`는 재귀 함수의 for-loop 위치, `stack.back()`은 현재 함수 프레임, `pop_back()`은 함수 반환에 해당한다. pop 직후 `low[child]`를 부모에 합치고 단절점 조건을 검사한다. CSES는 연결·단순 그래프를 보장하지만 코드는 모든 미방문 루트를 순회해 더 일반적인 입력에도 안전하다.

## 직접 검증

1. 삼각형, 일자 5개, 별 4개 그래프의 `discovered`와 `low`를 손으로 적고 단절점을 표시한다.
2. 루트 별도 조건을 일반 조건으로 바꿔 삼각형의 루트가 잘못 표시되는 과정을 찾는다.
3. 일자 그래프 마지막 정점부터 low가 부모로 합쳐지는 순서를 반복 스택 상태와 함께 적는다.
4. 정점 8개 이하 무작위 연결 그래프에서 정점 하나씩 제거하고 BFS 결과를 선형 알고리즘과 비교한다.
5. 간선 ID를 추가해 중복 간선을 허용하는 버전으로 바꾸고 두 정점 사이 간선 2개인 반례를 검증한다.
6. 단절선 조건 `low[child] > discovered[parent]`을 함께 구현하고 단절점과 결과가 다른 그래프를 만든다.
7. N=100,000 일자 그래프에서 재귀와 반복 구현의 저장 위치·실행 안정성을 비교한다.
