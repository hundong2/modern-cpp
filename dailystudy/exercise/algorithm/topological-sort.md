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

## 오늘 문제와의 연결

2026-08-06의 BOJ 2252 `줄 세우기`에서 학생은 정점, `A가 B보다 앞` 비교는 간선 `A -> B`다. `icpc_problem.cpp`는 진입 차수와 FIFO 큐를 사용해 모든 비교를 만족하는 순서를 `O(N+M)`에 만든다.
