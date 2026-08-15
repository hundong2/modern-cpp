/*
문제 ID/제목: BOJ 2150 - Strongly Connected Component
출처: https://www.acmicpc.net/problem/2150 (Baekjoon Online Judge)

문제 요약:
방향 그래프에서 두 정점 u, v가 서로에게 가는 경로를 모두 가질 때 같은 강결합 요소(SCC)에
속한다고 한다. 모든 정점을 겹치지 않는 SCC로 분해하고 문제에서 요구하는 정렬 순서로 출력한다.

입력:
첫 줄에 정점 수 V와 간선 수 E가 주어진다. 다음 E줄에는 방향 간선의 시작 정점 A와 끝 정점 B가
주어진다. 정점 번호는 1부터 V까지다.

출력:
첫 줄에 SCC 개수 K를 출력한다. 각 SCC 내부 정점은 오름차순으로 적고 끝에 -1을 붙인다.
SCC 목록도 각 컴포넌트의 가장 작은 정점 번호를 기준으로 오름차순 출력한다.

제약:
1 <= V <= 10,000, 1 <= E <= 100,000이다. 같은 시작점과 끝점을 가진 간선이 없다는 조건 아래
인접 리스트와 두 번의 DFS로 충분히 처리할 수 있다.

예제:
입력이 1->4, 4->5, 5->1의 순환과 2,3,7의 상호 도달 관계, 그리고 정점 6을 포함하면
출력 SCC는 {1,4,5}, {2,3,7}, {6}이며 각 줄 끝에는 -1이 붙는다.
*/

// <algorithm>은 각 SCC와 SCC 목록을 오름차순으로 정렬하는 std::sort를 제공한다.
#include <algorithm>
// <iostream>은 온라인 저지의 표준 입력 std::cin과 표준 출력 std::cout을 제공한다.
#include <iostream>
// <utility>는 SCC vector의 내부 버퍼 소유권을 옮기는 std::move를 제공한다.
#include <utility>
// <vector>는 인접 리스트, 방문 배열, 종료 순서, SCC 목록을 동적 배열로 저장한다.
#include <vector>

// Graph는 vector<vector<int>>라는 긴 타입을 읽기 쉽게 만든 using 타입 별칭이다.
using Graph = std::vector<std::vector<int>>;

// 알고리즘 문서: ../algorithm/strongly-connected-components-kosaraju.md
// 첫 DFS는 정방향 그래프에서 모든 후손을 끝낸 뒤 현재 정점을 종료 순서에 넣는다.
void build_finish_order(
    // graph는 복사하지 않고 읽기만 하는 const lvalue 참조 매개변수다.
    const Graph& graph,
    // vertex는 현재 방문할 정점 번호를 값으로 받는 기본 int 매개변수다.
    int vertex,
    // visited와 finish_order는 호출들이 같은 상태를 수정하도록 비const lvalue 참조로 받는다.
    std::vector<bool>& visited,
    std::vector<int>& finish_order) {
    // [] 연산자로 현재 정점의 방문 비트를 true로 저장해 재방문과 순환을 막는다.
    visited[vertex] = true;

    // 범위 기반 for가 graph[vertex]의 모든 도착 정점을 int 값으로 하나씩 복사한다.
    for (const int next : graph[vertex]) {
        // ! 연산자는 방문하지 않은 정점만 재귀 호출하도록 bool 값을 뒤집는다.
        if (!visited[next]) {
            // 함수 호출 스택에 다음 정점을 쌓아 그 정점에서 도달 가능한 영역을 먼저 마친다.
            build_finish_order(graph, next, visited, finish_order);
        }
    }

    // 모든 나가는 간선을 처리한 뒤 push_back하므로 뒤쪽일수록 종료 시각이 늦다는 불변식이 성립한다.
    finish_order.push_back(vertex);
}

// 알고리즘 문서: ../algorithm/strongly-connected-components-kosaraju.md
// 역그래프 DFS는 아직 배정되지 않은 시작점과 같은 SCC의 정점만 component에 모은다.
void collect_component(
    // reversed는 모든 간선 방향을 뒤집은 인접 리스트를 const 참조로 빌린다.
    const Graph& reversed,
    int vertex,
    std::vector<bool>& visited,
    std::vector<int>& component) {
    // 배정한 정점을 true로 표시해 정확히 한 SCC에만 들어가게 한다.
    visited[vertex] = true;
    // 현재 정점은 이번 SCC의 원소이므로 vector 끝에 추가한다.
    component.push_back(vertex);

    // 역방향으로 도달 가능한 모든 이웃을 한 번씩 검사한다.
    for (const int next : reversed[vertex]) {
        // 이전 SCC에 이미 들어간 정점은 건너뛰고 새 정점만 재귀 탐색한다.
        if (!visited[next]) {
            collect_component(reversed, next, visited, component);
        }
    }
}

// main은 입력, 그래프 구성, 코사라주 두 단계, 정렬 출력을 조립하고 종료 코드를 반환한다.
int main() {
    // sync_with_stdio(false)는 C와 C++ 스트림 동기화 비용을 줄여 많은 간선 입력을 빠르게 한다.
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 입력 전에 cout을 자동 flush하는 연결을 끊는다. nullptr는 아무 객체도 가리키지 않는다.
    std::cin.tie(nullptr);

    // int 변수는 문제의 최대 100,000을 안전하게 담고 {}로 0 값 초기화한다.
    int vertex_count{};
    int edge_count{};
    // >> 연산자가 첫 줄의 두 정수를 변수에 저장하며 입력이 없으면 조용히 정상 종료한다.
    if (!(std::cin >> vertex_count >> edge_count)) {
        return 0;
    }

    // 정점 번호를 그대로 인덱스로 쓰기 위해 크기 V+1인 인접 리스트 두 개를 직접 초기화한다.
    Graph graph(static_cast<std::size_t>(vertex_count + 1));
    Graph reversed(static_cast<std::size_t>(vertex_count + 1));

    // for 초기식은 int edge를 0으로 만들고, < 비교와 ++ 증가로 정확히 E번 반복한다.
    for (int edge{}; edge < edge_count; ++edge) {
        // from과 to는 각 방향 간선의 시작과 끝 번호를 담는다.
        int from{};
        int to{};
        // 표준 입력 호출이 두 정수를 읽어 각 lvalue 변수에 저장한다.
        std::cin >> from >> to;
        // 정방향 인접 리스트에는 from -> to를 추가한다.
        graph[from].push_back(to);
        // 역그래프에는 방향을 뒤집은 to -> from을 추가한다.
        reversed[to].push_back(from);
    }

    // vector<bool>은 방문 여부만 필요한 V+1개 비트를 false로 초기화하는 특수화 컨테이너다.
    std::vector<bool> visited(static_cast<std::size_t>(vertex_count + 1), false);
    // finish_order는 모든 정점 번호를 담으므로 reserve로 재할당 횟수를 줄인다.
    std::vector<int> finish_order{};
    finish_order.reserve(static_cast<std::size_t>(vertex_count));

    // 번호가 작은 정점부터 아직 방문하지 않은 정방향 DFS 트리를 시작한다.
    for (int vertex{1}; vertex <= vertex_count; ++vertex) {
        // !visited 조건으로 이미 다른 DFS 트리에서 도달한 정점은 다시 시작하지 않는다.
        if (!visited[vertex]) {
            build_finish_order(graph, vertex, visited, finish_order);
        }
    }

    // fill은 기존 V+1개 방문 비트를 모두 false로 저장해 두 번째 DFS 상태로 재사용한다.
    std::fill(visited.begin(), visited.end(), false);
    // vector<vector<int>>는 SCC마다 가변 길이 정점 목록을 소유한다.
    std::vector<std::vector<int>> components{};

    // rbegin/rend 역반복자는 첫 DFS 종료 시각이 늦은 정점부터 순회하게 한다.
    for (auto position{finish_order.rbegin()}; position != finish_order.rend(); ++position) {
        // * 연산자는 역반복자가 가리키는 정점 int 값을 읽는다.
        const int vertex{*position};
        // 이미 어떤 SCC에 배정된 정점은 두 번째 DFS를 시작하지 않는다.
        if (visited[vertex]) {
            continue;
        }

        // 빈 vector<int>는 이번 역그래프 DFS가 모을 하나의 SCC를 소유한다.
        std::vector<int> component{};
        // 종료 순서 역순이라는 탐욕적 선택 덕분에 이 호출은 다른 미배정 SCC로 새지 않는다.
        collect_component(reversed, vertex, visited, component);
        // 문제 출력 조건에 맞게 SCC 내부 정점 번호를 오름차순 정렬한다.
        std::sort(component.begin(), component.end());
        // std::move는 component lvalue를 xvalue로 바꿔 내부 버퍼 소유권을 바깥 vector로 이전한다.
        components.push_back(std::move(component));
    }

    // vector의 사전식 < 비교를 쓰는 기본 정렬은 각 SCC의 최소 정점을 기준으로 원하는 순서를 만든다.
    std::sort(components.begin(), components.end());

    // size()의 unsigned 크기를 << 연산자로 출력하고 '\n' 문자로 줄을 끝낸다.
    std::cout << components.size() << '\n';
    // const 참조 반복은 각 내부 vector를 복사하지 않고 읽기만 한다.
    for (const std::vector<int>& component : components) {
        // SCC의 모든 정점을 이미 정렬된 순서로 값 복사해 출력한다.
        for (const int vertex : component) {
            std::cout << vertex << ' ';
        }
        // 문제 규약의 SCC 종료 표식 -1과 줄바꿈을 출력한다.
        std::cout << -1 << '\n';
    }

    // 모든 출력이 끝났으므로 정상 종료 코드 0을 반환한다.
    return 0;
}
