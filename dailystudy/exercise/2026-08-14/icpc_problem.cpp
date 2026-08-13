/*
문제: BOJ 1916 - 최소비용 구하기
출처: https://www.acmicpc.net/problem/1916
요약: N개 도시와 M개 단방향 버스 노선에서 지정한 출발지부터 도착지까지 최소 비용을 구한다.
입력: N, M, M개의 (출발, 도착, 음수가 아닌 비용), 마지막에 출발지와 도착지가 주어진다.
출력: 가능한 경로 가운데 최소 비용 하나를 출력한다.
제약: 1<=N<=1,000, 1<=M<=100,000, 비용은 0 이상 100,000 미만이며 답이 존재한다.
예제: 5개 도시의 예시에서 1→4→5를 택하면 최소 비용 4가 된다.
*/

// 각 헤더는 최소 힙 비교자, 입출력, 수치 한계, 우선순위 큐, pair, vector를 제공한다.
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

// 구현 참고 문서: ../algorithm/dijkstra.md
// struct는 기본 public이며 간선 데이터를 단순하게 묶는다.
struct Edge {
    int to{};   // 도착 도시 번호를 중괄호로 0 초기화한다.
    int cost{}; // 음수가 아닌 간선 비용이다.
};

// using은 긴 템플릿 타입에 (누적 거리, 도시)라는 별칭을 붙인다.
using State = std::pair<long long, int>;

// graph는 const lvalue 참조로 복사 없이 읽고, 함수는 최소 비용 기본 타입을 반환한다.
[[nodiscard]] long long shortest_path(int start, int destination,
                                      const std::vector<std::vector<Edge>>& graph) {
    const long long infinity{std::numeric_limits<long long>::max() / 4}; // 덧셈 여유가 있는 무한대 표식이다.
    std::vector<long long> distance(graph.size(), infinity); // 도시별 현재 최선 거리를 O(V)에 저장한다.
    // 템플릿 인자는 원소, 내부 컨테이너, 비교자이며 greater가 최소 힙을 만든다.
    std::priority_queue<State, std::vector<State>, std::greater<State>> heap{};
    distance[start] = 0; // 시작점의 빈 경로 비용은 0이라는 초기 불변식이다.
    heap.push(State{0, start}); // prvalue 후보를 표준 라이브러리 함수로 힙에 넣는다.

    while (!heap.empty()) { // 후보가 남은 동안 반복한다.
        const auto [current_distance, city]{heap.top()}; // 구조적 바인딩으로 pair를 두 const 값에 복사한다.
        heap.pop(); // O(log V)로 현재 최소 후보를 제거한다.
        if (current_distance != distance[city]) { // 이미 개선된 오래된 항목인지 비교한다.
            continue; // 오래된 항목은 불변식과 답에 영향이 없어 건너뛴다.
        }
        if (city == destination) { // 비음수 간선이므로 최신 최소 후보로 꺼낸 목적지는 확정된다.
            return current_distance;
        }
        for (const Edge& edge : graph[city]) { // const 참조로 인접 간선을 복사 없이 순회한다.
            const long long candidate{current_distance + edge.cost}; // 새 경로 비용을 계산한다.
            if (candidate < distance[edge.to]) { // 완화: 기존 값보다 작을 때만 갱신한다.
                distance[edge.to] = candidate; // 저장으로 현재 최선 거리 불변식을 유지한다.
                heap.push(State{candidate, edge.to}); // 개선 후보를 O(log V)로 넣는다.
            }
        }
    }
    return distance[destination]; // 문제는 경로 존재를 보장하지만 일반적인 안전 반환이다.
}

int main() {
    std::ios::sync_with_stdio(false); // C/C++ 스트림 동기화를 끊어 입력을 빠르게 한다.
    std::cin.tie(nullptr); // nullptr는 아무 출력 스트림에도 묶지 않는 포인터 값이다.
    int city_count{};
    int bus_count{};
    std::cin >> city_count >> bus_count; // >> 연산자가 토큰을 변수에 저장한다.
    std::vector<std::vector<Edge>> graph(static_cast<std::size_t>(city_count + 1)); // 1 기반 인접 리스트다.
    for (int index{}; index < bus_count; ++index) { // 조건이 참인 동안 정확히 M번 반복한다.
        int from{};
        int to{};
        int cost{};
        std::cin >> from >> to >> cost;
        graph[from].push_back(Edge{to, cost}); // 방향 간선을 집합 초기화해 끝에 추가한다.
    }
    int start{};
    int destination{};
    std::cin >> start >> destination;
    std::cout << shortest_path(start, destination, graph) << '\n'; // 함수 호출 결과를 출력한다.
    // 로드·저장·비교·분기·호출의 구체 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return 0;
}
