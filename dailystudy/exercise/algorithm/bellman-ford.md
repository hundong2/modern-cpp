# 벨만–포드(Bellman–Ford) 최단 경로

## 정의

벨만–포드는 하나의 시작 정점에서 모든 정점까지의 최단 거리를 구하는 알고리즘이다. 다익스트라와 달리 **음수 가중치 간선**을 허용하며, 시작점에서 도달 가능한 **음수 사이클**도 판정한다. 음수 사이클을 계속 돌 수 있으면 비용을 끝없이 낮출 수 있으므로 그 영향을 받는 정점에는 유한한 최단 거리가 없다.

## 적용 조건

- 방향·무방향 그래프 모두 쓸 수 있다. 무방향 간선은 양방향 간선 두 개로 저장한다.
- 음수 가중치가 있거나 음수 사이클 존재 여부를 알아야 할 때 적합하다.
- 정점 수를 `V`, 간선 수를 `E`라 할 때 `O(VE)`가 제한 안에 들어와야 한다.
- 모든 가중치가 음수가 아니고 그래프가 크면 보통 우선순위 큐 다익스트라가 더 빠르다.
- 모든 쌍 거리가 필요하고 `V`가 작다면 플로이드–워셜도 비교한다.

## 핵심 아이디어와 불변식

간선 `(u, v, w)`에 대해 `distance[u] + w < distance[v]`라면, 현재 알고 있는 `v` 경로보다 `u`를 거치는 경로가 더 짧다. 이때 `distance[v]`를 낮추는 일을 **완화(relaxation)** 라고 한다.

전체 간선을 `i`번 순회한 뒤의 핵심 불변식은 다음과 같다.

> `distance[v]`는 시작점에서 `v`까지 간선 수가 최대 `i`인 경로 중 최소 비용이다. 아직 발견하지 못한 경우 INF다.

음수 사이클이 없는 최단 경로는 정점을 반복할 필요가 없는 단순 경로로 고를 수 있고, 단순 경로의 간선 수는 최대 `V-1`이다. 따라서 `V-1`번 순회면 모든 유한 최단 거리가 확정된다. `V`번째에도 도달 가능한 간선이 완화되면 정점을 반복하면서 더 짧아졌다는 뜻이고, 그 반복 부분에 음수 사이클이 있다.

중요한 필터는 `distance[u] != INF`다. 이 검사를 빼면 시작점에서 닿지 않는 음수 사이클까지 잘못 보고하거나 `INF + w`를 실제 경로처럼 취급할 수 있다.

## 단계별 절차

1. 모든 거리를 INF로 만들고 시작점 거리만 `0`으로 둔다.
2. 전체 간선을 `V-1`번 반복해서 완화한다.
3. 한 순회에서 갱신이 하나도 없으면 모든 최단 거리가 이미 안정됐으므로 조기 종료한다.
4. 음수 사이클 판정이 필요하면 전체 간선을 한 번 더 순회한다.
5. 시작점에서 도달 가능한 출발점의 간선이 다시 완화되면 도달 가능한 음수 사이클이 있다.

## 의사 코드

```text
distance[*] <- INF
distance[start] <- 0

for iteration <- 1 .. V:
    updated <- false
    for each edge (u, v, w):
        if distance[u] == INF:
            continue
        if distance[u] + w < distance[v]:
            distance[v] <- distance[u] + w
            updated <- true
            if iteration == V:
                reachable_negative_cycle <- true
    if not updated:
        break
```

## 컴파일 가능한 C++ 뼈대

아래 예제는 시작점 `0`에서 거리를 구한다. `nullopt`는 도달 가능한 음수 사이클 때문에 유한한 답이 없음을 뜻한다.

```cpp
// <iostream>은 예제 결과를 출력하는 std::cout을 선언한다.
#include <iostream>
// <limits>는 안전한 INF를 만드는 std::numeric_limits를 선언한다.
#include <limits>
// <optional>은 성공 거리 또는 실패를 표현하는 std::optional을 선언한다.
#include <optional>
// <vector>는 간선과 거리 배열을 소유하는 std::vector를 선언한다.
#include <vector>

// 간선은 출발점, 도착점, 가중치를 값으로 묶는다.
struct Edge {
    int from{};
    int to{};
    int weight{};
};

// 성공하면 거리 vector를 값으로 반환하고, 음수 사이클이면 nullopt를 반환한다.
std::optional<std::vector<long long>> bellman_ford(
    int vertex_count,
    const std::vector<Edge>& edges,
    int start) {
    // max()/4는 실제 거리 덧셈이 long long 범위를 넘지 않게 여유를 둔 도달 불가 표지다.
    constexpr long long infinity{std::numeric_limits<long long>::max() / 4};
    // count/value 생성자는 모든 거리를 infinity로 채운다.
    std::vector<long long> distance(static_cast<std::size_t>(vertex_count), infinity);
    // 호출자가 0 <= start < vertex_count를 지킨다는 전제에서 시작 거리를 0으로 둔다.
    distance[static_cast<std::size_t>(start)] = 0;

    // V번째 순회는 음수 사이클 검사용이다.
    for (int iteration{1}; iteration <= vertex_count; ++iteration) {
        bool updated{};
        // const 참조는 간선을 복사하지 않고 읽는다.
        for (const Edge& edge : edges) {
            const auto from{static_cast<std::size_t>(edge.from)};
            const auto to{static_cast<std::size_t>(edge.to)};
            // 도달하지 못한 출발점에서는 완화하지 않는다.
            if (distance[from] == infinity) {
                continue;
            }
            const long long candidate{distance[from] + edge.weight};
            if (candidate < distance[to]) {
                distance[to] = candidate;
                updated = true;
                // V번째 감소는 시작점에서 닿는 음수 사이클의 증거다.
                if (iteration == vertex_count) {
                    return std::nullopt;
                }
            }
        }
        // 안정된 거리에는 이후 순회가 영향을 주지 못한다.
        if (!updated) {
            break;
        }
    }

    // 반환문의 자동 지역 객체는 암시적 이동 후보가 되어 optional 내부 vector로 자원 이전될 수 있다.
    return distance;
}

int main() {
    // 0 -> 1 -> 2의 비용은 4 + (-2) = 2다.
    const std::vector<Edge> edges{{0, 1, 4}, {0, 2, 5}, {1, 2, -2}};
    const auto result{bellman_ford(3, edges, 0)};
    // 성공 상태를 먼저 확인한 뒤 vector의 2번 원소를 읽는다.
    if (result) {
        std::cout << (*result)[2] << '\n';
    }
}
```

예상 출력은 `2`다.

## 정확성 근거

### 보조정리 1: i회 순회 불변식

귀납법으로 증명한다. `i=0`일 때 시작점의 빈 경로 비용은 0이고 다른 정점은 INF이므로 성립한다. `i-1`회 뒤 간선 수가 최대 `i-1`인 최단 경로가 반영됐다고 하자. 간선 수가 최대 `i`인 어떤 최단 경로의 마지막 간선을 `(u,v,w)`라 하면 앞부분은 최대 `i-1`개 간선이다. i번째 순회가 `(u,v,w)`를 완화할 때 `distance[u]+w`를 `distance[v]` 후보로 반영하므로 명제가 성립한다.

### 보조정리 2: V-1회면 충분함

도달 가능한 음수 사이클이 없으면 최단 walk에서 반복 정점을 이루는 사이클을 제거해도 비용이 커지지 않는다. 따라서 정점을 반복하지 않는 단순 최단 경로가 존재하며 간선 수는 최대 `V-1`이다. 보조정리 1에 의해 `V-1`회 뒤 모든 유한 최단 거리가 반영된다.

### 보조정리 3: V번째 완화와 음수 사이클

V번째에도 거리가 감소하면 V개 이상 간선을 사용해 더 짧아지는 도달 가능한 walk가 있다. V개 간선을 지나면 비둘기집 원리에 따라 어떤 정점을 반복하며, 감소를 가능하게 한 반복 구간은 음수 사이클을 포함한다. 반대로 시작점에서 도달 가능한 음수 사이클이 있으면 그 사이클을 반복할 때마다 비용을 낮출 수 있어 V번째 이후에도 완화가 가능하다.

세 보조정리에 따라 알고리즘은 음수 사이클이 없을 때 정확한 최단 거리를 반환하고, 있을 때 유한한 답이 없음을 정확히 판정한다.

## 시간·공간 복잡도

- 시간: 전체 간선 `E`개를 최대 `V`번 순회하므로 `O(VE)`다.
- 공간: 간선 목록 `O(E)`와 거리 배열 `O(V)`를 합쳐 `O(V+E)`다.
- 조기 종료는 실제 실행을 줄일 수 있지만 최악 복잡도는 여전히 `O(VE)`다.

## 흔한 실수

- `distance[u] == INF` 검사를 빼서 도달 불가능한 음수 사이클을 보고한다.
- INF를 너무 큰 값으로 잡고 `INF + weight`에서 정수 오버플로를 일으킨다.
- `V-1`회만 돌고 음수 사이클 검사 순회를 하지 않는다.
- 무방향 간선을 한 방향만 저장한다.
- 음수 간선이 있는 그래프에 방문 확정 방식의 다익스트라를 그대로 쓴다.
- 음수 사이클이 그래프 어딘가에 있는지와 **시작점에서 도달 가능한지**를 구분하지 않는다.

## 변형과 대회 필수 연결

- 모든 정점을 가상의 슈퍼 시작점과 비용 0으로 연결하면 그래프 전체의 음수 사이클을 검사할 수 있다.
- V번째에 갱신된 정점을 기록하고 부모 포인터를 V번 거슬러 올라가면 실제 음수 사이클 하나를 복원할 수 있다.
- V번째 갱신 정점에서 그래프 탐색을 하면 최단 거리가 음의 무한대로 영향을 받는 정점을 표시할 수 있다.
- 차분 제약 `x_v <= x_u + w`의 모순 검사는 벨만–포드 완화와 같은 형태다.
- SPFA는 큐로 필요한 정점만 다시 처리하는 변형이지만 최악 `O(VE)`이고 적대적 데이터에서 느릴 수 있다.

## 오늘 문제와의 연결

BOJ 11657 「타임머신」은 1번 도시만 시작점이다. 따라서 `distance[from] != INF`인 간선만 완화해야 하며, N번째 순회에서 그런 간선이 다시 감소할 때만 `-1` 하나를 출력한다. 음수 간선이 있어도 음수 사이클이 없으면 2번부터 N번까지 유한 거리 또는 도달 불가 `-1`을 출력한다.

## 직접 해보기와 초보자 검증

1. 간선 `0->1(4), 1->2(-2), 0->2(5)`의 거리 배열을 순회마다 손으로 적는다.
2. `2->1(1)`을 추가하면 사이클 비용이 `-1`이 되는지 계산하고 V번째 갱신을 확인한다.
3. 시작점과 연결되지 않은 음수 사이클을 추가하고 `distance[u] == INF` 검사를 지웠을 때 생기는 오답을 설명한다.
4. 모든 가중치를 0 이상으로 바꾼 뒤 다익스트라와 답·복잡도를 비교한다.
5. `long long` 대신 `int`와 `INT_MAX`를 쓸 때 덧셈 전 어떤 검사가 필요한지 말로 증명한다.
