/*
문제 ID/제목: BOJ 2178 - 미로 탐색
출처: https://www.acmicpc.net/problem/2178

문제 요약: N행 M열 미로에서 1은 이동할 수 있는 칸, 0은 벽이다. (1,1)에서 출발해 상하좌우로만
이동하여 (N,M)에 도착할 때 지나야 하는 최소 칸 수를 구한다. 시작 칸과 도착 칸도 칸 수에 포함한다.
입력: 첫 줄에 N과 M, 다음 N줄에 공백 없는 0/1 문자열이 주어진다.
출력: 시작점에서 도착점까지 지나간 최소 칸 수 하나를 출력한다.
제약: 2 <= N, M <= 100이며 시작점과 도착점은 항상 1이고 도착 가능한 입력만 주어진다.
예제: 4 6 / 101111 / 101010 / 101011 / 111011 을 입력하면 15를 출력한다.
*/

// <array>는 크기가 컴파일 시간에 고정된 네 방향 배열을 제공한다.
#include <array>
// <iostream>은 표준 입력 std::cin과 출력 std::cout을 제공한다.
#include <iostream>
// <queue>는 먼저 들어온 좌표를 먼저 처리하는 FIFO std::queue를 제공한다.
#include <queue>
// <string>은 각 미로 행의 문자를 소유한다.
#include <string>
// <utility>는 좌표 쌍 std::pair를 제공한다.
#include <utility>
// <vector>는 실행 중 크기가 정해지는 2차원 배열을 제공한다.
#include <vector>

using Position = std::pair<int, int>; // using 별칭으로 (행, 열) 정수 쌍의 의미를 드러낸다.

int main() {
    std::ios::sync_with_stdio(false); // 많은 입출력을 위해 C와 C++ 스트림 동기화를 끈다.
    std::cin.tie(nullptr); // nullptr은 가리키는 객체가 없음을 뜻하며 자동 flush 연결을 해제한다.

    int rows{}; // 기본 타입 int를 중괄호로 0 초기화한다.
    int columns{};
    std::cin >> rows >> columns; // >>가 값을 읽어 이름 있는 lvalue 변수에 저장한다.
    std::vector<std::string> maze(static_cast<std::size_t>(rows)); // 템플릿 인자는 원소 타입이며 rows개 문자열을 만든다.
    for (std::string& line : maze) { std::cin >> line; } // 비const lvalue 참조로 각 행 객체에 직접 입력한다.

    // 사용 알고리즘 문서: ../algorithm/breadth-first-search-unweighted-grid.md
    // distance가 0이면 미방문, 양수면 확정된 최단 칸 수라는 불변식을 유지한다.
    std::vector<std::vector<int>> distance(static_cast<std::size_t>(rows),
        std::vector<int>(static_cast<std::size_t>(columns), 0)); // O(NM) 거리표가 방문 배열 역할도 겸한다.
    std::queue<Position> pending{}; // 같은 거리의 정점을 다음 거리보다 먼저 처리하는 FIFO 큐다.
    distance[0][0] = 1; // 시작 칸 자체를 포함하므로 거리는 1이다.
    pending.emplace(0, 0); // pair 임시 객체를 큐 내부에서 직접 생성한다.

    constexpr std::array<int, 4> dr{-1, 1, 0, 0}; // constexpr 값은 실행 중 바뀌지 않는 행 변화량이다.
    constexpr std::array<int, 4> dc{0, 0, -1, 1};
    while (!pending.empty()) { // 큐가 빌 때까지 각 칸을 최대 한 번 꺼낸다.
        const auto [row, column]{pending.front()}; // 구조적 바인딩으로 pair의 두 int를 복사한다.
        pending.pop(); // 복사 후 큐의 맨 앞 원소 수명을 끝낸다.
        for (std::size_t direction{}; direction < dr.size(); ++direction) { // 네 방향을 반복한다.
            const int next_row{row + dr[direction]}; // +로 이웃 행을 계산한다.
            const int next_column{column + dc[direction]};
            if (next_row < 0 || next_row >= rows || next_column < 0 || next_column >= columns) { continue; } // ||로 경계 밖 조건을 합친다.
            if (maze[static_cast<std::size_t>(next_row)][static_cast<std::size_t>(next_column)] == '0') { continue; } // 벽은 건너뛴다.
            if (distance[static_cast<std::size_t>(next_row)][static_cast<std::size_t>(next_column)] != 0) { continue; } // 이미 발견한 칸은 다시 넣지 않는다.
            distance[static_cast<std::size_t>(next_row)][static_cast<std::size_t>(next_column)] =
                distance[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)] + 1; // 점화식 d[next]=d[current]+1이다.
            pending.emplace(next_row, next_column); // 최초 발견은 BFS 층 순서 때문에 곧 최단거리 확정이다.
        }
    }
    std::cout << distance[static_cast<std::size_t>(rows - 1)][static_cast<std::size_t>(columns - 1)] << '\n';
    // 각 칸과 네 간선을 한 번씩 보므로 시간 O(NM), 미로·거리·큐를 합쳐 공간 O(NM)이다.
    // 로드·비교·분기·큐 함수 호출의 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return 0;
}
