# 최대 유량과 Edmonds–Karp

## 정의

유향 그래프에서 source에서 sink로 보내는 **유량(flow)** 의 최댓값을 구하는 문제다. 간선 `(u, v)`의 용량 `c(u,v)`와 현재 유량 `f(u,v)`는 다음 조건을 만족해야 한다.

- 용량 제한: `0 <= f(u,v) <= c(u,v)`
- 유량 보존: source와 sink를 제외한 정점은 들어온 유량의 합과 나간 유량의 합이 같다.
- 유량 값: source에서 나간 순유량, 또는 sink로 들어온 순유량이다.

**Ford–Fulkerson 방법**은 잔여 그래프에서 증가 경로를 반복해 찾는 틀이다. 그 증가 경로를 BFS로 찾아 간선 수가 가장 짧은 경로를 고르는 구체 구현이 **Edmonds–Karp 알고리즘**이다.

## 적용 조건

- 각 간선에 음수가 아닌 용량이 있고 한 source에서 한 sink로 최대 전송량을 구할 때 적용한다.
- 정수 용량이면 매 증가량도 정수이고 알고리즘이 유한 번에 끝난다.
- 파이프·통신망뿐 아니라 이분 매칭, 간선 서로소 경로, 프로젝트 선택, 최소 컷 모델링의 기초가 된다.
- 정점이 수백, 간선이 수천 정도인 교육·중간 규모 문제에 구현 안정성이 좋다.
- 큰 그래프나 성능 제한이 엄격하면 Dinic `O(V^2E)` 일반 상한, push-relabel 등 더 빠른 방법을 검토한다.
- 비용까지 최소화해야 하면 최대 유량만으로는 부족하고 최소 비용 최대 유량이 필요하다.

무방향 용량 간선은 문제 의미에 따라 양쪽 방향 용량을 각각 추가한다. 이는 “한 방향으로만 한 번 쓰는 유향 간선” 두 개와 같은 입력 모델이며, 같은 두 정점의 평행 간선은 용량을 합치거나 간선을 각각 보관한다.

## 핵심 아이디어와 불변식

### 잔여 용량과 역방향 간선

현재 유량에서 `(u,v)` 방향으로 더 보낼 수 있는 양은 `c(u,v)-f(u,v)`다. 이미 `u→v`로 `x`만큼 보냈다면 반대 방향 `v→u`에는 `x`만큼의 잔여 용량을 둔다. 반대 방향으로 흐르는 것은 실제 물을 거꾸로 새로 보내는 뜻이 아니라, 이전에 선택한 `u→v` 유량을 줄이고 다른 경로로 재배치하는 장부 표현이다.

역방향 잔여 간선이 없으면 먼저 고른 증가 경로가 나중의 더 좋은 조합을 막을 수 있다. 최대 유량을 단순 경로 탐욕이 아니라 올바른 알고리즘으로 만드는 핵심이 이 “선택 취소 가능성”이다.

반복마다 다음 불변식을 지킨다.

1. 현재 `flow`는 용량 제한을 위반하지 않는다.
2. 중간 정점의 유량 보존이 유지된다.
3. 정방향 유량을 `delta` 늘릴 때 역방향 유량을 `delta` 줄여 반대 잔여 용량을 만든다.
4. `total`은 현재 실행 가능한 유량의 값이다.
5. BFS의 `parent`는 source에서 각 발견 정점까지 잔여 용량이 양수인 최단 간선 수 경로를 표현한다.

## 단계별 절차

1. 모든 유량을 0으로 초기화한다.
2. 잔여 용량 `capacity[u][v] - flow[u][v] > 0`인 간선만 따라 source에서 BFS한다.
3. sink를 발견하지 못하면 종료한다.
4. `parent`를 sink에서 source까지 따라가며 경로의 최소 잔여 용량 `bottleneck`을 구한다.
5. 경로의 각 정방향 유량에 `bottleneck`을 더하고 역방향 유량에서는 뺀다.
6. 전체 유량에 `bottleneck`을 더하고 2단계로 돌아간다.

## 의사 코드

```text
flow[*][*] = 0
answer = 0

repeat:
    parent[*] = UNVISITED
    parent[source] = source
    BFS queue = [source]

    while queue is not empty and sink is unvisited:
        u = queue.pop_front()
        for each v adjacent to u:
            if v is unvisited and capacity[u][v] - flow[u][v] > 0:
                parent[v] = u
                queue.push_back(v)

    if sink is unvisited:
        return answer

    delta = infinity
    for v = sink back to source through parent:
        u = parent[v]
        delta = min(delta, capacity[u][v] - flow[u][v])

    for v = sink back to source through parent:
        u = parent[v]
        flow[u][v] += delta
        flow[v][u] -= delta

    answer += delta
```

## 컴파일 가능한 C++ 뼈대

아래 예제는 네 정점의 고정 네트워크에서 `S→T` 최대 유량 5를 출력한다. 실제 대회에서는 정점 수에 맞춰 컨테이너 크기와 입력 부분을 바꾼다.

```cpp
// <algorithm>은 병목의 작은 값을 고르는 std::min을 선언한다.
#include <algorithm>
// <array>는 고정 크기 용량·유량 행렬과 부모 배열 std::array를 선언한다.
#include <array>
// <iostream>은 결과 출력 객체 std::cout과 삽입 연산자를 선언한다.
#include <iostream>
// <limits>는 int 최댓값을 주는 std::numeric_limits를 선언한다.
#include <limits>
// <queue>는 BFS FIFO 컨테이너 어댑터 std::queue를 선언한다.
#include <queue>

constexpr int kVertices{4}; // 정점 0=S, 3=T인 작은 예제다.
using Matrix = std::array<std::array<int, kVertices>, kVertices>; // 4x4 정수 행렬 별칭이다.

// capacity를 const lvalue 참조로 빌리고 source/sink는 값으로 복사해 최대 유량 값을 반환한다.
[[nodiscard]] int edmonds_karp(const Matrix& capacity, int source, int sink) {
    Matrix flow{}; // 모든 유량을 0으로 값 초기화해 실행 가능한 흐름에서 시작한다.
    int answer{};  // 누적 최대 유량 후보를 0으로 초기화한다.

    while (true) { // 증가 경로가 없으면 함수 안에서 반환한다.
        std::array<int, kVertices> parent{}; // BFS 부모 저장소를 함수가 소유한다.
        // 범위 for는 array 반복자로 각 int를 lvalue 참조로 빌려 O(V)에 -1을 저장한다.
        for (int& value : parent) {
            value = -1;
        }
        parent[source] = source; // source를 방문 처리한다.

        std::queue<int> pending{}; // 기본 생성한 FIFO 큐는 비어 있다.
        // push(const int&)는 source를 복사해 큐 크기를 1 늘리고 void를 반환한다. 상수 시간, 할당 실패 가능성이 있다.
        pending.push(source);

        // empty()는 bool로 빈 상태만 관찰한다. sink가 발견되면 단락 평가로 BFS를 멈춘다.
        while (!pending.empty() && parent[sink] == -1) {
            // front()의 int&를 current 값에 복사해 다음 pop 뒤에도 안전하게 사용한다. 큐는 비어 있지 않아야 한다.
            const int current{pending.front()};
            // pop()은 void를 반환하고 첫 원소를 제거한다. 제거 원소 참조는 무효가 되지만 current 복사본은 유효하다.
            pending.pop();

            for (int next{}; next < kVertices; ++next) { // 모든 후보 정점을 O(V)에 검사한다.
                const int residual{capacity[current][next] - flow[current][next]};
                if (parent[next] == -1 && residual > 0) { // 미방문 잔여 간선만 BFS 트리에 넣는다.
                    parent[next] = current;
                    pending.push(next); // next 값을 큐 끝에 복사하고 크기를 1 늘린다.
                }
            }
        }

        if (parent[sink] == -1) {
            return answer; // 증가 경로가 없으므로 최대 유량-최소 컷 정리로 answer가 최대다.
        }

        // max()는 상태 없이 int 최댓값 prvalue를 O(1)에 반환하며 병목 첫 비교의 항등적 초기값으로 쓴다.
        int delta{std::numeric_limits<int>::max()};
        for (int vertex{sink}; vertex != source; vertex = parent[vertex]) {
            const int previous{parent[vertex]};
            const int residual{capacity[previous][vertex] - flow[previous][vertex]};
            // min은 두 const int&를 빌려 작은 값 참조를 반환하고 대입이 즉시 값을 복사한다. O(1), 상태 변화 없음이다.
            delta = std::min(delta, residual);
        }

        for (int vertex{sink}; vertex != source; vertex = parent[vertex]) {
            const int previous{parent[vertex]};
            flow[previous][vertex] += delta; // 정방향으로 병목만큼 더 보낸다.
            flow[vertex][previous] -= delta; // 역방향 취소 가능량을 같은 만큼 만든다.
        }
        answer += delta; // 양의 증가량을 현재 유량 값에 반영한다.
    }
}

int main() {
    Matrix capacity{}; // 고정 행렬 전체를 0으로 값 초기화한다.
    capacity[0][1] = 3; // S→A 용량 3이다.
    capacity[0][2] = 2; // S→B 용량 2이다.
    capacity[1][2] = 1; // A→B 재배치 통로 용량 1이다.
    capacity[1][3] = 2; // A→T 용량 2이다.
    capacity[2][3] = 3; // B→T 용량 3이다.

    // edmonds_karp는 int prvalue 5를 반환한다. operator<<는 cout에 쓰고 같은 ostream&를 반환하며 최종 참조는 버린다.
    std::cout << edmonds_karp(capacity, 0, 3) << '\n';
    return 0;
}
```

직접 컴파일한다.

```powershell
g++ -std=c++23 -Wall -Wextra -Wpedantic example.cpp -o example.exe
./example.exe
```

예상 출력은 `5`다.

## 정확성 근거

### 보조정리 1: 한 번의 갱신 뒤에도 흐름은 실행 가능하다

증가 경로의 모든 잔여 용량은 양수이고 `delta`는 그 최솟값이다. 따라서 정방향 유량에 `delta`를 더해도 어느 간선도 용량을 넘지 않는다. 경로의 중간 정점에는 들어오는 간선과 나가는 간선이 모두 같은 `delta`만큼 바뀌므로 유량 보존도 유지된다. 역방향 값을 함께 갱신해 이후 취소 가능량도 정확히 표현한다.

### 보조정리 2: 반복마다 전체 유량이 `delta`만큼 증가한다

source에서는 경로의 첫 간선으로 `delta`가 더 나가고 sink에는 마지막 간선으로 `delta`가 더 들어온다. 중간 정점의 순유량 변화는 0이다. 그러므로 흐름 값은 정확히 양의 `delta`만큼 증가한다.

### 정리: 알고리즘 종료 시 반환값은 최대 유량이다

BFS가 sink에 도달하지 못한 종료 시점에 잔여 그래프에서 source로부터 도달 가능한 정점 집합을 `S`, 나머지를 `T`라 하자. `S→T`의 모든 간선은 잔여 용량이 0이므로 포화됐고, `T→S` 방향의 양의 흐름이 있었다면 그 취소 역간선으로 도달할 수 있어 모순이다. 따라서 현재 유량 값은 이 컷의 용량과 같다. 어떤 유량도 어떤 s-t 컷 용량을 넘지 못하므로 현재 유량은 최대다.

Edmonds–Karp는 증가 경로를 BFS 최단 경로로 제한해 종료뿐 아니라 다항 시간도 보장한다.

## 시간·공간 복잡도

- 인접 목록 BFS 한 번: `O(E)`
- 경로 역추적과 갱신: `O(V)`
- BFS 최단거리의 단조성으로 증가 횟수: `O(VE)`
- 총 시간: `O(VE^2)`
- 용량·유량 행렬을 쓰는 오늘 구현 공간: `O(V^2+E)`
- `Edge{to, reverse, capacity}` 잔여 인접 목록 구현 공간: `O(V+E)`

행렬에서 모든 `next=0..V-1`을 훑으면 BFS가 `O(V^2)`가 된다. 오늘 BOJ 6086 풀이는 용량 조회는 행렬, 탐색은 인접 목록을 함께 써 `O(E)` BFS를 유지한다. 중복 이웃이 입력 횟수만큼 들어가도 `E`를 입력 간선 수 기준으로 보면 복잡도 안에 포함된다.

## 흔한 실수

- 역방향 유량을 갱신하지 않아 먼저 고른 경로를 취소하지 못한다.
- `capacity[u][v] = c`로 평행 간선을 덮어쓰고 합을 잃는다.
- 무방향 파이프인데 반대 방향 용량을 추가하지 않는다.
- BFS마다 `parent` 방문 배열을 초기화하지 않는다.
- source를 방문 처리하지 않아 큐에 다시 넣고 부모 경로를 손상한다.
- `queue::front()` 참조를 저장한 뒤 `pop()`하고 그 참조를 사용한다. 먼저 값으로 복사해야 한다.
- 병목을 경로 전체가 아니라 마지막 간선 용량으로 정한다.
- `int` 합계가 입력 상한을 넘는 문제에서도 무조건 `int`를 사용한다. 상한을 계산해 `long long` 필요 여부를 결정한다.
- 무향 간선을 단순히 `flow[v][u] = -flow[u][v]` 하나로만 생각해, 원래 반대 방향 용량과 취소 잔여 용량을 구분하지 못한다.
- Ford–Fulkerson의 임의 DFS 구현에도 Edmonds–Karp의 `O(VE^2)` 복잡도를 잘못 붙인다.

## 변형과 대회 필수 연결

- **최대 유량-최소 컷**: 마지막 잔여 그래프에서 source로 도달 가능한 집합과 나머지 사이가 최소 컷이다.
- **이분 매칭**: source→왼쪽, 가능한 짝 왼쪽→오른쪽, 오른쪽→sink 간선의 용량을 1로 둔다. 최대 유량이 최대 매칭 크기다.
- **간선 서로소 경로**: 각 간선 용량을 1로 두면 최대 유량이 동시에 사용할 수 있는 간선 서로소 경로 수다.
- **정점 용량/정점 서로소 경로**: 정점 `v`를 `v_in→v_out`으로 분할하고 그 사이 용량을 정점 제한으로 둔다.
- **다중 source/sink**: 초 source와 초 sink를 추가하고 충분히 큰 용량으로 연결한다.
- **Dinic**: BFS 레벨 그래프와 DFS blocking flow로 보통 더 빠르며 대회 표준 구현으로 자주 쓰인다.
- **최소 비용 최대 유량**: 간선마다 비용이 있고 최대량 중 최소 비용을 구할 때 사용한다.

우승권 풀이에서는 알고리즘 이름을 외우는 데서 멈추지 않고, 모델링에서 “무엇이 용량 1인가”, “정점 제한을 어떻게 간선 제한으로 바꾸는가”, “최종 잔여 그래프에서 답 구조를 어떻게 복원하는가”를 빠르게 결정해야 한다.

## 오늘 문제와의 연결

[2026-08-20 BOJ 6086 풀이](../2026-08-20/icpc_problem.cpp)는 알파벳 대소문자 52개를 정점으로 매핑한다. 같은 파이프 입력은 `capacity[from][to] += amount`로 합치고, 양방향이므로 반대쪽 용량에도 더한다. 용량·유량은 고정 행렬에서 `O(1)`로 조회하고, BFS는 인접 목록의 실제 이웃만 본다. `parent`가 찾은 경로의 병목을 흘린 뒤 역방향 유량을 빼는 줄이 정답성의 핵심이다.

## 직접 해보기와 초보자 검증

1. 위 예제의 각 증가 경로, 병목, 갱신 후 잔여 행렬을 종이에 적고 출력 5를 확인한다.
2. 역방향 갱신 두 줄 중 하나를 지우고 일반 그래프에서 왜 실패하는지 반례를 만든다.
3. 평행 간선 두 개를 입력하고 덮어쓰기와 누적의 결과 차이를 계산한다.
4. BFS 종료 후 source에서 도달 가능한 정점을 출력해 최소 컷을 복원한다.
5. 왼쪽 3명, 오른쪽 3명의 작은 이분 그래프를 유량 네트워크로 바꾸고 매칭 간선을 출력한다.
6. 행렬 탐색과 인접 목록 탐색의 BFS 연산 횟수를 희소 그래프와 밀집 그래프에서 비교한다.
7. `front()`의 참조를 `pop()` 뒤 사용하는 잘못된 코드를 만들고 수명 규칙을 설명한 뒤 값 복사로 고친다.
8. 다음 질문에 자료 없이 답하면 통과다: 잔여 용량 정의, 역간선 역할, 병목 선택, 불변식 두 개, 종료 시 최대성, `O(VE^2)` 유도.
