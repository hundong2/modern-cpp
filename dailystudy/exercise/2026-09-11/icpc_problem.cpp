/*
문제: CSES 1737 - Range Queries and Copies
출처: https://cses.fi/problemset/task/1737/

문제 요약
- 처음에는 정수 배열의 version 1 하나가 있다.
- type 1 `1 k a x`: version k의 a번째 값을 x로 바꾼다. 새 번호를 만들지 않고 version k의 현재 뿌리를 갱신한다.
- type 2 `2 k a b`: version k에서 a부터 b까지의 닫힌 구간 합을 출력한다.
- type 3 `3 k`: version k의 현재 내용을 복사한 새 version을 목록 끝에 추가한다.
- 복사본 하나를 바꿔도 다른 version의 값은 바뀌면 안 된다.

입력/출력
- 첫 줄 n, q 다음 줄에 n개 초기값이 주어진다. 이어서 q개 query가 위 세 형식 중 하나로 주어진다.
- type 2마다 답을 한 줄에 출력한다. 입력의 배열/버전/위치 번호는 모두 1부터 시작한다.

공식 제약
- 1 <= n,q <= 200,000
- 1 <= 초기값,x <= 1,000,000,000
- 모든 version 번호와 구간은 query 시점에 유효하다.
- 합은 int 범위를 넘을 수 있으므로 long long을 사용한다.

공식 예제
- 입력:
  5 6
  2 3 1 2 5
  3 1
  2 1 1 5
  2 2 1 5
  1 2 2 5
  2 1 1 5
  2 2 1 5
- 출력: 13, 13, 13, 15가 각각 한 줄에 나온다.

우승에 필요한 핵심
- 배열 전체를 복사하면 type 3마다 O(n)이라 최악 O(nq)다. 균형 이진 구간 트리의 root만 공유하고,
  point update가 지나가는 O(log n) 노드만 복제(path copying)하면 복사는 O(1), 갱신/질의는 O(log n)이다.
- 불변식: `nodes`의 해당 node가 가진 `sum`은 담당 구간의 합이고, 한 번 공개된 node는 절대 수정하지 않는다.
  update는 먼저 현재 node를 복사하고 새 경로에서만 자식 index/sum을 고친다. 그래서 다른 root가 공유하는 부분은 보존된다.
- version은 node pointer 대신 int root index를 가진다. vector가 성장하며 실제 주소를 바꿔도 index는 유지되므로
  재할당을 가로질러 Node&/Node*를 저장하는 위험이 없다.
- 초기 build O(n), update U개 총 O(U log n), query O(log n), copy O(1), 메모리 O(n+U log n+C)다.
- 원문 전체를 복제하지 않고 풀이에 필요한 의미·제약·예제를 한국어로 다시 설명했다.
*/

// <cstddef>는 컨테이너 크기와 예약량을 표현하는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 빠른 입출력 설정과 std::cin/std::cout 스트림 연산을 선언한다.
#include <iostream>
// <vector>는 값 배열, version root 목록, 영속 node pool을 연속 저장하는 std::vector를 선언한다.
#include <vector>

// struct와 class는 같은 사용자 정의 타입 문법이며 기본 접근만 다르다. struct는 기본 public이라
// 합·자식 인덱스를 숨길 불변식이 없는 단순 Node 레코드에 쓰고, 아래 class는 기본 private라 arena를 감춘다.
struct Node {
    // 구간 합은 최대 200,000 * 1,000,000,000이므로 최소 64비트 폭의 long long을 쓴다.
    long long sum;
    // 실제 포인터 대신 pool index를 쓴다. leaf의 자식 -1은 접근하지 않는 sentinel이다.
    int left_child;
    int right_child;
};

// using은 새 타입을 만들지 않는 별칭이다. vector의 템플릿 인자 long long/Node가 원소 타입,
// 정렬·수명·파괴 규칙을 정하고 별칭은 반복되는 긴 표기를 줄인다.
using Values = std::vector<long long>;
using NodePool = std::vector<Node>;

// 값 범주·복사·이동·수명 관점:
// - 이름 있는 values/nodes_/next는 lvalue이고 `Node{...}`는 Node prvalue다. next 초기화는 기존 Node의
//   세 scalar 필드를 값 복사하며, 갱신된 next를 append할 때도 작은 Node 값 하나만 복사한다.
// - pool vector가 재할당하면서 내부 Node를 이동/복사할 수 있지만 root와 child는 주소가 아닌 int index라
//   그 이동 뒤에도 의미가 유지된다. Node&/Node*를 재귀 호출 너머 보관하지 않는 것이 수명 안전의 핵심이다.
// - std::move로 기존 version을 빼앗을 필요가 없다. 영속 구조의 목적은 기존 Node를 공유·보존하는 것이다.
// - build/assign은 int, range_sum은 long long scalar prvalue를 반환하므로 무거운 class 반환과 RVO가 필요 없다.
//   일반적으로 같은 class prvalue 반환은 결과 객체에 직접 구성될 수 있지만 여기서는 index 값만 전달한다.

// 공용 알고리즘 문서: ../algorithm/persistent-segment-tree-path-copying.md
class PersistentSegmentTree {
public:
    // public 접근 지정자 아래 API만 main이 사용할 수 있고, 재귀 구현과 node arena에는 직접 접근할 수 없다.
    // explicit은 크기 정수가 실수로 tree 객체로 암시 변환되는 것을 막는다.
    // [생성/호출 계약: NodePool 기본 생성과 vector::reserve(expected_nodes)]
    // (1) 목적 nodes_는 아직 없고 expected_nodes는 공식 n,q에서 계산한 양수 std::size_t 값이다.
    // (2) nodes_에는 vector() 기본 생성자를, 본문에는 void reserve(size_type new_cap)를 선택한다.
    // (3) 기본 생성은 외부 인자가 없고 reserve 인자는 예약할 Node 개수 값이다. Node 소유권 입력은 없다.
    // (4) 두 연산 모두 반환값이 없다. 성공하면 nodes_.size()==0, capacity()>=expected_nodes다.
    // (5) 기존 원소는 없고 논리 size는 그대로 0이며 이후 공식 최악 update 수까지 재할당을 피한다.
    // (6) 기본 생성 O(1), reserve는 현재 size 0에서 저장소 크기만큼 할당할 수 있다. length_error 또는
    //     bad_alloc 시 생성이 실패하고 자원은 정리된다. 기존 참조는 없으며 thread 보장을 새로 만들지 않는다.
    explicit PersistentSegmentTree(const std::size_t expected_nodes) {
        nodes_.reserve(expected_nodes);
    }

    [[nodiscard]] int build(const Values& values, const int n) {
        // n>=1은 공식 입력 전제다. 함수 반환 int는 새로 append한 root의 pool index다.
        return build_impl(values, 0, n - 1);
    }

    [[nodiscard]] int assign(
        const int old_root,
        const int n,
        const int position,
        const long long value) {
        return assign_impl(old_root, 0, n - 1, position, value);
    }

    [[nodiscard]] long long range_sum(
        const int root,
        const int n,
        const int query_left,
        const int query_right) const {
        return range_sum_impl(root, 0, n - 1, query_left, query_right);
    }

private:
    // private 접근 지정자는 append-only arena와 재귀 불변식을 class 내부에 가둔다.
    [[nodiscard]] int append_node(const Node node) {
        // [호출 계약: vector<Node>::push_back(const Node&)]
        // (1) 수신 nodes_는 지금까지 만든 모든 immutable Node를 소유하는 유효한 NodePool lvalue이고,
        //     node는 완전히 초기화된 지역 const Node lvalue다. size는 공식 상한에서 int로 표현 가능하다.
        // (2) void push_back(const value_type&) overload가 선택되고 value_type=Node다.
        // (3) node를 const 참조로 빌려 새 마지막 원소에 값 복사한다. node나 기존 node 소유권은 이동하지 않는다.
        // (4) 반환형 void라 값은 없다. 이어지는 size()로 새 index를 계산한다.
        // (5) size가 1 증가하고 새 back은 node와 같은 세 필드다. 예약이 부족하면 capacity와 주소가 바뀔 수
        //     있지만 알고리즘은 index만 보관하므로 정확성에 영향이 없다.
        // (6) 여유 capacity가 있으면 상각 O(1)이고 기존 원소 관찰자는 유지되지만 옛 past-the-end iterator는
        //     무효화된다. 재할당 시 기존 size에 선형이고 모든 pointer/reference/iterator가 무효화되며
        //     bad_alloc/length_error가 가능하다. 한 실행 흐름만 pool을 수정한다.
        // 대표 문서: ../standard-library/containers-and-views.md
        nodes_.push_back(node);

        // [호출 계약: const vector<Node>::size()]
        // (1) nodes_는 방금 한 원소가 추가된 비어 있지 않은 NodePool lvalue다.
        // (2) size_type size() const noexcept가 선택되며 명시 인자는 없다.
        // (3) 숨은 const this만 읽고 원소 소유권을 건드리지 않는다.
        // (4) 현재 원소 수 size_type prvalue를 반환하며 1을 빼 새 원소의 0-based index로 쓴다.
        // (5) pool의 size/capacity/원소와 관찰자는 그대로다.
        // (6) O(1), 무할당·비무효화·noexcept다. 공식 상한상 size-1은 int로 안전하게 변환된다.
        return static_cast<int>(nodes_.size() - 1U);
    }

    [[nodiscard]] int build_impl(const Values& values, const int segment_left, const int segment_right) {
        if (segment_left == segment_right) {
            // [호출 계약: const vector<long long>::operator[]]
            // (1) 수신 values는 n개 초기값을 소유하는 const Values lvalue이고 segment_left는 [0,n)이다.
            // (2) const_reference operator[](size_type position) const overload가 선택된다.
            // (3) non-negative int를 size_type 값으로 변환하며 컨테이너를 읽기만 한다.
            // (4) const long long&를 반환해 Node.sum 값 초기화에 즉시 복사하고 참조를 저장하지 않는다.
            // (5) values의 원소·size·capacity는 그대로다.
            // (6) O(1), 무할당·비무효화다. []는 범위 검사 없이 범위 밖에서 미정의 동작이지만 재귀 구간
            //     불변식이 유효 index를 보장한다. 입력 vector에 동시 쓰기는 없다.
            // 대표 문서: ../standard-library/containers-and-views.md
            return append_node(Node{values[static_cast<std::size_t>(segment_left)], -1, -1});
        }

        // 오버플로를 피하는 중간점 식이다. 공식 범위에서는 단순 합도 안전하지만 이 관용구를 습관화한다.
        const int middle{segment_left + (segment_right - segment_left) / 2};
        const int left_index{build_impl(values, segment_left, middle)};
        const int right_index{build_impl(values, middle + 1, segment_right)};

        // [호출 계약: vector<Node>::operator[] 읽기]
        // (1) nodes_는 두 자식 index를 포함해 완성된 Node들을 소유하고 left_index/right_index는 유효하다.
        // (2) 비-const 수신의 reference operator[](size_type) overload가 각 호출에 선택된다.
        // (3) int index를 size_type 값으로 변환하고 해당 Node를 빌려 sum만 읽는다. 소유권 이동은 없다.
        // (4) Node& 반환에서 long long 값을 즉시 읽어 부모 합에 사용하며 참조를 재귀/append 너머 저장하지 않는다.
        // (5) pool과 자식 node는 바뀌지 않는다.
        // (6) 각 O(1), 무할당·비무효화다. 범위 밖은 미정의 동작이나 append 반환 index 불변식이 막는다.
        //     이후 append가 재할당할 수 있으므로 이 식을 마친 뒤 Node&를 보관하지 않는 것이 핵심이다.
        const long long sum{nodes_[static_cast<std::size_t>(left_index)].sum +
                            nodes_[static_cast<std::size_t>(right_index)].sum};
        return append_node(Node{sum, left_index, right_index});
    }

    [[nodiscard]] int assign_impl(
        const int old_index,
        const int segment_left,
        const int segment_right,
        const int position,
        const long long value) {
        // old Node를 지역 값 next로 먼저 복사한다. 재귀가 끝나기 전에는 pool에 넣지 않으므로 pool에
        // 공개된 모든 Node는 실제 코드에서도 immutable이다. 지역 값은 vector 재할당과 수명이 독립적이다.
        Node next{nodes_[static_cast<std::size_t>(old_index)]};

        if (segment_left == segment_right) {
            next.sum = value;
            return append_node(next);
        }

        const int middle{segment_left + (segment_right - segment_left) / 2};
        if (position <= middle) {
            // 자식 index는 int 값이다. 재귀 중 pool 재할당이 생겨도 지역 next와 값 index는 안전하다.
            next.left_child = assign_impl(next.left_child, segment_left, middle, position, value);
        } else {
            next.right_child = assign_impl(next.right_child, middle + 1, segment_right, position, value);
        }

        next.sum = nodes_[static_cast<std::size_t>(next.left_child)].sum +
                   nodes_[static_cast<std::size_t>(next.right_child)].sum;
        // 자식들이 먼저 완성된 뒤 부모 next를 딱 한 번 append한다. 따라서 갱신당 높이+1개만 추가된다.
        return append_node(next);
    }

    [[nodiscard]] long long range_sum_impl(
        const int node_index,
        const int segment_left,
        const int segment_right,
        const int query_left,
        const int query_right) const {
        // 완전히 포함된 구간은 저장한 합을 즉시 반환한다.
        if (query_left <= segment_left && segment_right <= query_right) {
            return nodes_[static_cast<std::size_t>(node_index)].sum;
        }

        const int middle{segment_left + (segment_right - segment_left) / 2};
        long long answer{0};
        if (query_left <= middle) {
            const int left_index{nodes_[static_cast<std::size_t>(node_index)].left_child};
            answer += range_sum_impl(left_index, segment_left, middle, query_left, query_right);
        }
        if (middle < query_right) {
            const int right_index{nodes_[static_cast<std::size_t>(node_index)].right_child};
            answer += range_sum_impl(right_index, middle + 1, segment_right, query_left, query_right);
        }
        return answer;
    }

    // NodePool의 vector 기본 생성 계약은 생성자 주석에 설명했다. 모든 공개 root가 가리키는 Node는
    // immutable이며 assign이 새 경로의 Node만 바꾼다는 규칙이 version 격리의 핵심이다.
    NodePool nodes_{};
};

int main() {
    // [호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 수신 없는 static 설정 함수이며 표준 스트림에 아직 I/O를 하지 않은 프로그램 시작 상태다.
    // (2) static bool sync_with_stdio(bool sync=true)에 false bool prvalue를 준다.
    // (3) false는 C/C++ 표준 스트림 동기화를 끄라는 값이며 버퍼 소유권이 아니다.
    // (4) 이전 동기화 bool을 반환하지만 복구하지 않아 의도적으로 버린다.
    // (5) 이후 C stdio와 C++ iostream 혼용 순서가 자동 보장되지 않을 수 있다.
    // (6) 점근 복잡도는 표준이 정하지 않고 첫 I/O 뒤 효과는 구현별이다. 시작 때 한 번 호출하며 동일
    //     설정을 여러 스레드가 동시에 바꾸지 않는다. 스트림 수명은 프로그램 전체다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::ios::sync_with_stdio(false);

    // [호출 계약: std::cin.tie(nullptr)]
    // (1) 수신자는 살아 있는 전역 std::cin istream lvalue이고 ostream에 tie돼 있을 수 있다.
    // (2) std::ostream* tie(std::ostream*) overload에 nullptr 하나를 준다.
    // (3) nullptr은 자동 flush 대상이 없다는 값이며 cout 소유권/수명을 넘기지 않는다.
    // (4) 이전 tied ostream*를 반환하지만 사용하지 않아 버린다.
    // (5) 이후 입력 전 자동 flush가 없어지고 cin/cout 객체 자체는 유지된다.
    // (6) 표준은 점근 복잡도를 정하지 않으며 일반적으로 상수 시간 작업이다. nullptr은 허용되며 할당·반복자 무효화가
    //     없다. 대화형 문제가 아니므로 수동 flush가 필요 없고 설정을 동시에 변경하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cin.tie(nullptr);

    int n{0};
    int query_count{0};

    // [호출 계약: std::istream의 int/long long 추출 operator>>]
    // (1) 왼쪽 피연산자는 입력 가능한 std::cin istream lvalue이고 대상 int/long long lvalue는 초기화됐다.
    // (2) operator>>(int&) 및 operator>>(long long&) 산술 overload가 각 대상 타입에 맞게 선택된다.
    // (3) 대상을 non-const 참조로 빌려 공백 구분 십진수를 쓴다. 외부 자원 소유권은 옮기지 않는다.
    // (4) 각 호출은 같은 istream&를 반환해 연쇄 입력에 사용하고 마지막 반환은 검사하지 않고 버린다.
    // (5) 성공하면 대상 값과 입력 위치가 갱신된다. 실패하면 fail/eof 상태가 설정되고 대상 값은 표준의
    //     형식 추출 규칙을 따르지만 공식 입력은 항상 성공한다.
    // (6) 소비 문자 수에 선형이고 locale/streambuf 동작 및 설정된 ios_base::failure 예외가 가능하다.
    //     범위를 벗어난 숫자는 fail 상태가 된다. 같은 stream을 다른 스레드가 동시에 추출하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::cin >> n >> query_count;

    // [생성 계약: vector<long long> count 생성자]
    // (1) values는 아직 없고 n은 공식 범위 [1,200000]의 int다.
    // (2) explicit vector(size_type count, const Allocator& = Allocator()) overload에서 value_type=long long다.
    // (3) n을 std::size_t로 변환한 count를 값으로 전달하고 외부 원소/소유권 입력은 없다.
    // (4) 생성자 반환값은 없고 values가 n개의 value-initialized 0을 소유한다.
    // (5) size==n, capacity>=n이며 각 원소는 입력으로 덮어쓸 준비가 된다.
    // (6) 시간·공간 O(n), 연속 저장소 할당 가능, length_error/bad_alloc 가능이다. 생성 실패 시 자원이
    //     정리되고 새 컨테이너라 기존 관찰자 무효화는 없다.
    // 대표 문서: ../standard-library/containers-and-views.md
    Values values(static_cast<std::size_t>(n));
    // [호출 계약: range-for가 숨겨 호출하는 vector::begin/end와 iterator 연산]
    // (1) 수신 values는 n개의 value-initialized long long을 소유하는 Values lvalue이고 구조 변경은 없다.
    // (2) iterator begin() noexcept/end() noexcept, 같은 vector iterator 비교, operator*, 전위
    //     operator++가 range-for 전개에서 선택된다.
    // (3) 명시 인자는 없고 values를 숨은 this로 빌린다. 역참조 결과 long long&는 추출 연산의 출력
    //     매개변수로 쓰며 iterator나 원소 소유권을 저장소 밖으로 옮기지 않는다.
    // (4) begin/end는 iterator 값, 비교는 bool 문맥에서 검사할 결과, 역참조는 long long&, 전위 증가는
    //     iterator&를 반환해 loop 경계·조건·value 바인딩·다음 위치 계산에 모두 사용된다.
    // (5) size/capacity와 iterator는 유지되고 성공한 각 입력마다 해당 원소 값만 0에서 입력값으로 바뀐다.
    // (6) 각 숨은 연산 O(1), 전체 O(n), 무할당·비무효화다. 과거-끝 역참조나 무효 iterator 사용은
    //     미정의 동작이며, 같은 values를 다른 스레드가 무동기 접근하지 않는다. 입력 실패 계약은 아래
    //     반복 적용하는 istream 추출 계약을 따른다.
    // 대표 문서: ../standard-library/containers-and-views.md
    for (long long& value : values) {
        // 위 istream 산술 추출 계약을 반복 적용한다.
        std::cin >> value;
    }

    // 높이는 19 node 이하이지만 query마다 24개를 잡아 충분한 여유를 둔다. 2*n은 build node 상한보다 크다.
    // std::size_t로 먼저 변환해 int 곱셈 overflow를 피한다.
    const std::size_t expected_nodes{
        2U * static_cast<std::size_t>(n) +
        24U * static_cast<std::size_t>(query_count) + 1U};
    PersistentSegmentTree tree{expected_nodes};
    const int initial_root{tree.build(values, n)};

    // [생성/호출 계약: vector<int> 기본 생성과 reserve]
    // (1) roots는 아직 없는 목적 vector이고 query_count+1은 가능한 최대 version 수 이상의 유효 크기다.
    // (2) vector() 기본 생성 후 void reserve(size_type)를 선택하며 value_type=int다.
    // (3) reserve 인자는 std::size_t 값으로 전달되고 root 원소 소유권 입력은 없다.
    // (4) 반환값은 없고 빈 roots가 최소 query_count+1 capacity를 확보한다.
    // (5) size는 0이며 이후 push_back에서 공식 query 수 안에서는 재할당하지 않는다.
    // (6) 기본 생성 O(1), reserve 할당 비용 가능, length_error/bad_alloc 가능이다. 기존 관찰자는 없다.
    std::vector<int> roots{};
    roots.reserve(static_cast<std::size_t>(query_count) + 1U);

    // [호출 계약: vector<int>::push_back(const int&)]
    // (1) roots는 빈 vector이고 initial_root는 유효한 pool index int lvalue다.
    // (2) void push_back(const value_type&)에서 value_type=int를 선택한다.
    // (3) initial_root 값을 const 참조로 읽어 새 원소에 복사하며 tree node 소유권은 roots로 옮기지 않는다.
    // (4) 반환형 void이고 값은 버릴 것이 없다.
    // (5) roots.size가 1이 되고 roots[0]이 version 1의 root가 된다.
    // (6) 예약 capacity 안에서 O(1)·무할당이고 기존 원소 관찰자는 유지되지만 옛 past-the-end iterator는
    //     무효화된다. 일반적으로 재할당 시 모든 관찰자가 무효화되고 bad_alloc 가능하지만 여기서는
    //     reserve 상한 안이다. 단일 흐름만 수정한다.
    roots.push_back(initial_root);

    for (int query_index{0}; query_index < query_count; ++query_index) {
        int type{0};
        std::cin >> type;

        if (type == 1) {
            int version{0};
            int position{0};
            long long value{0};
            std::cin >> version >> position >> value;
            const std::size_t root_slot{static_cast<std::size_t>(version - 1)};

            // [호출 계약: vector<int>::operator[] 읽기/쓰기]
            // (1) roots는 현재 모든 version root를 소유하고 root_slot은 공식 입력이 보장하는 [0,size)다.
            // (2) 비-const reference operator[](size_type) overload가 선택된다.
            // (3) root_slot 값을 전달해 int 원소를 빌리며 별도 소유권 이동은 없다.
            // (4) int&를 반환한다. 기존 root 값은 assign 인자로 먼저 읽고 새 root를 같은 원소에 대입한다.
            // (5) 해당 version의 root index만 바뀌고 다른 version root와 공유 old Node는 그대로다.
            // (6) O(1), 무할당·비무효화다. 범위 밖이면 미정의 동작이지만 입력 전제가 막는다. 같은 roots를
            //     동시에 수정하지 않는다. 대입은 int라 예외를 던지지 않는다.
            const int old_root{roots[root_slot]};
            roots[root_slot] = tree.assign(old_root, n, position - 1, value);
        } else if (type == 2) {
            int version{0};
            int query_left{0};
            int query_right{0};
            std::cin >> version >> query_left >> query_right;
            const int root{roots[static_cast<std::size_t>(version - 1)]};
            const long long answer{tree.range_sum(root, n, query_left - 1, query_right - 1)};

            // [호출 계약: std::ostream long long/char 삽입 operator<<]
            // (1) 왼쪽 수신 std::cout은 출력 가능한 ostream lvalue이고 answer는 초기화된 long long lvalue다.
            // (2) 멤버/비멤버 산술 long long 삽입 overload 뒤 operator<<(ostream&, char)를 선택한다.
            // (3) answer와 '\n' 값을 복사해 형식화하고 tree/version 소유권은 옮기지 않는다.
            // (4) 각 호출은 같은 ostream&를 반환해 연쇄하고 마지막 반환은 버린다.
            // (5) 십진 합과 개행이 출력 버퍼에 추가되며 알고리즘 상태는 바뀌지 않는다.
            // (6) 출력 문자 수에 선형이고 stream 상태 비트/버퍼 오류/설정 예외가 가능하다. node 관찰자를
            //     무효화하지 않고 동일 stream에 동시 무동기 쓰기를 하지 않는다.
            // 대표 문서: ../standard-library/io-parsing-and-utilities.md
            std::cout << answer << '\n';
        } else {
            int version{0};
            std::cin >> version;
            // type 3은 root index 하나만 복사하므로 O(1)이다. 두 version은 immutable 하위 tree를 공유한다.
            // push 전에 값을 지역 변수로 복사해 vector 내부 참조를 삽입 인자로 넘기지 않는다.
            const int copied_root{roots[static_cast<std::size_t>(version - 1)]};
            roots.push_back(copied_root);
        }
    }

    // 기계 실행 관점: query/update는 node index로 연속 pool에서 sum/child를 load하고 구간 비교 뒤 조건
    // 분기하며, update는 O(log n) 새 Node를 store한다. type 3은 root int 하나의 load/store다. 실제 cache
    // miss, 분기 예측, inlining과 명령 배치는 CPU·allocator·컴파일러·최적화 옵션에 따라 달라지므로 특정
    // 어셈블리나 정확한 cycle 수로 단정하지 않는다. 재귀 깊이는 배열 길이가 아니라 O(log n)이다.
    return 0;
}
