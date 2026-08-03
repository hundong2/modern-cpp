/*
문제 ID/제목: BOJ 1197 — 최소 스패닝 트리
출처: https://www.acmicpc.net/problem/1197

문제 요약: V개의 정점과 E개의 가중치 있는 무방향 간선으로 이루어진 연결 그래프에서,
모든 정점을 연결하면서 간선 가중치 합이 최소인 트리의 가중치를 구한다.
입력: 첫 줄에 정점 수 V와 간선 수 E, 다음 E줄에 두 끝점 A, B와 가중치 C가 주어진다.
출력: 최소 스패닝 트리의 가중치 합을 한 줄에 출력한다.
제약: 1 <= V <= 10,000, 1 <= E <= 100,000이며 가중치는 음수일 수도 있다.
예제: 입력 `3 3 / 1 2 1 / 2 3 2 / 1 3 3`의 출력은 `3`이다.
*/

// <algorithm>은 간선을 가중치 순으로 정렬하는 std::sort를 제공한다.
#include <algorithm>
// <iostream>은 빠른 표준 입력 std::cin과 출력 std::cout를 제공한다.
#include <iostream>
// <numeric>은 0,1,... 연속값을 채우는 std::iota를 제공한다.
#include <numeric>
// <vector>는 크기가 런타임에 정해지는 연속 저장 컨테이너를 제공한다.
#include <vector>

struct Edge { // struct의 기본 public 멤버로 한 간선의 두 끝점과 가중치를 묶는다.
    int from{};
    int to{};
    int weight{};
};

class DisjointSet { // class는 기본 private이며 부모·크기 불변식을 내부에서 지킨다.
public:
    explicit DisjointSet(int vertex_count)
        : parent_(static_cast<std::size_t>(vertex_count + 1)),
          size_(static_cast<std::size_t>(vertex_count + 1), 1) {
        // iota는 각 정점을 자기 자신의 대표로 초기화한다.
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    [[nodiscard]] int find(int vertex) {
        // 대표가 아니면 재귀 결과를 저장하는 경로 압축으로 이후 탐색을 거의 상수 시간으로 줄인다.
        if (parent_[vertex] != vertex) {
            parent_[vertex] = find(parent_[vertex]);
        }
        return parent_[vertex];
    }

    [[nodiscard]] bool unite(int left, int right) {
        int left_root{find(left)};   // 중괄호로 find 반환 int prvalue를 직접 초기화한다.
        int right_root{find(right)};
        if (left_root == right_root) { // 같은 집합이면 이 간선은 사이클을 만들므로 거부한다.
            return false;
        }
        if (size_[left_root] < size_[right_root]) { // 작은 트리를 큰 트리 아래에 붙이는 크기 기준 합치기다.
            std::swap(left_root, right_root);
        }
        parent_[right_root] = left_root;
        size_[left_root] += size_[right_root]; // +=는 기존 크기에 오른쪽 크기를 더해 저장한다.
        return true;
    }

private:
    std::vector<int> parent_; // parent_[v]는 부모이며 대표 정점은 자기 자신을 가리킨다.
    std::vector<int> size_;   // 대표에서만 집합 크기가 의미 있다.
};

int main() {
    std::ios::sync_with_stdio(false); // C와 C++ 스트림 동기화를 끊어 대량 입력을 빠르게 한다.
    std::cin.tie(nullptr); // 입력 전에 자동 출력 flush를 하지 않도록 null 포인터를 연결한다.

    int vertex_count{};
    int edge_count{};
    std::cin >> vertex_count >> edge_count; // >>는 공백을 건너뛰며 정수를 변수에 저장한다.

    std::vector<Edge> edges;
    edges.reserve(static_cast<std::size_t>(edge_count)); // 재할당을 줄이되 size는 아직 0이다.
    for (int index{}; index < edge_count; ++index) { // 초기화·조건 비교·증가로 E번 반복한다.
        Edge edge{};
        std::cin >> edge.from >> edge.to >> edge.weight;
        edges.push_back(edge); // lvalue edge를 벡터 끝에 복사한다.
    }

    // 구현 근처 공용 문서: ../algorithm/kruskal-minimum-spanning-tree.md
    std::sort(edges.begin(), edges.end(), [](const Edge& left, const Edge& right) {
        return left.weight < right.weight; // 탐욕적으로 가장 가벼운 간선부터 보도록 비교한다.
    });

    DisjointSet sets{vertex_count};
    long long total_weight{}; // 최악의 가중치 합 오버플로를 피하려 int보다 넓은 누적 타입을 쓴다.
    int selected_edges{};
    for (const Edge& edge : edges) { // const 참조는 Edge 복사 없이 읽기만 한다.
        if (sets.unite(edge.from, edge.to)) { // 서로 다른 컴포넌트만 연결해 숲 불변식을 유지한다.
            total_weight += edge.weight;
            ++selected_edges;
            if (selected_edges == vertex_count - 1) { // 트리는 정확히 V-1개 간선을 가지므로 조기 종료한다.
                break;
            }
        }
    }

    std::cout << total_weight << '\n';
    return 0;
}
