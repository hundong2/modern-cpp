/*
문제 요약 — CSES 2080 Fixed-Length Paths I
출처: https://cses.fi/problemset/task/2080

n개의 정점이 간선 n-1개로 연결된 트리에서, 서로 다른 두 정점을 잇는 단순 경로 중 간선 수가
정확히 k인 경로가 몇 개인지 센다. 경로의 방향은 구분하지 않으므로 (u, v)와 (v, u)는 하나다.

입력
- 첫 줄: 정점 수 n과 목표 거리 k
- 다음 n-1줄: 무방향 간선의 두 끝점 a, b (정점 번호는 1부터 시작)

출력
- 길이가 정확히 k인 서로 다른 경로의 수 하나

공식 제약
- 1 <= k <= n <= 200,000
- 1 <= a,b <= n
- 시간 제한 1초, 메모리 제한 512MB

예제
입력:
5 2
1 2
2 3
3 4
3 5

출력:
4

핵심은 센트로이드 하나를 제거했을 때 생기는 서로 다른 자식 컴포넌트 사이의 거리 쌍만 먼저 세는
것이다. 어떤 경로든 두 끝점이 처음으로 갈라지는 센트로이드가 정확히 하나이므로 중복도 누락도 없다.
*/

// <cstddef>는 컨테이너 크기와 인덱스에 사용하는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력 std::cin, 출력 std::cout과 정수 스트림 연산자를 선언한다.
#include <iostream>
// <vector>는 인접 목록과 반복 탐색 스택을 소유하는 가변 길이 연속 컨테이너를 선언한다.
#include <vector>

// using 별칭은 새 타입을 만들지 않고 long long에 "경로 수"라는 문제 의미를 붙인다.
// 최대 답 n(n-1)/2는 약 2*10^10이므로 최소 64비트인 long long이 필요하다.
using PathCount = long long;

// struct는 기본 접근이 public이라 순회 스택에 넣는 단순 값 묶음에 알맞다. class는 기본 접근이
// private이라 불변식과 동작을 숨기는 객체에 더 자연스럽다. 세 int는 각각 정점, 부모, 센트로이드 거리다.
// 이 타입은 사용자 생성자가 없는 aggregate다. 생성자는 반환형이 없고, 생성자가 있다면 멤버 초기화 목록이
// 본문보다 먼저 멤버를 직접 구성한다. 한 인자 생성자의 explicit은 뜻하지 않은 암시적 타입 변환을 막는다.
struct DistanceFrame {
    int node{};
    int parent{};
    int depth{};
};

// const 참조 매개변수는 graph/removed를 복사하거나 소유하지 않고 읽기만 한다. non-const 참조는 호출자가
// 소유한 작업 배열을 재사용한다. 참조는 여기서 null일 수 없고 다시 다른 객체를 가리킬 수도 없다. 포인터는
// null/재지정이 가능하지만 포인터와 참조 모두 별도 계약이 없으면 소유권을 뜻하지 않는다. int 반환은 독립
// prvalue라 지역 참조 수명 문제 없이 센트로이드 번호를 준다.
[[nodiscard]] int find_centroid(
    int component_root,
    const std::vector<std::vector<int>>& graph,
    const std::vector<unsigned char>& removed,
    std::vector<int>& parent,
    std::vector<int>& subtree_size,
    std::vector<int>& component,
    std::vector<int>& traversal_stack);

// 선택한 센트로이드를 지나는 길이 k 경로만 센다. 모든 scratch 컨테이너는 참조로 빌려 반복 재사용하므로
// 분할 단계마다 큰 저장소를 다시 소유하거나 복사하지 않는다. 반환 PathCount는 값으로 전달되는 prvalue다.
[[nodiscard]] PathCount count_paths_through_centroid(
    int centroid,
    int target_distance,
    const std::vector<std::vector<int>>& graph,
    const std::vector<unsigned char>& removed,
    std::vector<int>& depth_frequency,
    std::vector<int>& touched_depths,
    std::vector<int>& collected_depths,
    std::vector<DistanceFrame>& distance_stack);

int main() {
    // [호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 정적 함수라 수신 객체가 없고 아직 표준 스트림 I/O를 시작하지 않은 상태다.
    // (2) static bool sync_with_stdio(bool sync = true)에서 bool 인자 하나인 overload를 선택한다.
    // (3) 인자는 bool prvalue false이며 비소유 값이고 허용값이다.
    // (4) 이전 동기화 설정 bool을 반환하지만 사용하지 않는다.
    // (5) C stdio와 C++ iostream의 동기화를 끄되 스트림 객체의 소유권과 수명은 바꾸지 않는다.
    // (6) 첫 I/O 전이어야 효과가 이식 가능하다. 복잡도·할당 상한은 표준이 지정하지 않고, C/C++ I/O
    //     혼용 순서나 애플리케이션 레코드 단위 스레드 안전을 보장하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::ios::sync_with_stdio(false);

    // [호출 계약: std::cin.tie(nullptr)]
    // (1) 수신자는 정상 수명의 std::cin이며 정확한 타입은 std::istream lvalue다.
    // (2) std::ostream* tie(std::ostream*) setter overload가 선택된다.
    // (3) nullptr는 null pointer prvalue이고 비소유이며 자동 flush 연결 해제를 뜻한다.
    // (4) 이전 tied ostream의 비소유 포인터를 반환하지만 사용하지 않는다.
    // (5) 입력 전 출력 자동 flush 연결만 끊고 두 스트림의 상태·수명·소유권은 유지한다.
    // (6) nullptr는 tie cycle을 만들지 않는 유효 입력이다. 표준은 이 setter의 복잡도·할당·Throws를 따로
    //     보장하지 않고 선언도 noexcept가 아니다. 구현은 보통 포인터만 바꾸지만 그 관찰을 계약으로 삼지
    //     않는다. 별도 스레드 동기화를 추가하지 않으며 이 문제에는 선출력해야 할 대화형 prompt가 없다.
    std::cin.tie(nullptr);

    int node_count{};      // int{}는 0으로 값 초기화되며 공식 최댓값 200,000을 충분히 담는다.
    int target_distance{}; // 경로 길이도 int 범위 안이다.

    // [호출 계약: std::istream 정수 추출 연쇄]
    // (1) 수신자는 입력 가능한 std::cin/std::istream lvalue이고 두 int 목적 객체가 살아 있다.
    // (2) std::istream& operator>>(int&) overload를 두 번 선택한다.
    // (3) node_count와 target_distance는 수정 가능한 int lvalue이고 공식 범위 정수를 받으며 비소유다.
    // (4) 첫 반환 std::istream&는 다음 추출 수신자로 쓰고 마지막 반환 참조는 버린다.
    // (5) 성공하면 두 값과 읽기 위치가 갱신되고 다른 객체의 소유권·수명은 바뀌지 않는다.
    // (6) 비용은 문자 수·locale·버퍼에 의존한다. EOF/형식/범위 오류는 상태 비트를 세우며 예외 마스크에
    //     따라 ios_base::failure가 가능하다. 외부 동시 접근에 대한 논리 레코드 원자성은 없다.
    std::cin >> node_count >> target_distance;

    const std::size_t node_count_size{static_cast<std::size_t>(node_count)};

    // [호출 계약: std::vector count/fill 생성자]
    // (1) 아직 존재하지 않는 graph, parent, subtree_size, removed, depth_frequency 목적 객체를 만든다.
    // (2) graph에는 vector(size_type), 나머지에는 vector(size_type, const T&) overload가 선택되며 템플릿
    //     인자는 각각 vector<int>, int, int, unsigned char, int다.
    // (3) count는 공식 범위의 size_t prvalue다. fill 값 -1/0은 각 원소에 복사되며 소유권 이전은 없다.
    // (4) 생성자는 반환값이 없다.
    // (5) 각 vector가 지정 개수 원소의 수명과 저장소를 소유한다. graph의 안쪽 vector는 모두 비어 있다.
    // (6) 원소 수에 선형인 구성과 동적 할당이 있으며 length_error/bad_alloc 가능성이 있다. 실패하면 완성
    //     객체가 없고, 기존 관찰자는 없다. 성공한 원소는 바깥 vector가 파괴될 때까지 유효하고 단일 스레드다.
    // 대표 문서: ../standard-library/containers-and-views.md
    std::vector<std::vector<int>> graph(node_count_size);
    std::vector<int> parent(node_count_size, -1);
    std::vector<int> subtree_size(node_count_size, 0);
    std::vector<unsigned char> removed(node_count_size, static_cast<unsigned char>(0));
    std::vector<int> depth_frequency(static_cast<std::size_t>(target_distance + 1), 0);

    for (int edge_index{}; edge_index < node_count - 1; ++edge_index) {
        int first{};
        int second{};
        // 위 정수 추출 계약과 같다. 두 반환 중 첫 stream&만 연쇄에 쓰고 성공 시 두 끝점을 갱신한다.
        std::cin >> first >> second;
        --first;
        --second;

        // [호출 계약: vector::operator[]와 vector<int>::push_back(const int&)]
        // (1) graph는 size=n인 vector<vector<int>> lvalue이고 두 안쪽 vector도 유효하다.
        // (2) 바깥 operator[](size_type)의 non-const overload는 vector<int>&를 반환하고, 안쪽
        //     push_back(const int&) overload는 void를 반환한다.
        // (3) [] 인자는 검증된 [0,n) 정점을 바꾼 size_t prvalue다. push 인자는 반대 끝점 int lvalue이며
        //     값만 복사하므로 입력 변수의 소유권은 옮기지 않는다.
        // (4) [] 반환 참조는 즉시 push 수신자로 쓰고 push의 void에는 사용할 반환값이 없다.
        // (5) 각 인접 목록 size가 1 늘고 정점 사본을 소유한다. graph의 바깥 size는 그대로다.
        // (6) []는 O(1)·무할당이며 범위 밖은 UB이나 제약이 배제한다. push는 분할 상환 O(1)이고
        //     bad_alloc 가능, 재할당 시 해당 안쪽 vector의 원소 참조·포인터·반복자가 모두 무효화된다.
        graph[static_cast<std::size_t>(first)].push_back(second);
        graph[static_cast<std::size_t>(second)].push_back(first);
    }

    // [호출 계약: 빈 std::vector 기본 생성자와 reserve]
    // (1) 아직 없는 다섯 vector<int>와 vector<DistanceFrame> 하나를 빈 상태로 만들고 곧 용량을 예약한다.
    // (2) vector()와 void reserve(size_type) overload가 선택되며 기본 allocator를 쓴다.
    // (3) reserve 인자는 node_count_size prvalue이고 공식 메모리 범위 안이다. 소유권 이전은 없다.
    // (4) 생성자와 reserve 모두 사용할 반환값이 없다.
    // (5) size는 0인 채 capacity가 적어도 n이 되어 이후 scratch push의 반복 재할당을 막는다.
    // (6) 기본 생성은 상수 시간·통상 무할당이고 reserve는 저장소를 할당해 length_error/bad_alloc 가능성이
    //     있다. 성공 시 기존 관찰자는 모두 무효지만 빈 vector라 없다. 실패 시 이 원소 타입들은 상태가 유지된다.
    std::vector<int> component{};
    std::vector<int> traversal_stack{};
    std::vector<int> pending_components{};
    std::vector<int> touched_depths{};
    std::vector<int> collected_depths{};
    std::vector<DistanceFrame> distance_stack{};
    component.reserve(node_count_size);
    traversal_stack.reserve(node_count_size);
    pending_components.reserve(node_count_size);
    touched_depths.reserve(node_count_size);
    collected_depths.reserve(node_count_size);
    distance_stack.reserve(node_count_size);

    const int initial_root{};
    // 앞서 설명한 vector<int>::push_back(const int&) 계약을 재사용한다. capacity>=n이고 현재 size=0이라
    // 이 호출은 재할당 없이 루트 사본 하나를 저장하며 반환값은 없다.
    pending_components.push_back(initial_root);

    PathCount answer{};

    // 공용 알고리즘 문서: ../algorithm/centroid-decomposition-fixed-distance-pairs.md
    // 분할 불변식: pending의 각 정점은 아직 제거되지 않은 서로 분리된 컴포넌트 하나를 대표한다.
    // 재귀 호출 대신 명시적 vector 스택을 사용해 원래 트리가 일자여도 프로그램 호출 스택은 깊어지지 않는다.
    while (true) {
        // [호출 계약: vector<int>::empty, back, pop_back]
        // (1) 수신자는 살아 있는 pending_components vector<int> lvalue다. back/pop 때는 아래 empty 검사로
        //     size>0임이 보장된다.
        // (2) bool empty() const noexcept, int& back(), void pop_back() overload를 선택한다.
        // (3) 세 멤버 모두 데이터 인자가 없고 소유권을 받지 않는다.
        // (4) empty의 bool은 분기에, back의 int& 값은 root 복사에 쓰며 pop에는 반환값이 없다.
        // (5) pop은 마지막 int 수명을 끝내 size를 1 줄이지만 capacity와 다른 원소는 유지한다.
        // (6) 모두 O(1)·무할당이다. 빈 back/pop은 UB지만 선행 검사가 배제한다. pop은 지운 원소와 end
        //     관찰자를 무효화하며 복사한 root는 독립 값이다. 외부 동시 접근은 없다.
        if (pending_components.empty()) {
            break;
        }
        const int component_root{pending_components.back()};
        pending_components.pop_back();

        const int centroid{find_centroid(
            component_root,
            graph,
            removed,
            parent,
            subtree_size,
            component,
            traversal_stack)};

        // [호출 계약: vector<unsigned char>::operator[] 쓰기]
        // (1) removed는 size=n인 non-const vector<unsigned char> lvalue이고 centroid는 [0,n)다.
        // (2) reference operator[](size_type) non-const overload가 선택된다.
        // (3) 인자는 centroid의 size_t prvalue이며 소유권 의미가 없다.
        // (4) 반환 reference를 대입 왼쪽 피연산자로 사용한다.
        // (5) 해당 플래그만 1로 바뀌고 size/capacity/수명은 그대로라 관찰자를 무효화하지 않는다.
        // (6) O(1)·무할당이며 유효 범위에서 지정 예외가 없다. 범위 밖 C++20 UB는 계산이 배제하고,
        //     한 스레드만 수정하므로 데이터 경쟁도 없다.
        removed[static_cast<std::size_t>(centroid)] = static_cast<unsigned char>(1);

        answer += count_paths_through_centroid(
            centroid,
            target_distance,
            graph,
            removed,
            depth_frequency,
            touched_depths,
            collected_depths,
            distance_stack);

        // [호출 계약: vector<vector<int>>::operator[]와 vector<int>::size]
        // (1) graph는 size=n인 non-const lvalue이고 centroid 인접 목록은 입력 뒤 수정되지 않은 유효 vector다.
        // (2) 바깥 non-const operator[](size_type)은 vector<int>&, 안쪽 size() const noexcept는 size_type이다.
        // (3) [] 인자는 유효 size_t prvalue이고 size에는 데이터 인자가 없다. 소유권 이전은 없다.
        // (4) 반환 참조는 const 참조에 바인딩하고 size 반환값은 반복 상한으로 사용한다.
        // (5) graph와 목록은 바뀌지 않으며 참조는 graph 수명 안에서만 사용한다.
        // (6) 둘 다 O(1)·무할당·비무효화이고 size는 noexcept다. [] 범위 밖 UB는 centroid 범위로 배제한다.
        const std::vector<int>& centroid_neighbors{graph[static_cast<std::size_t>(centroid)]};
        const std::size_t neighbor_count{centroid_neighbors.size()};
        for (std::size_t index{}; index < neighbor_count; ++index) {
            // [호출 계약: const vector<int>::operator[] 읽기]
            // (1) centroid_neighbors는 size=neighbor_count인 const vector<int> lvalue다.
            // (2) const int& operator[](size_type) const overload가 선택된다.
            // (3) index는 루프가 [0,size)로 제한한 size_t lvalue 값이며 비소유다.
            // (4) 반환 const int& 값을 neighbor에 복사한다.
            // (5) 컨테이너·원소·관찰자는 변하지 않는다.
            // (6) O(1)·무할당이고 범위 밖 UB는 조건이 배제한다. 외부 수정과 동시 접근도 없다.
            const int neighbor{centroid_neighbors[index]};
            if (removed[static_cast<std::size_t>(neighbor)] == static_cast<unsigned char>(0)) {
                // 센트로이드를 제거한 뒤 각 살아 있는 이웃은 서로 다른 다음 컴포넌트의 대표다.
                pending_components.push_back(neighbor);
            }
        }
    }

    // [호출 계약: std::ostream 정수/문자 삽입 연쇄]
    // (1) 수신자는 출력 가능한 std::cout/std::ostream lvalue이고 answer는 초기화된 long long lvalue다.
    // (2) ostream의 operator<<(long long) 뒤 operator<<(std::ostream&, char) overload를 선택한다.
    // (3) answer는 읽기만 하는 lvalue, '\n'은 char prvalue이고 모두 비소유다.
    // (4) 각 호출은 같은 ostream&를 반환한다. 첫 반환은 연쇄 수신자로, 마지막 반환은 버린다.
    // (5) 십진 정답과 개행이 버퍼에 기록되고 answer/컨테이너의 상태·수명은 바뀌지 않는다.
    // (6) 복잡도와 비용은 자릿수·locale·버퍼/장치에 의존하고 실패는 상태 비트 또는 설정된
    //     ios_base::failure로 나타난다. 컨테이너 무효화나 여러 스레드 레코드 원자성을 보장하지 않는다.
    std::cout << answer << '\n';

    // 전체 시간은 각 분할 높이에서 정점당 O(1)번 방문하므로 O(n log n), 추가 공간은 O(n+k)=O(n)이다.
    // 기계 관점에서는 인접 목록/작업 배열의 load·store, 정수 덧셈·비교, 조건 분기가 중심이다. 실제 캐시,
    // 명령 배치와 최적화는 CPU·ABI·컴파일러·옵션에 따라 달라 특정 어셈블리 형태로 단정하지 않는다.
    // 이름 있는 vector와 answer는 lvalue, 함수가 돌려주는 centroid/count는 prvalue다. 소유 vector를 옮기는
    // xvalue 식은 없고 복사 생략(RVO)이 성능의 전제도 아니다. main 종료 시 역순 소멸로 저장소가 해제된다.
    return 0;
}

[[nodiscard]] int find_centroid(
    int component_root,
    const std::vector<std::vector<int>>& graph,
    const std::vector<unsigned char>& removed,
    std::vector<int>& parent,
    std::vector<int>& subtree_size,
    std::vector<int>& component,
    std::vector<int>& traversal_stack) {
    // [호출 계약: vector<int>::clear]
    // (1) component와 traversal_stack은 이전 단계의 int들을 소유할 수 있는 유효 vector<int> lvalue다.
    // (2) void clear() noexcept overload가 각각 선택되고 데이터 인자는 없다.
    // (3) 매개변수나 소유권 이전은 없다.
    // (4) 반환값은 없다.
    // (5) 저장 원소 수명을 끝내 size를 0으로 만들지만 capacity는 바꾸지 않는다.
    // (6) 원소 수에 선형이고 int 소멸은 던지지 않는다. 모든 원소 참조·포인터·반복자와 end가 무효화되며
    //     예약 저장소는 재사용된다. 외부 동시 접근은 없다.
    component.clear();
    traversal_stack.clear();
    traversal_stack.push_back(component_root);

    // [호출 계약: vector<int>::operator[] 작업 배열 쓰기]
    // (1) parent는 size=n인 non-const vector<int>이고 component_root는 살아 있는 [0,n) 정점이다.
    // (2) int& operator[](size_type) non-const overload가 선택된다.
    // (3) 인자는 유효 size_t prvalue이며 비소유다.
    // (4) 반환 int&를 대입 왼쪽에 사용한다.
    // (5) 루트 부모 값만 -1로 바뀌고 구조·수명·capacity는 그대로다.
    // (6) O(1)·무할당·비무효화이며 범위 밖 UB는 불변식이 배제한다. 단일 스레드라 경쟁이 없다.
    parent[static_cast<std::size_t>(component_root)] = -1;

    while (!traversal_stack.empty()) {
        // empty/back/pop_back/push_back은 main의 vector<int> 계약과 같다. nonempty 검사 뒤 마지막 값을 복사해
        // 제거하므로 pop으로 무효화되는 참조를 보관하지 않는다.
        const int current{traversal_stack.back()};
        traversal_stack.pop_back();
        component.push_back(current);

        // [호출 계약: const graph/인접 vector의 operator[]와 size]
        // (1) graph는 size=n인 const vector<vector<int>> lvalue이고 current는 이 컴포넌트의 유효 정점이다.
        // (2) 바깥 const operator[](size_type)은 const vector<int>&, 안쪽 size() const noexcept는 size_type이다.
        // (3) []의 size_t 인자는 [0,n), size에는 데이터 인자가 없고 소유권 이전은 없다.
        // (4) 반환 참조는 neighbors에 바인딩하고 size 값은 루프 상한으로 사용한다.
        // (5) 어떤 객체도 바뀌지 않고 참조는 graph보다 짧게 산다.
        // (6) O(1)·무할당·비무효화다. [] 범위 밖 UB는 정점 불변식으로 배제하고 동시 수정도 없다.
        const std::vector<int>& neighbors{graph[static_cast<std::size_t>(current)]};
        const std::size_t neighbor_count{neighbors.size()};
        for (std::size_t index{}; index < neighbor_count; ++index) {
            const int neighbor{neighbors[index]}; // const vector<int>::operator[] 계약은 main과 같다.

            // [호출 계약: const vector<unsigned char>::operator[] 읽기]
            // (1) removed는 size=n인 const vector<unsigned char>이고 neighbor는 [0,n)다.
            // (2) const_reference operator[](size_type) const overload가 선택된다.
            // (3) 유효 size_t prvalue 하나이며 비소유다.
            // (4) 반환 값을 제거 여부 비교에 사용한다.
            // (5) 컨테이너와 원소는 변하지 않는다.
            // (6) O(1)·무할당·비무효화이고 범위 밖 UB는 입력 트리 제약이 배제한다.
            if (removed[static_cast<std::size_t>(neighbor)] != static_cast<unsigned char>(0)
                || neighbor == parent[static_cast<std::size_t>(current)]) {
                continue;
            }
            parent[static_cast<std::size_t>(neighbor)] = current;
            traversal_stack.push_back(neighbor);
        }
    }

    // [호출 계약: vector<int>::size와 operator[]]
    // (1) component는 방금 모은 컴포넌트의 모든 정점을 한 번씩 가진 non-const vector<int> lvalue다.
    // (2) size() const noexcept는 size_type, non-const operator[](size_type)은 int&를 반환한다.
    // (3) [] 인덱스는 아래 루프가 [0,size)로 증명하며 비소유다.
    // (4) size 값은 총 정점 수/반복 상한에, [] 값은 정점 번호 복사에 사용한다.
    // (5) 읽기만 하므로 원소·구조·관찰자는 바뀌지 않는다.
    // (6) 모두 O(1)·무할당이며 size는 noexcept다. [] 범위 밖 UB는 루프 조건이 배제한다.
    const std::size_t component_count{component.size()};
    for (std::size_t reverse_index{component_count}; reverse_index > 0; --reverse_index) {
        const int current{component[reverse_index - 1]};
        subtree_size[static_cast<std::size_t>(current)] = 1;

        const std::vector<int>& neighbors{graph[static_cast<std::size_t>(current)]};
        const std::size_t neighbor_count{neighbors.size()};
        for (std::size_t index{}; index < neighbor_count; ++index) {
            const int neighbor{neighbors[index]};
            // parent[child]==current인 방향만 더하면 제거된 정점과 부모 쪽 크기는 섞이지 않는다.
            if (removed[static_cast<std::size_t>(neighbor)] == static_cast<unsigned char>(0)
                && parent[static_cast<std::size_t>(neighbor)] == current) {
                subtree_size[static_cast<std::size_t>(current)] +=
                    subtree_size[static_cast<std::size_t>(neighbor)];
            }
        }
    }

    const int total_size{static_cast<int>(component_count)};
    int centroid{component[0]};
    int smallest_largest_part{total_size + 1};

    // 센트로이드는 제거 뒤 남는 가장 큰 조각의 크기가 최소인 정점이다. 트리에는 그 값이 n/2 이하인
    // 정점이 항상 존재한다. 이 균형 성질 때문에 분할 깊이가 O(log n)이다.
    for (std::size_t component_index{}; component_index < component_count; ++component_index) {
        const int current{component[component_index]};
        int largest_part{total_size - subtree_size[static_cast<std::size_t>(current)]};

        const std::vector<int>& neighbors{graph[static_cast<std::size_t>(current)]};
        const std::size_t neighbor_count{neighbors.size()};
        for (std::size_t index{}; index < neighbor_count; ++index) {
            const int neighbor{neighbors[index]};
            if (removed[static_cast<std::size_t>(neighbor)] == static_cast<unsigned char>(0)
                && parent[static_cast<std::size_t>(neighbor)] == current) {
                const int child_part{subtree_size[static_cast<std::size_t>(neighbor)]};
                if (child_part > largest_part) {
                    largest_part = child_part;
                }
            }
        }

        if (largest_part < smallest_largest_part) {
            smallest_largest_part = largest_part;
            centroid = current;
        }
    }
    return centroid;
}

[[nodiscard]] PathCount count_paths_through_centroid(
    int centroid,
    int target_distance,
    const std::vector<std::vector<int>>& graph,
    const std::vector<unsigned char>& removed,
    std::vector<int>& depth_frequency,
    std::vector<int>& touched_depths,
    std::vector<int>& collected_depths,
    std::vector<DistanceFrame>& distance_stack) {
    touched_depths.clear(); // vector<int>::clear 계약은 find_centroid의 첫 clear와 같다.
    const int zero_depth{};
    depth_frequency[0] = 1;
    touched_depths.push_back(zero_depth);

    PathCount subtotal{};
    const std::vector<int>& centroid_neighbors{graph[static_cast<std::size_t>(centroid)]};
    const std::size_t neighbor_count{centroid_neighbors.size()};

    // 깊이 빈도에는 centroid와 이미 끝낸 자식 컴포넌트의 정점만 들어 있다. 현재 자식의 깊이 d를
    // k-d와 먼저 짝지은 뒤 등록해야 같은 자식 내부 경로를 이 센트로이드 단계에서 잘못 세지 않는다.
    for (std::size_t neighbor_index{}; neighbor_index < neighbor_count; ++neighbor_index) {
        const int child_root{centroid_neighbors[neighbor_index]};
        if (removed[static_cast<std::size_t>(child_root)] != static_cast<unsigned char>(0)) {
            continue;
        }

        collected_depths.clear();
        // [호출 계약: vector<DistanceFrame>::clear]
        // (1) distance_stack은 이전 자식의 frame을 가질 수 있는 유효 vector<DistanceFrame> lvalue다.
        // (2) void clear() noexcept overload를 데이터 인자 없이 선택한다.
        // (3) 소유권을 받는 인자는 없다.
        // (4) 반환값은 없다.
        // (5) 모든 frame 수명을 끝내 size=0으로 만들되 예약 capacity는 유지한다.
        // (6) 기존 원소 수에 선형이고 이 단순 struct 소멸은 던지지 않는다. 모든 원소 관찰자는 무효화되며
        //     저장소는 유지된다. 외부 동시 접근은 없다.
        distance_stack.clear();

        const DistanceFrame initial_frame{child_root, centroid, 1};
        // [호출 계약: vector<DistanceFrame>::push_back(const DistanceFrame&)]
        // (1) distance_stack은 size=0이고 capacity>=n인 vector<DistanceFrame> lvalue다.
        // (2) void push_back(const DistanceFrame&) overload가 선택된다.
        // (3) initial_frame은 살아 있는 const lvalue이고 세 int 값을 복사하며 소유권 이전은 없다.
        // (4) 반환값은 없다.
        // (5) frame 사본을 마지막에 구성해 size가 1 늘어난다. 원본은 변하지 않는다.
        // (6) 분할 상환 O(1)이고 예약 범위에서는 재할당이 없다. 일반적으로 bad_alloc/복사 예외와 재할당
        //     무효화가 가능하지만 이 용량과 단순 struct에서는 발생하지 않는다. 단일 스레드다.
        distance_stack.push_back(initial_frame);

        while (true) {
            // [호출 계약: vector<DistanceFrame>::empty, back, pop_back]
            // (1) 수신자는 유효한 distance_stack이고 back/pop은 empty가 false인 경로에서만 실행한다.
            // (2) bool empty() const noexcept, DistanceFrame& back(), void pop_back()을 선택한다.
            // (3) 데이터 인자는 없고 소유권 이전도 없다.
            // (4) bool은 분기, back 값은 state 복사에 쓰고 pop 반환값은 없다.
            // (5) pop은 마지막 frame을 파괴해 size를 줄이며 복사된 state는 독립 수명을 가진다.
            // (6) O(1)·무할당이다. 빈 back/pop UB는 검사로 배제하고 지운 원소/end 관찰자만 무효화된다.
            if (distance_stack.empty()) {
                break;
            }
            const DistanceFrame state{distance_stack.back()};
            distance_stack.pop_back();

            // 센트로이드에서 k보다 먼 정점은 거리 합 k에 참여할 수 없으므로 더 내려가지 않는다.
            if (state.depth > target_distance) {
                continue;
            }
            collected_depths.push_back(state.depth);

            if (state.depth == target_distance) {
                continue;
            }
            const std::vector<int>& neighbors{graph[static_cast<std::size_t>(state.node)]};
            const std::size_t child_neighbor_count{neighbors.size()};
            for (std::size_t index{}; index < child_neighbor_count; ++index) {
                const int neighbor{neighbors[index]};
                if (neighbor == state.parent
                    || removed[static_cast<std::size_t>(neighbor)] != static_cast<unsigned char>(0)) {
                    continue;
                }
                const DistanceFrame next_frame{neighbor, state.node, state.depth + 1};
                distance_stack.push_back(next_frame);
            }
        }

        const std::size_t depth_count{collected_depths.size()};
        for (std::size_t index{}; index < depth_count; ++index) {
            const int depth{collected_depths[index]};
            const int complement{target_distance - depth};
            // frequency[0]=1은 한 끝점이 centroid인 경로를, 양수 빈도는 앞선 다른 자식과의 경로를 센다.
            subtotal += static_cast<PathCount>(depth_frequency[static_cast<std::size_t>(complement)]);
        }
        for (std::size_t index{}; index < depth_count; ++index) {
            const int depth{collected_depths[index]};
            int& frequency{depth_frequency[static_cast<std::size_t>(depth)]};
            if (frequency == 0) {
                touched_depths.push_back(depth);
            }
            ++frequency;
        }
    }

    // 전체 k+1 배열을 매 단계 채우면 O(nk)가 될 수 있다. 실제로 건드린 깊이만 0으로 되돌려 모든 단계의
    // 초기화 비용도 방문한 정점 수에 비례하게 유지한다.
    const std::size_t touched_count{touched_depths.size()};
    for (std::size_t index{}; index < touched_count; ++index) {
        const int depth{touched_depths[index]};
        depth_frequency[static_cast<std::size_t>(depth)] = 0;
    }
    return subtotal;
}
