# 코사라주 강결합 요소(Strongly Connected Components)

## 정의

방향 그래프에서 정점 `u`에서 `v`로 가는 경로와 `v`에서 `u`로 가는 경로가 모두 존재하면 두 정점은 같은 **강결합 요소(SCC)**에 속한다. “서로 도달 가능”은 동치 관계이므로 모든 정점은 겹치지 않는 SCC들로 유일하게 분할된다. 각 SCC를 정점 하나로 압축하면 결과 그래프는 반드시 방향 비순환 그래프(DAG)가 된다.

## 적용 조건

- 방향 그래프에서 서로 왕복 가능한 정점 묶음을 찾아야 할 때 사용한다.
- 의존성 순환 탐지, 웹 링크/상태 전이의 상호 도달 집단, 2-SAT의 변수·부정 리터럴 판정에 적용한다.
- SCC 압축 DAG에서 위상 정렬, 동적 계획법, 원천·싱크 컴포넌트 계산을 이어서 할 때 유용하다.
- 무방향 연결 요소는 일반 DFS/BFS로 충분하며 SCC 알고리즘이 필요하지 않다.

## 핵심 아이디어와 불변식

코사라주는 원래 그래프 `G`와 모든 간선을 뒤집은 역그래프 `G^R`를 사용한다.

1. `G`에서 DFS를 수행하고 정점을 **종료할 때** 순서 목록에 넣는다.
2. 종료 순서의 역순으로 정점을 고른다.
3. 아직 배정되지 않은 정점에서 `G^R` DFS를 하면 정확히 하나의 SCC가 모인다.

SCC 압축 DAG를 생각하면 첫 단계에서 늦게 종료한 정점은 아직 처리하지 않은 압축 DAG의 원천 SCC에 속한다. 원래 그래프에서 원천이던 SCC는 역그래프에서 싱크가 된다. 따라서 그 SCC에서 역그래프 DFS를 시작해도 다른 미배정 SCC로 나갈 수 없고, SCC 내부 정점은 상호 도달 가능하므로 전부 모인다.

유지해야 할 불변식은 다음과 같다.

- 첫 DFS에서 `visited[v] == true`인 정점은 종료 순서에 정확히 한 번만 들어간다.
- 두 번째 DFS에서 `visited[v] == true`인 정점은 이미 정확히 한 SCC에 배정되었다.
- 종료 순서 역순에서 처음 만나는 미배정 정점은 남은 SCC 압축 DAG의 원천 SCC에 속한다.

## 단계별 절차

1. 정점 `1..V`의 정방향 인접 리스트와 역방향 인접 리스트를 만든다.
2. 모든 미방문 정점에서 정방향 DFS를 시작한다.
3. 한 정점의 모든 자식을 처리한 뒤 `finish_order`에 그 정점을 추가한다.
4. 방문 배열을 모두 `false`로 초기화한다.
5. `finish_order`를 뒤에서 앞으로 순회한다.
6. 미방문 정점을 만나면 역그래프 DFS를 시작해 새 SCC에 정점을 모은다.
7. 문제의 출력 조건이 있으면 각 SCC와 SCC 목록을 필요한 기준으로 정렬한다.

## 의사 코드

```text
dfs1(v):
    visited[v] = true
    for next in graph[v]:
        if not visited[next]: dfs1(next)
    finish_order.push_back(v)

dfs2(v, component):
    visited[v] = true
    component.push_back(v)
    for next in reversed[v]:
        if not visited[next]: dfs2(next, component)

for v = 1..V:
    if not visited[v]: dfs1(v)

visited를 모두 false로 초기화
for v in reverse(finish_order):
    if not visited[v]:
        component = 빈 목록
        dfs2(v, component)
        components.push_back(component)
```

## 컴파일 가능한 C++ 뼈대

```cpp
// <algorithm>은 방문 배열을 초기화하는 std::fill을 제공한다.
#include <algorithm>
// <iostream>은 표준 입출력을 제공한다.
#include <iostream>
// <vector>는 인접 리스트와 방문/순서 배열을 제공한다.
#include <vector>

// 인접 리스트 타입에 읽기 쉬운 별칭을 붙인다.
using Graph = std::vector<std::vector<int>>;

// 첫 DFS는 모든 자식 호출이 끝난 뒤 vertex를 order에 추가한다.
void dfs_order(const Graph& graph, int vertex,
               std::vector<bool>& visited, std::vector<int>& order) {
    // 현재 정점을 방문 처리해 방향 사이클에서도 재귀가 끝나게 한다.
    visited[vertex] = true;
    // 현재 정점에서 나가는 모든 간선을 검사한다.
    for (const int next : graph[vertex]) {
        // 미방문 이웃만 재귀 호출한다.
        if (!visited[next]) {
            dfs_order(graph, next, visited, order);
        }
    }
    // 후손보다 나중에 추가하는 것이 종료 순서의 핵심이다.
    order.push_back(vertex);
}

// 두 번째 DFS는 역그래프에서 하나의 SCC 크기를 센다.
int dfs_component(const Graph& reversed, int vertex,
                  std::vector<bool>& visited) {
    // 이번 SCC에 배정했음을 기록한다.
    visited[vertex] = true;
    // 현재 정점 하나를 포함하므로 크기를 1에서 시작한다.
    int size{1};
    // 역방향 이웃을 모두 검사한다.
    for (const int next : reversed[vertex]) {
        // 새 정점이 속한 하위 탐색 크기를 += 연산자로 누적한다.
        if (!visited[next]) {
            size += dfs_component(reversed, next, visited);
        }
    }
    // 계산한 컴포넌트 크기 int 값을 호출자에게 반환한다.
    return size;
}

// 실행 예는 1<->2와 단독 정점 3이라는 두 SCC를 만든다.
int main() {
    // 정점 번호를 그대로 인덱스로 사용하려고 크기 4를 만든다.
    Graph graph(4);
    Graph reversed(4);
    // 1 -> 2 간선과 그 역간선을 각각 저장한다.
    graph[1].push_back(2);
    reversed[2].push_back(1);
    // 2 -> 1 간선과 그 역간선을 각각 저장한다.
    graph[2].push_back(1);
    reversed[1].push_back(2);

    // 방문 비트는 처음에 모두 false다.
    std::vector<bool> visited(4, false);
    std::vector<int> order{};
    // 1부터 3까지 미방문 DFS를 시작한다.
    for (int vertex{1}; vertex <= 3; ++vertex) {
        if (!visited[vertex]) {
            dfs_order(graph, vertex, visited, order);
        }
    }

    // 두 번째 단계가 사용할 방문 상태를 false로 되돌린다.
    std::fill(visited.begin(), visited.end(), false);
    // SCC 개수를 담는 기본 int를 0으로 초기화한다.
    int component_count{};
    // 종료 순서를 뒤에서부터 읽는다.
    for (auto it{order.rbegin()}; it != order.rend(); ++it) {
        // *it는 현재 정점 번호를 읽는 역참조 연산이다.
        const int vertex{*it};
        // 미배정 정점마다 역그래프 DFS 한 번, 즉 SCC 하나를 센다.
        if (!visited[vertex]) {
            dfs_component(reversed, vertex, visited);
            ++component_count;
        }
    }
    // 예상 SCC 개수 2를 출력한다.
    std::cout << component_count << '\n';
}
```

## 정확성 근거

첫 DFS가 만든 종료 순서를 SCC 압축 DAG 위에서 보면, 간선 `C -> D`가 있고 반대 경로가 없다면 `C`에서 시작한 DFS는 `D`를 먼저 끝내거나, 이미 끝난 `D`를 본 뒤 `C`를 끝낸다. 따라서 남은 압축 DAG의 원천 SCC에는 가장 늦은 종료 정점이 있다.

그 정점에서 역그래프 DFS를 시작한다. 원래 압축 DAG의 원천 SCC는 역그래프에서 싱크이므로 다른 미배정 SCC로 나가는 간선이 없다. 반면 같은 SCC 내부에서는 모든 정점이 서로 도달 가능하고 간선을 모두 뒤집어도 상호 도달 가능성이 유지된다. 그러므로 DFS는 그 SCC의 모든 정점과 오직 그 정점들만 방문한다. 이를 SCC 하나씩 제거하며 반복하므로 모든 정점은 정확히 한 SCC에 배정된다.

## 시간·공간 복잡도

- 정방향·역방향 인접 리스트 구성: `O(V+E)` 시간, `O(V+E)` 공간
- 첫 DFS 전체: 각 정점·간선을 한 번씩 보아 `O(V+E)`
- 두 번째 DFS 전체: 각 정점·간선을 한 번씩 보아 `O(V+E)`
- 코사라주 전체: `O(V+E)` 시간, `O(V+E)` 공간
- 출력 정렬이 필요하면 별도로 최악 `O(V log V)` 시간이 추가될 수 있다.
- 재귀 구현은 최악 `O(V)` 호출 스택을 사용하므로 정점 수가 매우 크면 명시적 `std::vector<int>` 스택을 쓰는 반복형 DFS를 고려한다.

## 흔한 실수

- 첫 DFS에서 정점을 발견하자마자 순서에 넣는다. 필요한 것은 발견 순서가 아니라 **종료 순서**다.
- 두 번째 DFS 전에 방문 배열을 초기화하지 않는다.
- 두 번째 DFS도 원래 그래프에서 수행한다. 반드시 모든 간선을 뒤집은 역그래프를 사용한다.
- `finish_order`를 앞에서부터 순회한다. 반드시 뒤에서부터, 즉 종료 시각 내림차순으로 순회한다.
- 1기반 정점 번호인데 인접 리스트 크기를 `V`만 만든다.
- 깊은 그래프에서 재귀 호출 스택 한도를 확인하지 않는다.

## 변형

- **타잔 알고리즘**: 한 번의 DFS에서 발견 시각과 low-link를 사용해 SCC를 찾는다. 역그래프가 필요 없지만 불변식이 더 섬세하다.
- **SCC 압축 DAG**: 각 정점을 SCC 번호로 바꾸고 서로 다른 SCC 사이 간선만 남긴다. 중복 간선 제거 후 위상 정렬·DP를 적용한다.
- **2-SAT**: 각 논리 변수와 부정을 정점으로 만든 implication graph에서 `x`와 `not x`가 같은 SCC면 해가 없다.
- **최소 간선 추가**: 압축 DAG의 진입 차수 0과 진출 차수 0인 SCC 수를 이용하는 유형이 있다.

## 오늘 문제와의 연결

2026-08-16의 BOJ 2150은 SCC 자체와 정렬 출력을 요구한다. `icpc_problem.cpp`의 `build_finish_order`가 첫 DFS 종료 순서를 만들고, `collect_component`가 역그래프에서 한 SCC를 모은다. 각 SCC를 정렬한 뒤 SCC 벡터를 사전식 정렬하면 가장 작은 정점 번호 기준의 출력 조건을 만족한다.

## 직접 해보기와 초보자 검증

1. `1->2, 2->1, 2->3` 그래프의 첫 DFS 종료 순서와 역그래프 DFS 결과를 손으로 적는다.
2. 첫 단계의 `push_back(vertex)`를 함수 첫 줄로 옮겨 잘못된 결과가 나오는 반례를 찾는다.
3. SCC 압축 그래프에 사이클이 있다고 가정하면 그 사이클의 SCC들이 사실 하나의 SCC여야 함을 모순으로 증명한다.
4. 재귀 함수를 `struct Frame { int vertex; std::size_t next_index; };` 명시적 스택으로 바꾸고 종료 시점을 어떻게 표현할지 설명한다.
5. 오늘 코드의 CTest 세 사례를 통과시킨 뒤, 정점 하나와 자기 루프 하나인 입력 `1 1 / 1 1`도 추가해 결과가 `1 / 1 -1`인지 확인한다.
