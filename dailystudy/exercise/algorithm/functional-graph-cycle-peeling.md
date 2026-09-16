# 함수형 그래프의 진입 차수 제거와 사이클 DP

## 정의와 적용 조건

함수형 그래프(functional graph)는 모든 정점의 **나가는 간선이 정확히 하나**인 유향 그래프다. `next[v]`를 계속 따라가면 유한한 정점 중 하나를 반드시 다시 만나므로, 각 연결 성분은 정확히 하나의 유향 사이클과 그 사이클로 흘러드는 나무 형태의 꼬리들로 이루어진다. 간선이 정확히 하나라는 조건이 깨지면 이 구조와 아래 점화식은 일반적으로 성립하지 않는다.

`next[v]`와 각 정점으로 들어오는 간선 수 `indegree[v]`를 저장할 수 있고, 사이클 길이 또는 사이클까지의 거리처럼 후속 정점의 답으로 현재 답을 계산할 때 유용하다. 입력 크기 `n`만큼 메모리를 쓰며, 재귀 깊이를 제한해야 하는 큰 입력에도 적합하다.

## 핵심 아이디어와 불변식

1. 진입 차수 0인 정점은 사이클에 있을 수 없으므로 제거 대기열에 넣는다.
2. `v`를 제거할 때 `next[v]`의 진입 차수를 하나 줄이고 0이 되면 그 정점도 제거한다. `peeled`는 제거 순서이자 배열 기반 FIFO다.
3. 제거가 끝난 뒤 `indegree[v] > 0`인 정점은 **정확히 사이클 위의 정점**이다. 남은 그래프는 모든 정점의 진입·출력 차수가 각각 1인 유향 사이클들의 합집합이다.
4. 사이클 정점의 답을 길이로 채운 후 `peeled`를 역순으로 보면 항상 `next[v]`의 답이 이미 확정되어 있다. 그래서 `answer[v] = answer[next[v]] + 1`이다.

실전에서 가장 중요한 구분은 **제거 순서가 위상 순서, 역순이 DP 순서**라는 점이다. 사이클을 먼저 처리하지 않고 역순 DP를 시작하면 기저값이 비어 있다.

## 단계별 절차

1. 입력의 목적지를 `next[1..n]`에 저장하면서 `indegree[next[v]]`를 하나씩 증가시킨다.
2. 진입 차수 0인 정점을 모두 `peeled`에 넣는다.
3. 인덱스 `head`를 0부터 증가시키며 `peeled[head]`를 제거한다. 그 목적지의 진입 차수가 0이 되면 뒤에 추가한다.
4. 남은 정점 중 답이 아직 0인 정점마다 `next`를 출발점으로 돌아올 때까지 따라가 사이클 길이를 센다.
5. 같은 사이클을 다시 한 바퀴 돌며 각 정점의 답을 그 길이로 기록한다.
6. `peeled`를 역순으로 훑으며 후속 정점 답에 1을 더한다.
7. 모든 답을 원래 정점 번호 순서로 출력한다.

## 의사코드와 C++ 뼈대

```text
for v = 1..n:
    indegree[next[v]]++
for v = 1..n:
    if indegree[v] == 0: peeled.push(v)
for head = 0..peeled.size()-1:  // 뒤에 더 넣을 수 있는 동적 끝
    v = peeled[head]
    w = next[v]
    indegree[w]--
    if indegree[w] == 0: peeled.push(w)
for each v with indegree[v] > 0 and answer[v] == 0:
    L = length of cycle beginning at v
    set answer[u] = L for every u on that cycle
for v in reverse(peeled):
    answer[v] = answer[next[v]] + 1
```

다음 골격은 완전한 C++17 제출 예제다. 학습용 정밀 호출 계약은 날짜별 파일에 있다.

```cpp
#include <cstddef>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n{};
    std::cin >> n;
    std::vector<int> next(static_cast<std::size_t>(n + 1), 0);
    std::vector<int> indegree(static_cast<std::size_t>(n + 1), 0);
    std::vector<int> answer(static_cast<std::size_t>(n + 1), 0);
    std::vector<int> peeled;
    peeled.reserve(static_cast<std::size_t>(n));
    for (int v{1}; v <= n; ++v) {
        std::cin >> next[static_cast<std::size_t>(v)];
        ++indegree[static_cast<std::size_t>(next[static_cast<std::size_t>(v)])];
    }
    for (int v{1}; v <= n; ++v) {
        if (indegree[static_cast<std::size_t>(v)] == 0) peeled.push_back(v);
    }
    for (std::size_t head{}; head < peeled.size(); ++head) {
        const int w{next[static_cast<std::size_t>(peeled[head])]};
        if (--indegree[static_cast<std::size_t>(w)] == 0) peeled.push_back(w);
    }
    for (int v{1}; v <= n; ++v) {
        if (indegree[static_cast<std::size_t>(v)] == 0 ||
            answer[static_cast<std::size_t>(v)] != 0) continue;
        int length{};
        int u{v};
        do { ++length; u = next[static_cast<std::size_t>(u)]; } while (u != v);
        u = v;
        do {
            answer[static_cast<std::size_t>(u)] = length;
            u = next[static_cast<std::size_t>(u)];
        } while (u != v);
    }
    for (std::size_t remaining{peeled.size()}; remaining > 0; --remaining) {
        const int v{peeled[remaining - 1]};
        answer[static_cast<std::size_t>(v)] =
            answer[static_cast<std::size_t>(next[static_cast<std::size_t>(v)])] + 1;
    }
    for (int v{1}; v <= n; ++v) {
        std::cout << answer[static_cast<std::size_t>(v)] << (v == n ? '\n' : ' ');
    }
}
```

## 정확성 근거

**보조정리 1 — 제거 후 남는 정점은 정확히 사이클 정점이다.** 사이클 위 정점은 사이클의 직전 정점에서 항상 간선을 하나 받으므로 진입 차수가 0이 되어 제거될 수 없다. 반대로 사이클 밖 정점에서 후속 간선을 계속 따라가면 유일한 사이클에 닿는다. 사이클 밖 정점들만 보면 유향 비순환 그래프이므로, 남아 있는 사이클 밖 정점이 있다면 그중 진입 차수 0인 정점이 존재해 제거 절차가 끝났다는 가정과 모순이다.

**보조정리 2 — 사이클 답은 그 길이다.** 사이클의 정점에서 시작하면 사이클의 서로 다른 모든 정점을 차례로 보고, 길이 `L`번째 간선으로 처음 출발 정점에 재도착한다. 따라서 어느 시작 정점에서든 최초 재방문까지 순간이동 수는 `L`이다.

**보조정리 3 — 역순 점화식은 정확하다.** `v`가 제거될 때 `next[v]`는 아직 대기열 뒤쪽에 있거나 사이클에 남는다. 그러므로 제거 순서를 뒤집으면 `next[v]`의 답을 이미 알고 있다. `v`에서 첫 순간이동 후에는 `next[v]`에서 시작한 과정과 같고, `v`는 그 과정 중 이미 방문한 적이 없다. 따라서 최초 재방문까지 정확히 `1 + answer[next[v]]`번 이동한다. 역순 귀납으로 모든 제거 정점의 답이 정확하다.

세 보조정리로 남은 사이클과 제거된 모든 정점의 답이 정확하므로 출력 전체가 문제의 답이다.

## 시간·공간 복잡도

- 각 정점은 초기 진입 차수 계산에서 한 번, 대기열에서 최대 한 번, 사이클 길이·기록에서 각각 최대 한 번, 역순 DP에서 최대 한 번 방문한다.
- 총 시간 `O(n)`, `next`·`indegree`·`answer`·`peeled`를 합친 추가 공간 `O(n)`이다.
- 재귀 호출이 없어 길이 200,000의 꼬리에서도 호출 스택이 커지지 않는다. `reserve(n)`은 상각 선형이라는 점근 보장을 바꾸지 않지만 재할당을 줄인다.

## 흔한 실수

1. 진입 차수 0을 한 번만 제거하고 새로 0이 된 정점을 대기열에 넣지 않는다.
2. 사이클 정점의 답을 1로 일괄 처리한다. 자기 루프만 1이고 길이 `L` 사이클은 `L`이다.
3. 첫 방문 정점에 *도착하는 순간의 이동*을 세지 않아 1만큼 작은 답을 낸다.
4. `peeled`를 정순 DP에 사용한다. 후속 정점 답이 아직 없다.
5. 모든 시작점에서 직접 따라가 `O(n^2)`가 된다.
6. 재귀 DFS로 길이 200,000의 꼬리를 처리해 호출 스택을 넘긴다.
7. 입력의 1-based 정점 번호를 0-based vector 크기와 섞어 범위 밖 접근을 한다.
8. `peeled.size()`가 처리 중 커질 수 있는데 처음 크기로 반복 끝을 고정한다.

## 변형

- **사이클 길이와 꼬리 거리 분리:** 사이클 정점에 `cycle_length=L`, `distance=0`을 두고 역순에서 길이는 복사, 거리는 1 증가시킨다.
- **도착 사이클 번호:** 각 사이클에 ID를 매긴 후 역순으로 `cycle_id[v]=cycle_id[next[v]]`를 전파한다.
- **목적지 질의:** 매우 큰 `k`번째 후속 정점은 binary lifting으로 `O(n log K)` 전처리, 질의 `O(log K)`에 처리한다. 사이클 구조를 이용하면 거대한 `k`의 모듈러 처리도 가능하다.
- **가중 이동:** 간선마다 가중치가 있으면 사이클 합과 꼬리 누적값을 따로 구한다. 값 범위에 따라 64비트 정수가 필요하다.
- **사이클 밖 정점 수:** 제거된 정점의 수가 곧 전체 정점 수에서 사이클 정점 수를 뺀 값이다.

## 오늘 문제와의 연결

[CSES 1751 Planets Cycles](https://cses.fi/problemset/task/1751/)에서는 모든 행성에 정확히 하나의 도착지가 있으므로 함수형 그래프 조건을 만족한다. [`../2026-09-17/icpc_problem.cpp`](../2026-09-17/icpc_problem.cpp)는 `peeled`를 큐와 역방향 DP 순서로 재사용하고, `indegree > 0`을 사이클 판별 불변식으로 쓴다. 공식 예제의 `1→2→4→1`은 길이 3, `3→3`은 길이 1, `5→4`는 꼬리 길이 1이라 답은 `3 3 1 3 4`다.

직접 검증할 때는 자기 루프 하나, 단일 긴 사이클, 사이클에 여러 꼬리가 합쳐지는 경우, 모든 정점이 자기 루프인 경우를 먼저 살펴본다. 작은 무작위 그래프는 각 출발점에서 방문 여부 배열로 직접 시뮬레이션한 `O(n^2)` 결과와 비교할 수 있다.
