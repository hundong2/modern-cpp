# 트리 차분으로 여러 노드 경로 사용 횟수 세기

## 정의

트리에서 여러 경로 `(u, v)`가 주어질 때 각 정점이 몇 개의 경로에 포함되는지 구하는 문제를 생각하자. 경로마다 실제 정점을 모두 걸으면 일자 트리의 `1 → n` 경로가 반복될 때 `O(nm)`이 된다. **트리 차분(tree difference)** 은 경로마다 상수 개의 표식만 남기고, 마지막에 자식 서브트리 합을 부모로 한 번 모아 모든 경로의 영향을 동시에 복원한다.

오늘 다루는 것은 **정점 사용 횟수**다. 루트를 정하고 `w = LCA(u,v)`라 하면 한 경로의 표식은 다음과 같다.

```text
delta[u]              += 1
delta[v]              += 1
delta[w]              -= 1
if w is not root:
    delta[parent[w]]  -= 1
```

모든 경로의 표식을 더한 뒤 postorder로 `delta[child]`를 `delta[parent]`에 누적하면 `delta[x]`가 정점 `x`를 포함하는 경로 수가 된다.

## 적용 조건

- 그래프가 연결된 무방향 트리라서 두 정점 사이 단순 경로가 정확히 하나다.
- 모든 경로 질의를 먼저 읽고 전체 정점의 최종 사용 횟수를 한꺼번에 출력할 수 있다.
- 온라인으로 질의 직후 답해야 하거나 간선 삽입·삭제가 있으면 Heavy-Light Decomposition, Fenwick/segment tree, link-cut tree 같은 다른 구조가 필요하다.
- `LCA`를 빠르게 구할 수 있어야 한다. 오늘은 [`binary-lifting-lca.md`](binary-lifting-lca.md)의 이진 리프팅을 사용한다.
- 입력 크기가 커 재귀 깊이가 위험하면 부모·깊이·순서를 명시적 스택으로 만든다.

## 핵심 아이디어와 불변식

루트 트리에서 정점 `x`의 최종 값은 `x`의 서브트리 안에 놓인 모든 차분 표식의 합이다.

```text
answer[x] = sum(delta[y]) for every y in subtree(x)
```

한 경로 `(u,v)`만 놓고 이 합을 관찰한다. `w=LCA(u,v)`다.

- `u`에서 `w` 직전까지의 정점 `x`는 자기 서브트리에 `+1 at u`를 포함하지만 `w`의 감산은 포함하지 않으므로 합이 `1`이다.
- `v` 쪽도 대칭적으로 `1`이다.
- `x=w`의 서브트리는 두 끝점 `+2`와 `delta[w] -= 1`을 포함하고 `parent[w]`는 포함하지 않아 합이 `1`이다.
- `w`의 엄격한 조상 `x`는 `+2 -1 -1 = 0`이다.
- 경로와 무관한 서브트리는 네 표식 어느 것도 불균형하게 포함하지 않아 `0`이다.
- `w`가 루트이면 엄격한 조상이 없으므로 `parent[w]` 감산을 생략한다. 루트 값은 `+2-1=1`이다.

여러 경로는 덧셈의 선형성 때문에 표식을 모두 합친 뒤 한 번만 누적해도 각 경로를 따로 복원한 결과와 같다.

반복 전처리에서 유지할 불변식은 다음과 같다.

1. `parent[root] = root`, `depth[root] = 0`이다.
2. 트리 간선에서 부모만 건너뛰면 나머지 이웃은 아직 방문하지 않은 자식이다.
3. 정점을 명시적 스택에서 꺼낼 때 `order`에 넣으면 부모가 항상 자식보다 앞선다.
4. 따라서 `order`를 역순으로 읽으면 모든 자식이 부모보다 먼저 처리된다.
5. `up[k][v]`는 정점 `v`의 `2^k`번째 조상이다.
6. 모든 질의를 반영한 후 역순 누적을 마친 정점은 자기 서브트리 표식 합을 정확히 가진다.

## 단계별 절차

1. 인접 목록으로 트리를 양방향 저장한다.
2. 루트 `1`을 명시적 스택에 넣고 반복 DFS를 수행한다.
3. 각 자식의 `parent`, `depth`를 기록하고 부모-선행 `order`를 만든다.
4. `up[0][v]=parent[v]`, `up[k][v]=up[k-1][up[k-1][v]]`로 조상 표를 만든다.
5. LCA 질의에서는 깊이를 맞춘 뒤 두 정점을 큰 점프부터 함께 올린다.
6. 각 경로 `(u,v)`에 노드 차분 공식 네 항을 적용한다. `LCA==root`이면 부모 항만 생략한다.
7. `order`를 뒤에서 앞으로 읽어 root 직전까지 `delta[parent[v]] += delta[v]`를 수행한다.
8. `delta[1..n]`을 출력한다.

## 의사코드

```text
root <- 1
parent[root] <- root
stack.push(root)

while stack is not empty:
    v <- stack.back(); stack.pop()
    order.push(v)
    for each neighbor u of v:
        if u == parent[v]: continue
        parent[u] <- v
        depth[u] <- depth[v] + 1
        stack.push(u)

up[0] <- parent
for k from 1 to LOG-1:
    for v from 1 to n:
        up[k][v] <- up[k-1][up[k-1][v]]

for each path (u,v):
    w <- lca(u,v)
    delta[u] <- delta[u] + 1
    delta[v] <- delta[v] + 1
    delta[w] <- delta[w] - 1
    if w != root:
        delta[parent[w]] <- delta[parent[w]] - 1

for v in reverse(order), excluding root:
    delta[parent[v]] <- delta[parent[v]] + delta[v]
```

## 컴파일 가능한 C++ 뼈대

다음 예제는 일자 트리 `1-2-3-4-5`에서 `(1,5)`, `(2,4)`, `(3,3)`을 세어 `1 2 3 2 1`을 검증한다. 실제 제출 코드는 동적 입력을 받지만 핵심 불변식은 같다.

```cpp
#include <cstddef>
#include <iostream>
#include <vector>

int main() {
    constexpr int n{5};
    constexpr int log{3};
    std::vector<std::vector<int>> graph(static_cast<std::size_t>(n) + 1U);
    for (int vertex{1}; vertex < n; ++vertex) {
        graph[static_cast<std::size_t>(vertex)].push_back(vertex + 1);
        graph[static_cast<std::size_t>(vertex + 1)].push_back(vertex);
    }

    std::vector<int> parent(static_cast<std::size_t>(n) + 1U, 0);
    std::vector<int> depth(static_cast<std::size_t>(n) + 1U, 0);
    std::vector<int> order;
    std::vector<int> stack{1};
    parent[1] = 1;
    while (!stack.empty()) {
        const int vertex{stack.back()};
        stack.pop_back();
        order.push_back(vertex);
        for (const int next : graph[static_cast<std::size_t>(vertex)]) {
            if (next == parent[static_cast<std::size_t>(vertex)]) {
                continue;
            }
            parent[static_cast<std::size_t>(next)] = vertex;
            depth[static_cast<std::size_t>(next)] = depth[static_cast<std::size_t>(vertex)] + 1;
            stack.push_back(next);
        }
    }

    std::vector<std::vector<int>> up(
        log, std::vector<int>(static_cast<std::size_t>(n) + 1U, 0));
    for (int vertex{1}; vertex <= n; ++vertex) {
        up[0][static_cast<std::size_t>(vertex)] = parent[static_cast<std::size_t>(vertex)];
    }
    for (int level{1}; level < log; ++level) {
        for (int vertex{1}; vertex <= n; ++vertex) {
            const int middle{up[static_cast<std::size_t>(level - 1)]
                                [static_cast<std::size_t>(vertex)]};
            up[static_cast<std::size_t>(level)][static_cast<std::size_t>(vertex)] =
                up[static_cast<std::size_t>(level - 1)][static_cast<std::size_t>(middle)];
        }
    }

    const auto lca = [&depth, &up](int a, int b) {
        if (depth[static_cast<std::size_t>(a)] < depth[static_cast<std::size_t>(b)]) {
            const int temporary{a};
            a = b;
            b = temporary;
        }
        const int gap{depth[static_cast<std::size_t>(a)] - depth[static_cast<std::size_t>(b)]};
        for (int level{}; level < log; ++level) {
            if ((gap & (1 << level)) != 0) {
                a = up[static_cast<std::size_t>(level)][static_cast<std::size_t>(a)];
            }
        }
        if (a == b) {
            return a;
        }
        for (int level{log - 1}; level >= 0; --level) {
            if (up[static_cast<std::size_t>(level)][static_cast<std::size_t>(a)] !=
                up[static_cast<std::size_t>(level)][static_cast<std::size_t>(b)]) {
                a = up[static_cast<std::size_t>(level)][static_cast<std::size_t>(a)];
                b = up[static_cast<std::size_t>(level)][static_cast<std::size_t>(b)];
            }
        }
        return up[0][static_cast<std::size_t>(a)];
    };

    std::vector<long long> delta(static_cast<std::size_t>(n) + 1U, 0LL);
    const int paths[3][2]{{1, 5}, {2, 4}, {3, 3}};
    for (const auto& path : paths) {
        const int ancestor{lca(path[0], path[1])};
        ++delta[static_cast<std::size_t>(path[0])];
        ++delta[static_cast<std::size_t>(path[1])];
        --delta[static_cast<std::size_t>(ancestor)];
        if (ancestor != 1) {
            --delta[static_cast<std::size_t>(parent[static_cast<std::size_t>(ancestor)])];
        }
    }

    for (int index{n - 1}; index > 0; --index) {
        const int vertex{order[static_cast<std::size_t>(index)]};
        delta[static_cast<std::size_t>(parent[static_cast<std::size_t>(vertex)])] +=
            delta[static_cast<std::size_t>(vertex)];
    }

    for (int vertex{1}; vertex <= n; ++vertex) {
        std::cout << delta[static_cast<std::size_t>(vertex)] << (vertex == n ? '\n' : ' ');
    }
}
```

## 정확성 근거

### 보조정리 1: 반복 전처리의 parent, depth, order가 정확하다

루트의 부모와 깊이는 정의대로 설정한다. 트리에는 사이클이 없으므로 정점 `v`에서 부모 간선을 제외한 모든 인접 정점은 `v`의 아직 발견되지 않은 자식이다. 따라서 기록한 부모는 루트까지의 유일 경로에서 바로 앞 정점이고 깊이는 부모 깊이보다 1 크다. 자식은 부모를 처리한 뒤 stack에 들어가므로 `order`에서는 부모가 자식보다 항상 앞선다.

### 보조정리 2: 이진 리프팅 LCA가 정확하다

`up[0][v]`는 정의상 한 칸 부모다. `up[k-1]`이 `2^(k-1)`번째 조상이라는 귀납 가정 아래 이를 두 번 적용한 `up[k][v]`는 `2^k`번째 조상이다. 깊이 차의 켜진 비트만큼 깊은 정점을 올리면 두 깊이가 같아진다. 두 정점이 다르면 큰 단계부터 조상이 서로 다른 경우만 함께 올려 LCA 바로 아래에서 멈추므로 한 칸 부모가 LCA다.

### 보조정리 3: 한 경로의 서브트리 합은 경로 위에서만 1이다

위 핵심 불변식의 다섯 위치 관계를 적용하면 `(u,v)` 경로 위 정점의 서브트리 표식 합은 정확히 1이고, 경로 밖 정점은 0이다. 특히 LCA에서는 두 끝점의 `+2`와 자기 `-1`만 포함해 1이며, 엄격한 조상에서는 LCA 부모의 `-1`까지 포함해 0이다. LCA가 루트이면 엄격한 조상이 없으므로 부모 감산을 생략하는 것이 정확하다.

### 보조정리 4: 역순 누적이 모든 서브트리 합을 계산한다

보조정리 1에 따라 역순 order에서 모든 자식은 부모보다 먼저 처리된다. 정점 `v`를 처리할 때 `delta[v]`에는 이미 모든 자식 서브트리 표식 합이 들어 있다. 이를 부모에 더하면 부모가 지금까지 처리한 자식 전체의 표식을 포함한다. 귀납적으로 루트까지 모든 정점이 정확한 서브트리 합을 얻는다.

### 정리

보조정리 3에서 한 경로의 누적 결과가 정확하고 덧셈은 선형이다. 모든 경로의 표식을 합한 뒤 보조정리 4의 누적을 한 번 수행한 값은 각 경로를 따로 복원해 더한 값과 같다. 따라서 출력 `delta[v]`는 정점 `v`를 포함하는 경로 수다.

## 시간·공간 복잡도

- 반복 DFS와 마지막 누적: `O(n)`
- 이진 리프팅 표 구성: `O(n log n)`
- 경로당 LCA와 상수 개 차분 갱신: `O(log n)`, 총 `O(m log n)`
- 전체 시간: `O((n+m) log n)`
- 인접 목록·배열·스택: `O(n)`, 조상 표: `O(n log n)`
- 전체 공간: `O(n log n)`

`n=200000`이면 `LOG=19`이고 `int` 조상 표는 약 `200000 × 19 × 4 ≈ 15.2MB`다. 한 정점 답은 최대 `m`이지만 차분과 누적은 `long long`으로 두면 합산 타입을 바꾸는 변형에도 안전 여유가 있다.

## 자료구조 선택

- `vector<vector<int>> graph`: 희소한 트리의 `2(n-1)` 방향 항목만 저장한다.
- `vector<int> parent/depth/order/stack`: 정점 번호로 O(1) 접근하고 반복 DFS 프레임을 heap 저장소로 옮긴다.
- `vector<vector<int>> up`: `up[k][v]` 의미가 코드에서 직접 보이는 교육적 배치다. 성능 극한에서는 평탄한 `vector<int>` 하나로 cache locality와 할당 횟수를 개선할 수 있다.
- `vector<long long> delta`: 음수 임시 표식과 양수 누적을 모두 한 타입에 담는다. unsigned 타입은 감산 표식에 부적합하다.

## 흔한 실수

1. **정점 공식과 간선 공식 혼동**: 정점 횟수는 `-1 at LCA`, `-1 at parent(LCA)`다. 간선 횟수의 `-2 at LCA`를 쓰면 LCA 정점을 잃는다.
2. **root의 부모까지 감산**: `parent[root]=root`일 때 무조건 부모 감산을 하면 root에 총 `-2`가 되어 root 포함 경로를 0으로 만든다.
3. **root를 부모에 다시 누적**: 역순 루프에서 root까지 처리하면 `delta[root] += delta[root]`가 된다.
4. **order 방향 반대**: 차분 복원은 부모→자식이 아니라 자식→부모다.
5. **재귀 깊이 무시**: 길이 200000인 일자 트리는 환경의 호출 스택을 넘을 수 있다.
6. **그래프 입력을 일반 그래프로 확장하면서 visited 생략**: 부모만 건너뛰는 순회는 트리 전제에서만 안전하다.
7. **조상 표 높이 부족**: 가장 큰 깊이 차의 최상위 비트까지 포함해야 한다.
8. **답 배열을 unsigned로 선언**: 최종 답은 음수가 아니어도 중간 차분에는 `-1`이 필요하다.
9. **각 질의 경로 직접 순회**: 별/일자 반례에서 `O(nm)`이 된다.
10. **LCA 깊이 맞춤 누락**: 같은 높이로 올리지 않고 동시 점프하면 조상-자손 질의를 틀린다.

## 변형

- **간선별 경로 사용 횟수**: `delta[u]++`, `delta[v]++`, `delta[lca]-=2` 후 누적한다. 비루트 정점 `v`의 누적값이 간선 `(parent[v],v)` 사용 횟수다.
- **가중 경로 추가**: 각 질의 가중치 `w`를 `±1` 대신 `±w`로 적용한다.
- **경로 위 값 일괄 더하기 후 정점 최종값**: 같은 차분 원리로 오프라인 덧셈을 처리한다.
- **온라인 질의/갱신**: Euler tour와 Fenwick tree 또는 Heavy-Light Decomposition으로 바꾼다.
- **서브트리 질의와 혼합**: Euler 구간 차분과 경로 차분을 별도 배열에 모아 마지막에 결합할 수 있다.
- **메모리 최적화 LCA**: Euler tour + RMQ, offline Tarjan LCA, 평탄한 조상 표를 제약에 맞춰 선택한다.

## 오늘 문제와의 연결

[`../2026-10-01/icpc_problem.cpp`](../2026-10-01/icpc_problem.cpp)는 [CSES 1136 Counting Paths](https://cses.fi/problemset/task/1136/)를 이 방식으로 해결한다. `order`가 부모-선행임을 이용해 역순 인덱스 `node_count-1 .. 1`만 처리하고, LCA가 root일 때 부모 감산을 생략한다. 각 질의는 조상 표를 `O(log n)`번 읽고 차분 배열 네 칸만 갱신하므로 경로 길이에 의존하지 않는다.

## 직접 검증

1. 단일 정점에서 `(1,1)`을 세 번 넣고 root 부모 감산을 생략해야 답이 3인지 확인한다.
2. 일자 트리 5개와 경로 `(1,5)`, `(2,4)`, `(3,3)`의 표식과 역누적을 손으로 적는다.
3. 별 트리에서 리프-리프 경로 두 개를 넣고 root가 정확히 두 번 세어지는지 확인한다.
4. `delta[lca]-=2`로 잘못 바꿔 공식 예제 어느 정점이 틀리는지 찾는다.
5. 정점 30개 이하 무작위 트리에서 각 질의를 BFS predecessor로 복원하는 느린 oracle과 대조한다.
6. 길이 200000 일자 트리에서 `(1,n)`을 200000번 넣어 모든 답이 200000인지 확인한다.
7. 같은 일자 트리에서 `(n,n)`만 반복해 마지막 정점만 200000인지 확인한다.
8. 재귀 버전과 반복 버전의 저장 위치, 최대 깊이, 복잡도, 실패 가능성을 비교한다.
