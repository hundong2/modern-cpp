/*
문제 ID·제목: BOJ 13510 - 트리와 쿼리 1
출처: Baekjoon Online Judge, https://www.acmicpc.net/problem/13510

문제 요약:
1번부터 N번까지 번호가 붙은 정점이 N-1개의 가중 간선으로 연결되어 트리를 이룬다.
간선도 입력 순서대로 1번부터 N-1번까지 번호를 가진다. 온라인으로 다음 두 연산을 처리한다.
  1 i c: i번 간선의 가중치를 c로 바꾼다.
  2 u v: u에서 v까지 유일한 단순 경로에 있는 간선 가중치의 최댓값을 출력한다.

입력:
첫 줄에 정점 수 N이 주어진다. 다음 N-1줄에 각 간선의 양 끝점 u, v와 초기 가중치 w가 주어진다.
그 다음 줄에 쿼리 수 M이 주어지고, 이어지는 M줄에는 위 두 형식 중 하나가 주어진다.

출력:
2번 쿼리마다 현재 가중치를 기준으로 경로의 최댓값을 한 줄에 하나씩 출력한다.

제약:
2 <= N <= 100,000, 1 <= M <= 100,000이며 간선 가중치와 갱신 값은 1 이상 1,000,000 이하이다.
O(N) 스캔을 매 쿼리마다 하면 최악 O(NM)이므로 통과할 수 없다.

예제(구조를 설명하도록 한글로 재서술):
정점 1-2의 비용이 1, 2-3의 비용이 2인 세 정점 트리에서 1과 2 사이의 최댓값은 1이다.
이후 1번 간선 비용을 3으로 바꾸고 같은 경로를 물으면 3이 출력된다.

풀이 개요:
Heavy-Light Decomposition(HLD)은 각 정점의 가장 큰 자식 서브트리 간선을 heavy로 고른다.
heavy 간선을 따라가는 정점들은 선형 배열에서 연속인 체인이 된다. 임의의 루트 경로가 light 간선을
지나는 횟수는 O(log N)이므로 두 정점 경로도 O(log N)개의 연속 구간으로 나뉜다.
각 간선 가중치를 더 깊은 끝 정점의 위치에 저장하고 세그먼트 트리로 구간 최댓값을 처리한다.
HLD 전처리 O(N), 현재 구현의 N회 점 갱신 초기화 O(N log N), 간선 갱신 O(log N),
경로 질의 O(log^2 N), 전체 저장 공간 O(N)이다.
*/

#include <algorithm>  // std::max로 구간·경로 최댓값과 heavy 자식을 선택한다.
#include <cstddef>    // std::size_t로 vector 인덱스·원소 개수 타입을 명시한다.
#include <iostream>   // std::cin/std::cout과 스트림 입력·출력 연산자를 선언한다.
#include <utility>    // std::pair로 정점과 체인 머리를 한 값으로 묶는다.
#include <vector>     // 그래프, 스택, 배열, 세그먼트 트리를 연속 동적 저장한다.

// 인접 리스트의 한 간선을 값으로 보관한다. struct의 멤버는 기본 public이다.
struct Edge {
    int to{};      // 연결된 정점 번호다.
    int weight{};  // 현재가 아닌 초기 가중치이며 세그먼트 트리 초기화에 쓴다.
    int index{};   // 입력 순서로 정해진 1-based 간선 번호다.
};

// 세그먼트 트리는 선형화 배열의 한 점 갱신과 닫힌 구간 최댓값을 캡슐화한다.
class SegmentTree {
public:
    // 생성자에는 반환형이 없다. explicit은 int가 SegmentTree로 암시 변환되는 것을 막는다.
    explicit SegmentTree(int element_count) {
        // base_를 원소 수 이상인 가장 작은 2의 거듭제곱으로 키운다.
        while (base_ < element_count) {
            // *= 복합 대입은 오른쪽 값을 곱한 결과를 같은 변수에 저장한다.
            base_ *= 2;
        }

        // assign(count, value)는 vector<int> 수신 객체 tree_의 기존 원소를 버리고 base_*2개의 0을 복사 저장한다.
        // 두 인자는 각각 size_type으로 변환되는 양의 개수와 int 값 0이다. 반환형은 void라 결과를 저장하지 않는다.
        // 호출 뒤 size와 capacity는 충분한 크기가 되고 기존 포인터·참조·반복자는 모두 무효화된다.
        // O(base_) 시간·공간이며 할당 실패 시 예외가 나고, 단일 스레드 전처리에서만 호출한다.
        tree_.assign(static_cast<std::size_t>(base_ * 2), 0);
    }

    // position의 값을 value로 교체하고 루트까지 최댓값을 다시 계산한다.
    void update(int position, int value) {
        // 배열 위치에 leaf 시작점을 더해 실제 트리 노드 인덱스를 만든다.
        int node{base_ + position};
        // operator[]은 경계 검사 없이 int lvalue 참조를 반환한다. 0<=position<element_count 전제가 깨지면 UB다.
        tree_[static_cast<std::size_t>(node)] = value;

        // 부모 인덱스가 0이 될 때까지 올라가며 불변식을 복구한다.
        for (node /= 2; node >= 1; node /= 2) {
            // std::max(const int&, const int&)는 두 자식 lvalue를 읽어 큰 값의 const 참조를 반환하고,
            // 대입이 그 값을 부모 int에 복사한다. 인자는 유지되고 호출 자체는 O(1), 할당·예외가 없다.
            tree_[static_cast<std::size_t>(node)] =
                std::max(tree_[static_cast<std::size_t>(node * 2)],
                         tree_[static_cast<std::size_t>(node * 2 + 1)]);
        }
    }

    // [left, right] 닫힌 구간의 최댓값을 반환한다. 빈 구간에는 양의 가중치 max의 항등값 0을 쓴다.
    [[nodiscard]] int query(int left, int right) const {
        if (left > right) {
            return 0;
        }

        // 반복형 세그먼트 트리에서 두 포인터를 leaf 구간으로 옮긴다.
        int left_node{left + base_};
        int right_node{right + base_};
        int answer{};

        // 아직 처리하지 않은 구간이 남는 동안 경계 노드를 선택하고 부모로 이동한다.
        while (left_node <= right_node) {
            // 홀수인 왼쪽 노드는 부모 구간에서 오른쪽 자식이므로 현재 답에 포함하고 다음 노드로 간다.
            if ((left_node % 2) == 1) {
                answer = std::max(answer, tree_[static_cast<std::size_t>(left_node)]);
                ++left_node;
            }
            // 짝수인 오른쪽 노드는 왼쪽 자식이므로 현재 답에 포함하고 이전 노드로 간다.
            if ((right_node % 2) == 0) {
                answer = std::max(answer, tree_[static_cast<std::size_t>(right_node)]);
                --right_node;
            }
            // 두 경계를 부모 레벨로 올린다. 매 반복마다 구간 높이가 하나 줄어 O(log N)이다.
            left_node /= 2;
            right_node /= 2;
        }
        return answer;
    }

private:
    int base_{1};           // leaf 시작 인덱스인 2의 거듭제곱이다.
    std::vector<int> tree_; // 각 내부 노드는 담당 구간의 최대를 보관한다.
};

int main() {
    // sync_with_stdio(false)는 C/C++ 표준 스트림 동기화 설정을 false로 바꾸고 이전 bool을 반환한다.
    // 반환값은 무시하며 이후 printf/scanf와 섞지 않는다. 이미 I/O한 뒤 호출하는 것은 피한다.
    std::ios::sync_with_stdio(false);
    // tie(nullptr)는 cin 수신 객체의 선행 flush 대상 ostream*을 null로 바꾸고 이전 포인터를 반환한다.
    // 반환은 무시하며 비대화형 온라인 저지에서 출력 전 자동 flush 비용을 줄인다.
    std::cin.tie(nullptr);

    int vertex_count{};
    // operator>>는 cin과 vertex_count lvalue 참조를 받아 성공 시 값을 저장하고 입력 위치를 전진시킨다.
    // 같은 istream&를 반환하며 bool 문맥에서 실패를 검사한다. 실패하면 조용히 정상 종료한다.
    if (!(std::cin >> vertex_count)) {
        return 0;
    }

    // vector(count)는 count개의 내부 vector<Edge>를 값 초기화한다. 그래프 전체 수명과 소유권은 graph에 있다.
    std::vector<std::vector<Edge>> graph(static_cast<std::size_t>(vertex_count + 1));
    // 간선 번호에서 입력 양 끝점을 보관한다. 갱신 시 어느 끝이 자식인지 찾는 데 쓴다.
    std::vector<std::pair<int, int>> endpoints(static_cast<std::size_t>(vertex_count));

    // 트리는 정확히 N-1개의 간선을 가진다.
    for (int edge_index{1}; edge_index < vertex_count; ++edge_index) {
        int from{};
        int to{};
        int weight{};
        // 연쇄 operator>> 각각은 대상 int lvalue를 갱신하고 같은 cin 참조를 다음 호출로 넘긴다.
        std::cin >> from >> to >> weight;
        endpoints[static_cast<std::size_t>(edge_index)] = {from, to};

        // push_back(Edge&&)는 Edge prvalue를 각 인접 vector 끝으로 이동/생성한다.
        // 성공하면 size가 1 늘고 재할당 시 그 vector의 기존 참조·포인터·반복자가 무효화된다.
        // 분할 상환 O(1), 할당 실패 시 예외가 가능하며 반대편 vector와 endpoints는 유지된다.
        graph[static_cast<std::size_t>(from)].push_back(Edge{to, weight, edge_index});
        graph[static_cast<std::size_t>(to)].push_back(Edge{from, weight, edge_index});
    }

    // 모두 N+1개의 int를 소유한다. 0번 칸은 사용하지 않거나 루트의 부모 sentinel로 쓴다.
    std::vector<int> parent(static_cast<std::size_t>(vertex_count + 1), 0);
    std::vector<int> depth(static_cast<std::size_t>(vertex_count + 1), 0);
    std::vector<int> subtree_size(static_cast<std::size_t>(vertex_count + 1), 1);
    std::vector<int> heavy_child(static_cast<std::size_t>(vertex_count + 1), 0);
    std::vector<int> parent_weight(static_cast<std::size_t>(vertex_count + 1), 0);
    std::vector<int> edge_child(static_cast<std::size_t>(vertex_count), 0);
    std::vector<int> order{};
    std::vector<int> stack{};

    // reserve(vertex_count)는 order의 size를 바꾸지 않고 최소 N capacity를 확보한다.
    // 재할당 시 기존 반복자·포인터·참조가 무효화되지만 아직 원소가 없다. O(N) 공간, 할당 실패 가능성이 있다.
    order.reserve(static_cast<std::size_t>(vertex_count));
    stack.reserve(static_cast<std::size_t>(vertex_count));
    stack.push_back(1);

    // 재귀 대신 명시적 vector 스택으로 부모·깊이·순서를 정해 호출 스택 오버플로를 피한다.
    while (!stack.empty()) {
        // back()은 마지막 int의 lvalue 참조를 O(1)에 반환한다. empty가 거짓이라는 전제라 UB가 아니다.
        const int vertex{stack.back()};
        // pop_back()은 마지막 원소를 파괴하고 size를 1 줄이며 반환값은 없다. 다른 원소 참조는 유지된다.
        stack.pop_back();
        order.push_back(vertex);

        for (const Edge& edge : graph[static_cast<std::size_t>(vertex)]) {
            if (edge.to == parent[static_cast<std::size_t>(vertex)]) {
                continue;
            }
            parent[static_cast<std::size_t>(edge.to)] = vertex;
            depth[static_cast<std::size_t>(edge.to)] = depth[static_cast<std::size_t>(vertex)] + 1;
            parent_weight[static_cast<std::size_t>(edge.to)] = edge.weight;
            edge_child[static_cast<std::size_t>(edge.index)] = edge.to;
            stack.push_back(edge.to);
        }
    }

    // 역순은 자식이 부모보다 먼저 나온다. 각 부모에 서브트리 크기를 합치고 가장 큰 자식을 heavy로 고른다.
    for (int index{vertex_count - 1}; index > 0; --index) {
        const int vertex{order[static_cast<std::size_t>(index)]};
        const int p{parent[static_cast<std::size_t>(vertex)]};
        subtree_size[static_cast<std::size_t>(p)] += subtree_size[static_cast<std::size_t>(vertex)];

        const int previous_heavy{heavy_child[static_cast<std::size_t>(p)]};
        if (previous_heavy == 0 ||
            subtree_size[static_cast<std::size_t>(vertex)] >
                subtree_size[static_cast<std::size_t>(previous_heavy)]) {
            heavy_child[static_cast<std::size_t>(p)] = vertex;
        }
    }

    std::vector<int> head(static_cast<std::size_t>(vertex_count + 1), 0);
    std::vector<int> position(static_cast<std::size_t>(vertex_count + 1), 0);
    // pair의 first는 새 체인 시작 정점, second는 그 체인의 머리다.
    std::vector<std::pair<int, int>> pending_chains{};
    pending_chains.push_back({1, 1});
    int next_position{};

    // 참고 알고리즘 문서: ../algorithm/heavy-light-decomposition.md
    // 각 light 자식은 새 체인을 시작하고, heavy 자식은 같은 체인에서 바로 다음 위치를 받는다.
    while (!pending_chains.empty()) {
        const std::pair<int, int> chain{pending_chains.back()};
        pending_chains.pop_back();

        // heavy_child가 0 sentinel이 될 때까지 한 체인을 연속 위치로 편다.
        for (int vertex{chain.first}; vertex != 0;
             vertex = heavy_child[static_cast<std::size_t>(vertex)]) {
            head[static_cast<std::size_t>(vertex)] = chain.second;
            position[static_cast<std::size_t>(vertex)] = next_position++;

            for (const Edge& edge : graph[static_cast<std::size_t>(vertex)]) {
                const bool is_child{parent[static_cast<std::size_t>(edge.to)] == vertex};
                const bool is_light{edge.to != heavy_child[static_cast<std::size_t>(vertex)]};
                if (is_child && is_light) {
                    pending_chains.push_back({edge.to, edge.to});
                }
            }
        }
    }

    SegmentTree segment_tree{vertex_count};
    // 부모-자식 간선 값을 자식 정점 위치에 저장한다. 루트 위치는 어떤 간선도 나타내지 않아 0이다.
    for (int vertex{2}; vertex <= vertex_count; ++vertex) {
        segment_tree.update(position[static_cast<std::size_t>(vertex)],
                            parent_weight[static_cast<std::size_t>(vertex)]);
    }

    int query_count{};
    std::cin >> query_count;
    for (int query_index{}; query_index < query_count; ++query_index) {
        int type{};
        int first{};
        int second{};
        std::cin >> type >> first >> second;

        if (type == 1) {
            // first는 간선 번호다. edge_child가 가리키는 더 깊은 끝점 위치의 값만 second로 바꾼다.
            const int child{edge_child[static_cast<std::size_t>(first)]};
            segment_tree.update(position[static_cast<std::size_t>(child)], second);
            continue;
        }

        // type 2에서 first와 second는 경로 양 끝 정점이다. 지역 복사라 위로 올려도 입력값은 유지된다.
        int from{first};
        int to{second};
        int answer{};

        // 서로 다른 체인인 동안 더 깊은 체인의 머리부터 현재 정점까지를 한 구간으로 소비한다.
        // light 간선을 하나 넘을 때 남은 서브트리 크기가 절반 이하가 되어 반복은 O(log N)번이다.
        while (head[static_cast<std::size_t>(from)] != head[static_cast<std::size_t>(to)]) {
            if (depth[static_cast<std::size_t>(head[static_cast<std::size_t>(from)])] <
                depth[static_cast<std::size_t>(head[static_cast<std::size_t>(to)])]) {
                // 직접 swap 대신 두 지역 int를 교환한다. 알고리즘 불변식은 from 쪽 head가 더 깊다는 것이다.
                const int temporary{from};
                from = to;
                to = temporary;
            }

            const int chain_head{head[static_cast<std::size_t>(from)]};
            answer = std::max(
                answer,
                segment_tree.query(position[static_cast<std::size_t>(chain_head)],
                                   position[static_cast<std::size_t>(from)]));
            from = parent[static_cast<std::size_t>(chain_head)];
        }

        // 같은 체인에서는 더 깊은 정점을 from으로 맞춘다.
        if (depth[static_cast<std::size_t>(from)] < depth[static_cast<std::size_t>(to)]) {
            const int temporary{from};
            from = to;
            to = temporary;
        }

        // 간선 값을 자식 위치에 저장했으므로 LCA인 to 자신의 위치는 +1로 제외한다.
        answer = std::max(
            answer,
            segment_tree.query(position[static_cast<std::size_t>(to)] + 1,
                               position[static_cast<std::size_t>(from)]));

        // operator<<는 answer를 cout 버퍼에 쓰고 ostream&를 반환해 개행 출력을 연쇄한다.
        // answer는 유지되고 cout 상태·출력 위치가 바뀐다. 실패는 stream 상태로 남으며 반환 참조는 저장하지 않는다.
        std::cout << answer << '\n';
    }

    return 0;
}
