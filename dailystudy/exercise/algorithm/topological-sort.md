# 위상 정렬

## 정의와 적용 조건

위상 정렬은 방향 그래프의 모든 간선 `u -> v`에 대해 `u`가 `v`보다 먼저 나오도록 정점을 나열하는 알고리즘이다. 이런 순서는 방향 비순환 그래프(DAG)에만 존재한다. 작업 의존성, 선수 과목, 빌드 순서처럼 “먼저 해야 할 일”이 있는 문제에 적용한다.

## 핵심 아이디어와 불변식

Kahn 알고리즘은 아직 제거되지 않은 간선만 보며 진입 차수가 0인 정점을 고른다. 핵심 불변식은 다음과 같다.

- `indegree[v]`는 아직 결과에 넣지 않은 정점에서 `v`로 들어오는 간선 수다.
- 준비 큐에는 현재 `indegree`가 0인 정점만 있다.
- 결과에 확정된 모든 정점은 그보다 앞서야 하는 정점 뒤에 놓이지 않는다.

진입 차수가 0이면 남은 선행 조건이 없으므로 다음에 선택해도 안전하다.

## 단계별 절차

1. 모든 방향 간선을 인접 리스트에 저장하면서 도착 정점의 진입 차수를 센다.
2. 진입 차수가 0인 모든 정점을 큐에 넣는다.
3. 큐에서 정점 하나를 꺼내 결과 뒤에 붙인다.
4. 그 정점에서 나가는 간선을 제거한다고 생각하고 각 도착 정점의 진입 차수를 1 줄인다.
5. 새로 진입 차수가 0이 된 정점을 큐에 넣는다.
6. 큐가 빌 때까지 반복한다. 결과 정점 수가 전체보다 작으면 순환이 있다.

## 의사 코드

```text
각 간선 u -> v에 대해 graph[u]에 v 추가, indegree[v] 증가
indegree가 0인 모든 정점을 queue에 추가
while queue가 비지 않음:
    u = queue에서 제거
    answer에 u 추가
    for v in graph[u]:
        indegree[v] 감소
        if indegree[v] == 0:
            queue에 v 추가
answer 크기가 정점 수와 같은지 확인
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <cstddef>  // std::size_t
#include <iostream> // std::cin, std::cout
#include <queue>    // std::queue
#include <vector>   // std::vector

int main() {
    int n{};
    int m{};
    std::cin >> n >> m;
    std::vector<std::vector<int>> graph(static_cast<std::size_t>(n + 1));
    std::vector<int> indegree(static_cast<std::size_t>(n + 1), 0);
    for (int i{}; i < m; ++i) {
        int u{};
        int v{};
        std::cin >> u >> v;
        graph[static_cast<std::size_t>(u)].push_back(v);
        ++indegree[static_cast<std::size_t>(v)];
    }
    std::queue<int> ready{};
    for (int v{1}; v <= n; ++v) {
        if (indegree[static_cast<std::size_t>(v)] == 0) {
            ready.push(v);
        }
    }
    std::vector<int> order{};
    while (!ready.empty()) {
        const int u{ready.front()};
        ready.pop();
        order.push_back(u);
        for (const int v : graph[static_cast<std::size_t>(u)]) {
            if (--indegree[static_cast<std::size_t>(v)] == 0) {
                ready.push(v);
            }
        }
    }
    if (order.size() != static_cast<std::size_t>(n)) {
        std::cout << "cycle\n";
    }
}
```

## 정확성 근거

큐에서 꺼낸 정점은 남은 진입 간선이 0이므로 아직 출력하지 않은 어떤 정점 뒤에 있어야 한다는 제약이 없다. 따라서 현재 결과 뒤에 붙여도 모든 간선 제약을 지킨다. 그 정점의 간선을 제거한 뒤에도 진입 차수 불변식이 유지된다. DAG에는 항상 진입 차수 0인 정점이 하나 이상 있으므로 반복하면 모든 정점을 출력한다.

## 시간·공간 복잡도

각 정점을 큐에 최대 한 번 넣고, 인접 리스트의 각 간선을 한 번만 따라간다. 시간은 `O(V + E)`, 인접 리스트·진입 차수·큐·결과를 포함한 공간은 `O(V + E)`다.

## 흔한 실수

- `A가 B보다 앞`을 `B -> A`로 반대로 저장한다.
- 간선을 읽을 때 진입 차수를 세지 않거나 잘못된 끝점의 차수를 늘린다.
- 진입 차수가 0이 “된 순간” 한 번만 큐에 넣지 않아 중복 처리한다.
- 결과 길이로 순환 존재 여부를 검사하지 않는다.
- 답이 여러 개인데 특정 순서를 가정한다. 가장 작은 번호 우선이 필요하면 `queue` 대신 최소 힙을 쓴다.

## 변형

- DFS 종료 순서를 뒤집어 위상 순서를 만들 수 있으며, 방문 중 정점을 다시 만나면 순환이다.
- 최소 힙을 쓰면 가능한 위상 순서 중 번호가 사전순으로 가장 작은 답을 얻는다.
- 위상 순서대로 동적 계획법을 수행하면 DAG의 최장 경로·작업 완료 시간을 구할 수 있다.
- 위상 순서가 유일한지 알려면 매 단계 준비 가능한 정점이 정확히 하나인지 확인한다.

## DAG 최장 경로와 경로 복원

일반 그래프의 최장 단순 경로는 어렵지만 DAG는 위상 순서 때문에 각 상태를 한 번만 확정할 수 있다.
간선 가중치가 없고 방문 정점 수를 최대화한다면 `best[source] = 1`, 나머지를 도달 불가능 sentinel로
초기화한다. 위상 순서에서 `u`를 처리할 때 도달 가능한 경우에만 모든 `u -> v`에 다음 완화를 한다.

```text
best[source] = 1
나머지 best = UNREACHABLE, predecessor = NONE
모든 정점을 포함하는 위상 순서 order를 구한다
for u in order:
    if best[u] == UNREACHABLE:
        continue
    for v in graph[u]:
        candidate = best[u] + 1
        if candidate > best[v]:
            best[v] = candidate
            predecessor[v] = u
```

핵심 불변식은 `u`를 처리하기 직전에 `u`로 들어오는 모든 간선의 시작점이 이미 처리되어
`best[u]`가 source에서 `u`까지의 최댓값으로 확정되었다는 것이다. 모든 source-to-`v` 경로의 마지막
간선은 어떤 `u -> v`이고 그 후보를 전부 비교하므로, 위상 순서에 대한 귀납으로 각 `best[v]`가 정확하다.

엄격한 개선 때만 `predecessor[v] = u`를 기록하면 각 포인터는 실제 간선을 가리키고 DP 값이 한 단계씩
감소한다. target에서 predecessor를 따라 source까지 모은 뒤 역순으로 읽으면 최장 경로 하나를 복원할
수 있다. 동률 후보는 갱신하지 않아도 길이는 최적이며, 문제에서 별도 순서를 요구할 때만 동률 정책을
추가한다. 정점과 간선을 한 번씩 보고 복원 길이는 최대 `V`이므로 전체 시간 `O(V+E)`, 그래프를 포함한
공간 `O(V+E)`다.

가중 DAG라면 `candidate = best[u] + weight(u,v)`로 바꿀 수 있다. 음수 가중치가 있을 수 있으므로
도달 불가능 값을 단순한 0으로 두지 말고, sentinel에는 절대 가중치를 더하지 않아 가짜 경로를 막는다.
다만 sentinel 검사는 **유한한 두 값의 덧셈 오버플로까지 막지 않는다**. 입력 제약으로 합이 표현 범위 안임을
증명하거나, 더 넓은 타입과 덧셈 전 범위 검사를 사용해야 한다.

### 대회에서 자주 틀리는 지점

- 전체 그래프의 진입 차수를 계산했다면 Kahn 큐에도 **전체 그래프의** 진입 차수 0 정점을 넣는다.
  source만 넣으면 source에서 도달 가능하지만 다른 도달 불가능 정점의 간선을 받는 정점이 영원히
  진입 차수 0이 되지 않을 수 있다. 다른 선택은 먼저 도달 가능 부분 그래프를 만들고 그 안에서만
  진입 차수를 다시 세는 것이다.
- `best[u]`가 도달 불가능한데도 `best[u] + 1`로 완화하면 존재하지 않는 경로가 생긴다.
- predecessor를 모든 간선에서 덮어쓰지 말고 최댓값이 실제로 개선될 때만 바꾼다.
- 경로는 target부터 역방향으로 모이므로 `reverse`하거나 역인덱스로 출력해야 한다.
- DAG 보장이 없다면 위상 결과 길이가 `V`인지 검사해야 한다. 더 짧다면 순환 때문에 이 DP 전제가 깨진다.

## 오늘 문제와의 연결

2026-08-06의 BOJ 2252 `줄 세우기`에서 학생은 정점, `A가 B보다 앞` 비교는 간선 `A -> B`다. `icpc_problem.cpp`는 진입 차수와 FIFO 큐를 사용해 모든 비교를 만족하는 순서를 `O(N+M)`에 만든다.

2026-09-22의 CSES 1680 `Longest Flight Route`는 전체 DAG를 Kahn 방식으로 처리하면서
`best[v] = max(best[v], best[u] + 1)`을 계산한다. 엄격한 개선을 만든 `u`를 저장해 도시 n부터
역추적하므로 최장 경로 하나를 추가 탐색 없이 복원하며, 전체 시간과 공간은 `O(n+m)`이다.
