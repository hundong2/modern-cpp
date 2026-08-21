# 플로이드–워셜 모든 쌍 최단 경로

## 정의

플로이드–워셜(Floyd–Warshall)은 정점 수를 `V`라 할 때 모든 `i → j` 최단 거리를 `O(V³)`에 구하는 동적 계획법이다. 한 출발점만이 아니라 **모든 출발·도착 쌍**이 필요하고 `V`가 수백 정도일 때 구현이 짧고 강력하다.

## 적용 조건

- 방향·무방향 가중 그래프 모두 적용할 수 있다.
- 음수 간선도 허용하지만, 최단 거리가 정의되지 않는 음수 사이클이 없어야 일반적인 거리 답이 의미가 있다.
- 인접 행렬 `O(V²)` 공간과 세 중첩 반복 `O(V³)` 시간이 제한 안에 들어야 한다.
- 희소 그래프에서 몇 출발점만 묻는다면 다익스트라를 여러 번 돌리는 편이 나을 수 있다.
- 도달 가능성만 필요하면 거리 대신 bool 행렬과 논리 연산을 쓰는 Warshall 변형을 고려한다.

## 핵심 아이디어와 불변식

`D[k][i][j]`를 **중간 정점으로 1..k만 허용했을 때** `i`에서 `j`까지의 최소 비용이라 하자. 최적 경로는 두 경우뿐이다.

1. 정점 `k`를 중간에 쓰지 않는다: 비용은 `D[k-1][i][j]`다.
2. 정점 `k`를 쓴다: 단순 최단 경로에서 `k`를 한 번 기준으로 나누어 `D[k-1][i][k] + D[k-1][k][j]`다.

따라서 점화식은 다음과 같다.

```text
D[k][i][j] = min(D[k-1][i][j], D[k-1][i][k] + D[k-1][k][j])
```

2차원 행렬 하나를 제자리 갱신해도 된다. 바깥 반복을 반드시 `k`로 두면, `k` 단계가 끝난 직후 `distance[i][j]`가 중간 정점 `1..k`만 허용한 최솟값이라는 불변식이 성립한다.

## 단계별 절차

1. 모든 거리를 `INF`로 초기화한다.
2. 빈 경로인 `distance[i][i]`를 0으로 둔다.
3. 각 간선 `u → v`에 대해 `distance[u][v] = min(distance[u][v], cost)`로 둔다. 병렬 간선을 덮어쓰지 말고 최소 비용을 남긴다.
4. `k = 1..V`, `i = 1..V`, `j = 1..V` 순서로 반복한다.
5. `i → k` 또는 `k → j`가 `INF`면 덧셈을 건너뛴다.
6. 아니면 `distance[i][j]`와 두 구간 비용 합 중 작은 값을 저장한다.
7. 모든 단계 뒤 행렬이 모든 쌍 최단 거리다.

## 의사 코드

```text
distance의 모든 칸 = INF
각 i에 대해 distance[i][i] = 0
각 간선 (u, v, w)에 대해 distance[u][v] = min(distance[u][v], w)

for k = 1 .. V:
    for i = 1 .. V:
        for j = 1 .. V:
            if distance[i][k]와 distance[k][j]가 모두 유한:
                distance[i][j] = min(distance[i][j],
                                     distance[i][k] + distance[k][j])
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <algorithm> // std::min을 선언한다.
#include <iostream>  // std::cout을 선언한다.
#include <limits>    // std::numeric_limits를 선언한다.
#include <vector>    // std::vector를 선언한다.

int main() {
    constexpr int vertex_count{4};
    // 최댓값의 1/4을 sentinel로 잡아 유한 거리 합이 표현 범위를 넘을 위험을 낮춘다.
    constexpr long long infinity{std::numeric_limits<long long>::max() / 4LL};
    // 4×4 행렬을 만들고 모든 칸을 INF로 복사 초기화한다.
    std::vector<std::vector<long long>> distance(
        vertex_count, std::vector<long long>(vertex_count, infinity));

    for (int vertex{}; vertex < vertex_count; ++vertex) {
        distance[vertex][vertex] = 0LL; // 자기 자신까지 빈 경로 비용은 0이다.
    }

    distance[0][1] = 3LL;
    distance[1][2] = 4LL;
    distance[2][3] = 5LL;
    distance[0][3] = 20LL;

    // via가 가장 바깥이어야 "허용 중간 정점 집합" 불변식이 단계별로 확장된다.
    for (int via{}; via < vertex_count; ++via) {
        for (int from{}; from < vertex_count; ++from) {
            for (int to{}; to < vertex_count; ++to) {
                if (distance[from][via] == infinity || distance[via][to] == infinity) {
                    continue; // 도달 불가 sentinel끼리 더하지 않는다.
                }
                distance[from][to] = std::min(
                    distance[from][to], distance[from][via] + distance[via][to]);
            }
        }
    }

    std::cout << distance[0][3] << '\n'; // 0→1→2→3 비용 12를 출력한다.
    return distance[0][3] == 12LL ? 0 : 1;
}
```

## 정확성 근거

`k=0`일 때 행렬은 직접 간선과 자기 자신으로 가는 빈 경로만 표현하므로 불변식이 맞다. `k-1` 단계가 맞다고 가정하자. 중간 정점 `1..k`만 쓰는 `i → j` 최단 경로는 `k`를 쓰지 않거나 쓴다. 쓰지 않으면 기존 값이 정확하다. 쓴다면 `i → k`와 `k → j` 두 부분은 중간 정점 `1..k-1`만 쓰므로 귀납 가정에 의해 두 저장값의 합이 정확하다. 둘의 최소를 택하면 `k` 단계도 맞다. 귀납적으로 `k=V` 뒤 모든 중간 정점을 허용한 최단 거리가 된다.

## 시간·공간 복잡도

- 초기화와 출력: `O(V²)`
- 점화식: `O(V³)`
- 거리 행렬: `O(V²)`
- 간선 목록을 별도로 보관하지 않으면 추가 간선 저장 공간은 필요 없다.

## 흔한 실수

- `i`, `j`, `k` 순서로 돌려 불변식을 깨뜨린다. `k`가 가장 바깥이다.
- 병렬 간선을 마지막 입력으로 무조건 덮어쓴다. 입력 시 `min`을 취해야 한다.
- `INF + INF` 또는 `INF + 유한값`을 계산해 오버플로한다. 두 부분이 유한할 때만 더한다.
- 대각선을 0으로 초기화하지 않는다.
- 갈 수 없는 쌍을 문제 요구인 0이 아니라 INF로 출력한다.
- `int` 범위를 제약으로 확인하지 않고 비용 합을 저장한다. 여유 있는 정수 타입과 안전한 INF를 선택한다.
- 음수 사이클이 있는데도 행렬을 평범한 최단 거리로 해석한다. 완료 후 `distance[v][v] < 0`이면 음수 사이클을 탐지할 수 있다.

## 변형

- bool 행렬의 `reachable[i][j] |= reachable[i][k] && reachable[k][j]`: 전이 폐쇄/도달 가능성.
- `distance[v][v] < 0` 검사: 음수 사이클 존재 여부.
- 경로 복원용 `next[i][j]`: 다음 정점을 함께 갱신해 실제 경로를 재구성한다.
- min-plus 대신 max-min 같은 연산을 쓰면 병목 경로 등 다른 모든 쌍 문제로 일반화된다.

## 오늘 문제와의 연결

[BOJ 11404 플로이드](https://www.acmicpc.net/problem/11404)는 `N ≤ 100`이라 `N³ = 1,000,000` 갱신 규모가 충분히 작다. 병렬 버스 중 최소 직접 비용을 남기고, 세 중첩 반복으로 모든 도시 쌍을 한 번에 해결한다. 플로이드–워셜은 대회에서 반드시 알아야 할 모든 쌍 최단 경로의 기준 풀이이며, 점화식의 중간 정점 집합 불변식을 말로 증명할 수 있어야 반복문 순서를 실수하지 않는다.

## 직접 해보기와 초보자 검증

1. 위 예제에서 `via=0`, `1`, `2`가 끝날 때 `distance[0][3]`을 손으로 기록한다.
2. `distance[0][3]=20`을 지우고 도달 불가 상태가 언제 12로 바뀌는지 확인한다.
3. 병렬 간선 `0→1` 비용 8과 3을 역순으로 입력하고 초기화에 `min`이 필요한 이유를 설명한다.
4. 반복 순서를 `from-via-to`로 바꾼 뒤 작은 반례를 찾는다.
5. 음수 간선은 있지만 음수 사이클이 없는 그래프를 만들고 정답을 검증한다.
6. 자료를 보지 않고 INF 검사까지 포함해 다시 작성하고 `O(V³)` 시간과 `O(V²)` 공간의 근거를 말한다.
