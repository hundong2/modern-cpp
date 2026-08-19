/*
문제 ID/제목: BOJ 6086 - 최대 유량
출처: https://www.acmicpc.net/problem/6086 (Baekjoon Online Judge)

문제 요약:
알파벳 대문자와 소문자로 이름 붙인 연결 지점 사이에 양방향 배수관이 있다. 한 경로로 직렬 연결된 관은 가장 작은
용량만큼만 흘릴 수 있고, 같은 두 지점을 잇는 병렬 관의 용량은 합쳐진다. 우물 A에서 외양간 Z로 동시에 보낼 수 있는
물의 최대량을 구한다. 대문자와 소문자는 서로 다른 정점이며, 같은 두 정점 사이의 파이프가 여러 번 주어질 수 있다.

입력:
첫 줄에 파이프 수 N이 주어진다. 다음 N줄에는 서로 다른 두 알파벳 문자 a, b와 양의 정수 용량 c가 주어진다.
각 파이프는 a에서 b와 b에서 a 양쪽으로 흐를 수 있으므로 두 방향 용량에 c를 더해야 한다.

출력:
A에서 Z로 보낼 수 있는 최대 유량을 정수 하나로 출력한다.

제약:
1 <= N <= 700, 1 <= c <= 1,000이다. 정점은 A-Z와 a-z의 52개뿐이다. 최대 합계는 int 범위 안이지만,
평행 간선을 덮어쓰지 않고 반드시 누적해야 한다. Edmonds-Karp의 시간 복잡도는 O(VE^2), 저장 공간은 O(V^2+E)이다.

예제:
입력
5
A B 3
B C 3
C D 5
D Z 4
B Z 6

출력
3
*/

// <algorithm>은 증가 경로의 병목 용량을 고르는 std::min 함수 템플릿을 선언한다.
#include <algorithm>
// <array>는 52x52 용량·유량 행렬과 부모 배열을 소유하는 고정 크기 std::array를 선언한다.
#include <array>
// <iostream>은 온라인 저지 입력 std::cin, 출력 std::cout과 스트림 연산자를 선언한다.
#include <iostream>
// <limits>는 int의 최댓값을 얻는 std::numeric_limits를 선언한다.
#include <limits>
// <queue>는 BFS의 FIFO 대기열 std::queue와 push/front/pop/empty를 선언한다.
#include <queue>
// <vector>는 각 정점의 실제 이웃만 순회할 동적 연속 배열 std::vector를 선언한다.
#include <vector>

constexpr int kVertexCount{52}; // 대문자 26개와 소문자 26개를 0..51에 대응시킨다.
using Matrix = std::array<std::array<int, kVertexCount>, kVertexCount>; // 고정 크기 2차원 행렬 별칭이다.
using Graph = std::array<std::vector<int>, kVertexCount>; // 정점별 이웃 vector를 가진 인접 목록 별칭이다.

// 문자를 정점 인덱스로 바꾸며 입력 char는 값으로 복사되고 int prvalue가 반환된다.
[[nodiscard]] constexpr int vertex_index(char label) noexcept {
    // &&의 단락 평가로 대문자 범위를 검사한다. 문자 비교와 뺄셈 결과는 int로 승격된다.
    if ('A' <= label && label <= 'Z') {
        return label - 'A'; // A..Z를 0..25로 옮긴다.
    }
    return label - 'a' + 26; // 공식 입력의 소문자 a..z를 26..51로 옮긴다.
}

// 알고리즘 문서: ../algorithm/maximum-flow-edmonds-karp.md
// capacity와 graph는 const lvalue 참조로 빌리므로 복사·수정하지 않고, 결과 최대 유량 int를 값으로 반환한다.
[[nodiscard]] int maximum_flow(const Matrix& capacity, const Graph& graph, int source, int sink) {
    Matrix flow{}; // 값 초기화로 모든 현재 유량을 0으로 만든다. 함수가 행렬 메모리를 소유한다.
    int total{};   // 지금까지 보낸 유량이며 매 반복 뒤 실행 가능한 흐름의 값이라는 불변식을 유지한다.

    // 증가 경로가 없을 때 함수 안에서 return하므로 의도적인 무한 반복을 사용한다.
    while (true) {
        std::array<int, kVertexCount> parent{}; // BFS 트리의 직전 정점을 소유하며 우선 0으로 값 초기화한다.
        // 범위 for는 parent의 begin/end 반복자를 얻어 각 int를 lvalue 참조 value로 빌린다.
        // 모든 원소를 -1로 저장하고 크기·수명은 유지한다. O(V), 할당·예외·무효화가 없다.
        for (int& value : parent) {
            value = -1;
        }
        // array<int,52>::operator[](size_type) -> int&는 source 정수값을 부호 없는 인덱스로 바꿔 O(1)에 원소 참조를 반환한다.
        // 범위 검사를 하지 않으므로 0<=source<52가 전제다. 저장은 해당 int만 바꾸며 배열 크기·수명·다른 참조는 유지되고 예외·할당은 없다.
        parent[source] = source; // 시작점은 자기 자신을 부모 표식으로 삼아 방문 완료로 표시한다.

        std::queue<int> pending{}; // 기본 생성한 FIFO 어댑터는 비어 있고 내부 컨테이너를 소유한다.
        // queue<int>::push(const int&)는 source lvalue 값을 내부 끝에 복사하고 void를 반환한다.
        // 성공 뒤 크기는 1 증가한다. 기반 컨테이너가 할당에 실패하면 예외가 날 수 있으나 source는 바뀌지 않으며 O(1)이다.
        pending.push(source);

        // queue::empty() const -> bool은 인자 없이 O(1)로 비었는지 관찰하고 큐를 바꾸지 않는다.
        while (!pending.empty() && parent[sink] == -1) {
            // queue::front() -> int&는 비어 있지 않다는 전제에서 첫 원소 참조를 O(1)에 반환한다.
            // 반환값을 current에 복사하므로 다음 pop 뒤에도 값은 안전하다. 큐의 크기·소유권은 아직 유지된다.
            const int current{pending.front()};
            // queue::pop() -> void는 첫 원소를 제거해 크기를 1 줄인다. 제거 원소 참조는 무효화되지만 current 복사본은 유지된다.
            // while 조건이 비어 있지 않음을 보장하며 O(1), 보통 할당 없음이다.
            pending.pop();

            // 바깥 array::operator[]은 유효한 current로 vector<int>&를 O(1)에 반환하고, 그 vector의 begin/end 반복자를 얻어 각 int를 값으로 복사한다.
            // 순회 중 수신 vector를 바꾸지 않아 반복자·참조·크기·용량과 원소 수명이 유지되며, 이 조회·반복자 획득은 할당·예외가 없다.
            for (const int next : graph[current]) {
                if (parent[next] != -1) {
                    continue; // 이미 발견한 정점은 다시 넣지 않아 BFS 트리와 O(E) 순회를 유지한다.
                }
                const int residual{capacity[current][next] - flow[current][next]}; // 더 보낼 수 있는 잔여 용량이다.
                if (residual <= 0) {
                    continue; // 포화 간선은 잔여 그래프에 없으므로 건너뛴다.
                }

                parent[next] = current; // next에 이르는 BFS 최단 증가 경로의 직전 정점을 기록한다.
                // push(int const&)는 next 값을 복사하며 성공 뒤 큐 크기만 증가한다. graph/parent와 next는 바뀌지 않는다.
                pending.push(next);
            }
        }

        if (parent[sink] == -1) {
            return total; // 증가 경로가 없으므로 최대 유량-최소 컷 정리에 따라 현재 실행 가능 유량이 최대다.
        }

        // numeric_limits<int>::max()는 인자와 수신 객체 없이 int 최댓값 prvalue를 constexpr O(1)로 반환한다.
        // 상태·수명·할당·예외는 없고, 첫 residual과 비교할 병목 초기값으로 저장한다.
        int bottleneck{std::numeric_limits<int>::max()};
        // 부모 포인터를 sink에서 source까지 따라가며 경로의 최소 잔여 용량을 구한다.
        for (int vertex{sink}; vertex != source; vertex = parent[vertex]) {
            const int previous{parent[vertex]};
            const int residual{capacity[previous][vertex] - flow[previous][vertex]};
            // std::min<int>(const int&, const int&)는 두 lvalue를 빌려 작은 값의 const 참조를 반환한다.
            // 대입 전에 값을 복사하므로 지역 인자 수명 문제는 없고 O(1), 상태 변화·할당·예외가 없다.
            bottleneck = std::min(bottleneck, residual);
        }

        // 같은 경로에 병목만큼 보내고 역방향 유량을 빼서 나중에 선택을 취소할 수 있게 한다.
        for (int vertex{sink}; vertex != source; vertex = parent[vertex]) {
            const int previous{parent[vertex]};
            flow[previous][vertex] += bottleneck; // 정방향 잔여 용량을 병목만큼 줄이는 효과다.
            flow[vertex][previous] -= bottleneck; // 역방향 잔여 용량을 만들어 이전 선택을 되돌릴 수 있게 한다.
        }
        total += bottleneck; // 양의 병목만 더해 총 유량이 엄격히 증가한다.
    }
}

int main() {
    // sync_with_stdio(false)는 bool false를 받아 C와 C++ 표준 스트림 동기화를 끄고 이전 bool 상태를 반환하지만 여기서는 버린다.
    // 전역 스트림 설정만 바꾸고 입력 데이터는 바꾸지 않는다. 이후 C stdio와 섞을 순서를 보장하지 않는다.
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 ostream* 널 포인터를 받아 이전 연결 포인터를 반환하지만 버리고, 입력 전 cout 자동 flush 연결을 해제한다.
    std::cin.tie(nullptr);

    int pipe_count{}; // int{}로 0 초기화되며 성공한 입력이 실제 N으로 덮어쓴다.
    // operator>>(istream&, int&)는 cin과 pipe_count lvalue를 받아 성공 시 값을 저장하고 입력 위치를 전진시킨 뒤 같은 istream&를 반환한다.
    // if는 스트림 상태를 bool로 검사한다. 실패하면 pipe_count의 신뢰를 중단하고 정상 종료하며, I/O 실패는 상태 비트로 표현된다.
    if (!(std::cin >> pipe_count)) {
        return 0;
    }

    Matrix capacity{}; // 모든 정점 쌍 용량을 0으로 값 초기화하며 고정 배열이라 동적 할당이 없다.
    Graph graph{};     // 52개 빈 vector를 기본 생성하며 각 vector가 이후 이웃 저장소를 소유한다.

    // 정확히 N개 파이프를 읽으므로 O(N)회 반복한다.
    for (int index{}; index < pipe_count; ++index) {
        char from_label{};
        char to_label{};
        int amount{};
        // 첫 operator>>는 std::istream 수신 객체 cin과 char lvalue from_label, 둘째는 반환된 같은 stream과 char lvalue to_label,
        // 셋째는 같은 stream과 int lvalue amount를 받는다. 성공한 각 호출은 대상 값을 바꾸고 입력 위치를 전진시키며 istream&를 반환한다.
        // 최종 반환 참조는 저장하지 않는다. 실패하면 상태 비트를 세우고 뒤 추출도 실패 상태를 유지하며, 입력 대상 소유권·수명은 옮기지 않는다.
        std::cin >> from_label >> to_label >> amount;

        const int from{vertex_index(from_label)};
        const int to{vertex_index(to_label)};
        capacity[from][to] += amount; // 평행 파이프를 덮어쓰지 않고 누적한다.
        capacity[to][from] += amount; // 문제의 파이프가 양방향이므로 반대 방향 용량도 같은 만큼 누적한다.

        // vector<int>::push_back(const int&)는 to lvalue를 graph[from] 끝에 복사하고 void를 반환한다.
        // 성공하면 size가 1 증가한다. 용량 부족 재할당 시 그 vector의 기존 반복자·참조가 무효화되지만 지금 보관한 관찰자는 없다.
        // 상각 O(1), 추가 공간 O(1), 할당 실패 시 bad_alloc과 강한 예외 보장을 제공하며 from/to 값은 유지된다.
        graph[from].push_back(to);
        graph[to].push_back(from); // 같은 계약으로 역방향과 잔여 역간선의 탐색 가능성을 보장한다.
    }

    const int source{vertex_index('A')};
    const int sink{vertex_index('Z')};
    // maximum_flow는 입력 구조를 빌리고 int 값을 반환한다. operator<<는 그 값을 cout에 쓰고 ostream&를 반환하며 최종 참조는 버린다.
    std::cout << maximum_flow(capacity, graph, source, sink) << '\n';

    // 배열 접근은 로드·저장, BFS는 비교·조건 분기·함수 호출을 만들 수 있다. 구체 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return 0;
}
