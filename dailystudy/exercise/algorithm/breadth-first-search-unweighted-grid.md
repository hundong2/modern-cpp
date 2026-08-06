# 무가중 격자 최단거리 너비 우선 탐색(BFS)

## 정의와 적용 조건

격자의 각 칸을 정점, 이동 가능한 이웃 관계를 간선으로 본다. 모든 이동 비용이 동일할 때 BFS는 시작 칸에서 각 칸까지 필요한 최소 이동 횟수 또는 최소 칸 수를 구한다. 가중치가 서로 다르면 Dijkstra 같은 알고리즘을 사용해야 한다.

## 핵심 아이디어와 불변식

FIFO 큐는 시작점과의 거리가 작은 층을 큰 층보다 먼저 꺼낸다. 큐에서 현재 칸의 거리가 `d`라면 아직 방문하지 않은 이웃에 `d+1`을 기록한다. 불변식은 다음과 같다.

- 큐에 들어간 칸의 거리는 이미 최단거리다.
- 큐는 거리가 감소하지 않는 순서로 처리된다.
- 각 칸은 최초 발견 때 한 번만 큐에 들어간다.

## 단계별 절차

1. 거리 배열을 미방문 표식(예: `-1` 또는 0)으로 초기화한다.
2. 시작점 거리를 0(이동 횟수) 또는 1(방문 칸 수)로 정하고 큐에 넣는다.
3. 큐 앞의 칸을 꺼내 상하좌우 이웃을 조사한다.
4. 경계 밖, 벽, 이미 방문한 이웃은 건너뛴다.
5. 나머지 이웃에 `현재 거리 + 1`을 기록하고 즉시 큐에 넣는다.
6. 큐가 빌 때까지 반복하거나 목적지를 꺼냈을 때 종료한다.

## 의사 코드

```text
distance[*] = UNVISITED
distance[start] = INITIAL_DISTANCE
queue.push(start)
while queue is not empty:
    current = queue.pop_front()
    for next in four_neighbors(current):
        if outside(next) or wall(next) or visited(next): continue
        distance[next] = distance[current] + 1
        queue.push(next)
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <array>   // 고정 크기 방향 배열
#include <queue>   // FIFO 큐
#include <utility> // std::pair 좌표
#include <vector>  // 2차원 거리표

using Position = std::pair<int, int>; // 행과 열의 타입 별칭

int main() {
    const int rows{3};
    const int columns{3};
    std::vector<std::vector<int>> distance(rows, std::vector<int>(columns, -1));
    std::queue<Position> pending{};
    distance[0][0] = 0;
    pending.emplace(0, 0);
    constexpr std::array<int, 4> dr{-1, 1, 0, 0};
    constexpr std::array<int, 4> dc{0, 0, -1, 1};
    while (!pending.empty()) {
        const auto [row, column]{pending.front()}; // pair를 두 int로 구조적 바인딩
        pending.pop();
        for (std::size_t direction{}; direction < dr.size(); ++direction) {
            const int nr{row + dr[direction]};
            const int nc{column + dc[direction]};
            if (nr < 0 || nr >= rows || nc < 0 || nc >= columns || distance[nr][nc] != -1) { continue; }
            distance[nr][nc] = distance[row][column] + 1;
            pending.emplace(nr, nc);
        }
    }
    return distance[2][2] == 4 ? 0 : 1;
}
```

## 정확성 근거

시작점은 거리 0으로 정확하다. 거리 `d` 이하의 칸이 정확하다고 가정하자. 거리 `d`인 칸에서 처음 발견한 이웃은 길이 `d+1` 경로를 가진다. 더 짧은 경로가 있었다면 그 이전 칸은 더 작은 층에서 먼저 처리되어 이웃을 이미 발견했어야 하므로 모순이다. 귀납적으로 모든 기록 거리가 최단거리다.

## 시간·공간 복잡도

N×M 격자에서 각 칸은 최대 한 번 큐에 들어가고 네 방향만 검사하므로 시간은 `O(NM)`이다. 거리표와 최악의 경우 큐가 모든 칸을 보관하므로 공간은 `O(NM)`이다.

## 흔한 실수

- 큐에서 꺼낼 때 방문 표시하면 같은 칸이 여러 번 들어갈 수 있다. 넣을 때 표시한다.
- 행/열 경계를 검사하기 전에 배열에 접근하면 정의되지 않은 동작이 생긴다.
- 이동 횟수와 방문 칸 수를 혼동해 시작 거리를 잘못 정한다.
- `std::size_t`와 음수가 가능한 `int` 좌표를 섞어 경계 검사가 무너질 수 있다. 음수 검사를 먼저 하고 변환한다.
- 가중치가 다른 간선에 BFS를 적용하면 최단거리가 보장되지 않는다.

## 변형

- 여러 시작점을 처음부터 큐에 넣는 다중 시작점 BFS
- 부모 좌표를 저장해 실제 최단 경로 복원
- 열쇠 보유 여부처럼 작은 상태를 좌표에 추가한 상태 공간 BFS
- 간선 비용이 0 또는 1일 때 deque를 쓰는 0-1 BFS

## 오늘 문제와의 연결

2026-08-07의 BOJ 2178 미로 탐색은 1인 칸만 정점으로 보고 상하좌우 이동을 동일 비용 간선으로 본다. 출력이 이동 횟수가 아니라 지나간 칸 수이므로 시작 거리를 1로 둔다. 구현은 [`../2026-08-07/icpc_problem.cpp`](../2026-08-07/icpc_problem.cpp)에 있다.

## 직접 해보기와 이해 검증

1. 3×3 빈 격자에서 매 반복 뒤 큐와 거리표를 손으로 적는다.
2. 벽 하나를 추가하고 도착 거리 변화를 예측한 뒤 코드를 실행한다.
3. 목적지를 처음 발견한 즉시 종료해도 되는 이유를 불변식으로 설명한다.
4. 부모 좌표 배열을 추가해 최단 경로 좌표를 역순으로 복원한다.
