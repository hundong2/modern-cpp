# 최소 힙 Best-First 탐색으로 k개 최단 Walk 열거하기

## 정의

가중 방향 그래프에서 시작점 `s`부터 도착점 `t`까지의 **walk**는 정점과 간선을 다시 방문할 수 있는
간선열이다. k개 최단 walk 문제는 가능한 모든 `s -> t` walk의 비용을 중복을 유지한 채 오름차순으로
보았을 때 앞의 `k`개를 구한다. 서로 다른 간선열의 비용이 같아도 서로 다른 답으로 센다.

이 문서의 알고리즘은 다익스트라처럼 최소 힙에서 현재 가장 싼 walk 접두사를 꺼내되, 정점마다 하나의
최단 거리만 확정하지 않는다. 대신 각 정점을 비용 오름차순으로 최대 `k`번 받아들이고 확장한다. 도착점이
받아들여지는 1번째, 2번째, ..., k번째 비용이 바로 답이다.

`path`는 보통 정점 중복이 없는 단순 경로를 뜻할 수 있지만, 오늘 문제는 정점 재방문을 허용한다. 따라서
Yen 알고리즘처럼 k개 최단 **단순 경로**를 찾는 문제와 구별해 `walk`라고 부른다.

## 적용 조건

- 간선 가중치가 모두 0 이상이어야 한다. 오늘 CSES 문제는 더 강하게 모든 비용이 양수다.
- 정점과 간선 재방문을 허용하는 walk를 구해야 한다.
- 같은 비용의 서로 다른 route도 각각 세어야 한다.
- `k`가 작아 각 정점·간선을 최대 `k`번 다루는 비용을 감당할 수 있어야 한다.
- 문제에서 `s -> t` walk가 적어도 `k`개 존재함을 보장하거나, 답 개수가 `k`보다 작을 때의 출력을
  따로 정의해야 한다.

음수 간선에서는 최소 힙에서 먼저 꺼낸 접두사가 전역적으로 다음 최소라는 성질이 깨진다. 정점 중복을
금지한 k개 최단 단순 경로, 서로 다른 비용만 세는 문제, 경로 자체를 복원해야 하는 문제도 상태와
알고리즘이 달라진다.

## 상태와 핵심 아이디어

힙 원소 `(cost, vertex)` 하나는 실제로 발견한 어떤 `s -> vertex` walk 접두사 하나를 나타낸다. 경로를
직접 저장하지 않아도 동일한 정점·비용의 원소를 합치지 않으므로 multiplicity는 보존된다.

정점 `v`마다 `accepted[v]`를 둔다.

- `accepted[v]`는 힙에서 꺼내 실제로 받아들인 `s -> v` walk 접두사 수다.
- `accepted[v] < k`일 때만 새 pop을 받아들이고 `v`의 모든 나가는 간선을 확장한다.
- `accepted[v] == k`가 된 뒤 같은 정점으로 오는 후보는 버린다.
- 받아들인 정점이 `t`면 그 비용을 답 뒤에 붙인다.

## 핵심 불변식

### 불변식 1: 힙은 아직 처리하지 않은 발견된 walk 접두사를 비용순으로 준다

최소 힙의 `top()`은 현재 발견된 후보 가운데 비용이 가장 작다. 모든 간선 비용이 음수가 아니므로 더
비싼 접두사를 확장해 아직 발견하지 않은 후속 walk가 현재 후보보다 싸질 수 없다.

### 불변식 2: 정점의 r번째 accepted 비용은 그 정점까지의 r번째 최소 walk 비용이다

힙에서 처음 `v`를 받아들일 때가 최소 walk, 두 번째가 두 번째 최소 walk다. 같은 비용 후보도 제거하지
않으므로 등수는 비용의 중복을 포함한다. 이 성질은 다익스트라의 “첫 pop으로 하나의 최단 거리 확정”을
“앞의 k번 pop을 차례로 확정”으로 확장한 것이다.

### 불변식 3: 정점별 k개 뒤의 접두사는 정답 앞 k개에 필요 없다

어떤 `s -> v` 접두사 `P`가 `v`의 `(k+1)`번째 이후라 하자. `P`보다 비용이 크지 않은 접두사가 이미
`k`개 있다. `P` 뒤에 같은 `v -> t` suffix walk를 붙이면 그 `k`개에도 똑같은 suffix를 붙일 수 있고,
그 결과 비용은 모두 `P`를 쓴 결과보다 크지 않다. 따라서 `P`를 포함하는 완성 walk는 전체 앞 `k`개를
만드는 데 필요하지 않다.

### 불변식 4: multiplicity를 보존한다

평행 간선, 다른 중간 정점, 다른 반복 횟수로 생긴 후보가 우연히 같은 `(cost, vertex)`여도 각각 힙에
남긴다. 답 비용이 같다는 이유로 `set`이나 `unique`로 합치면 문제의 “같은 가격 route도 각각 계산”
조건을 깨뜨린다.

## 단계별 절차

1. 인접 리스트를 만들고 모든 `accepted[v]`를 0으로 둔다.
2. 최소 힙에 빈 시작 walk `(0, s)`를 넣는다.
3. 힙에서 최소 비용 후보 `(d, v)`를 꺼낸다.
4. 이미 `accepted[v] == k`이면 이 후보를 버린다.
5. 아니라면 `accepted[v]`를 1 늘린다.
6. `v == t`이면 `d`를 답에 추가한다. k번째 답이면 즉시 종료해도 된다.
7. 아직 종료하지 않았다면 모든 간선 `v -> u (w)`에 대해 `(d+w, u)`를 힙에 넣는다.
   `accepted[u] == k`인 이웃은 미리 생략해도 정확성은 같다.
8. 답을 `k`개 얻을 때까지 3~7을 반복한다.

도착점도 k번째 답을 얻기 전에는 확장해야 한다. `s -> ... -> t -> ... -> t`처럼 도착점을 중간에
방문했다가 다시 돌아오는 walk도 문제 정의상 유효할 수 있기 때문이다.

## 의사코드

```text
accepted[1..n] = 0
answer = empty list
min_heap.push((0, start))

while min_heap is not empty and answer.size < k:
    (cost, vertex) = min_heap.pop_min()

    if accepted[vertex] == k:
        continue

    accepted[vertex] += 1

    if vertex == target:
        answer.push_back(cost)
        if answer.size == k:
            break

    for each edge (vertex -> next, weight):
        if accepted[next] == k:
            continue
        if cost + weight would overflow int64:
            continue
        min_heap.push((cost + weight, next))
```

## 컴파일 가능한 C++ 뼈대

아래 예제는 C++20 이상에서 컴파일된다. `std::priority_queue`의 비교자는 “왼쪽 후보가 오른쪽보다
우선순위가 낮은가?”를 반환하므로 `left.cost > right.cost`가 최소 힙을 만든다.

```cpp
#include <cstddef>    // 컨테이너 크기와 인덱스용 std::size_t를 선언한다.
#include <cstdint>    // 정확히 64비트인 std::int64_t를 선언한다.
#include <iostream>   // 표준 입력과 출력을 선언한다.
#include <queue>      // std::priority_queue를 선언한다.
#include <vector>     // 인접 리스트와 카운터를 소유할 std::vector를 선언한다.

struct Edge {
    int to{};
    std::int64_t cost{};
};

struct Candidate {
    std::int64_t cost{};
    int vertex{};
};

struct CandidateGreater {
    [[nodiscard]] bool operator()(const Candidate& left,
                                  const Candidate& right) const noexcept {
        return left.cost > right.cost;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n{};
    int m{};
    int k{};
    if (!(std::cin >> n >> m >> k)) {
        return 0;
    }

    std::vector<std::vector<Edge>> graph(static_cast<std::size_t>(n) + 1U);
    for (int edge_index{}; edge_index < m; ++edge_index) {
        int from{};
        int to{};
        long long cost{};
        std::cin >> from >> to >> cost;
        graph[static_cast<std::size_t>(from)].push_back(
            Edge{to, static_cast<std::int64_t>(cost)});
    }

    std::vector<int> accepted(static_cast<std::size_t>(n) + 1U, 0);
    std::priority_queue<Candidate, std::vector<Candidate>, CandidateGreater> frontier{};
    std::vector<std::int64_t> answer{};
    answer.reserve(static_cast<std::size_t>(k));
    frontier.push(Candidate{0, 1});

    constexpr std::int64_t max_distance{9'223'372'036'854'775'807LL};

    while (!frontier.empty() && answer.size() < static_cast<std::size_t>(k)) {
        const Candidate current{frontier.top()};
        frontier.pop();
        const auto vertex_index{static_cast<std::size_t>(current.vertex)};

        if (accepted[vertex_index] >= k) {
            continue;
        }
        ++accepted[vertex_index];

        if (current.vertex == n) {
            answer.push_back(current.cost);
            if (answer.size() == static_cast<std::size_t>(k)) {
                break;
            }
        }

        const auto& outgoing{graph[vertex_index]};
        for (std::size_t index{}; index < outgoing.size(); ++index) {
            const Edge& edge{outgoing[index]};
            const auto next_index{static_cast<std::size_t>(edge.to)};
            if (accepted[next_index] >= k) {
                continue;
            }
            if (current.cost > max_distance - edge.cost) {
                continue;
            }
            frontier.push(Candidate{current.cost + edge.cost, edge.to});
        }
    }

    for (std::size_t index{}; index < answer.size(); ++index) {
        std::cout << static_cast<long long>(answer[index])
                  << (index + 1U == answer.size() ? '\n' : ' ');
    }
}
```

## 정확성 근거

### 보조정리 1: accepted 순서는 정점별 walk 비용의 비감소 순서다

정점 `v`에서 앞 k개 안에 드는 아직 받아들이지 않은 최소 walk를 `Q`라 하자. `Q`의 접두사 중 아직
힙에 들어가지 않은 첫 접두사를 잡고 그 직전 접두사를 `R`이라 하자. `R`이 끝나는 정점까지 `R`보다
비싸지 않은 접두사가 이미 k개라면, 그 각각에 `Q`의 남은 suffix를 붙여 `Q`보다 비싸지 않은
`s -> v` walk를 k개 만들 수 있다. 이는 `Q`가 아직 필요한 앞 k개 후보라는 가정과 모순이다.

따라서 `R`은 그 정점의 앞 k개 접두사이고 이미 받아들여져 확장됐어야 하므로, 그 다음 접두사도 힙에
들어 있다. 이 논리를 반복하면 `Q` 자체가 힙에 있다. 음이 아닌 간선만 더하므로 아직 발견되지 않은
후속 walk가 더 싼 후보를 건너뛸 수도 없다. 그러므로 최소 힙은 `Q`보다 비싼 후보를 먼저 받아들이지
않으며, 각 정점의 accepted 순서는 해당 정점까지의 walk 비용 비감소 순서다.

### 보조정리 2: 정점당 k번 제한은 앞 k개 도착 walk를 제거하지 않는다

정점 `v`의 `(k+1)`번째 이후 접두사를 포함하는 완성 walk를 생각하자. 불변식 2에서 설명했듯 그 접두사
보다 비싸지 않은 `s -> v` 접두사가 이미 k개 있다. 완성 walk의 나머지 suffix를 각각 붙이면 원래
walk보다 비싸지 않은 완성 walk가 k개 생긴다. 그러므로 원래 walk는 전체 순위 앞 k개에 꼭 들어갈 수
없고, 그 접두사를 확장하지 않아도 답은 보존된다.

### 보조정리 3: 도착점의 accepted 비용은 전체 route 비용의 비감소 순서다

보조정리 1을 `v=t`에 적용하면 도착점에서 첫 번째로 받아들인 후보가 전체 최소 route, r번째가 r번째
최소 route다. 같은 비용의 서로 다른 힙 항목을 합치지 않으므로 같은 가격의 multiplicity도 보존된다.

### 정리

보조정리 2에 의해 필요한 후보는 정점당 k번 제한으로 제거되지 않는다. 보조정리 3에 의해 도착점에서
받아들인 앞 k개 비용이 요구한 k개 최단 walk 비용과 정확히 같고 이미 비감소 순서다. 따라서 알고리즘은
정답을 정확히 출력한다.

## 시간·공간 복잡도

정점 수를 `n`, 간선 수를 `m`이라 하자.

- 각 정점은 최대 `k`번 받아들여지고, 받아들일 때만 나가는 간선을 훑는다. 전체 간선 확장은 최대
  `k*m`번이다.
- 힙 삽입 수는 시작 후보를 포함해 `O(km)`이고, 삽입된 후보는 최대 한 번 pop된다.
- 이진 힙 한 번의 삽입·제거는 `O(log(km))`이므로 안전한 시간 상한은
  `O(km log(km))`이다.
- 인접 리스트와 카운터는 `O(n+m)`, 답은 `O(k)`, 힙은 최악 `O(km)`이므로 전체 공간은
  `O(n+m+km)`이다.

오늘 제약은 `k <= 10`이라 일반 다익스트라에 작은 상수배를 더한 실전 범위다. 조기 종료 덕분에 실제로는
도착점의 k번째 pop 뒤 남은 힙을 처리하지 않는다.

## 대회에서 자주 틀리는 지점

- `distance[v]` 하나만 두면 두 번째 이후 walk 정보를 즉시 잃는다.
- **push 횟수**가 아니라 **유효 pop 횟수**를 센다. 힙에 일찍 들어갔다는 사실만으로 비용 순위가
  확정되지는 않는다.
- 같은 `(cost, vertex)` 후보를 중복 제거하지 않는다. 같은 비용 route도 각각 답이다.
- 기본 `std::priority_queue`는 최대 힙이다. 비교 방향을 반대로 써 최소 힙을 만든다.
- 도착점을 처음 만났을 때 종료하면 하나의 최단 route만 얻는다. k번째 유효 pop에서 종료한다.
- k번째 답 전에는 도착점의 나가는 간선도 확장한다. 도착점을 재방문하는 walk를 놓치지 않기 위해서다.
- 누적 비용에 `int`를 쓰지 않는다. 64비트 타입을 쓰고 덧셈 전에 `max - edge_cost`와 비교한다.
- `accepted[next] == k`인 이웃을 push하지 않는 가지치기는 안전하지만, 현재 후보의 정점 카운트를
  증가시키기 전후 순서를 섞지 않는다.
- 음수 간선에 그대로 적용하지 않는다. best-first 확정 불변식이 무너진다.

## 변형과 선택 기준

- 각 정점의 k개 최솟값 배열을 유지하며 완화하는 방식도 가능하다. 새 비용이 현재 k번째 값보다 작으면
  교체하고 힙에 넣는다. 정점별 순위표가 직접 필요할 때 유용하다.
- 실제 route를 복원하려면 힙 상태에 부모 상태 ID와 사용 간선 ID를 저장한다. 정점 하나당 부모 하나만
  두면 여러 route가 합쳐져 복원이 깨진다.
- 서로 다른 **비용 값** k개만 요구하면 같은 비용을 건너뛰는 별도 정책이 필요하다. 오늘은 그러면 안 된다.
- 정점 중복을 금지한 k개 최단 단순 경로에는 Yen, Eppstein 계열을 검토한다.
- DAG라면 위상 순서 DP로 각 정점의 상위 k개 비용을 합치는 방법이 더 직접적일 수 있다.
- 휴리스틱 하한이 있으면 A*식 우선순위를 검토할 수 있지만, 출력 순서와 admissibility 증명을 다시 해야 한다.

## 오늘 문제와의 연결

2026-10-15의 [CSES 1196 `Flight Routes`](https://cses.fi/problemset/task/1196/)는 도시 `1`에서 `n`까지
가장 싼 route `k`개의 가격을 요구한다. 방향 간선 비용은 모두 양수이고 `k <= 10`이며, 같은 도시를
여러 번 방문할 수 있고 같은 가격의 route도 각각 세므로 이 문서의 pop-count best-first 모델과 정확히
맞는다.

[`../2026-10-15/icpc_problem.cpp`](../2026-10-15/icpc_problem.cpp)는 다음 대회용 세부사항까지 반영한다.

- 동일 비용 후보를 합치지 않는 최소 힙
- 정점별 최대 k번의 유효 pop과 확장
- 도착점의 k번째 유효 pop에서만 조기 종료
- `std::int64_t` 누적 비용과 덧셈 전 overflow guard
- 재귀 없는 구현으로 호출 스택 깊이와 무관한 실행

## 초보자 직접 해보기

1. 간선 `1->2(1)`, `2->3(1)`, `1->3(5)`, `2->2(2)`와 `k=4`를 종이에 그린다. 힙에서
   꺼내는 `(비용, 정점)` 순서와 `accepted` 배열을 매 단계 적고 도착 비용을 구한다.
2. `1->2(5)`인 평행 간선을 두 개 넣고 `k=2`로 실행한다. 출력이 `5 5`인지 확인하고, 힙 항목을
   `set`으로 바꾸면 왜 오답인지 설명한다.
3. `accepted[v]`를 pop이 아니라 push 때 증가시키도록 바꿔 반례를 찾는다. 비싼 후보가 먼저 push된 뒤
   싼 우회 후보가 들어오는 그래프를 만들면 된다.
4. 도착점의 인접 간선 확장을 막고 `1->2(1)`, `2->1(1)`, `k=3`을 실행한다. 올바른 답
   `1 3 5` 중 무엇을 잃는지 확인한다.
5. `current.cost + edge.cost`를 guard 없이 계산했을 때 부호 있는 정수 overflow가 왜 미정의 동작인지,
   `current.cost > MAX - edge.cost` 검사가 왜 덧셈보다 먼저여야 하는지 설명한다.
6. 각 정점이 최대 k번만 확장된다는 사실에서 모든 인접 리스트 순회 횟수의 합이 왜 `k*m` 이하인지
   식으로 증명한다.

## 초보자 이해 검증

- `Candidate{7, 3}`은 “정점 3의 최단 거리가 7로 확정됐다”가 아니라 무엇을 뜻하는가?
- `accepted[3] == 2`라면 어떤 두 사실이 이미 확정되었고, 세 번째 후보에는 무엇을 해야 하는가?
- `CandidateGreater`가 `left.cost > right.cost`를 반환할 때 작은 비용이 `top()`에 오는 이유는 무엇인가?
- 도착 비용 `4 4 7`에서 두 개의 4를 하나로 합치면 안 되는 이유는 무엇인가?
- 시간 복잡도의 `k*m`은 힙 크기가 아니라 어떤 연산 횟수를 센 것인가?
- 양수 간선 조건이 최소 힙의 전역 순서 증명에서 쓰이는 정확한 위치는 어디인가?

이 질문을 코드 없이 설명하고 작은 예제를 손으로 추적할 수 있으면, 일반 다익스트라와 k-shortest-walk
best-first 탐색의 차이를 이해한 것이다.
