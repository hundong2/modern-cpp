/*
문제 요약 — CSES 1136 Counting Paths

노드가 1부터 n까지 번호 매겨진 트리와 m개의 경로 질의가 주어진다. 트리에서는 두 노드 사이의
단순 경로가 정확히 하나다. 각 노드가 m개 경로 중 몇 개에 포함되는지 모든 노드 순서대로 구한다.

입력
- 첫 줄: 노드 수 n, 경로 수 m
- 다음 n-1줄: 무방향 트리 간선 a b
- 다음 m줄: 세려는 경로의 두 끝점 a b. a와 b가 같을 수도 있다.

출력
- 노드 1..n 각각을 포함하는 경로 수 n개를 공백으로 출력한다.

제약
- 1 <= n,m <= 200000
- 1 <= a,b <= n
- 입력 간선은 연결된 트리를 이룬다.
- 제한: 1초, 512MB

예제
입력:
5 3
1 2
1 3
3 4
3 5
1 3
2 5
1 4

출력:
3 1 3 1 1

핵심은 각 경로를 직접 걷지 않는 것이다. Binary Lifting으로 LCA를 O(log n)에 구하고, 끝점 두 곳에
+1, LCA에 -1, LCA의 부모에 -1을 둔 뒤 자식부터 부모로 한 번 누적한다. 모든 경로를 선형 결합하면
O((n+m) log n) 시간, O(n log n) 공간이다. 재귀 DFS 대신 명시적 vector 스택을 사용해 길이 200000인
일자 트리에서도 호출 스택 한도를 의존하지 않는다.
*/

// <cstddef>는 vector 인덱스·크기 타입 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 std::cin/std::cout, stream 상태와 추출·삽입 연산자를 선언한다.
#include <iostream>
// <vector>는 인접 목록, 조상 표, 차분 배열, 명시적 DFS 스택을 소유하는 std::vector를 선언한다.
#include <vector>

// 구현 가까운 공용 알고리즘 문서: ../algorithm/tree-difference-path-counting.md
// LCA 세부 대표 문서: ../algorithm/binary-lifting-lca.md

int main() {
    // [첫 호출 계약: std::ios_base::sync_with_stdio(false)]
    // (1) 특정 수신 객체 없이 표준 stream 전체의 C stdio 동기화 플래그를 바꾸는 정적 함수다.
    // (2) 선택 시그니처는 static bool ios_base::sync_with_stdio(bool sync = true)다.
    // (3) false는 bool prvalue이며 C/C++ 표준 stream 동기화를 끄라는 허용값이다. 소유권 의미는 없다.
    // (4) 반환형 bool은 호출 전 동기화 설정이며 이 풀이에서는 의도적으로 버린다.
    // (5) 이후 표준 C++ stream이 독립 buffering을 사용할 수 있고 입력 데이터 자체는 변하지 않는다.
    // (6) 이 호출은 입출력 전에 수행해야 이식 가능한 효과를 기대할 수 있다. 표준은 점근 복잡도를
    //     정하지 않으며 C stdio와 순서를 섞으면 결과가 달라질 수 있다. 전역 설정이므로 동시 호출하지 않는다.
    std::ios_base::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie(nullptr)]
    // (1) 수신 std::cin은 프로그램 시작 때 생성된 유효한 std::istream lvalue다.
    // (2) 선택 overload는 basic_ostream<char>* tie(basic_ostream<char>* tied_stream)다.
    // (3) nullptr prvalue는 null 출력 stream 포인터로 변환되며 어떤 객체도 소유하거나 파괴하지 않는다.
    // (4) 반환값은 이전 tied ostream 포인터이며 이 코드는 사용하지 않고 버린다.
    // (5) cin은 더 이상 입력 전에 cout을 자동 flush하지 않고 cout 객체 자체는 그대로다.
    // (6) O(1)·할당 없음이며 표준 예외를 던지지 않는다. 대화형 문제에서는 명시적 flush가 필요하고,
    //     같은 stream의 동시 설정/입출력은 외부 동기화 없이 안전하다고 가정하지 않는다.
    std::cin.tie(nullptr);

    // int는 최소 32비트 범위를 제공하는 기본 정수 타입이고 오늘 최대 200000을 충분히 담는다.
    // {}는 0 값 초기화이며 아직 읽지 않은 상태도 결정적이다.
    int node_count{};
    int path_count{};

    // [첫 호출 계약: std::istream 정수 추출 연산자 체인]
    // (1) 최초 수신 std::cin은 tie가 해제된 유효한 istream lvalue이며 정상 입력 상태여야 한다.
    // (2) basic_istream<char>::operator>>(int&)가 두 번 선택된다.
    // (3) node_count와 path_count는 수정 가능한 int lvalue이며 유효 범위 값이 입력된다는 문제 전제가 있다.
    // (4) 각 호출은 같은 istream&를 반환해 다음 >>의 수신으로 쓰고 마지막 반환 참조는 버린다.
    // (5) 성공하면 두 변수와 입력 위치가 갱신된다. 실패하면 failbit/eofbit가 설정되고 대상 값은
    //     표준 변환 규칙에 따른 상태가 되지만, 온라인 저지 입력은 항상 유효하다고 보장한다.
    // (6) 소비 문자 수에 비례하며 locale 처리 예외나 exceptions mask에 따른 ios_base::failure가 가능하다.
    //     한 stream을 여러 스레드에서 논리 레코드 단위로 읽는 동기화는 제공하지 않는다.
    std::cin >> node_count >> path_count;

    // [첫 생성 계약: std::vector<std::vector<int>>(count)]
    // (1) 수신 graph와 node_count+1개의 내부 vector는 아직 생성 전이다.
    // (2) 선택 overload는 vector(size_type count) 생성자이며 바깥 value_type=vector<int>, 기본 allocator를 쓴다.
    // (3) count는 node_count+1 int prvalue가 size_type으로 변환된 값이고 1-based 인덱스의 0번을 비운다.
    // (4) 생성자는 반환값 없이 count개의 빈 내부 vector를 소유하는 graph를 완성한다.
    // (5) graph.size()==node_count+1이고 모든 인접 목록은 비어 있다.
    // (6) O(node_count), 바깥 저장소 할당/bad_alloc·length_error 가능, 실패 시 부분 객체를 정리한다.
    //     외부 iterator/reference는 아직 없으며 생성 중 객체를 다른 스레드와 공유하지 않는다.
    std::vector<std::vector<int>> graph(static_cast<std::size_t>(node_count) + 1U);

    for (int edge_index{}; edge_index < node_count - 1; ++edge_index) {
        int first{};
        int second{};
        // 앞에서 설명한 int 추출 계약을 그대로 적용한다. 각 간선 끝점은 [1,node_count]다.
        std::cin >> first >> second;

        // [첫 호출 계약: vector::operator[]와 push_back(const int&)]
        // (1) graph는 node_count+1개 내부 vector를 가진 lvalue이고, graph[first/second]는 유효한 내부 vector다.
        // (2) 바깥/안쪽 operator[](size_type)와 내부 vector의 void push_back(const value_type&)가 선택된다.
        // (3) first/second는 int lvalue에서 값을 읽어 유효 인덱스로 변환된다. push 인자는 반대 끝점 int lvalue며 복사된다.
        // (4) operator[]는 내부 vector&를 반환해 push 수신으로 사용하고, push_back은 void라 반환값이 없다.
        // (5) 성공하면 양쪽 인접 목록 size가 각각 1 증가해 무방향 간선을 두 방향으로 저장한다.
        // (6) operator[]는 O(1)이며 범위 검사 없이 잘못된 인덱스면 UB다. push는 amortized O(1), 재할당 시
        //     그 내부 vector의 iterator/reference를 모두 무효화하고 bad_alloc이 가능하다. 같은 목록 동시 수정은 안전하지 않다.
        graph[static_cast<std::size_t>(first)].push_back(second);
        graph[static_cast<std::size_t>(second)].push_back(first);
    }

    // [첫 생성 계약: std::vector<int>(count,value), vector<int>()]
    // (1) parent/depth는 생성 전이고 order/stack도 생성 전이다.
    // (2) parent/depth에는 vector(size_type,const int&) fill 생성자, order/stack에는 vector()가 선택된다.
    // (3) count=node_count+1은 유효한 size_type, value=0은 int prvalue에서 const int&에 바인딩된다.
    //     기본 생성 두 개에는 명시 인자가 없다.
    // (4) 생성자는 반환값 없이 두 0 배열과 두 빈 vector를 완성한다.
    // (5) parent/depth의 모든 값은 0, order/stack의 size는 0이다.
    // (6) fill 생성은 O(n)·할당 가능, 기본 생성은 O(1)·일반 allocator에서 무할당이다. bad_alloc/
    //     length_error 가능, 실패 시 부분 원소를 정리하며 서로 다른 vector의 수명은 독립적이다.
    std::vector<int> parent(static_cast<std::size_t>(node_count) + 1U, 0);
    std::vector<int> depth(static_cast<std::size_t>(node_count) + 1U, 0);
    std::vector<int> order{};
    std::vector<int> stack{};

    // [첫 호출 계약: vector::reserve(count)]
    // (1) 수신 order와 stack은 유효한 빈 vector<int> lvalue다.
    // (2) 각각 void reserve(size_type new_capacity)가 선택된다.
    // (3) node_count는 비음수 int에서 size_type 값으로 변환되고 최대 200000이다. 원소 소유권은 없다.
    // (4) 반환형 void라 결과값은 없고 용량 효과만 사용한다.
    // (5) 성공하면 size는 0인 채 capacity가 적어도 node_count가 되어 이후 push 재할당을 막는다.
    // (6) O(현재 size)=O(0), 할당 가능, length_error/bad_alloc 가능하다. 재할당이면 기존 관찰자가
    //     무효지만 아직 없다. 같은 vector 동시 접근에는 외부 동기화가 필요하다.
    order.reserve(static_cast<std::size_t>(node_count));
    stack.reserve(static_cast<std::size_t>(node_count));

    constexpr int root{1};
    parent[static_cast<std::size_t>(root)] = root;
    // 앞에서 설명한 push_back 계약을 적용한다. reserve 덕분에 DFS 동안 stack 저장소는 재할당하지 않는다.
    stack.push_back(root);

    // [첫 호출 계약: vector::empty(), back(), pop_back()]
    // (1) 수신 stack은 root 하나를 가진 유효한 vector<int> lvalue다.
    // (2) bool empty() const noexcept, reference back(), void pop_back()가 차례로 선택된다.
    // (3) 세 호출 모두 명시 인자가 없다. back/pop 전에 !empty()가 참이라는 전제조건을 루프가 보장한다.
    // (4) empty는 bool로 조건에 사용, back은 마지막 int&를 반환해 int 값으로 복사, pop_back은 void다.
    // (5) pop 뒤 size가 1 감소하고 제거 원소 수명은 끝난다. 나머지 원소와 capacity는 유지된다.
    // (6) 모두 O(1), 할당 없음이다. 빈 vector의 back/pop은 UB다. pop은 제거 원소 및 과거 end를
    //     무효화하고 예외를 던지지 않는다. 같은 vector의 동시 읽기/수정은 데이터 경쟁이다.
    while (!stack.empty()) {
        const int vertex{stack.back()};
        stack.pop_back();
        order.push_back(vertex);

        // [첫 숨은 호출 계약: vector<int> range-for]
        // (1) 수신 graph[vertex]는 완성된 non-const vector<int> lvalue이고 루프 동안 구조 변경되지 않는다.
        // (2) begin/end, iterator 비교, 역참조, 전위 증가가 선택된다.
        // (3) 명시 인자는 없고 neighbor는 역참조 int lvalue 값을 복사한다. 참조/포인터 수명을 만들지 않는다.
        // (4) iterator·bool·int&가 순회에 사용되고 neighbor에는 int 값이 복사되며 ++ 반환 참조는 버린다.
        // (5) 반복자만 전진하고 graph와 원소는 그대로다.
        // (6) 목록 길이에 선형, 무할당이다. 순회 중 목록 수정/owner 파괴 시 iterator가 무효이고,
        //     end 역참조는 UB다. 읽기 전용 동시 순회는 가능하지만 동시 수정은 데이터 경쟁이다.
        for (const int neighbor : graph[static_cast<std::size_t>(vertex)]) {
            // 입력이 트리이므로 부모 간선만 건너뛰면 나머지 이웃은 아직 발견되지 않은 자식이다.
            if (neighbor == parent[static_cast<std::size_t>(vertex)]) {
                continue;
            }
            parent[static_cast<std::size_t>(neighbor)] = vertex;
            depth[static_cast<std::size_t>(neighbor)] = depth[static_cast<std::size_t>(vertex)] + 1;
            stack.push_back(neighbor);
        }
    }

    // 2^18=262144이므로 level 0..18이면 최대 깊이 199999를 올리기에 충분하다.
    constexpr int max_log{19};
    // [첫 생성 계약: 중첩 std::vector 조상 표 fill 생성]
    // (1) 수신 up은 아직 생성 전이고 내부 vector도 아직 없다.
    // (2) 안쪽 vector<int>(node_count+1,0)을 만든 뒤 바깥 vector(count=max_log, value=안쪽 vector)의
    //     fill 생성자가 그 값을 max_log번 복사한다.
    // (3) 두 count는 양수 size_type 허용값이고 0은 int 값이다. 외부 저장소를 빌리지 않는다.
    // (4) 반환값 없이 max_log x (node_count+1) 정수 표를 소유한다.
    // (5) 모든 칸은 0이며 각 내부 vector는 서로 독립 저장소를 가진다.
    // (6) O(n log n), 여러 할당과 bad_alloc/length_error 가능, 실패 시 생성된 내부 vector를 정리한다.
    std::vector<std::vector<int>> up(
        static_cast<std::size_t>(max_log),
        std::vector<int>(static_cast<std::size_t>(node_count) + 1U, 0));

    for (int vertex{1}; vertex <= node_count; ++vertex) {
        up[0U][static_cast<std::size_t>(vertex)] = parent[static_cast<std::size_t>(vertex)];
    }
    for (int level{1}; level < max_log; ++level) {
        for (int vertex{1}; vertex <= node_count; ++vertex) {
            const int halfway{
                up[static_cast<std::size_t>(level - 1)][static_cast<std::size_t>(vertex)]};
            up[static_cast<std::size_t>(level)][static_cast<std::size_t>(vertex)] =
                up[static_cast<std::size_t>(level - 1)][static_cast<std::size_t>(halfway)];
        }
    }

    // lambda는 depth/up을 const reference로 캡처한다. 참조는 소유권과 수명을 늘리지 않지만 두 vector가
    // lambda 호출보다 오래 산다. 캡처 대상은 수정하지 않는다.
    const auto lowest_common_ancestor = [&depth, &up](int first, int second) {
        // 긴 쪽을 first로 맞춘다. 기본 int 세 개의 대입이며 표준 라이브러리 호출은 아니다.
        if (depth[static_cast<std::size_t>(first)] < depth[static_cast<std::size_t>(second)]) {
            const int temporary{first};
            first = second;
            second = temporary;
        }

        int difference{
            depth[static_cast<std::size_t>(first)] - depth[static_cast<std::size_t>(second)]};
        for (int level{}; level < max_log; ++level) {
            if ((difference & (1 << level)) != 0) {
                first = up[static_cast<std::size_t>(level)][static_cast<std::size_t>(first)];
            }
        }
        if (first == second) {
            return first;
        }

        // 큰 2^level 점프부터 두 조상이 다른 경우만 함께 올리면 마지막에 LCA 바로 아래에서 멈춘다.
        for (int level{max_log - 1}; level >= 0; --level) {
            const int first_ancestor{
                up[static_cast<std::size_t>(level)][static_cast<std::size_t>(first)]};
            const int second_ancestor{
                up[static_cast<std::size_t>(level)][static_cast<std::size_t>(second)]};
            if (first_ancestor != second_ancestor) {
                first = first_ancestor;
                second = second_ancestor;
            }
        }
        return up[0U][static_cast<std::size_t>(first)];
    };

    // long long는 한 노드의 최대 답 m뿐 아니라 차분 누적의 여유 범위까지 제공한다.
    std::vector<long long> path_difference(
        static_cast<std::size_t>(node_count) + 1U, 0LL);

    for (int query_index{}; query_index < path_count; ++query_index) {
        int first{};
        int second{};
        std::cin >> first >> second;
        const int ancestor{lowest_common_ancestor(first, second)};

        // 노드 경로 차분 불변식:
        // endpoint 두 곳 +1, LCA -1, LCA의 부모 -1이다. 간선 경로 공식의 LCA -2와 혼동하지 않는다.
        ++path_difference[static_cast<std::size_t>(first)];
        ++path_difference[static_cast<std::size_t>(second)];
        --path_difference[static_cast<std::size_t>(ancestor)];
        if (ancestor != root) {
            --path_difference[static_cast<std::size_t>(parent[static_cast<std::size_t>(ancestor)])];
        }
    }

    // order에는 부모가 자식보다 먼저 들어갔다. 역순은 모든 자식 값을 부모보다 먼저 확정한다.
    // index>0으로 root를 제외하면 root의 parent=root에 자기 값을 다시 더하는 실수를 막는다.
    for (int index{node_count - 1}; index > 0; --index) {
        const int vertex{order[static_cast<std::size_t>(index)]};
        const int ancestor{parent[static_cast<std::size_t>(vertex)]};
        path_difference[static_cast<std::size_t>(ancestor)] +=
            path_difference[static_cast<std::size_t>(vertex)];
    }

    for (int vertex{1}; vertex <= node_count; ++vertex) {
        if (vertex > 1) {
            // [첫 호출 계약: std::ostream의 char 삽입]
            // (1) 수신 std::cout은 지금까지 정상 출력 상태인 ostream lvalue다.
            // (2) 비멤버 operator<<(basic_ostream<char>&, char)가 선택된다.
            // (3) ' '은 char prvalue이며 소유권이나 수명 의존성이 없다.
            // (4) ostream&를 반환하지만 이번 단일 호출의 반환 참조는 버린다.
            // (5) 공백 한 문자가 기록되고 실패하면 상태 비트가 바뀔 수 있다.
            // (6) O(1) 문자 기록이며 내부 buffering/할당은 구현에 달린다. 예외 mask에 따라 실패 예외가
            //     가능하고 여러 스레드의 논리 출력 원자성은 보장하지 않는다.
            std::cout << ' ';
        }

        // [첫 호출 계약: std::ostream의 long long 삽입]
        // (1) 수신 std::cout은 유효한 ostream lvalue이고 앞 공백까지 기록된 상태일 수 있다.
        // (2) basic_ostream<char>::operator<<(long long)가 선택된다.
        // (3) vector 원소 long long lvalue는 lvalue-to-rvalue 변환으로 읽히며 소유권 이동이 없다.
        // (4) 반환형 ostream&는 이 호출 뒤 사용하지 않고 버린다.
        // (5) 십진 표현이 기록되고 path_difference와 graph는 변하지 않는다.
        // (6) 숫자 자리 수에 비례하며 locale/streambuf 오류, 상태 비트, exceptions mask에 따른
        //     ios_base::failure가 가능하다. 같은 stream의 동시 레코드 출력에는 외부 동기화가 필요하다.
        std::cout << path_difference[static_cast<std::size_t>(vertex)];
    }
    // 위 char 삽입 계약과 같으며 마지막 개행으로 line-buffer 환경의 flush 기회를 만든다.
    std::cout << '\n';

    // 기계 실행 관점에서 전처리는 연속 배열 load/store, 깊이 비교, 비트 검사, 조건 분기와 간접 주소
    // 계산으로 나타날 수 있다. lambda는 구체 타입이라 소스 수준의 virtual 호출은 없다. 다만 실제 인라인,
    // 명령 수, cache miss, 분기 예측, vector 할당 배치는 CPU·ABI·표준 라이브러리·컴파일러·최적화 옵션에
    // 따라 달라지므로 특정 어셈블리나 실행 횟수를 단정하지 않는다.
}
