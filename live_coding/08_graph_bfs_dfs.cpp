/*
Chapter 08. 그래프 BFS/DFS, 격자 탐색

문제 1: 무방향 그래프의 연결 요소 개수를 구하라.
접근: 방문하지 않은 정점에서 DFS/BFS를 시작할 때마다 component가 하나 증가한다.
복잡도: O(V + E)

문제 2: 격자에서 S에서 G까지 최단 거리를 구하라. '#'은 벽이다.
접근: 가중치가 모두 1이므로 BFS가 최단 거리를 보장한다.

문제 3: 섬의 개수를 구하라.
접근: '1'을 만나면 DFS로 같은 섬을 모두 방문 처리한다.
*/

#include <iostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>
using namespace std;

int countComponents(int n, const vector<pair<int, int>>& edges) {
    vector<vector<int>> graph(n);
    for (auto [u, v] : edges) {
        graph[u].push_back(v);
        graph[v].push_back(u); // 무방향 그래프라 양쪽 인접 리스트에 모두 넣는다.
    }

    vector<bool> visited(n, false);
    int components = 0;

    for (int start = 0; start < n; ++start) {
        if (visited[start]) continue;
        ++components;
        queue<int> q;
        q.push(start);
        visited[start] = true; // 큐에 넣는 순간 방문 처리해 중복 삽입을 막는다.

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v : graph[u]) {
                if (visited[v]) continue;
                visited[v] = true; // 발견 시점에 방문 처리하는 것이 BFS의 기본 패턴이다.
                q.push(v);
            }
        }
    }
    return components;
}

int shortestPathInGrid(const vector<string>& grid) {
    int n = (int)grid.size();
    int m = (int)grid[0].size();
    pair<int, int> start{-1, -1}, goal{-1, -1};

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            if (grid[r][c] == 'S') start = {r, c};
            if (grid[r][c] == 'G') goal = {r, c};
        }
    }

    vector<vector<int>> dist(n, vector<int>(m, -1));
    queue<pair<int, int>> q;
    q.push(start);
    dist[start.first][start.second] = 0;

    int dr[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};

    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();
        if (make_pair(r, c) == goal) return dist[r][c];

        for (int dir = 0; dir < 4; ++dir) {
            int nr = r + dr[dir];
            int nc = c + dc[dir];
            if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
            if (grid[nr][nc] == '#' || dist[nr][nc] != -1) continue; // 벽이거나 이미 최단 거리로 방문한 칸은 제외한다.
            dist[nr][nc] = dist[r][c] + 1;                           // 간선 비용이 1이라 이전 거리 + 1이다.
            q.push({nr, nc});
        }
    }
    return -1;
}

void markIsland(vector<string>& grid, int r, int c) {
    int n = (int)grid.size();
    int m = (int)grid[0].size();
    if (r < 0 || r >= n || c < 0 || c >= m || grid[r][c] != '1') return;

    grid[r][c] = '0'; // 방문 처리와 동시에 물을 만들어 중복 DFS를 방지한다.
    markIsland(grid, r + 1, c);
    markIsland(grid, r - 1, c);
    markIsland(grid, r, c + 1);
    markIsland(grid, r, c - 1);
}

int countIslands(vector<string> grid) {
    int count = 0;
    for (int r = 0; r < (int)grid.size(); ++r) {
        for (int c = 0; c < (int)grid[0].size(); ++c) {
            if (grid[r][c] == '1') {
                ++count;
                markIsland(grid, r, c);
            }
        }
    }
    return count;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "[components] " << countComponents(5, {{0, 1}, {1, 2}, {3, 4}}) << '\n';

    vector<string> maze = {
        "S..#",
        ".#..",
        "...G"
    };
    cout << "[grid shortest path] " << shortestPathInGrid(maze) << '\n';

    vector<string> islands = {
        "11000",
        "11010",
        "00100",
        "00011"
    };
    cout << "[islands] " << countIslands(islands) << '\n';
    return 0;
}
