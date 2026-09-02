/*
문제 ID·제목: CSES 2077 - Necessary Cities
출처: CSES Problem Set / Advanced Techniques
출처 URL: https://cses.fi/problemset/task/2077

문제 요약:
n개의 도시와 m개의 양방향 도로가 있고 처음에는 모든 도시 쌍 사이에 경로가 있다. 어떤 도시와 그 도시에
닿은 모든 도로를 제거했을 때, 남은 도시 중 서로 갈 수 없는 쌍이 생기면 그 도시는 필수 도시(단절점)다.
모든 필수 도시를 찾아 출력한다. 아래 설명은 저작권이 있는 원문 전체가 아니라 조건을 한글로 재서술한 것이다.

입력:
첫 줄에 도시 수 n과 도로 수 m이 주어진다. 다음 m줄에는 서로 다른 두 도시 a, b를 잇는 도로가 주어진다.
도시는 1부터 n까지 번호가 붙고, 자기 자신을 잇는 도로나 같은 두 도시 사이의 중복 도로는 없다.

출력:
첫 줄에 필수 도시 수 k를 출력한다. 다음 줄에 k개 도시 번호를 출력하며 순서는 자유다.

제약:
2 <= n <= 100,000, 1 <= m <= 200,000, 1 <= a,b <= n. 시간 1초, 메모리 512MB다.

예제:
입력
5 5
1 2
1 4
2 4
3 5
4 5

출력의 한 가지 형태
2
4 5
*/

// <algorithm>은 두 값 중 작은 값을 고르는 std::min 함수 템플릿을 선언한다.
#include <algorithm>
// <cstddef>는 vector 인덱스와 크기에 쓰는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 std::cin/std::cout, 스트림 설정, operator>>/operator<<를 선언한다.
#include <iostream>
// <vector>는 원소를 연속 메모리에 소유하는 std::vector 클래스 템플릿을 선언한다.
#include <vector>

int main() {
    // sync_with_stdio(false)는 bool 값 인자 false 하나를 받고 C stdio 동기화를 끈 뒤 이전 bool 설정을 반환하지만
    // 여기서는 버린다. 표준 입출력 전에 호출하며 이후 C stdio와 섞지 않는다. 스트림 수명·소유권은 유지되고
    // 동기화 전환은 O(1) 설정 작업이지만 실제 버퍼 동작은 구현에 달린다. 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 std::cin(정확한 타입 std::istream)의 자동 flush 연결을 해제한다. 데이터 인자는 null 포인터
    // 하나이며 이전 std::ostream*를 반환하지만 버린다. cin/cout 소유권은 유지되고 대화형 문제가 아니라 수동 prompt flush가 없다.
    std::cin.tie(nullptr);

    int city_count{}; // int{}는 0으로 값 초기화하며 최대 100,000을 안전하게 담는다.
    int road_count{}; // 최대 200,000도 int 범위다.
    // 각 operator>>(istream&, int&)는 cin lvalue와 수정할 int lvalue를 받고 같은 istream&를 반환해 연쇄한다.
    // 성공하면 두 변수와 입력 위치가 바뀌며 소유권은 유지된다. 실패하면 상태 비트가 설정되지만 올바른 입력을 문제 계약이 보장한다.
    std::cin >> city_count >> road_count;

    // vector<vector<int>>(count)는 count=city_count+1개의 빈 vector<int>를 값 초기화해 그래프가 소유한다.
    // O(n) 시간·공간과 동적 할당이 들고 실패 시 bad_alloc 가능성이 있다. 1기반 번호 때문에 0번은 비워 둔다.
    std::vector<std::vector<int>> graph(static_cast<std::size_t>(city_count + 1));
    for (int road{}; road < road_count; ++road) { // 각 도로를 정확히 한 번 읽는 반복문이다.
        int from{};
        int to{};
        std::cin >> from >> to; // 두 int lvalue를 갱신하고 마지막 istream& 반환은 버린다.
        // push_back(const int&)는 수신 vector<int> 끝에 to 값을 복사해 size를 1 늘린다. 반환은 void라 저장하지 않는다.
        // 분할 상환 O(1), 재할당 시 해당 인접 목록의 기존 포인터·참조·반복자는 무효이며 할당 예외가 가능하다.
        graph[static_cast<std::size_t>(from)].push_back(to);
        graph[static_cast<std::size_t>(to)].push_back(from); // 무방향 도로라 반대 방향 값도 독립 복사한다.
    }

    // 다음 구현의 low-link 정의와 단절점 판정 근거는 공용 문서 ../algorithm/articulation-points-low-link.md 를 따른다.
    // 각 vector(count, value)는 count개 값을 복사 초기화해 독립 저장소를 소유한다. O(n) 시간·공간과 할당 가능성이 있다.
    std::vector<int> discovered(static_cast<std::size_t>(city_count + 1), 0); // 0은 아직 방문하지 않음을 뜻한다.
    std::vector<int> low(static_cast<std::size_t>(city_count + 1), 0); // DFS 서브트리에서 도달 가능한 최소 방문 순서다.
    std::vector<int> parent(static_cast<std::size_t>(city_count + 1), -1); // -1은 DFS 루트 또는 미설정 상태다.
    std::vector<int> child_count(static_cast<std::size_t>(city_count + 1), 0); // DFS 트리 자식 수다.
    std::vector<std::size_t> next_edge(static_cast<std::size_t>(city_count + 1), std::size_t{0}); // 다음에 검사할 인접 인덱스다.
    std::vector<char> articulation(static_cast<std::size_t>(city_count + 1), char{0}); // vector<bool> proxy 대신 0/1 바이트를 쓴다.
    std::vector<int> stack; // 재귀 호출 스택 대신 heap 저장소를 가진 명시적 DFS 스택이다.
    // reserve(count)는 size=0을 유지한 채 최소 city_count 용량을 확보한다. 반환은 void이고 O(n) 재할당 가능성이 있다.
    // 재할당이 일어나면 기존 관찰자는 무효지만 아직 원소가 없어 관찰자가 없다. 할당 실패 시 예외가 가능하다.
    stack.reserve(static_cast<std::size_t>(city_count));

    int timer{}; // 방문 순서를 1부터 증가시키는 int이며 최대 n이라 안전하다.
    for (int root{1}; root <= city_count; ++root) { // 연결 보장이 있지만 일반 그래프에도 안전하도록 모든 시작점을 본다.
        if (discovered[static_cast<std::size_t>(root)] != 0) {
            continue;
        }

        discovered[static_cast<std::size_t>(root)] = ++timer; // 전위 ++ 결과를 저장해 최초 방문 순서를 부여한다.
        low[static_cast<std::size_t>(root)] = discovered[static_cast<std::size_t>(root)];
        // push_back(root)는 int lvalue 값을 복사하고 size를 1 늘린다. reserve 덕분에 이 문제에서는 재할당하지 않는다.
        stack.push_back(root);

        // empty()는 데이터 인자 없이 O(1)에 bool을 반환하고 stack을 바꾸지 않는다. false일 때만 back/pop_back을 호출한다.
        while (!stack.empty()) {
            // back()은 데이터 인자 없이 마지막 int의 lvalue 참조를 O(1)에 반환한다. 비어 있지 않음은 while 조건이 보장한다.
            // 참조를 저장하지 않고 int로 복사하므로 이후 pop_back 뒤 댕글링 참조가 남지 않는다.
            const int vertex{stack.back()};
            std::size_t& edge_index{next_edge[static_cast<std::size_t>(vertex)]}; // lvalue 참조로 진행 상태를 제자리 갱신한다.

            // size()는 정확한 수신 타입 vector<int>를 바꾸지 않고 데이터 인자 없이 O(1)에 size_type 값을 반환한다.
            // 반환값을 edge_index 비교에 쓰며 할당·무효화·예외가 없고, true 결과가 다음 operator[] 범위를 증명한다.
            if (edge_index < graph[static_cast<std::size_t>(vertex)].size()) {
                const int neighbor{graph[static_cast<std::size_t>(vertex)][edge_index]};
                ++edge_index; // 같은 간선을 다시 보지 않도록 다음 위치로 이동한다.

                if (discovered[static_cast<std::size_t>(neighbor)] == 0) {
                    parent[static_cast<std::size_t>(neighbor)] = vertex;
                    ++child_count[static_cast<std::size_t>(vertex)]; // 새 DFS 트리 간선일 때만 자식 수를 늘린다.
                    discovered[static_cast<std::size_t>(neighbor)] = ++timer;
                    low[static_cast<std::size_t>(neighbor)] = discovered[static_cast<std::size_t>(neighbor)];
                    stack.push_back(neighbor); // 자식 프레임을 쌓아 자식의 모든 간선을 먼저 처리한다.
                } else if (neighbor != parent[static_cast<std::size_t>(vertex)]) {
                    // min<int>(a,b)는 두 int lvalue를 const 참조로 읽고 작은 값의 const int&를 반환한다.
                    // 즉시 int에 복사해 low에 저장하므로 반환 참조 수명 문제는 없다. 두 입력은 유지되고 O(1), 할당·예외 없음이다.
                    low[static_cast<std::size_t>(vertex)] =
                        std::min(low[static_cast<std::size_t>(vertex)], discovered[static_cast<std::size_t>(neighbor)]);
                }
                continue; // 자식을 방금 넣었거나 한 역방향 간선을 처리했으므로 스택 상단부터 다시 본다.
            }

            // pop_back()은 인자·반환값 없이 마지막 원소를 파괴하고 size를 1 줄이며 capacity는 유지한다.
            // 비어 있지 않음은 while이 보장하고 제거 원소 참조만 무효가 된다. O(1), int 소멸은 예외가 없다.
            stack.pop_back();
            const int parent_vertex{parent[static_cast<std::size_t>(vertex)]};
            if (parent_vertex == -1) { // 루트는 일반 low 조건이 아니라 DFS 트리 자식이 둘 이상일 때 단절점이다.
                if (child_count[static_cast<std::size_t>(vertex)] > 1) {
                    articulation[static_cast<std::size_t>(vertex)] = 1;
                }
            } else {
                low[static_cast<std::size_t>(parent_vertex)] =
                    std::min(low[static_cast<std::size_t>(parent_vertex)], low[static_cast<std::size_t>(vertex)]);
                // 자식 서브트리가 부모보다 앞선 조상으로 갈 수 없으면 부모 제거 시 그 서브트리가 분리된다.
                // 부모가 루트일 때는 별도 자식 수 규칙을 써야 하므로 parent[parent_vertex] 조건으로 제외한다.
                if (parent[static_cast<std::size_t>(parent_vertex)] != -1 &&
                    low[static_cast<std::size_t>(vertex)] >= discovered[static_cast<std::size_t>(parent_vertex)]) {
                    articulation[static_cast<std::size_t>(parent_vertex)] = 1;
                }
            }
        }
    }

    int answer_count{};
    for (int city{1}; city <= city_count; ++city) {
        if (articulation[static_cast<std::size_t>(city)] != 0) {
            ++answer_count;
        }
    }

    // operator<<는 cout lvalue와 int/문자 값을 차례로 읽어 같은 ostream&를 반환한다. cout 문자 위치·상태만 바뀌며
    // 반환 참조는 연쇄에 쓰고 마지막에 버린다. 출력 길이에 선형이고 실패는 기본적으로 상태 비트에 남는다.
    std::cout << answer_count << '\n';
    bool first_output{true}; // 첫 번호 앞에는 공백을 쓰지 않도록 출력 상태를 값으로 추적한다.
    for (int city{1}; city <= city_count; ++city) {
        if (articulation[static_cast<std::size_t>(city)] != 0) {
            if (!first_output) {
                std::cout << ' '; // 두 번째 이후 번호 앞에만 구분 문자를 쓰며 cout의 소유권은 유지된다.
            }
            std::cout << city; // 답 순서는 자유이며 int 값의 소유권은 그대로다.
            first_output = false;
        }
    }
    std::cout << '\n';
    return 0;
}
