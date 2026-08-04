# 그래프 순회와 연결 요소

## 정의와 적용 조건

그래프 순회는 한 정점에서 간선을 따라 도달 가능한 정점을 빠짐없이 방문하는 방법이다. 깊이 우선 탐색(DFS)은 한 경로를 깊게 진행하고, 너비 우선 탐색(BFS)은 시작점과 가까운 정점부터 진행한다. 무방향 그래프의 **연결 요소**는 서로 도달 가능한 정점의 최대 집합이다. 단순 연결 여부나 요소 개수에는 DFS와 BFS 어느 쪽도 쓸 수 있다. 가중 최단 거리가 필요하면 Dijkstra 같은 별도 알고리즘이 필요하다.

## 핵심 아이디어와 불변식

모든 정점을 순서대로 보며 미방문 정점을 발견하면 새 요소의 대표로 센다. 그 정점에서 순회하여 도달 가능한 정점을 전부 방문 표시한다.

- 대기 컨테이너에 들어간 정점은 이미 방문 표시되어 중복 삽입되지 않는다.
- 순회 중 방문한 정점은 모두 시작점과 같은 연결 요소다.
- 순회가 끝나면 시작점에서 도달 가능한 모든 정점이 방문됐다.

## 단계별 절차

1. 간선을 인접 리스트에 저장하고 `visited`를 거짓으로 초기화한다.
2. 정점 1부터 N까지 확인한다.
3. 이미 방문한 정점은 건너뛴다.
4. 미방문 정점이면 요소 수를 늘리고 스택(DFS) 또는 큐(BFS)에 넣으며 방문 표시한다.
5. 대기 컨테이너가 빌 때까지 정점을 꺼내 미방문 이웃을 표시하고 넣는다.

## 의사 코드

```text
count = 0
for start in vertices:
  if visited[start]: continue
  count += 1
  push(start); visited[start] = true
  while pending is not empty:
    current = pop()
    for next in graph[current]:
      if not visited[next]:
        visited[next] = true
        push(next)
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <vector> // 인접 리스트와 명시적 DFS 스택을 제공한다.

int count_components(const std::vector<std::vector<int>>& graph) {
    std::vector<char> visited(graph.size(), false); // 정점별 방문 상태다.
    int count{};
    for (std::size_t start{}; start < graph.size(); ++start) {
        if (visited[start]) {
            continue;
        }
        ++count;
        std::vector<std::size_t> stack{start};
        visited[start] = true; // 삽입 시 표시해 같은 정점의 중복 삽입을 막는다.
        while (!stack.empty()) {
            const std::size_t current{stack.back()};
            stack.pop_back();
            for (const int raw_next : graph[current]) {
                const auto next{static_cast<std::size_t>(raw_next)};
                if (!visited[next]) {
                    visited[next] = true;
                    stack.push_back(next);
                }
            }
        }
    }
    return count;
}

int main() {
    const std::vector<std::vector<int>> graph{{1}, {0}, {}};
    return count_components(graph) == 2 ? 0 : 1;
}
```

## 정확성 근거

새 시작점은 이전 순회에서 방문되지 않았으므로 기존 어떤 요소에도 속하지 않는다. 순회가 간선을 따라 추가하는 정점은 모두 시작점에서 도달 가능하므로 같은 요소다. 반대로 시작점에서 도달 가능한 정점은 경로 길이에 대한 귀납법으로 차례로 대기 컨테이너에 들어가 방문된다. 따라서 순회 한 번은 정확히 하나의 연결 요소를 표시하며, 외부 반복의 증가 횟수는 요소 개수와 같다.

## 시간·공간 복잡도

인접 리스트에서 각 정점은 한 번 처리되고 각 무방향 간선은 양 끝에서 한 번씩 조사되어 시간은 `O(V+E)`다. 인접 리스트, 방문 배열, 스택 또는 큐를 합친 공간은 `O(V+E)`다. 인접 행렬을 쓰면 순회가 `O(V^2)`가 된다.

## 흔한 실수

- 무방향 간선을 한 방향만 저장한다.
- 방문 표시를 꺼낼 때 해서 같은 정점을 여러 번 넣는다.
- 정점 번호가 1부터 시작하는데 크기 N 배열을 만들어 범위를 벗어난다.
- 큰 그래프에서 재귀 DFS로 호출 스택 한도를 넘는다.
- BFS의 간선 수 최단 거리 성질과 가중 최단 거리를 혼동한다.

## 변형

- BFS: FIFO 큐를 사용하며 무가중 최단 간선 수까지 구할 수 있다.
- 재귀 DFS: 코드가 짧지만 깊은 그래프에서 호출 스택 위험이 있다.
- 방향 그래프: 약한 연결 요소와 강한 연결 요소(SCC)는 정의와 알고리즘이 다르다.
- 동적 연결성: 간선 합치기 위주라면 서로소 집합(DSU)이 적합하다.

## 오늘 문제와의 연결

2026-08-05의 BOJ 11724 풀이는 1부터 N까지 미방문 정점을 세고, `std::vector<int>`를 LIFO 스택으로 사용한 반복 DFS로 각 연결 요소를 표시한다. 제출 코드는 [`../2026-08-05/icpc_problem.cpp`](../2026-08-05/icpc_problem.cpp)에서 확인한다.
