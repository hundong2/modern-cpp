/*
CSES 2138 — Reachable Nodes (제출 가능한 완전한 풀이)
공식 출처: https://cses.fi/problemset/task/2138/

문제 요약
- 정점 1..n과 m개의 방향 간선으로 이루어진 DAG가 주어진다.
- 각 정점 u마다 u 자신을 포함하여 u에서 도달할 수 있는 서로 다른 정점 수를 출력한다.

입력/출력
- 첫 줄: 정점 수 n, 간선 수 m.
- 다음 m줄: 서로 다른 두 정점 a, b. a에서 b로 가는 간선이다.
- 출력: 1번부터 n번까지 각 정점의 도달 가능 정점 수 n개.

제약
- 1 <= n <= 50,000, 1 <= m <= 100,000.
- 입력 그래프는 방향 비순환 그래프(DAG)다.
- 시간 제한 1초, 메모리 제한 512MB다.

예제
입력:
5 6
1 2
1 3
1 4
2 3
3 5
4 5

출력:
5 3 2 2 1

핵심 아이디어
- `R[u] = {u} union (모든 u->v에 대한 R[v])`라는 집합 점화식을 쓴다.
- DAG의 역위상 순서에서는 모든 후속 정점 v의 R[v]가 먼저 완성된다.
- 집합을 bool 50,000개가 아니라 64비트 word 배열로 압축하면 합집합은 word별 OR가 된다.
- 마지막에 각 word의 1비트 수를 더하면 서로 다른 도달 정점 수다.

자세한 원리와 변형 문서: ../algorithm/dag-transitive-closure-word-bitsets.md
*/

// <bit>는 부호 없는 정수 word의 1비트 수를 세는 std::popcount를 선언한다.
#include <bit>
// <cstddef>는 컨테이너 크기와 인덱스 타입 std::size_t를 제공한다.
#include <cstddef>
// <cstdint>는 정확히 64비트인 부호 없는 word 타입 std::uint64_t를 제공한다.
#include <cstdint>
// <iostream>은 대회 입력/출력 객체 std::cin, std::cout과 stream 연산을 제공한다.
#include <iostream>
// <vector>는 그래프, 위상 순서, 압축 도달 행렬을 소유하는 std::vector를 제공한다.
#include <vector>

// 이 학습/제출 target의 int는 32비트 이상이어야 n<=50,000과 m<=100,000을 표현한다.
// 아래 compile-time 검사는 16비트 int 구현에서 조용히 잘못 빌드되는 일을 막는다.
using Node = int;
static_assert(sizeof(Node) >= 4U, "CSES 2138 requires a 32-bit-or-wider int");
// Word는 정확히 64개 도달 여부 비트를 소유하는 기본 값 타입 별칭이다.
using Word = std::uint64_t;
// 바깥 vector는 정점별 목록을, 안쪽 vector는 그 정점의 outgoing 이웃을 소유한다.
using Graph = std::vector<std::vector<Node>>;

// 한 word의 값 비트 수다. std::uint64_t가 존재하는 구현에서는 정확히 64다.
constexpr std::size_t kWordBits{64U};

// Kahn 위상 정렬을 반복문으로 수행해 재귀 stack 깊이 위험을 없앤다.
// graph는 const reference로 빌리고 indegree는 더 이상 필요 없으므로 non-const reference로 감소시킨다.
[[nodiscard]] std::vector<Node> make_topological_order(
    const Graph& graph,
    std::vector<Node>& indegree) {
    // [첫 호출 계약: vector::size]
    // (1) 수신 graph는 n개의 살아 있는 내부 vector를 소유하는 유효한 const Graph lvalue다.
    // (2) 선택 오버로드는 `size_type vector::size() const noexcept`이며 데이터 인자는 없다.
    // (3) 명시적 매개변수는 없고 수신 객체의 소유권도 바뀌지 않는다.
    // (4) 반환형 Graph::size_type 값은 정점 수다. node_count 초기화에 사용한다.
    // (5) 호출 뒤 graph의 크기·용량·원소·수명은 모두 그대로다.
    // (6) O(1), 무할당·noexcept·무효화 없음이다. 같은 vector를 다른 스레드가 동시에
    //     변경하지 않아야 하며 오늘 프로그램은 단일 스레드다.
    const std::size_t node_count{graph.size()};

    // [첫 생성 계약: std::vector<Node> 기본 생성자]
    // (1) ready와 order는 아직 수명이 시작되지 않은 std::vector<Node> 지역 객체다.
    // (2) 선택 시그니처는 `vector() noexcept(noexcept(Allocator()))`와 기본 allocator다.
    // (3) 데이터 인자는 없고 `{}`는 빈 직접 목록 초기화다. 외부 저장소를 빌리지 않는다.
    // (4) 생성자는 반환값이 없으며 두 빈 owner 객체의 수명을 시작한다.
    // (5) 성공 뒤 size 0이고 구현이 별도 저장소를 갖지 않을 수 있다.
    // (6) O(1), 일반 기본 allocator에서 무할당·noexcept다. 원소/관찰자가 없어 무효화도 없다.
    //     allocator 생성 오류 가능성은 해당 allocator 계약을 따르고 객체 자체는 thread-safe가 아니다.
    std::vector<Node> ready{};
    std::vector<Node> order{};

    // [첫 호출 계약: vector::reserve]
    // (1) 수신 ready와 order는 size 0인 유효한 std::vector<Node> lvalue다.
    // (2) 두 호출 모두 `void reserve(size_type new_cap)` overload를 선택한다.
    // (3) 인자 node_count는 n인 size_type lvalue에서 복사한 값이고 소유권 의미가 없다.
    // (4) 반환형 void라 결과값은 없다. 효과만 사용한다.
    // (5) 성공하면 capacity>=n이고 size는 0이다. 재할당이 있었다면 과거 관찰자는 모두
    //     무효지만 현재는 관찰자가 없다. 이후 n개까지 push_back은 재할당하지 않는다.
    // (6) 표준 복잡도는 현재 size에 최대 선형이다. 여기서는 size 0이라 원소 이동은 없지만
    //     allocator 저장소 요청 비용은 별도다. capacity 한계면 length_error, 실패면 bad_alloc
    //     가능이고 강한 예외 보장을 가진다. 같은 vector 병행 접근은 동기화하지 않는다.
    ready.reserve(node_count);
    order.reserve(node_count);

    // int 정점 번호와 size_t 저장 인덱스를 구분한다. 공식 범위라 명시 변환이 안전하다.
    for (std::size_t node_index{}; node_index < node_count; ++node_index) {
        // [첫 호출 계약: vector::operator[]]
        // (1) 수신 indegree는 정확히 node_count개의 Node를 소유하는 non-const vector이고
        //     node_index는 현재 [0,node_count) 범위다.
        // (2) 선택 오버로드는 `Node& vector<Node>::operator[](size_type)`다.
        // (3) 인자 node_index는 범위 안 size_type lvalue의 값이며 소유권을 옮기지 않는다.
        // (4) 반환형 Node&를 0과 비교해 읽는다. reference를 반복 밖에 저장하지 않는다.
        // (5) 호출만으로 indegree 값·크기·용량은 바뀌지 않는다.
        // (6) O(1), 무할당·무효화·예외 없음이나 범위 검사도 없어 벗어나면 UB다.
        //     loop 불변식이 범위를 보장하고 같은 원소에 병행 쓰기는 없다.
        if (indegree[node_index] == Node{0}) {
            // [첫 호출 계약: vector::push_back]
            // (1) 수신 ready는 size<=n이고 capacity>=n인 유효한 vector<Node>이며 삽입 위치가 있다.
            // (2) 인자가 Node prvalue이므로 `void push_back(Node&&)` overload를 선택한다.
            // (3) static_cast 결과는 [0,n)의 Node prvalue이고 기본 타입 값만 복사하며 소유권이 없다.
            // (4) 반환형 void라 반환값은 사용하지 않는다.
            // (5) 성공하면 size가 1 늘고 끝 원소가 node 번호다. 미리 reserve해 재할당하지 않으므로
            //     기존 원소 pointer/reference/iterator는 유지되고 과거 end만 바뀐다.
            // (6) 상각 O(1), 여기서는 저장소 준비 뒤 O(1)이다. Node 이동은 noexcept이고 추가
            //     할당/예외가 없다. capacity 전제가 깨지면 재할당·bad_alloc 및 전체 관찰자 무효화가 가능하다.
            ready.push_back(static_cast<Node>(node_index));
        }
    }

    // vector를 FIFO로 사용한다. head는 이미 처리한 prefix 길이이고 erase가 없어 O(n)이다.
    // [첫 호출 계약: std::vector<Node>::size() — ready]
    // (1) 수신 ready는 0개 이상의 대기 정점을 소유하는 non-const vector<Node> lvalue다.
    // (2) `size_type vector<Node>::size() const noexcept`를 선택하며 명시적 인자는 없다.
    // (3) 인자·소유권 이전이 없고 const 멤버라 non-const 수신 객체도 읽기만 한다.
    // (4) 반환 size_type 값은 현재 FIFO 길이이며 매 반복의 head와 비교해 즉시 사용한다.
    // (5) 호출 뒤 원소·크기·용량·관찰자 수명은 그대로다.
    // (6) O(1), 무할당·noexcept·무효화 없음이다. loop 본문 push_back 뒤 다음 조건에서는
    //     새 크기를 읽으며, 단일 스레드라 같은 vector와의 데이터 경쟁이 없다.
    for (std::size_t head{}; head < ready.size(); ++head) {
        const Node current{ready[head]};

        // [첫 호출 계약: std::vector<Node>::push_back(const Node&) — order]
        // (1) 수신 order는 size<n, capacity>=n인 유효한 vector<Node> lvalue이고 current는
        //     현재 ready 원소에서 복사한 살아 있는 const Node lvalue다.
        // (2) lvalue 인자이므로 `void vector<Node>::push_back(const Node&)`를 선택한다.
        // (3) current 값은 [0,n)이며 복사 입력이다. 기본 타입이라 소유권을 옮기지 않는다.
        // (4) 반환형 void라 효과만 사용한다.
        // (5) 끝에 같은 정점 값이 추가되고 size가 1 증가한다. reserve 덕분에 재할당이 없다.
        // (6) 여기서는 O(1), 무할당·무예외이며 기존 원소 관찰자는 유지되고 과거 end만
        //     바뀐다. reserve 전제가 없으면 상각 O(1), bad_alloc과 전체 관찰자 무효화가 가능하다.
        order.push_back(current);

        // [첫 호출 계약: const Graph::operator[]]
        // (1) 수신 graph는 node_count개 내부 vector를 소유하는 const Graph lvalue이며
        //     current를 size_type으로 바꾼 값은 위상 정렬 불변식상 [0,node_count)다.
        // (2) 선택 overload는 `const std::vector<Node>& Graph::operator[](size_type) const`다.
        // (3) 인덱스는 기본 타입 prvalue이고 저장소 소유권을 옮기지 않는다.
        // (4) 반환 const reference를 outgoing에 묶어 현재 함수 호출 동안 읽는다.
        // (5) graph와 내부 vector의 값·크기·용량은 바뀌지 않고 reference 수명은 graph를 넘지 않는다.
        // (6) O(1), 무할당·무효화·예외 없음이나 범위 검사가 없어 잘못된 인덱스면 UB다.
        //     공식 입력과 위상 순서 불변식이 범위를 보장하고 graph 병행 변경은 없다.
        const std::vector<Node>& outgoing{graph[static_cast<std::size_t>(current)]};

        // [첫 호출 계약: const std::vector<Node>::size() — outgoing]
        // (1) 수신 outgoing은 graph 내부의 살아 있는 const vector<Node> lvalue다.
        // (2) `size_type vector<Node>::size() const noexcept`를 선택하고 데이터 인자는 없다.
        // (3) 소유권·값 전달이 없고 반환된 크기만 반복 조건에서 읽는다.
        // (4) size_type 값은 이 정점의 나가는 간선 수이며 edge_index 상한으로 사용한다.
        // (5) graph와 outgoing, 모든 관찰자는 그대로다.
        // (6) O(1), 무할당·noexcept·무효화 없음이며 단일 스레드 읽기다.
        for (std::size_t edge_index{}; edge_index < outgoing.size(); ++edge_index) {
            // [첫 호출 계약: const std::vector<Node>::operator[] — outgoing]
            // (1) 수신 outgoing은 살아 있는 const vector<Node>이고 edge_index는 loop상 범위 안이다.
            // (2) `const Node& vector<Node>::operator[](size_type) const`를 선택한다.
            // (3) edge_index는 이름 있는 std::size_t lvalue이며 lvalue-to-rvalue로 범위 안
            //     size_type 값 매개변수에 복사된다. 소유권 이동은 없다.
            // (4) 반환 const Node&의 값을 next에 복사하고 reference 자체는 저장하지 않는다.
            // (5) outgoing의 원소·크기·용량·수명은 그대로다.
            // (6) O(1), 무할당·무효화·예외 없음이나 범위 밖이면 UB다. loop가 전제조건을
            //     보장하고 같은 원소의 병행 수정은 없다.
            const Node next{outgoing[edge_index]};
            Node& next_indegree{indegree[static_cast<std::size_t>(next)]};
            --next_indegree;
            if (next_indegree == Node{0}) {
                ready.push_back(next);
            }
        }
    }

    // [반환 계약: std::vector<Node> NRVO 또는 이동 생성]
    // (1) order는 완성된 위상 순서를 소유하는 유효한 local vector이고 호출자 결과 객체는
    //     아직 별도 수명이 시작되지 않았거나 NRVO로 order와 같은 객체가 된다.
    // (2) 구현이 NRVO를 적용하면 추가 생성이 없고, 아니면 move-eligible return 식에서
    //     `vector<Node>(vector<Node>&&) noexcept`가 선택된다.
    // (3) 이름 있는 order 식은 문법상 lvalue지만 return에서 이동 후보가 된다. 복사본을
    //     장기 보관하지 않고 연속 저장소 소유권을 결과 객체로 넘긴다.
    // (4) 생성자는 별도 반환값이 없고 함수 결과 vector 객체의 수명을 시작한다.
    // (5) NRVO면 order가 곧 결과다. 이동이면 결과가 저장소를 소유하고 원본은 유효하지만
    //     값이 미지정인 상태로 곧 소멸한다. 이 지점에 저장한 원소 관찰자는 없다.
    // (6) NRVO는 추가 비용·할당·무효화가 없고 기본 allocator의 이동은 O(1), noexcept다.
    //     호출자는 반환된 vector를 다른 thread와 공유해 쓸 때 직접 동기화해야 한다.
    return order;
}

// 행 우선 flat vector를 써서 n개의 작은 vector allocation과 pointer chase를 피한다.
// row(u,w)의 물리 인덱스는 u*word_count+w다.
[[nodiscard]] std::vector<Word> make_reachability_words(
    const Graph& graph,
    const std::vector<Node>& topological_order) {
    const std::size_t node_count{graph.size()};
    const std::size_t word_count{(node_count + kWordBits - 1U) / kWordBits};

    // [첫 생성 계약: std::vector<Word> fill 생성자]
    // (1) reachability는 아직 수명이 시작되지 않은 std::vector<Word> 지역 owner다.
    // (2) 선택 overload는 `vector(size_type count, const Word& value)`와 기본 allocator다.
    // (3) 첫 인자는 n*ceil(n/64)인 size_type prvalue, 둘째 Word{0}은 모든 원소에 복사할
    //     const reference 입력이다. 공식 최대에서 곱은 약 3,910만이고 size_type 범위 안이다.
    // (4) 생성자는 반환값 없이 reachability 자체와 count개 원소 수명을 시작한다.
    // (5) 성공하면 모든 도달 비트가 0이고 vector가 연속 저장소를 독점 소유한다.
    // (6) O(n*ceil(n/64)) 시간·공간, 최대 약 298.3MiB를 동적 할당한다. length_error나
    //     bad_alloc 가능, 실패 시 완성 원소는 정리된다. 성공 뒤 재할당하지 않아 원소 참조는
    //     vector 수명까지 유지되지만 같은 원소의 병행 쓰기는 동기화하지 않는다.
    std::vector<Word> reachability(node_count * word_count, Word{0});

    // 모든 행의 자기 비트를 먼저 켜 두면 미처리 정점도 최소한 길이 0 경로를 나타낸다.
    // 마지막 word의 정점 밖 padding bit는 건드리지 않으므로 끝까지 0으로 유지된다.
    for (std::size_t current_index{}; current_index < node_count; ++current_index) {
        const std::size_t current_row{current_index * word_count};
        const std::size_t self_word{current_index / kWordBits};
        const std::size_t self_bit{current_index % kWordBits};
        // [첫 호출 계약: non-const std::vector<Word>::operator[] — reachability]
        // (1) 수신 reachability는 n*word_count개 Word를 소유하는 non-const vector lvalue이며
        //     current_row+self_word는 계산상 [0,size())에 있다.
        // (2) 선택 overload는 `Word& vector<Word>::operator[](size_type)`다.
        // (3) 인덱스는 범위 안 size_type prvalue이고 저장소 소유권을 옮기지 않는다.
        // (4) 반환 Word&를 복합 OR의 읽기/쓰기 대상으로 즉시 사용하며 따로 저장하지 않는다.
        // (5) 해당 원소의 자기 bit만 1이 되고 vector 크기·용량과 다른 원소는 유지된다.
        // (6) O(1), 무할당·무효화·예외 없음이나 범위 밖이면 UB다. 공식 n과 행 계산이
        //     범위를 보장하며 같은 Word에 대한 병행 쓰기는 없다.
        reachability[current_row + self_word] |= Word{1} << self_bit;
    }

    // 역위상 순서 불변식: current를 처리할 때 모든 current->next의 행은 이미 완성돼 있다.
    for (std::size_t position{topological_order.size()}; position > 0U; --position) {
        const Node current{topological_order[position - 1U]};
        const std::size_t current_index{static_cast<std::size_t>(current)};
        const std::size_t current_row{current_index * word_count};

        const std::vector<Node>& outgoing{graph[current_index]};
        for (std::size_t edge_index{}; edge_index < outgoing.size(); ++edge_index) {
            const std::size_t next_index{
                static_cast<std::size_t>(outgoing[edge_index])};
            const std::size_t next_row{next_index * word_count};

            // word OR는 최대 64개의 bool 합집합을 한 기본 연산으로 묶는다.
            for (std::size_t word{}; word < word_count; ++word) {
                reachability[current_row + word] |= reachability[next_row + word];
            }
        }
    }

    // [반환 계약: std::vector<Word> NRVO 또는 이동 생성]
    // (1) reachability는 완성된 flat 행렬을 소유하는 유효한 local vector이며 결과 객체는
    //     NRVO가 아니면 아직 생성 전이다.
    // (2) NRVO가 허용되고, 미적용 시 move-eligible return이
    //     `vector<Word>(vector<Word>&&) noexcept`를 선택한다.
    // (3) 문법상 lvalue인 이름 있는 local의 저장소 소유권을 결과로 넘기며 외부 참조 입력은 없다.
    // (4) 생성자는 반환값 없이 호출자의 결과 vector 수명을 시작한다.
    // (5) NRVO면 같은 객체가 결과가 되고, 이동이면 결과가 행렬 저장소를 소유하며 원본은
    //     유효하지만 값이 미지정인 채 소멸한다. 내부 원소 reference를 따로 저장하지 않았다.
    // (6) NRVO는 추가 비용이 없고 기본 allocator 이동은 O(1), 무할당·noexcept다.
    //     결과 vector 자체의 병행 변경은 동기화가 필요하다.
    return reachability;
}

// 각 행의 1비트를 합하고 답을 즉시 출력해 별도 answer vector를 만들지 않는다.
void print_reachable_counts(
    const std::vector<Word>& reachability,
    std::size_t node_count) {
    const std::size_t word_count{(node_count + kWordBits - 1U) / kWordBits};

    for (std::size_t node{}; node < node_count; ++node) {
        const std::size_t row{node * word_count};
        Node reachable_count{};

        for (std::size_t word{}; word < word_count; ++word) {
            // [첫 호출 계약: const vector<Word>::operator[]와 std::popcount]
            // (1) 수신 reachability는 살아 있는 const vector<Word> lvalue이고 row+word는
            //     [0,size())다. popcount는 수신 객체 없는 자유 함수다.
            // (2) 먼저 `const Word& vector<Word>::operator[](size_type) const`, 이어
            //     `template<class T> constexpr int popcount(T) noexcept`의 T=Word를 선택한다.
            // (3) operator[] 인덱스는 범위 안 size_type prvalue다. 그 const Word&에서
            //     lvalue-to-rvalue로 복사된 unsigned 64비트 값이 popcount 입력이며 소유권은 없다.
            // (4) operator[] 반환형 const Word&는 즉시 읽고, popcount 반환형 int [0,64]는
            //     reachable_count 합계에 누적해 실제 사용한다.
            // (5) 호출 뒤 vector 원소·크기·용량·관찰자 수명과 Word 값은 모두 그대로다.
            // (6) 둘 다 O(1), 무할당·무효화·예외 없음이다. operator[]는 범위 밖이면 UB지만
            //     loop 계산이 방지한다. 실제 popcount 명령은 CPU·컴파일러·옵션에 따라 다르며
            //     공유 vector 병행 변경은 외부 동기화가 필요하다.
            reachable_count += std::popcount(reachability[row + word]);
        }

        // [첫 호출 계약: std::cout 산술·문자 operator<<]
        // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 앞선 출력 오류가 없다고 전제한다.
        // (2) Node 값에는 멤버 `ostream::operator<<(int)`, 조건식 char에는 비멤버
        //     `operator<<(std::ostream&, char)` overload가 차례로 선택된다.
        // (3) reachable_count는 [1,n] int lvalue의 값, 둘째 피연산자는 마지막이면 '\n',
        //     아니면 공백인 char prvalue다. 둘 다 읽기 입력이고 소유권 의미가 없다.
        // (4) 첫 호출의 std::ostream& 반환은 두 번째 삽입 수신자로 쓰고 마지막 참조는 버린다.
        // (5) 성공하면 출력 위치·버퍼만 바뀌고 답과 reachability는 유지된다. 실패 시
        //     failbit/badbit가 설정될 수 있다.
        // (6) 형식 출력 전체에 표준의 단일 O(...) 상한은 없고 문자 수·locale·streambuf에
        //     달린다. 내부 할당/출력 오류와 예외 mask에 따른 예외가 가능하다. 컨테이너
        //     관찰자를 무효화하지 않는다. 앞서 C/C++ 동기화를 껐으므로 이 코드는 단일
        //     thread에서만 출력하며, 병행 호출에는 외부 동기화가 필요하다고 전제한다.
        std::cout << reachable_count << (node + 1U == node_count ? '\n' : ' ');
    }
}

// main은 입력 경계와 알고리즘 단계를 조립하고 0을 반환해 정상 종료를 알린다.
int main() {
    // [첫 호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 정적 함수라 수신 객체가 없고 표준 stream들은 아직 이 프로그램에서 사용되지 않았다.
    // (2) 선택 시그니처는 `static bool ios_base::sync_with_stdio(bool sync = true)`다.
    // (3) 인자 false는 C stdio와 C++ iostream 동기화를 끄는 bool prvalue이며 소유권이 없다.
    // (4) 반환형 bool은 이전 설정이지만 여기서는 버린다.
    // (5) 이후 C와 C++ stream을 섞을 때 자동 순서 동기화를 기대하지 않는다.
    // (6) 입출력 전 호출해야 이후 동작이 구현 정의가 되지 않는다. 표준의 별도 O(...)·
    //     noexcept 보장은 없고 전역 설정의 병행 변경은 하지 않는다. 사용자 관찰자 수명은 유지된다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie(nullptr)]
    // (1) 수신 std::cin은 초기화된 std::istream lvalue이며 출력 stream에 tie됐을 수 있다.
    // (2) 선택 overload는 `std::ostream* basic_ios<char>::tie(std::ostream*)`다.
    // (3) nullptr는 새 tie 대상이 없다는 null pointer prvalue이며 stream 소유권을 옮기지 않는다.
    // (4) 이전 tie 대상 pointer를 반환하지만 사용하지 않는다.
    // (5) 이후 입력 전 std::cout 자동 flush 연결만 제거되고 두 stream 수명은 그대로다.
    // (6) 표준의 별도 O(...)·noexcept 보장은 없다. 순환 tie 전제는 nullptr라 안전하고,
    //     할당·사용자 관찰자 무효화가 없으며 전역 stream 설정을 동시에 바꾸지 않는다.
    std::cin.tie(nullptr);

    Node node_count_input{};
    Node edge_count{};
    // [첫 호출 계약: std::cin 정수 operator>> 연쇄]
    // (1) 수신 std::cin은 오류 비트가 없는 유효한 std::istream lvalue이고 두 목적지는
    //     수정 가능한 Node(int) lvalue다.
    // (2) 두 호출 모두 `std::istream& operator>>(int&)` 산술 overload를 선택한다.
    // (3) 인자는 node_count_input, edge_count의 비소유 lvalue reference이며 공식 범위의
    //     십진 정수를 받을 저장소다. 소유권 이전은 없다.
    // (4) 첫 std::istream& 반환은 두 번째 추출 수신자로 쓰고 마지막 반환 참조는 버린다.
    // (5) 성공하면 값과 입력 위치가 바뀐다. 실패하면 failbit/eofbit/badbit가 설정될 수 있다.
    // (6) 표준은 단일 O(...) 상한을 두지 않고 locale·streambuf에 달린다. 범위 오류나
    //     예외 mask 설정에 따른 예외가 가능하다. 공식 유효 입력을 전제하고 동시 입력은 없다.
    std::cin >> node_count_input >> edge_count;

    const std::size_t node_count{static_cast<std::size_t>(node_count_input)};

    // [첫 생성 계약: Graph의 바깥 std::vector count 생성자]
    // (1) graph는 아직 수명이 시작되지 않은 Graph 지역 owner다.
    // (2) 선택 overload는 `vector(size_type count)`이고 원소 타입은 std::vector<Node>다.
    // (3) 인자 node_count는 이름 있는 const std::size_t lvalue이며 lvalue-to-rvalue로
    //     [1,50000]인 size_type 값 매개변수에 복사된다. count개의 내부 빈 vector를
    //     기본 생성하며 외부 객체를 빌리거나 소유권 이전하지 않는다.
    // (4) 생성자는 반환값 없이 graph와 모든 내부 vector 수명을 시작한다.
    // (5) 성공하면 graph.size()==n이고 각 outgoing 목록은 비어 있다.
    // (6) O(n) 시간·공간과 동적 할당 가능, length_error/bad_alloc 가능이다. 실패 시
    //     완성 원소를 정리한다. 재할당 전 바깥 원소 관찰자는 유효하고 자체 동기화는 없다.
    Graph graph(node_count);

    // [첫 생성 계약: std::vector<Node> fill 생성자 — indegree]
    // (1) indegree는 아직 수명이 시작되지 않은 std::vector<Node> 지역 owner다.
    // (2) `vector<Node>(size_type count, const Node& value)`와 기본 allocator를 선택한다.
    // (3) node_count는 이름 있는 const std::size_t lvalue이며 [1,50000]인 size_type 값으로
    //     복사된다. Node{0}은 각 원소로 복사할 int prvalue이며 호출 동안 const reference에
    //     묶인다. 외부 저장소나 소유권을 받지 않는다.
    // (4) 생성자는 별도 반환값 없이 vector와 n개 Node 원소의 수명을 시작한다.
    // (5) 성공하면 size()==n이고 모든 진입 차수가 0이며 연속 저장소를 indegree가 소유한다.
    // (6) O(n) 시간·공간, 동적 할당 가능, length_error/bad_alloc 가능이다. 실패 시 완성된
    //     원소를 정리하고 객체 수명은 시작되지 않으며 아직 관찰자·thread 공유는 없다.
    std::vector<Node> indegree(node_count, Node{0});

    for (Node edge{}; edge < edge_count; ++edge) {
        Node from{};
        Node to{};
        std::cin >> from >> to;
        --from;
        --to;

        // [첫 호출 계약: non-const Graph::operator[] + inner push_back(const Node&)]
        // (1) 수신 graph는 n개 내부 vector를 소유하는 non-const Graph lvalue이고 from은
        //     0-base 변환 뒤 [0,n)다. 반환될 내부 vector도 유효하며 to는 살아 있는 Node lvalue다.
        // (2) `std::vector<Node>& Graph::operator[](size_type)` 뒤
        //     `void vector<Node>::push_back(const Node&)`를 선택한다.
        // (3) 첫 인자는 from의 size_type prvalue, 둘째는 [0,n)인 to의 const reference 입력이다.
        //     정수 값만 복사하며 graph나 to의 소유권을 옮기지 않는다.
        // (4) operator[]의 reference는 push_back 수신자로 즉시 쓰고, push_back은 void다.
        // (5) 해당 outgoing 목록 끝에 to가 복사되고 그 내부 vector size만 1 증가한다.
        // (6) 접근 O(1), 삽입 상각 O(1)이다. 내부 vector는 할당·bad_alloc과 자체 원소
        //     pointer/reference/iterator 무효화를 일으킬 수 있지만 바깥 graph 원소 주소는
        //     바뀌지 않는다. from 범위가 전제이며 이 입력 단계는 단일 스레드다.
        graph[static_cast<std::size_t>(from)].push_back(to);
        ++indegree[static_cast<std::size_t>(to)];
    }

    // 각 함수 반환 prvalue는 목적 vector를 직접 초기화할 수 있다. 이름을 얻은 뒤 두 식은 lvalue다.
    const std::vector<Node> topological_order{
        make_topological_order(graph, indegree)};
    const std::vector<Word> reachability{
        make_reachability_words(graph, topological_order)};

    print_reachable_counts(reachability, node_count);
    return 0;
}
