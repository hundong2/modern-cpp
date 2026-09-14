# 트리 재루팅 DP로 모든 정점의 거리 합 구하기

## 정의

트리 재루팅(rerooting) DP는 임의의 한 정점을 루트로 정해 부분 문제를 계산한 뒤, 루트를 부모에서 자식으로 한 간선씩 옮길 때 답이 어떻게 변하는지 나타내는 점화식으로 **모든 정점을 루트로 삼은 답**을 구하는 기법이다.

이 문서의 목표는 가중치가 모두 1인 n개 정점 트리에서 다음 값을 모든 정점 `v`에 대해 구하는 것이다.

```text
answer[v] = sum(dist(v, x))  (모든 정점 x)
```

각 정점에서 BFS/DFS를 새로 시작하면 `O(n^2)`이지만, 한 번의 루트 기준 전처리와 한 번의 재루팅 전파만 쓰면 `O(n)`이다.

## 적용 조건

- 그래프가 연결되고 사이클이 없는 트리여야 한다. 그래야 두 정점 사이 단순 경로가 유일하고, 한 간선을 끊었을 때 정확히 두 성분으로 나뉜다.
- 모든 정점에서 동일한 형태의 전역 값을 요구하고, 부모 답에서 자식 답으로 옮기는 변화량을 빠르게 계산할 수 있어야 한다.
- 이 거리 합 공식은 무가중치 트리 또는 모든 간선 가중치가 1인 트리에 직접 적용된다.
- 정점 수가 커서 `O(n^2)` 완전 탐색이 허용되지 않는 경우에 적합하다.
- 트리가 긴 경로일 수 있다면 재귀 DFS 깊이가 `O(n)`이 된다. 실행 환경의 호출 스택 한도를 확신할 수 없을 때는 명시적 스택과 순서 배열을 사용한다.
- 답의 최댓값은 경로 트리 끝점에서 `0+1+...+(n-1)=n(n-1)/2`이므로, `n=200000`에서는 32비트 `int`를 넘는다. 거리 합은 64비트 정수로 계산해야 한다.

일반 그래프에는 이 점화식을 그대로 쓸 수 없다. 사이클이 있으면 부모-자식 간선을 하나 건넜을 때 모든 최단거리가 반드시 정확히 1씩 변하지 않기 때문이다.

## 핵심 아이디어와 불변식

### 1. 임시 루트와 부모 우선 순서

임의로 정점 `r`을 루트로 정한다. 반복형 DFS에서 정점을 스택에서 꺼내는 즉시 `order`에 넣고, 그 뒤 자식을 스택에 넣는다. 그러면 다음 불변식이 성립한다.

> 루트가 아닌 모든 정점 `v`에 대해 `parent[v]`는 `order`에서 `v`보다 앞에 있다.

따라서 `order`를 뒤집으면 모든 자식과 더 아래 자손이 조상보다 먼저 나온다. 하나의 순서 배열이 두 방향 DP의 위상 순서 역할을 한다.

### 2. 역순 불변식: 서브트리 크기

처음에는 모든 `subtree_size[v]=1`로 둔다. `order`를 루트를 제외하고 역순으로 처리하며 다음을 수행한다.

```text
subtree_size[parent[v]] += subtree_size[v]
```

`v`를 처리할 때 모든 자손이 이미 합쳐졌으므로 `subtree_size[v]`는 완성된 값이다. 결국 각 값은 임시 루트 기준 `v` 서브트리의 정점 수가 된다.

### 3. 첫 루트의 거리 합

반복형 DFS에서 `depth[child]=depth[parent]+1`로 루트 거리를 구한다. 그러면 첫 루트의 답은 단순히 다음과 같다.

```text
answer[r] = sum(depth[v])  (모든 v)
```

거리 합에는 최대 약 `n^2/2`가 들어가므로 `depth`는 int여도 누적값은 반드시 64비트여야 한다. 구현에서는 역순 서브트리 계산과 같은 루프에서 각 `depth[v]`를 누적해도 정확하다. 덧셈의 순서는 결과에 영향을 주지 않는다.

### 4. 재루팅 점화식

현재 기준점이 부모 `u`이고 그 자식이 `v`라고 하자. 간선 `(u,v)`를 끊으면 정점은 두 집합으로 나뉜다.

- `v`의 서브트리 `subtree_size[v]`개: 기준점을 `u`에서 `v`로 옮기면 각각 1 가까워진다.
- 나머지 `n-subtree_size[v]`개: 각각 1 멀어진다.

따라서 전체 변화량은 다음과 같다.

```text
answer[v]
= answer[u] - subtree_size[v] + (n - subtree_size[v])
= answer[u] + n - 2 * subtree_size[v]
```

이 공식은 정점 자신도 자연스럽게 처리한다. `v`는 첫 집합에 속해 `u`에서 거리가 1이었다가 `v`에서 0이 되어 1 감소하고, `u`는 두 번째 집합에 속해 0에서 1로 증가한다.

### 5. 정순 불변식: 부모 답이 먼저 완성됨

`order`를 루트 다음 원소부터 정순으로 보면 항상 부모가 먼저 나온다. 그러므로 `answer[v]`를 계산하는 순간 오른쪽의 `answer[parent[v]]`가 이미 정확하다. 각 간선을 부모에서 자식 방향으로 정확히 한 번 사용해 모든 답을 전파한다.

### 6. 반복형 DFS의 스택 안전성

재귀 함수는 경로 트리에서 호출 깊이가 n이 되어 플랫폼 호출 스택을 넘길 수 있다. 명시적 `vector<int>` 스택은 힙 저장소에 최대 n개 정점 번호만 보관한다. 알고리즘의 점근 공간은 둘 다 `O(n)`이지만, 명시적 스택은 제한된 호출 스택이 아니라 프로그램이 관리하는 동적 저장소를 사용한다.

## 단계별 절차

1. 입력 간선을 양방향 인접 리스트에 저장한다.
2. 임의의 루트 `r`을 고르고 `parent[r]=r`로 둔다.
3. 명시적 스택에 `r`을 넣는다.
4. 스택이 빌 때까지 정점을 하나 꺼내 `order`에 추가한다.
5. 현재 정점의 이웃 중 부모가 아닌 정점에 부모와 루트 거리를 기록하고 스택에 넣는다.
6. 모든 `subtree_size`를 1로 시작한다.
7. `order`를 루트 직전까지 역순으로 보며 자식 크기를 부모에 합친다. 동시에 루트 거리를 64비트 합에 누적해 `answer[r]`을 만든다.
8. `order`를 루트 다음부터 정순으로 보며 `answer[v]=answer[parent[v]]+n-2*subtree_size[v]`를 적용한다.
9. 정점 번호 순서대로 답을 출력한다.

## 의사코드

```text
graph = undirected adjacency list
parent[*] = -1
subtree_size[*] = 1
depth[*] = 0
order = empty
stack = [root]
parent[root] = root

while stack is not empty:
    u = stack.pop()
    order.push(u)
    for v in graph[u]:
        if v == parent[u]:
            continue
        parent[v] = u
        depth[v] = depth[u] + 1
        stack.push(v)

root_sum = 0
for v in reverse(order without root):
    subtree_size[parent[v]] += subtree_size[v]
    root_sum += depth[v]

answer[root] = root_sum
for v in order without root:
    u = parent[v]
    answer[v] = answer[u] + n - 2 * subtree_size[v]

return answer
```

## 컴파일 가능한 C++20 뼈대

아래 예제는 CSES 1133과 같은 입력을 받아 반복형 DFS와 재루팅으로 답을 출력한다. 학습용 전체 호출 계약 주석은 날짜 폴더의 실제 제출 파일에 있고, 여기서는 알고리즘 골격을 한눈에 보이게 줄였다.

```cpp
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n{};
    std::cin >> n;
    std::vector<std::vector<int>> graph(static_cast<std::size_t>(n));

    for (int edge{}; edge < n - 1; ++edge) {
        int a{};
        int b{};
        std::cin >> a >> b;
        --a;
        --b;
        graph[static_cast<std::size_t>(a)].push_back(b);
        graph[static_cast<std::size_t>(b)].push_back(a);
    }

    std::vector<int> parent(static_cast<std::size_t>(n), -1);
    std::vector<int> subtree_size(static_cast<std::size_t>(n), 1);
    std::vector<int> depth(static_cast<std::size_t>(n), 0);
    std::vector<int> order;
    std::vector<int> stack;
    order.reserve(static_cast<std::size_t>(n));
    stack.reserve(static_cast<std::size_t>(n));

    parent[0] = 0;
    stack.push_back(0);
    while (!stack.empty()) {
        const int u{stack.back()};
        stack.pop_back();
        order.push_back(u);

        for (std::size_t i{}; i < graph[static_cast<std::size_t>(u)].size(); ++i) {
            const int v{graph[static_cast<std::size_t>(u)][i]};
            if (v == parent[static_cast<std::size_t>(u)]) {
                continue;
            }
            parent[static_cast<std::size_t>(v)] = u;
            depth[static_cast<std::size_t>(v)] = depth[static_cast<std::size_t>(u)] + 1;
            stack.push_back(v);
        }
    }

    std::int64_t root_sum{};
    for (int i{n - 1}; i > 0; --i) {
        const int v{order[static_cast<std::size_t>(i)]};
        subtree_size[static_cast<std::size_t>(parent[static_cast<std::size_t>(v)])]
            += subtree_size[static_cast<std::size_t>(v)];
        root_sum += depth[static_cast<std::size_t>(v)];
    }

    std::vector<std::int64_t> answer(static_cast<std::size_t>(n), 0);
    answer[0] = root_sum;
    for (int i{1}; i < n; ++i) {
        const int v{order[static_cast<std::size_t>(i)]};
        const int u{parent[static_cast<std::size_t>(v)]};
        answer[static_cast<std::size_t>(v)] =
            answer[static_cast<std::size_t>(u)] + static_cast<std::int64_t>(n)
            - 2LL * subtree_size[static_cast<std::size_t>(v)];
    }

    for (int v{}; v < n; ++v) {
        std::cout << answer[static_cast<std::size_t>(v)] << (v + 1 == n ? '\n' : ' ');
    }
}
```

## 정확성 근거

### 보조정리 1: 역순 처리가 정확한 서브트리 크기를 만든다

반복형 DFS가 만든 `order`에서는 모든 부모가 자식보다 앞선다. 따라서 역순에서 정점 `v`를 처리할 때 `v`의 모든 자손은 이미 처리되어 `subtree_size[v]`에 합쳐졌다. 초기값 1은 `v` 자신을 세며, 완성된 값을 부모에 정확히 한 번 더한다. 귀납적으로 모든 `subtree_size[v]`는 임시 루트 기준 서브트리 정점 수와 같다.

### 보조정리 2: 첫 루트의 답이 정확하다

트리에서 루트 `r`부터 정점 `v`까지의 단순 경로는 유일하다. DFS가 부모에서 자식으로 갈 때마다 `depth`를 1 늘리므로 `depth[v]=dist(r,v)`다. 모든 `depth[v]`를 합친 값은 정의에 의해 `answer[r]`이며, 루트 자신의 거리 0을 생략해도 합은 같다.

### 보조정리 3: 재루팅 점화식이 부모의 정확한 답을 자식의 정확한 답으로 바꾼다

부모 `u`에서 자식 `v`로 기준점을 옮긴다고 하자. `(u,v)`를 끊으면 모든 정점은 `v` 서브트리와 그 밖의 집합 중 정확히 하나에 속한다. 첫 집합의 `subtree_size[v]`개 정점까지의 거리는 각각 1 감소하고, 두 번째 집합의 `n-subtree_size[v]`개 정점까지의 거리는 각각 1 증가한다. 따라서 총 변화는 `-subtree_size[v]+n-subtree_size[v]=n-2*subtree_size[v]`이고 점화식이 정확하다.

### 보조정리 4: 정순 전파는 모든 정점의 답을 정확히 한 번 계산한다

첫 루트의 답은 보조정리 2로 정확하다. `order`에서 부모가 자식보다 앞서므로 어떤 자식 `v` 차례에도 부모 `u`의 답이 이미 정확하다. 보조정리 3을 적용하면 `v`의 답도 정확하다. 루트에서의 순서에 대한 귀납으로 모든 정점 답이 정확하고, 루트가 아닌 각 정점은 자신의 유일한 부모 간선에서 한 번만 계산된다.

### 정리

보조정리 1이 점화식에 필요한 모든 서브트리 크기를 제공하고, 보조정리 2가 귀납의 시작값을 제공하며, 보조정리 3과 4가 그 값을 전체 트리에 정확히 전파한다. 따라서 알고리즘이 출력하는 각 `answer[v]`는 `v`에서 모든 정점까지의 거리 합이다.

## 시간·공간 복잡도

- 인접 리스트 구성: 무방향 간선 n-1개를 두 번 저장하므로 `O(n)` 시간과 공간이다.
- 반복형 DFS: 정점마다 한 번 push/pop하고 방향 간선 `2(n-1)`개를 한 번씩 보므로 `O(n)`이다.
- 역순 서브트리 계산과 루트 거리 합: 정점마다 한 번이므로 `O(n)`이다.
- 정순 재루팅: 루트가 아닌 정점마다 한 번이므로 `O(n)`이다.
- **총 시간 복잡도: `O(n)`**
- 그래프, 부모, 순서, 스택, 크기, 거리, 답 배열이 각각 선형이므로 **총 추가 공간: `O(n)`**이다.
- 재귀 호출을 쓰지 않으므로 입력 깊이에 비례하는 호출 스택은 사용하지 않는다.

## 흔한 실수

1. 각 정점에서 DFS/BFS를 다시 시작해 `O(n^2)`가 된다.
2. `answer[v] = answer[u] + n - subtree_size[v]`처럼 가까워지는 정점의 감소분을 한 번만 반영해 `2*subtree_size[v]`를 놓친다.
3. 서브트리 크기를 정순으로 합쳐 자식 크기가 아직 완성되기 전에 부모에 더한다.
4. 재루팅 답을 역순으로 계산해 부모 답이 준비되지 않은 상태에서 읽는다.
5. 무방향 인접 리스트에서 부모 간선을 건너뛰지 않아 부모와 자식을 계속 다시 방문한다.
6. 일반 그래프에서도 부모만 제외하면 된다고 생각한다. 사이클이 있으면 별도 visited가 필요하고 재루팅 공식 자체도 성립하지 않는다.
7. `order`나 명시적 스택의 capacity를 고려하지 않아도 점근 복잡도는 맞지만, 큰 입력에서 불필요한 재할당 비용을 낸다.
8. 거리 합을 `int`에 저장해 경로 트리에서 signed overflow를 일으킨다. C++의 signed integer overflow는 미정의 동작이다.
9. 입력의 1-based 정점 번호와 내부 0-based 인덱스를 섞는다.
10. `n=1`을 빠뜨린다. 역순/정순 루프는 모두 비고 유일한 답은 0이어야 한다.
11. 경로 트리에 재귀 DFS를 사용해 논리적으로는 맞지만 실행 환경의 호출 스택을 넘긴다.
12. `subtree_size[v]`를 현재 재루팅된 루트 기준으로 다시 구해야 한다고 오해한다. 점화식은 처음 고정한 임시 루트 기준 크기를 그대로 쓴다.

## 변형

- **가중 트리**: 부모-자식 간선 가중치가 `w`이면 `answer[v]=answer[u]+w*(n-2*subtree_size[v])`다. 첫 루트 거리도 가중치 합으로 계산한다.
- **정점 가중치가 있는 거리 합**: 각 정점 `x`의 가중치가 `weight[x]`라면 서브트리의 정점 수 대신 서브트리 가중치 합 `subtree_weight[v]`를 쓴다. 총가중치를 `W`라 할 때 변화량은 `W-2*subtree_weight[v]`다.
- **거리 제곱합**: 단순히 크기만으로는 부족하다. 루트를 옮길 때 각 집합의 기존 거리 합도 필요하므로 DP 상태를 늘려야 한다.
- **모든 정점의 최대 거리**: 거리 합 점화식 대신 아래 방향 최댓값과 위쪽 기여를 합치는 prefix/suffix 또는 상위 두 최댓값 재루팅을 쓴다.
- **일반 결합형 트리 DP**: 자식 기여의 결합이 결합법칙을 만족하면, 각 정점에서 이웃별 prefix/suffix 결합을 만들어 부모 방향 기여까지 `O(n)`에 전달할 수 있다.
- **재귀 구현**: 전형적인 두 DFS가 더 짧지만, n이 크고 트리 높이가 제한되지 않았다면 반복형 구현이 제출 안정성이 높다.
- **여러 테스트 케이스**: `order`, `stack`, 부모와 DP 배열을 케이스마다 정확히 초기화하고 이전 그래프 저장소가 섞이지 않게 해야 한다.

## 오늘 문제와의 연결

[CSES 1133 Tree Distances II](https://cses.fi/problemset/task/1133/)는 n이 최대 200,000이라 모든 시작점 탐색이 불가능하다. 오늘의 [`../2026-09-15/icpc_problem.cpp`](../2026-09-15/icpc_problem.cpp)는 1번 정점을 임시 루트로 잡고 다음 세 문장을 코드 불변식으로 사용한다.

1. `order`에서는 부모가 자식보다 먼저 나온다.
2. 역순에서는 자식의 서브트리 크기가 완성되어 있고, 정순에서는 부모의 답이 완성되어 있다.
3. 부모에서 자식으로 옮길 때 답의 변화량은 `n-2*subtree_size[child]`다.

실전에서는 이 세 문장과 64비트 필요성부터 적은 뒤 구현하면 순회 방향과 부호를 뒤집는 실수를 크게 줄일 수 있다. 날짜별 제출 파일은 경로 트리에서도 안전하도록 두 순회를 모두 반복형으로 구현한다.

## 직접 해보기

1. 예제 트리를 1번에 루팅하고 `parent`, `order`, `depth`, `subtree_size`, `answer`를 손으로 채운다.
2. 경로 `1-2-3-4`에서 첫 루트를 1로 잡아 답 `6,4,4,6`이 점화식으로 나오는지 확인한다.
3. 중심 1과 잎 2,3,4,5인 별 모양 트리에서 중심 답 4, 각 잎 답 7이 되는 이유를 두 집합 변화량으로 설명한다.
4. `n=1`, 두 정점, 경로, 별, 균형 이진 트리를 각각 만들어 `O(n^2)` brute-force BFS 결과와 비교한다.
5. 무작위 트리를 수천 개 생성해 선형 재루팅 결과와 모든 시작점 BFS 결과를 대조하는 oracle 테스트를 작성한다.
6. 각 간선에 가중치를 붙이고 점화식에 `w`를 곱한 뒤 작은 입력을 손으로 검증한다.
7. `order`를 정순으로 서브트리 크기를 합쳤을 때 틀리는 가장 작은 반례를 찾는다.
8. 재귀 버전과 반복형 버전을 정점 200,000개의 경로에서 실행해 호출 스택 안전성을 비교한다.
