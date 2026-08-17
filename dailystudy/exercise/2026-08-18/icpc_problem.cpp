/*
문제 ID/제목: BOJ 11438 - LCA 2
출처: https://www.acmicpc.net/problem/11438

문제 요약:
1번부터 N번까지 번호가 붙은 정점 N개가 N-1개의 무방향 간선으로 연결되어 트리를 이룬다.
트리의 루트는 1번이다. 두 정점이 주어질 때 두 정점에서 루트로 올라가는 경로가 처음 만나는
가장 깊은 정점, 즉 최소 공통 조상(LCA)을 여러 질의에 대해 구한다.

입력:
첫 줄에 정점 수 N이 주어진다. 다음 N-1줄에는 간선의 양 끝 정점 u, v가 주어진다.
그다음 줄에 질의 수 M이 주어지고, 이어지는 M줄에는 LCA를 구할 두 정점 a, b가 주어진다.

출력:
각 질의마다 a와 b의 최소 공통 조상 번호를 한 줄에 하나씩 출력한다.

제약:
2 <= N <= 100,000, 1 <= M <= 100,000이다. 단순히 부모를 한 칸씩 따라가면 최악 O(NM)이므로,
2^k번째 조상을 전처리하는 이진 리프팅으로 전처리 O(N log N), 질의 O(log N), 공간 O(N log N)에 푼다.

설명용 예제:
입력
5
1 2
1 3
2 4
2 5
3
4 5
4 3
2 4

출력
2
1
2
*/

// <iostream>은 온라인 저지 입력 std::cin, 출력 std::cout과 스트림 연산을 제공한다.
#include <iostream>
// <queue>는 루트에서 깊이와 직계 부모를 정하는 FIFO std::queue를 제공한다.
#include <queue>
// <vector>는 인접 리스트, 깊이 배열, 2^k 조상 표를 동적으로 소유한다.
#include <vector>

// 알고리즘 문서: ../algorithm/binary-lifting-lca.md
// 두 정점의 깊이를 맞춘 뒤 큰 2의 거듭제곱부터 동시에 올려 최소 공통 조상을 찾는다.
[[nodiscard]] int lowest_common_ancestor(
    int first,
    int second,
    const std::vector<int>& depth,
    const std::vector<std::vector<int>>& ancestor,
    int maximum_log) {
    // first와 second는 값 매개변수라 호출자의 정점 번호는 바뀌지 않는다.
    // depth의 정확한 타입은 const vector<int>&다. operator[](size_type)는 first/second int를 부호 없는
    // 인덱스로 변환해 const int&를 O(1)에 반환하고 vector는 바꾸지 않는다. 인덱스가 [0,size) 밖이면
    // 미정의 동작이며, 여기서는 문제의 정점 범위와 N+1 크기가 전제조건을 보장한다. 재할당이 없으므로 참조도 유지된다.
    if (depth[first] < depth[second]) {
        const int temporary{first}; // 임시 int 값에 첫 정점 번호를 복사한다.
        first = second;             // 더 깊은 두 번째 정점을 first에 저장한다.
        second = temporary;         // 원래 첫 정점을 second로 옮겨 논리적으로 교환한다.
    }

    // 큰 점프부터 검사해 first를 second와 같은 깊이까지 O(log N)에 올린다.
    for (int level{maximum_log - 1}; level >= 0; --level) {
        // 1 << level은 2^level이고, 점프해도 second보다 얕아지지 않을 때만 적용한다.
        if (depth[first] - (1 << level) >= depth[second]) {
            first = ancestor[level][first]; // 전처리한 2^level번째 조상으로 이동한다.
        }
    }

    // 깊이를 맞춘 직후 같은 정점이면 그 정점이 곧 최소 공통 조상이다.
    if (first == second) {
        return first;
    }

    // 알고리즘 문서: ../algorithm/binary-lifting-lca.md
    // 두 조상이 서로 다른 가장 큰 level만큼 함께 올리면 LCA 바로 아래 관계를 유지한다.
    for (int level{maximum_log - 1}; level >= 0; --level) {
        if (ancestor[level][first] != ancestor[level][second]) {
            first = ancestor[level][first];
            second = ancestor[level][second]; // 두 정점의 깊이는 항상 같다는 불변식을 보존한다.
        }
    }

    // 이제 두 정점은 서로 다르지만 직계 부모가 같으므로 그 부모가 LCA다.
    return ancestor[0][first];
}

int main() {
    // std::ios::sync_with_stdio(false)는 이전 동기화 bool을 반환하지만 버린다. C와 C++ 스트림 동기화를 끄며
    // 이후 C stdio와 순서를 섞지 않는다는 전제에서 입출력 오버헤드를 줄인다. 전역 스트림 상태가 바뀐다.
    std::ios::sync_with_stdio(false);
    // std::cin의 정확한 타입은 std::istream이고 tie(nullptr)는 ostream* 입력 하나를 받는다.
    // 이전 연결 ostream*를 반환하지만 버리며, 호출 뒤 입력 전 자동 std::cout flush가 해제된다.
    std::cin.tie(nullptr);

    int vertex_count{}; // int를 중괄호로 0 초기화한다.
    // operator>>(istream&, int&)는 vertex_count lvalue에 값을 저장하고 istream&를 반환한다.
    // bool 문맥에서 입력 성공을 검사하며, 실패하면 변수는 유효하고 스트림 실패 상태가 유지된다.
    if (!(std::cin >> vertex_count)) {
        return 0;
    }

    // vector<vector<int>>(size_type count)는 unsigned int 식을 size_type으로 변환해 vertex_count+1개의
    // 빈 내부 vector를 O(N)에 값 초기화하며 생성된 바깥 vector prvalue가 graph를 직접 초기화한다.
    // 바깥 vector가 모든 내부 vector와 인접 원소 저장소를 소유하며 할당 실패 시 bad_alloc이 발생한다.
    std::vector<std::vector<int>> graph(static_cast<unsigned int>(vertex_count + 1));

    // 트리는 N-1개의 간선을 가지며 index는 지금까지 읽은 간선 수다.
    for (int index{}; index < vertex_count - 1; ++index) {
        int from{};
        int to{}; // 각 간선 양 끝 정점을 0 초기화한다.
        // 두 operator>> 호출은 from, to 참조에 차례로 저장하고 같은 istream& 반환값으로 연쇄된다.
        std::cin >> from >> to;
        // graph[from]은 vector<int>&다. push_back(const int&)는 to lvalue를 복사해 끝에 추가하고 void를 반환한다.
        // 성공 뒤 크기가 하나 늘며 상각 O(1), 재할당 시 해당 내부 vector의 포인터·참조·반복자가 무효화된다.
        // bad_alloc이면 원래 vector 상태를 유지하며 graph와 from/to의 값·수명은 바뀌지 않는다.
        graph[from].push_back(to);
        // 무방향 간선이므로 반대 방향도 같은 계약으로 추가한다.
        graph[to].push_back(from);
    }

    int maximum_log{1}; // 최소 한 개의 조상 행(2^0)을 사용한다.
    // 2^maximum_log가 N을 초과할 때까지 증가해 필요한 이진 점프 행 수를 계산한다.
    while ((1 << maximum_log) <= vertex_count) {
        ++maximum_log;
    }

    // vector<int>(count, -1)은 N+1개 깊이를 -1로 복사 초기화해 미방문 표시로 삼는다.
    std::vector<int> depth(static_cast<unsigned int>(vertex_count + 1), -1);
    // 바깥 vector는 maximum_log개 행을 만들고, 각 행은 N+1개의 int 0을 소유한다.
    std::vector<std::vector<int>> ancestor(
        static_cast<unsigned int>(maximum_log),
        std::vector<int>(static_cast<unsigned int>(vertex_count + 1), 0));
    // queue<int> 기본 생성자는 인자 없이 빈 기저 deque를 만들어 소유하고 FIFO로 다음 탐색 정점을 관리한다.
    // 반환값은 없고 통상 상수 시간이며 할당 여부는 구현의 기저 컨테이너에 달려 있다.
    std::queue<int> pending{};

    depth[1] = 0; // 루트 1의 깊이는 0이다.
    // queue<int>::push(const int&)는 값 1을 뒤에 복사하고 void를 반환한다.
    // 호출 전 queue는 비어 있고 호출 뒤 크기는 1이다. 통상 O(1), 할당 실패 시 예외가 날 수 있으며
    // queue는 반복자를 노출하지 않고 기존 원소 참조의 보장은 기저 컨테이너 규칙을 따른다.
    pending.push(1);

    // empty()는 인자 없이 queue를 읽어 bool을 O(1)에 반환하고 queue 상태는 바꾸지 않는다.
    while (!pending.empty()) {
        // front()는 인자 없이 맨 앞 int&를 O(1)에 반환한다. 비어 있지 않음은 while 조건이 보장한다.
        // current에 값을 복사하므로 곧 pop해도 안전하며 반환 참조 자체는 저장하지 않는다.
        const int current{pending.front()};
        // pop()은 인자와 반환값 없이 맨 앞 원소를 제거·파괴한다. 비어 있지 않아 정의되며 통상 O(1)이다.
        // current 복사본과 graph는 유지되고 제거된 원소를 가리키던 참조는 무효가 된다.
        pending.pop();

        // 범위 for는 graph[current]의 begin/end 반복자를 얻어 인접 정수를 값으로 복사하며 총 O(N)회 순회한다.
        // BFS 중 graph는 바꾸지 않으므로 반복자는 유효하고 빈 인접 리스트면 본문을 실행하지 않는다.
        for (const int next : graph[current]) {
            if (depth[next] != -1) {
                continue; // 이미 부모가 정해진 정점은 다시 방문하지 않는다.
            }
            depth[next] = depth[current] + 1; // 트리 간선 하나만큼 깊이를 늘린다.
            ancestor[0][next] = current;      // 2^0=1번째 조상은 BFS에서 온 부모다.
            pending.push(next);               // 아직 처리하지 않은 자식 정점을 FIFO 뒤에 복사한다.
        }
    }

    // 점화식 up[k][v] = up[k-1][ up[k-1][v] ]로 모든 2^k번째 조상을 계산한다.
    for (int level{1}; level < maximum_log; ++level) {
        for (int vertex{1}; vertex <= vertex_count; ++vertex) {
            const int halfway{ancestor[level - 1][vertex]};
            ancestor[level][vertex] = ancestor[level - 1][halfway];
        }
    }

    int query_count{};
    std::cin >> query_count; // 질의 수를 int lvalue에 저장하고 반환 스트림 참조는 버린다.
    for (int query{}; query < query_count; ++query) {
        int first{};
        int second{};
        std::cin >> first >> second; // 이번 질의의 두 정점 번호를 입력 순서대로 저장한다.
        const int answer{lowest_common_ancestor(first, second, depth, ancestor, maximum_log)};
        // operator<<(ostream&, int)와 문자 오버로드가 answer와 개행을 출력하고 ostream&를 연쇄 반환한다.
        // 반환 참조는 최종적으로 버리고 입력 값과 조상 표는 유지된다. I/O 실패는 스트림 상태에 기록된다.
        std::cout << answer << '\n';
    }

    // 배열 접근은 메모리 load/store와 비교·분기를 만들 수 있지만 구체 기계 명령은 CPU, ABI,
    // 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.
    return 0;
}
