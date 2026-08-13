# 다익스트라 최단 경로

## 정의와 적용 조건

다익스트라는 한 시작 정점에서 다른 모든 정점까지의 최단 거리를 구하는 알고리즘이다. 모든 간선 가중치가 **0 이상**이어야 한다. 음수 간선이 있으면 이미 확정한 거리가 뒤늦게 줄 수 있으므로 벨만–포드 같은 알고리즘을 사용한다.

## 핵심 아이디어와 불변식

`distance[v]`는 지금까지 발견한 시작점→`v` 경로 중 최소 길이다. 최소 힙은 아직 확장할 후보 중 거리가 가장 작은 상태를 준다. 그 상태의 거리가 현재 배열 값과 같다면, 음수 간선이 없으므로 그 정점의 거리는 확정된다. 더 큰 값은 같은 정점의 오래된 후보이므로 버린다.

## 단계별 절차

1. 모든 거리를 무한대로, 시작점 거리를 0으로 초기화한다.
2. 최소 힙에 `(0, 시작점)`을 넣는다.
3. 힙에서 최소 상태를 꺼낸다. 배열의 현재 거리와 다르면 건너뛴다.
4. 인접한 모든 간선 `(v, w)`에 대해 `distance[u] + w < distance[v]`이면 완화한다.
5. 갱신한 `(distance[v], v)`를 힙에 넣고 힙이 빌 때까지 반복한다.

## 의사 코드

```text
dist[*] = INF
dist[start] = 0
heap.push(0, start)
while heap is not empty:
    (d, u) = heap.pop_min()
    if d != dist[u]: continue
    for (v, w) in graph[u]:
        if d + w < dist[v]:
            dist[v] = d + w
            heap.push(dist[v], v)
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <functional> // std::greater
#include <queue>      // std::priority_queue
#include <utility>    // std::pair
#include <vector>     // std::vector

using State = std::pair<long long, int>; // (거리, 정점)

void dijkstra(int start, const std::vector<std::vector<std::pair<int, int>>>& graph,
              std::vector<long long>& distance) {
    std::priority_queue<State, std::vector<State>, std::greater<State>> heap;
    distance[start] = 0;
    heap.push({0, start});
    while (!heap.empty()) {
        const auto [d, u] = heap.top();
        heap.pop();
        if (d != distance[u]) continue; // 오래된 후보 제거
        for (const auto& [v, weight] : graph[u]) {
            if (d + weight < distance[v]) {
                distance[v] = d + weight;
                heap.push({distance[v], v});
            }
        }
    }
}
```

## 정확성 근거

힙에서 현재 최솟값 `d`로 정점 `u`를 꺼냈다고 하자. `u`로 가는 더 짧은 미발견 경로가 있다면 그 경로에서 아직 확정되지 않은 첫 정점 직전까지의 거리는 `d` 이하이고, 다음 간선도 음수가 아니다. 그러면 그 후보가 `u`보다 먼저 힙에서 처리되어 완화됐어야 하므로 모순이다. 따라서 꺼낸 현재값은 최단 거리이며, 모든 간선을 완화하므로 도달 가능한 최단 경로가 빠지지 않는다.

## 시간·공간 복잡도

인접 리스트와 이진 최소 힙을 사용하면 각 간선 완화가 힙 연산을 최대 한 번 만들므로 시간은 `O((V+E) log V)`(흔히 `O(E log V)`), 그래프·거리·힙을 합친 공간은 `O(V+E)`이다.

## 흔한 실수

- 음수 가중치에 적용한다.
- 최대 힙인 기본 `priority_queue`를 그대로 쓴다.
- 오래된 힙 항목을 걸러내지 않아 불필요한 확장이 폭증한다.
- `int` 누적 거리의 오버플로를 무시한다.
- 무방향 그래프인데 한 방향 간선만 넣거나, 방향 그래프인데 양쪽에 넣는다.

## 변형

- 목표 정점 하나만 필요하면 그 정점이 현재 최솟값으로 꺼질 때 종료할 수 있다.
- 시작점이 여러 개면 모두 거리 0으로 힙에 넣는 다중 시작점 다익스트라가 된다.
- 가중치가 0 또는 1뿐이면 덱을 쓰는 0-1 BFS가 더 적합하다.
- 경로 복원이 필요하면 완화할 때 `parent[v] = u`를 함께 기록한다.

## 오늘 문제와의 연결

2026-08-14의 BOJ 1916 `최소비용 구하기`는 비음수 단방향 그래프이다. [`../2026-08-14/icpc_problem.cpp`](../2026-08-14/icpc_problem.cpp)는 목적지가 최신 최소 후보로 꺼지는 순간 종료하고, 평행 간선도 각각 완화해 전처리 없이 최소 비용을 구한다.

2026-08-02의 BOJ 1753은 양의 가중치 방향 그래프이다. [`../2026-08-02/icpc_problem.cpp`](../2026-08-02/icpc_problem.cpp)는 `long long` 거리, 인접 리스트, `greater<pair<...>>` 최소 힙, 오래된 항목 건너뛰기를 그대로 구현한다. 평행 간선도 각각 완화하므로 가장 짧은 경로가 자연스럽게 선택된다.

2026-08-03의 Kattis `shortestpath1`도 음이 아닌 가중치 방향 그래프이다. [`../2026-08-03/icpc_problem.cpp`](../2026-08-03/icpc_problem.cpp)는 시작점 하나의 거리 배열을 한 번 계산한 뒤 여러 질의가 공유한다. 따라서 질의마다 다익스트라를 다시 실행하지 않고 `O((V+E) log V + Q)`에 답한다.
