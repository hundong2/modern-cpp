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
    // static bool ios_base::sync_with_stdio(bool sync=true)는 인스턴스 수신 객체 없이 bool prvalue false 하나를 받아
    // C stdio 동기화를 끄고 이전 bool 설정을 반환하지만 여기서는 버린다. 표준 입출력 전에 호출하며 이후 C stdio와
    // 섞지 않는다. 스트림 수명·소유권은 유지되며 표준은 별도 복잡도 상한을 두지 않고 실제 버퍼 동작도 구현에 달린다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::ios::sync_with_stdio(false);
    // std::cin의 정확한 타입은 std::istream이다. setter std::ostream* tie(std::ostream* tied)에서 nullptr prvalue는
    // 소유권 없는 null 포인터 인자이고, 호출은 자동 flush 연결을 해제해 이전 std::ostream*를 반환하지만 버린다.
    // cin/cout 객체와 버퍼 소유권은 유지되고 문자 저장소를 할당하지 않는다. 표준의 별도 복잡도 상한은 없으며
    // 대화형 문제가 아니라 수동 prompt flush도 없다.
    std::cin.tie(nullptr);

    int city_count{}; // int{}는 0으로 값 초기화하며 최대 100,000을 안전하게 담는다.
    int road_count{}; // 최대 200,000도 int 범위다.
    // 첫 std::istream 멤버 operator>>(int&)의 수신자는 cin lvalue, 유일한 데이터 인자는 city_count int lvalue다.
    // 반환 std::istream&가 두 번째 같은 멤버 호출의 수신자가 되고 road_count lvalue를 인자로 받으며 마지막 반환은
    // 버린다. 성공하면 두 int와 입력 위치가 바뀌고 소유권은 유지된다. 표준은 별도 복잡도 상한을 두지 않으며
    // 실제 비용은 소비 문자 수, locale facet, stream buffer 구현에 달린다. int 재할당은 없고 실패는 상태 비트다.
    std::cin >> city_count >> road_count;

    // 생성자에는 기존 수신 객체나 반환값이 없다. 선택된 std::vector<std::vector<int>>(size_type count)에서 유일한
    // 명시 인자는 static_cast가 만든 std::size_t prvalue city_count+1이고 allocator 인자는 기본값을 쓴다.
    // count에는 음수나 max_size() 초과 값을 허용할 수 없다.
    // 성공하면 graph가 count개의 값 초기화된 빈 vector<int>를 소유한다. O(n) 시간·공간, 바깥 저장소 할당이 들고
    // length_error/bad_alloc 가능성이 있으며 아직 외부 관찰자가 없어 무효화 대상은 없다. 1기반 번호라 0번은 비운다.
    std::vector<std::vector<int>> graph(static_cast<std::size_t>(city_count + 1));
    for (int road{}; road < road_count; ++road) { // 각 도로를 정확히 한 번 읽는 반복문이다.
        int from{};
        int to{};
        std::cin >> from >> to; // 위와 같은 두 멤버 오버로드가 from/to lvalue를 갱신하고 마지막 istream&는 버린다.
        // 첫 operator[]의 수신자는 graph lvalue(vector<vector<int>>)이고 size_t prvalue from이 위치 인자다. 비const
        // reference 오버로드는 해당 vector<int>&를 O(1)에 반환하며 graph는 바뀌지 않는다. 입력 계약이 from<=n을
        // 보장하고 범위 검사는 없으므로 위반 시 UB다. 반환 참조는 graph의 수명 및 바깥 vector 재할당 전까지 유효하며
        // graph는 이후 재할당하지 않는다. 이어지는 push_back(const int&)은 그 vector<int>에 to lvalue 값을 복사해
        // size를 1 늘리고 void를 반환한다. 분할 상환 O(1), 내부 재할당 시 그 목록의 관찰자는 무효이며 예외 가능하다.
        // 대표 문서: ../standard-library/containers-and-views.md
        graph[static_cast<std::size_t>(from)].push_back(to);
        graph[static_cast<std::size_t>(to)].push_back(from); // 무방향 도로라 반대 방향 값도 독립 복사한다.
    }

    // 다음 구현의 low-link 정의와 단절점 판정 근거는 공용 문서 ../algorithm/articulation-points-low-link.md 를 따른다.
    // vector<int>(size_type count, const int& value)에는 size_t prvalue count와 const int&에 바인딩되는 int prvalue 0/-1
    // 두 명시 인자가 있고 allocator는 기본값이다. 생성자는 반환값 없이 count개 복사본을 소유하며 원본 임시 값은 유지된 채 전체 식 뒤 소멸한다.
    // 성공 후 size=count이고 O(n) 시간·공간, length_error/bad_alloc 가능성이 있으며 새 객체라 무효화할 관찰자는 없다.
    std::vector<int> discovered(static_cast<std::size_t>(city_count + 1), 0); // 0은 아직 방문하지 않음을 뜻한다.
    std::vector<int> low(static_cast<std::size_t>(city_count + 1), 0); // DFS 서브트리에서 도달 가능한 최소 방문 순서다.
    std::vector<int> parent(static_cast<std::size_t>(city_count + 1), -1); // -1은 DFS 루트 또는 미설정 상태다.
    std::vector<int> child_count(static_cast<std::size_t>(city_count + 1), 0); // DFS 트리 자식 수다.
    // 같은 count/value 오버로드에서 T=size_t/char로 바뀌며 두 번째 prvalue를 각 원소에 복사한다.
    std::vector<std::size_t> next_edge(static_cast<std::size_t>(city_count + 1), std::size_t{0}); // 다음에 검사할 인접 인덱스다.
    std::vector<char> articulation(static_cast<std::size_t>(city_count + 1), char{0}); // vector<bool> proxy 대신 0/1 바이트를 쓴다.
    // vector<int>() 기본 생성자는 인자·반환값 없이 size=0인 소유 컨테이너를 O(1)에 만든다. 원소 수명은 아직
    // 시작하지 않고 물리적 capacity·할당 여부는 단정하지 않는다. 이후 reserve가 최소 필요 용량을 보장한다.
    std::vector<int> stack; // 재귀 호출 스택 대신 heap 저장소를 가진 명시적 DFS 스택이다.
    // void reserve(size_type new_cap)의 수신자는 size=0인 vector<int> stack이고, size_t prvalue city_count가 유일한
    // 인자다. max_size() 이하여야 하며 성공하면 size는 0, capacity는 최소 city_count다. 현재 size에 선형인데 여기서는
    // 이동할 원소가 0개다. length_error/bad_alloc 실패 시 이 int vector는 그대로이고, 관찰자도 아직 없다.
    stack.reserve(static_cast<std::size_t>(city_count));

    int timer{}; // 방문 순서를 1부터 증가시키는 int이며 최대 n이라 안전하다.
    for (int root{1}; root <= city_count; ++root) { // 연결 보장이 있지만 일반 그래프에도 안전하도록 모든 시작점을 본다.
        // discovered의 정확한 타입은 vector<int>이고 size_t prvalue root를 받는 비const operator[](size_type)이 int&를
        // O(1)에 반환한다. loop가 1<=root<=n을 보장하며 범위 검사가 없어 위반은 UB다. 비교는 참조가 가리킨 값을
        // 읽기만 해 vector를 바꾸지 않고, 참조는 저장하지 않으며 배열들은 이후 재할당되지 않아 원소 수명도 유지된다.
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
            // next_edge vector<size_t>의 같은 operator[] 계약으로 size_t&를 얻어 저장한다. 인덱스는 DFS 정점이라
            // 범위 안이고 배열은 재할당되지 않으므로 참조는 이번 반복에서 유효하며 ++가 원소 진행 상태를 제자리 갱신한다.
            std::size_t& edge_index{next_edge[static_cast<std::size_t>(vertex)]};

            // size()는 정확한 수신 타입 vector<int>를 바꾸지 않고 데이터 인자 없이 O(1)에 size_type 값을 반환한다.
            // 반환값을 edge_index 비교에 쓰며 할당·무효화·예외가 없고, true 결과가 바로 다음 안쪽 operator[] 범위를
            // 증명한다. 그 operator[](edge_index)는 int&를 O(1)에 반환하며 읽어서 neighbor에 복사할 뿐 저장하지 않는다.
            // 바깥 graph[vertex]도 앞서 설명한 비const 참조 오버로드이며 vertex의 1..n 범위는 DFS 불변식이 보장한다.
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
            // 비어 있지 않음은 while이 보장한다. 제거 원소 관찰자와 이전 past-the-end 반복자는 무효가 되고,
            // 그보다 앞선 원소 관찰자와 capacity는 유지된다. O(1)이며 int 소멸은 예외가 없다.
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

    // 정확한 타입 std::ostream인 cout의 멤버 operator<<(int)는 answer_count 값을 복사 입력으로 받고 ostream&를
    // 반환한다. 이어지는 비멤버 operator<<(ostream&, char)는 그 참조와 '\n' prvalue를 받아 같은 참조를 반환해
    // 버린다. cout의 문자 위치·상태만 바뀐다. 표준은 별도 복잡도 상한을 두지 않으며 실제 비용은 형식화할 문자 수,
    // locale facet, stream buffer와 장치 구현에 달리고 실패는 기본적으로 상태 비트에 남는다.
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
