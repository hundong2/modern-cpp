# 0-1 BFS 최단 경로

## 정의

0-1 BFS는 모든 간선 가중치가 정확히 `0` 또는 `1`인 그래프의 단일 시작점 최단 거리를 덱(`deque`)으로 구하는 알고리즘이다. 일반 BFS의 “모든 간선 비용이 같다”는 조건을 한 단계 확장하며, 우선순위 큐를 쓰는 다익스트라의 특수화로 볼 수 있다.

## 적용 조건

- 간선 가중치는 음수가 아니고 오직 `0`과 `1`이어야 한다.
- 한 시작점에서 여러 정점까지의 최소 비용 또는 한 목적지까지의 최소 비용이 필요할 때 쓴다.
- 격자처럼 간선을 명시적으로 저장하지 않고 이웃을 즉석에서 만들 수 있는 암시적 그래프에도 맞는다.
- 가중치가 `2` 이상이거나 다양한 비음수 값이면 일반 다익스트라를 사용한다. 음수 간선이 있으면 Bellman–Ford 같은 다른 알고리즘이 필요하다.

## 핵심 아이디어와 불변식

정점 `u`를 처리하며 간선 `(u,v,w)`를 완화한다.

```text
candidate = distance[u] + w
candidate < distance[v] 이면 distance[v] = candidate
```

- `w == 0`이면 `v`의 새 거리는 `u`와 같으므로 덱 **앞**에 넣는다.
- `w == 1`이면 `v`의 새 거리는 `u`보다 1 크므로 덱 **뒤**에 넣는다.

덱에 들어 있는 아직 처리할 후보의 거리는 앞쪽의 값 `d`와 그보다 최대 1 큰 `d+1` 순서로 유지된다. 따라서 덱 앞에서 꺼내는 동작이 최소 거리 후보를 고르는 우선순위 큐 역할을 한다. 이것이 “0은 앞, 1은 뒤” 규칙의 핵심 불변식이다.

같은 정점이 거리 개선 때 여러 번 들어갈 수 있지만, 성공한 완화마다 거리가 엄격히 작아지고 0/1 가중치 구조에서 총 작업량은 `O(V+E)`로 제한된다. 구현에 `(거리, 정점)`을 함께 저장해 오래된 항목을 건너뛰는 방식도 가능하다.

## 단계별 절차

1. 모든 거리를 충분히 큰 `INF`로 초기화한다.
2. 시작점 거리를 `0`으로 만들고 덱 앞에 넣는다.
3. 덱이 빌 때까지 앞 정점 `u`를 꺼낸다.
4. `u`의 각 간선 `(u,v,w)`에 대해 새 비용을 계산한다.
5. 새 비용이 더 작으면 거리를 갱신한다.
6. `w==0`이면 `v`를 앞에, `w==1`이면 뒤에 넣는다.
7. 반복이 끝난 뒤 거리 배열이 최단 거리다.

## 의사 코드

```text
distance[*] = INF
distance[start] = 0
deque.push_front(start)

while deque is not empty:
    u = deque.front()
    deque.pop_front()
    for (v, weight) in edges_from(u):
        candidate = distance[u] + weight
        if candidate >= distance[v]:
            continue
        distance[v] = candidate
        if weight == 0:
            deque.push_front(v)
        else:
            deque.push_back(v)
```

## 컴파일 가능한 C++ 뼈대

아래 예제는 정점 0에서 모든 정점까지의 거리를 구한다. 실제 문제에서는 인접 리스트 대신 격자 이웃을 즉석에서 생성할 수 있다.

```cpp
#include <cstddef>  // std::size_t를 선언한다.
#include <deque>    // std::deque를 선언한다.
#include <iostream> // std::cout을 선언한다.
#include <limits>   // std::numeric_limits를 선언한다.
#include <utility>  // std::pair를 선언한다.
#include <vector>   // std::vector를 선언한다.

struct Edge {       // struct의 기본 접근은 public이다.
    int to{};       // 도착 정점 번호를 값으로 저장한다.
    int weight{};   // 반드시 0 또는 1인 간선 비용이다.
};

std::vector<int> zero_one_bfs(const std::vector<std::vector<Edge>>& graph, int start) {
    constexpr int infinity{std::numeric_limits<int>::max() / 4};
    std::vector<int> distance(graph.size(), infinity); // 모든 거리를 INF로 소유한다.
    std::deque<int> frontier{};                        // 양끝 삽입이 가능한 작업 덱이다.
    distance[static_cast<std::size_t>(start)] = 0;
    frontier.push_front(start); // 시작점 비용은 0이므로 앞에 넣는다.

    while (!frontier.empty()) {
        const int current{frontier.front()}; // pop 전에 값을 복사한다.
        frontier.pop_front();                // pop_front는 제거 값을 반환하지 않는다.
        for (const Edge& edge : graph[static_cast<std::size_t>(current)]) {
            const int candidate{distance[static_cast<std::size_t>(current)] + edge.weight};
            if (candidate >= distance[static_cast<std::size_t>(edge.to)]) {
                continue;
            }
            distance[static_cast<std::size_t>(edge.to)] = candidate;
            if (edge.weight == 0) {
                frontier.push_front(edge.to); // 같은 거리 층을 우선 처리한다.
            } else {
                frontier.push_back(edge.to);  // 한 단계 비싼 후보는 뒤로 미룬다.
            }
        }
    }
    return distance; // 반환 prvalue로 결과 vector를 직접 초기화하거나 이동할 수 있다.
}

int main() {
    std::vector<std::vector<Edge>> graph(4);
    graph[0] = {{1, 0}, {2, 1}};
    graph[1] = {{2, 0}, {3, 1}};
    graph[2] = {{3, 0}};
    const auto distance{zero_one_bfs(graph, 0)};
    std::cout << distance[3] << '\n'; // 0->1->2->3은 모두 0비용이므로 0이다.
    return distance[3] == 0 ? 0 : 1;
}
```

## 정확성 근거

### 보조정리 1: 덱의 거리 순서

덱 앞 정점의 거리를 `d`라 하자. 처리 중 새로 들어오는 정점의 거리는 `d` 또는 `d+1`이다. `d`는 앞에, `d+1`은 뒤에 넣으므로 덱 앞에서 뒤로 갈수록 거리가 감소하지 않는 구조가 유지된다. 이미 들어 있던 후보도 이전 단계에서 같은 규칙으로 배치되었다.

### 보조정리 2: 꺼낸 최소 후보의 안전성

덱 앞 후보보다 더 작은 거리의 미처리 경로가 있다고 가정하자. 그 경로에서 처음 아직 완화되지 않은 간선의 직전 정점은 더 작은 거리로 먼저 처리됐어야 한다. 간선 비용은 0 또는 1이므로 그 완화 결과는 덱 앞 후보보다 앞에 배치된다. 이는 현재 후보가 앞이라는 사실과 모순이다.

### 정리

모든 성공 완화는 실제 경로의 비용이므로 거리 값은 최단 거리보다 작아질 수 없다. 보조정리 2에 따라 덱이 처리하는 최소 후보의 필요한 완화가 빠지지 않는다. 덱이 비면 더 개선 가능한 간선이 없고, 최단 경로의 간선을 순서대로 적용하면 모든 도달 가능 정점의 최단 비용이 거리 배열에 반영된다.

## 시간·공간 복잡도

- 인접 리스트: `O(V+E)` 시간, 거리·덱을 합쳐 `O(V)` 추가 공간
- 여덟 방향 `R×C` 격자: `V=RC`, `E≤8RC`이므로 한 질문당 `O(RC)` 시간과 `O(RC)` 공간
- Q개의 독립 질문을 매번 새로 풀면 `O(QRC)` 시간이다.

일반 다익스트라는 같은 그래프에서 우선순위 큐 때문에 보통 `O((V+E) log V)`다. 0/1 조건을 발견하면 로그 인자를 없앨 수 있다는 점이 대회에서 중요하다.

## 흔한 실수

1. 모든 이웃을 뒤에만 넣어 일반 BFS처럼 작성한다. 0비용 연속 경로가 먼저 처리되지 않아 최단 거리 순서가 깨진다.
2. 비용 0을 뒤, 비용 1을 앞에 넣어 우선순위를 뒤집는다.
3. 방문 여부를 처음 넣을 때 영구 확정한다. 이후 더 싼 0비용 경로가 발견될 수 있으므로 거리 완화 조건을 사용한다.
4. `front()` 참조를 보관한 채 `pop_front()`한다. 제거된 원소 참조는 무효이므로 값을 먼저 복사한다.
5. 가중치가 2 이상인 간선을 억지로 넣는다. 덱 거리 층 불변식이 성립하지 않는다.
6. 격자 경계 검사 전에 인덱싱해 미정의 동작을 만든다.
7. `INF + weight`가 정수 범위를 넘도록 `max()`를 그대로 쓴다. 여유 있는 sentinel이나 도달 검사로 막는다.

## 변형

- **다중 시작점**: 모든 시작점 거리를 0으로 두고 덱 앞에 넣는다.
- **목적지 조기 종료**: 거리와 정점을 함께 저장하고 오래된 항목을 제거하는 등 “꺼낸 값이 현재 최솟값”을 명확히 보장할 때 목적지에서 멈춘다.
- **경로 복원**: 완화 성공 때 `parent[v]=u`를 갱신하고 목적지에서 역추적한다.
- **가중치가 작은 정수 범위**: 0..K 가중치에는 여러 버킷을 쓰는 Dial 알고리즘을 검토한다.
- **최소 방향 전환·벽 부수기**: 선호 행동 비용을 0, 그 외 행동을 1로 모델링하면 같은 틀을 쓸 수 있다.

## 오늘 문제와의 연결

2026-08-24의 Kattis `oceancurrents`에서는 격자 한 칸이 정점이고 여덟 방향 이동이 간선이다. 현재 칸 숫자와 같은 방향은 비용 0, 다른 방향은 비용 1이다. 간선을 따로 저장하면 최대 8RC 공간이 추가되지만, 방향 배열로 즉석에서 만들어 거리와 덱만 `O(RC)`에 유지한다.

대회에서 반드시 알아야 할 판별 질문은 하나다. **“간선 비용이 0과 1뿐인가?”** 그렇다면 일반 BFS도, 우선순위 큐 다익스트라도 먼저 쓰지 말고 0-1 BFS와 `push_front/push_back` 불변식을 떠올린다.

## 직접 해보기와 초보자 검증

1. `0→1(1), 0→2(0), 2→1(0)` 그래프에서 일반 BFS 방문 확정이 왜 틀릴 수 있는지 손으로 추적한다.
2. 위 뼈대의 `push_front`와 `push_back`을 바꾸고 출력이 달라지는 작은 반례를 만든다.
3. `parent` 배열을 추가해 실제 최단 경로를 출력한다.
4. `deque::front`, `pop_front`, `push_front`, `push_back`의 수신 상태·입력·반환·호출 후 상태·무효화 규칙을 자료 없이 설명한다.
5. 오늘 문제의 공식 예제 첫 질문처럼 시작과 목적지가 같을 때 0이 나오는 이유를 설명한다.
6. R=C=1000, Q=50일 때 최대 정점 검사 횟수의 규모를 계산하고 인접 행렬이 부적합한 이유를 말한다.
