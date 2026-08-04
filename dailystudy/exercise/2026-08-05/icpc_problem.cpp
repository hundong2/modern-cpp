/*
문제 ID/제목: BOJ 11724 - 연결 요소의 개수
출처: https://www.acmicpc.net/problem/11724

문제 요약: 1번부터 N번까지 번호가 붙은 정점과 방향 없는 간선 M개가 주어진다.
서로 간선을 따라 도달할 수 있는 정점들의 최대 묶음인 연결 요소가 모두 몇 개인지 구한다.
입력: 첫 줄에 정점 수 N과 간선 수 M, 다음 M줄에 서로 다른 두 끝점 u, v가 주어진다.
출력: 그래프의 연결 요소 개수를 한 줄에 출력한다.
제약: 1 <= N <= 1,000, 0 <= M <= N*(N-1)/2, 같은 간선은 한 번만 주어진다.
예제: 입력 `6 5 / 1 2 / 2 5 / 5 1 / 3 4 / 4 6`의 출력은 `2`이다.
*/

// <iostream>은 온라인 저지의 표준 입력과 출력을 제공한다.
#include <iostream>
// <vector>는 인접 리스트, 방문 배열, 반복 DFS 스택을 제공한다.
#include <vector>

int main() { // 제출 프로그램의 진입점이며 정상 종료 시 0을 반환한다.
    std::ios::sync_with_stdio(false); // C와 C++ 스트림 동기화를 꺼 입력 비용을 줄인다.
    std::cin.tie(nullptr); // nullptr은 널 포인터 값이며 입력 전 자동 출력 flush 연결을 끊는다.

    int vertex_count{}; // 정점 수를 저장할 int를 0으로 중괄호 초기화한다.
    int edge_count{}; // 간선 수를 저장한다.
    std::cin >> vertex_count >> edge_count; // >> 연산자가 공백 구분 정수를 읽는다.

    // 구현 근거: ../algorithm/graph-traversal-connected-components.md
    // vector<vector<int>>는 각 정점의 이웃 수가 다른 희소 그래프용 인접 리스트다.
    std::vector<std::vector<int>> graph(static_cast<std::size_t>(vertex_count + 1));
    for (int edge{}; edge < edge_count; ++edge) { // 정확히 M개 간선을 한 번씩 읽는다.
        int from{};
        int to{};
        std::cin >> from >> to;
        graph[static_cast<std::size_t>(from)].push_back(to); // 무방향 간선의 한 방향을 저장한다.
        graph[static_cast<std::size_t>(to)].push_back(from); // 반대 방향도 저장해 양쪽 이동을 허용한다.
    }

    // char는 정점마다 0/1 방문 상태만 필요해 간단하며 공간은 O(N)이다.
    std::vector<char> visited(static_cast<std::size_t>(vertex_count + 1), false);
    int component_count{};

    for (int start{1}; start <= vertex_count; ++start) { // 모든 정점을 후보 시작점으로 검사한다.
        if (visited[static_cast<std::size_t>(start)]) { // 이미 기존 요소에서 도달했다면 건너뛴다.
            continue;
        }

        ++component_count; // 미방문 시작점은 새 연결 요소 하나의 대표다.
        std::vector<int> stack{start}; // 재귀 호출 스택 대신 명시적 LIFO 컨테이너를 쓴다.
        visited[static_cast<std::size_t>(start)] = true; // 넣을 때 방문 표시해 중복 삽입을 막는다.

        // 불변식: stack의 모든 정점은 방문 표시됐고, 같은 연결 요소에서 아직 이웃 검사를 기다린다.
        while (!stack.empty()) { // 처리 대기 정점이 없을 때까지 DFS를 계속한다.
            const int current{stack.back()}; // back은 마지막 정점을 const 값으로 복사한다.
            stack.pop_back(); // pop_back은 마지막 원소를 제거해 LIFO 순서를 구현한다.

            for (const int next : graph[static_cast<std::size_t>(current)]) { // 모든 인접 정점을 조사한다.
                if (!visited[static_cast<std::size_t>(next)]) { // !는 방문하지 않음으로 조건을 뒤집는다.
                    visited[static_cast<std::size_t>(next)] = true;
                    stack.push_back(next); // 새 정점은 같은 요소에 속하며 나중에 이웃을 검사한다.
                }
            }
        }
    }

    // 각 정점과 인접 리스트 항목을 한 번씩 처리하므로 시간 O(N+M), 그래프·상태 공간 O(N+M)이다.
    std::cout << component_count << '\n';
    return 0;
}
