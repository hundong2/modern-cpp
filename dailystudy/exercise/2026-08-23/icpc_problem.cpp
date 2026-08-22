/*
문제 ID·제목: BOJ 11657 - 타임머신
출처: https://www.acmicpc.net/problem/11657 (Baekjoon Online Judge)

문제 요약:
1번부터 N번까지 번호가 붙은 도시와 M개의 단방향 버스 노선이 있다. 노선 시간은 양수일 수도 있고,
0이거나 음수일 수도 있다. 1번 도시에서 나머지 각 도시까지 가는 가장 짧은 시간을 구한다. 1번에서
도달할 수 있는 경로에 시간을 끝없이 줄이는 음수 사이클이 있으면 유한한 최단 시간이 없다고 판정한다.
아래 설명은 원문의 요지를 한글로 다시 쓴 것이며 저작권이 있는 문제 원문 전체를 복제하지 않는다.

입력:
첫 줄에 도시 수 N과 버스 노선 수 M이 주어진다. 다음 M줄에는 출발 도시 A, 도착 도시 B, 이동 시간 C가
주어진다. 같은 두 도시 사이에 여러 노선이 있을 수 있다.

출력:
1번 도시에서 도달 가능한 음수 사이클이 있으면 첫 줄에 -1 하나만 출력한다. 그렇지 않으면 2번부터
N번 도시까지 최단 시간을 한 줄에 하나씩 출력하고, 갈 수 없는 도시는 -1을 출력한다.

제약:
1 <= N <= 500, 1 <= M <= 6,000, 1 <= A,B <= N, -10,000 <= C <= 10,000이다.
정답 경로 합과 안전한 INF 표현을 위해 long long을 사용한다. 음수 사이클은 반드시 1번에서 도달 가능한
간선만 완화할 때 판정해야 한다.

예제:
입력
3 4
1 2 4
1 3 3
2 3 -1
3 1 -2

출력
4
3
*/

// <cstddef>는 vector 크기와 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력·출력과 고속 입출력 설정을 선언한다.
#include <iostream>
// <limits>는 long long 표현 한계를 질의하는 std::numeric_limits를 선언한다.
#include <limits>
// <vector>는 간선 목록과 거리 배열을 연속 메모리에 소유하는 std::vector를 선언한다.
#include <vector>

// struct는 기본 접근이 public이다. Bellman–Ford가 순회할 단방향 간선 값 세 개를 단순히 묶는다.
struct Edge {
    int from{}; // 출발 도시 번호를 값으로 소유한다.
    int to{};   // 도착 도시 번호를 값으로 소유한다.
    int cost{}; // 음수를 포함할 수 있는 이동 시간을 값으로 소유한다.
};

// 공용 알고리즘 문서: ../algorithm/bellman-ford.md
int main() {
    // ios::sync_with_stdio(false)는 C/C++ 표준 스트림 동기화를 끄고 이전 bool 설정을 반환한다.
    // false는 유일한 값 인자이고 반환값은 버린다. 전역 I/O 상태가 바뀌므로 이후 C stdio와 임의 순서로 섞지 않는다.
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 ostream* 널 포인터를 받아 입력 전 자동 flush 연결을 해제한다.
    // 이전 ostream* 반환은 버리며 cin/cout의 소유권은 유지된다. 대화형 문제가 아니므로 자동 flush가 필요 없다.
    std::cin.tie(nullptr);

    int city_count{}; // int{}는 0으로 값 초기화되며 입력 뒤 N을 보관한다.
    int edge_count{}; // 입력 뒤 M을 보관한다.
    // istream operator>>는 두 int lvalue 참조에 값을 저장하고 istream&를 연쇄 반환한다.
    // 반환 참조는 조건 검사 없이 버린다. 유효한 온라인 저지 입력이 전제이며 실패하면 failbit가 남고 변수는 안전한 0이다.
    std::cin >> city_count >> edge_count;

    // vector<Edge> 기본 생성자는 크기·용량 0인 간선 소유 컨테이너를 만든다. 아직 동적 할당은 필요 없다.
    std::vector<Edge> edges{};
    // reserve(size_type)는 edge_count를 size_t로 변환한 용량 이상을 미리 확보하고 반환형 void라 저장하지 않는다.
    // 크기는 0으로 유지된다. 성공하면 이후 M번 push_back 중 재할당을 피하고, 실패하면 bad_alloc을 던져 원래 vector를 유지한다.
    edges.reserve(static_cast<std::size_t>(edge_count));

    // for 초기식은 int index를 0으로 만들고, <는 M보다 작은 동안만 본문을 실행하며 ++는 매번 1 증가시킨다.
    for (int index{0}; index < edge_count; ++index) {
        Edge edge{}; // 세 int 멤버를 모두 0으로 값 초기화한다.
        // 각 >>는 from, to, cost lvalue에 순서대로 입력을 저장한다. 같은 노선의 중복도 그대로 보존한다.
        std::cin >> edge.from >> edge.to >> edge.cost;
        // vector::push_back(const Edge&)는 edge lvalue를 const 참조로 빌려 한 원소를 복사한다.
        // 반환형은 void다. 호출 뒤 크기는 1 늘고 edge는 유지된다. 예약 용량 안에서는 기존 참조가 유효하고 상각 O(1)이다.
        edges.push_back(edge);
    }

    // numeric_limits<long long>::max()는 인자·상태 없이 long long 최댓값을 constexpr 값으로 반환한다.
    // 4로 나눠 실제 거리와 간선 비용을 더해도 표현 범위를 넘지 않는 도달 불가 표지를 만든다.
    constexpr long long infinity{std::numeric_limits<long long>::max() / 4};
    // vector(count, value)는 city_count+1개 long long을 각각 infinity로 복사해 소유한다.
    // 0번 칸은 버리고 도시 번호를 그대로 인덱스로 쓴다. O(N) 시간·공간, 할당 실패 시 bad_alloc 가능성이 있다.
    std::vector<long long> distance(static_cast<std::size_t>(city_count + 1), infinity);
    // vector::operator[]는 범위 검사 없이 long long&를 O(1)에 반환한다. N>=1이므로 인덱스 1은 유효하다.
    // 대입 뒤 시작점의 최단 거리 상한은 정확히 0이고 다른 원소와 vector 크기·용량은 유지된다.
    distance[1] = 0;

    bool has_reachable_negative_cycle{}; // bool{}는 false이며 N번째 완화 성공 시 true가 된다.

    // Bellman–Ford 핵심: i번째 전체 순회 뒤 간선 수가 최대 i인 최단 경로가 반영된다.
    // 단순 최단 경로는 간선이 최대 N-1개이므로 N번째에도 감소하면 시작점에서 닿는 음수 사이클이 있다.
    for (int iteration{1}; iteration <= city_count; ++iteration) {
        bool updated{}; // 이번 전체 간선 순회에서 거리 하나라도 감소했는지 기록한다.

        // range-for의 const Edge&는 vector 원소 lvalue에 읽기 전용 참조로 바인딩한다.
        // 복사하지 않고 M개를 순서대로 읽으며, 본문에서 vector 구조를 바꾸지 않아 참조가 계속 유효하다.
        for (const Edge& edge : edges) {
            // from이 아직 infinity면 1번에서 이 간선 출발점까지 닿지 못한다.
            // ==는 bool을 만들고 continue는 아래 덧셈을 건너뛰어 INF + 음수의 가짜 완화를 막는다.
            if (distance[static_cast<std::size_t>(edge.from)] == infinity) {
                continue;
            }

            // 도달 가능한 출발점의 현재 상한에 간선 비용을 더한 새 경로 후보를 계산한다.
            // cost를 long long으로 승격해 덧셈하며 infinity가 아니라는 분기와 여유 있는 표지가 오버플로를 막는다.
            const long long candidate{
                distance[static_cast<std::size_t>(edge.from)] + static_cast<long long>(edge.cost)};

            // 더 작은 후보만 채택한다. 이것이 distance[v]가 발견한 경로 길이의 최솟값이라는 불변식을 보존한다.
            if (candidate < distance[static_cast<std::size_t>(edge.to)]) {
                distance[static_cast<std::size_t>(edge.to)] = candidate;
                updated = true;

                // N번째 순회에서의 감소는 N개 이상 간선을 쓰는 더 짧은 walk가 있다는 뜻이다.
                // 시작점에서 도달 가능한 간선만 처리했으므로 정답에 영향을 주는 음수 사이클임을 판정한다.
                if (iteration == city_count) {
                    has_reachable_negative_cycle = true;
                }
            }
        }

        // 한 번도 갱신하지 못하면 모든 삼각 부등식이 만족되어 이후 순회도 상태를 바꿀 수 없다.
        // 이 조기 종료는 최악 O(NM)을 개선할 수 있지만 최악 복잡도 자체는 변하지 않는다.
        if (!updated) {
            break;
        }
    }

    if (has_reachable_negative_cycle) {
        // ostream operator<<는 -1과 줄바꿈을 cout 버퍼에 쓰고 ostream&를 반환해 연쇄한다.
        // 반환 참조는 버리며 출력 실패는 기본적으로 예외 대신 상태 비트에 기록된다.
        std::cout << -1 << '\n';
        return 0;
    }

    // 1번은 출력하지 않고 2번부터 N번까지 증가시킨다. 각 반복은 O(1) 조회와 출력 한 번이다.
    for (int city{2}; city <= city_count; ++city) {
        const long long answer{distance[static_cast<std::size_t>(city)]};
        // 조건 연산자는 도달 불가면 -1, 가능하면 answer를 고르고 long long prvalue를 만든다.
        // <<는 그 값과 줄바꿈을 출력하고 ostream& 반환을 버린다. distance와 answer는 바뀌지 않는다.
        std::cout << (answer == infinity ? -1LL : answer) << '\n';
    }

    return 0;
}
