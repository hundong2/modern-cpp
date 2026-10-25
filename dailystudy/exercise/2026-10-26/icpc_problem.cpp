/*
문제 요약 — CSES 2074 "Reversals and Sums"

길이 n의 정수 배열에 m개 연산을 순서대로 적용한다.
1 a b: 1-based 닫힌 구간 [a,b]의 원소 순서를 뒤집는다.
2 a b: 같은 구간의 원소 합을 출력한다.

입력
- 첫 줄: n, m
- 둘째 줄: x_1 ... x_n
- 다음 m줄: t, a, b

출력
- t=2인 연산마다 현재 배열의 [a,b] 합을 한 줄에 출력한다.

제약
- 1 <= n <= 200,000, 1 <= m <= 100,000
- 0 <= x_i <= 1,000,000,000
- 1 <= a <= b <= n
- 합은 최대 2*10^14이므로 64비트 부호 있는 정수로 저장한다.

예제
입력:
8 3
2 1 3 4 5 3 4 4
2 2 4
1 3 6
2 2 4

출력:
8
9

핵심은 배열의 "위치"를 key로 쓰는 암시적 트립이다. subtree 크기가 곧 순서 통계가 되므로
split(root,k)은 앞 k개와 나머지를 기대 O(log n)에 나눈다. reverse는 구간 subtree의 두 자식을
맞바꾸고 lazy flag만 토글하며, sum은 subtree aggregate를 읽는다.

공용 문서: ../algorithm/implicit-treap-range-reversal-sum.md
*/

// <chrono>는 입력과 독립적인 실행 시점 seed를 섞을 단조 시계를 선언한다.
#include <chrono>
// <cstddef>는 vector 크기/인덱스 변환에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 std::int64_t와 std::uint64_t 고정 폭 정수 별칭을 선언한다.
#include <cstdint>
// <iostream>은 std::cin/std::cout과 stream 연산을 선언한다.
#include <iostream>
// <random>은 운영체제 entropy를 요청할 수 있는 std::random_device를 선언한다.
#include <random>
// <utility>는 std::pair와 std::swap을 선언한다.
#include <utility>
// <vector>는 노드를 한 arena에 연속 소유하는 std::vector를 선언한다.
#include <vector>

using Value = std::int64_t;
using Priority = std::uint64_t;

// 배열 index가 tree link 역할을 한다. -1은 자식이 없음을 뜻한다.
constexpr int kNull{-1};

// SplitMix64는 64비트 state를 priority로 고르게 섞는 함수다. 암호학적 난수는 아니다.
[[nodiscard]] constexpr Priority mix_priority(Priority value) noexcept {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31U);
}

// 제출 실행마다 입력에 없는 seed를 만든다. test 작성자가 priority 순서를 미리 알고 reverse로
// 선형 사슬을 구성하지 못하게 하는 것이 목적이며, 비밀번호용 entropy를 제공하는 API는 아니다.
[[nodiscard]] Priority make_priority_seed() noexcept {
    Priority entropy_bits{};
    try {
        // [첫 생성/호출 계약: std::random_device()와 operator()]
        // (1) entropy_source 목적 객체는 생성 전이고, 호출 시에는 유효한 local random_device lvalue다.
        // (2) `random_device()` 기본 생성자와 `result_type operator()()`가 선택된다. result_type은
        //     unsigned int이며 여기서는 두 번 얻은 반환 표본을 받는다. 두 값이 같을 수도 있다.
        // (3) 생성자와 호출 모두 명시 인자가 없다. 장치/구현 상태를 빌릴 뿐 C++ 객체 소유권을
        //     넘기지 않으며, 각 unsigned int prvalue를 소유 정수 Priority 값으로 확대한다.
        // (4) 생성자는 별도 반환값 없이 source 수명을 시작하고, 두 operator() 반환값은 seed 상·하위
        //     bit를 섞는 데 사용한다.
        // (5) 각 호출 뒤 source 내부/외부 entropy 상태는 구현 방식대로 전진할 수 있고, 반환 정수는
        //     source 객체 수명과 독립적으로 보관 가능한 값이다. source는 block 끝에서 파괴되며
        //     treap이 장치를 보관하지 않는다.
        // (6) 비용·blocking·실제 entropy·내부 할당은 구현/OS 의존이다. 생성/호출은 실패 예외가
        //     가능해 catch 뒤 clock fallback을 쓴다. local 객체라 같은 객체의 thread 경쟁은 없다.
        std::random_device entropy_source{};
        entropy_bits = (static_cast<Priority>(entropy_source()) << 32U) ^
                       static_cast<Priority>(entropy_source());
    } catch (...) {
        // random_device가 없는 구현에서도 제출을 중단하지 않고 아래 실행 시점 seed를 사용한다.
    }

    // [첫 호출 계약: steady_clock::now().time_since_epoch().count()]
    // (1) now는 수신 객체 없는 static 호출이다. 이어지는 수신자는 살아 있는 time_point prvalue와
    //     그 time_point가 값으로 반환한 duration prvalue다.
    // (2) `static steady_clock::time_point now() noexcept`, `constexpr duration
    //     time_point::time_since_epoch() const`, `constexpr rep duration::count() const`를 차례로 고른다.
    // (3) 세 호출 모두 명시 인자가 없고 임시 객체를 읽기만 하며 소유권을 넘기지 않는다.
    // (4) 마지막 duration::rep 산술값을 Priority로 변환해 seed에 사용한다. 중간 두 값도 버리지 않고
    //     다음 호출의 수신자로 즉시 소비한다.
    // (5) 프로그램 객체는 변하지 않고 임시 time_point/duration은 full-expression 끝에 파괴된다.
    //     steady clock은 뒤로 가지 않지만 epoch와 tick 단위의 구체 값은 구현에 맡긴다.
    // (6) 표준은 점근 비용·할당 방식을 지정하지 않고 now는 명시적으로 noexcept다. 뒤 두 선언에는
    //     noexcept가 없어 언어의 `noexcept(식)` 판정과 구분해야 하지만, steady_clock이 만족하는
    //     TrivialClock의 관련 시간 값 연산은 실제 예외를 던지지 않는다. clock 값은 암호학적 난수가
    //     아니며 해상도도 구현 의존이다. 이 조회는 공유 객체 데이터 경쟁을 만들지 않지만 thread 사이
    //     사건을 게시하는 동기화 수단은 아니다.
    // using은 새 clock 타입을 만들지 않고 긴 표준 타입에 지역 별칭만 붙인다.
    using SeedClock = std::chrono::steady_clock;
    const Priority clock_bits{
        static_cast<Priority>(SeedClock::now().time_since_epoch().count())};

    return mix_priority(entropy_bits ^ clock_bits ^ 0x243f6a8885a308d3ULL);
}

struct Node {
    Value value{};       // 이 위치의 원소 값이다.
    Value sum{};         // 현재 subtree의 합이다.
    Priority priority{}; // heap 불변식을 정하는 pseudo-random priority다.
    int left{kNull};
    int right{kNull};
    int size{1};         // 현재 subtree 원소 수다.
    int id{};            // priority 충돌 때도 전순서를 만드는 고유 삽입 번호다.
    bool reversed{};     // 이 subtree에 아직 자식으로 내리지 않은 reverse lazy tag다.
};

class ImplicitTreap {
private:
    // [첫 생성 계약: std::vector<Node>() 기본 생성자]
    // (1) nodes_ 목적 객체는 생성 전이며 Node 원소가 없다.
    // (2) `vector()` 기본 생성자가 기본 allocator로 선택된다.
    // (3) 데이터 인자는 없고 allocator 기본 객체만 내부적으로 사용한다.
    // (4) 생성자는 반환값 없이 빈 vector의 수명을 시작한다.
    // (5) size=0인 유효 owner가 되고 capacity의 구체 값은 구현에 맡기며 root_/next_id_와 독립이다.
    // (6) 상수 시간·통상 무할당·조건부 noexcept다. 이후 재할당은 기존 Node reference를
    //     무효화하고, vector는 동기화를 제공하지 않는다.
    std::vector<Node> nodes_{};
    int root_{kNull};
    int next_id_{};
    Priority priority_state_{};

    // 입력과 독립적으로 seed된 state를 node마다 전진시킨 뒤 SplitMix64로 섞는다.
    [[nodiscard]] Priority next_priority() noexcept {
        priority_state_ += 0x9e3779b97f4a7c15ULL;
        return mix_priority(priority_state_);
    }

    // index는 반드시 kNull이 아니고 nodes_의 살아 있는 원소 범위 안이어야 한다.
    [[nodiscard]] Node& node(int index) noexcept {
        // [첫 호출 계약: vector<Node>::operator[](size_type)]
        // (1) 수신 객체 nodes_는 reserve 뒤 생성된 Node들을 소유하며 index는 [0,next_id_)의 유효 int다.
        // (2) 수정 가능한 `reference operator[](size_type)`가 선택된다.
        // (3) 유일한 인자 index를 std::size_t prvalue로 명시 변환해 값으로 전달한다. 소유권 이동은 없다.
        // (4) 반환 Node&를 읽기/갱신에 사용한다.
        // (5) vector 크기·용량은 그대로이며 반환 reference는 해당 원소를 alias한다.
        // (6) O(1)·무할당이고 범위 검사를 하지 않는다. 표준 선언 자체에 noexcept가 명시된
        //     overload는 아니며, 전제조건을 만족한 built-in reference 접근은 예외를 만들지 않는다.
        //     범위 전제조건 위반은 UB다.
        //     이후 vector 재할당/파괴 또는 같은 원소의 무동기 쓰기와 겹치면 사용할 수 없다.
        return nodes_[static_cast<std::size_t>(index)];
    }

    [[nodiscard]] const Node& node(int index) const noexcept {
        // [첫 호출 계약: const vector<Node>::operator[](size_type)]
        // (1) 수신 객체 nodes_는 살아 있는 const 관점의 vector<Node>이고 index는 유효 범위다.
        // (2) `const_reference operator[](size_type) const` overload가 선택된다.
        // (3) 유일한 인자는 명시 변환된 std::size_t prvalue이며 소유권을 전달하지 않는다.
        // (4) 반환 const Node&를 aggregate 관찰에 사용하고 저장하지 않는다.
        // (5) vector와 Node는 바뀌지 않으며 reference 수명은 원소/owner와 재할당 규칙에 묶인다.
        // (6) O(1)·무할당·무검사다. 표준 signature 자체의 noexcept를 가정하지 않으며 범위 위반은
        //     UB, 같은 Node의 무동기 쓰기와 겹친 읽기는 데이터 경쟁이다.
        return nodes_[static_cast<std::size_t>(index)];
    }

    [[nodiscard]] int subtree_size(int index) const noexcept {
        return index == kNull ? 0 : node(index).size;
    }

    [[nodiscard]] Value subtree_sum(int index) const noexcept {
        return index == kNull ? Value{0} : node(index).sum;
    }

    // 서로 다른 id를 tie-breaker로 써 priority 충돌에도 엄격한 heap 순서를 만든다.
    [[nodiscard]] bool higher(int left, int right) const noexcept {
        if (node(left).priority != node(right).priority) {
            return node(left).priority > node(right).priority;
        }
        return node(left).id > node(right).id;
    }

    void pull(int index) noexcept {
        Node& current{node(index)};
        current.size = 1 + subtree_size(current.left) + subtree_size(current.right);
        current.sum = current.value + subtree_sum(current.left) + subtree_sum(current.right);
    }

    void apply_reverse(int index) noexcept {
        if (index == kNull) {
            return;
        }
        Node& current{node(index)};

        // [첫 호출 계약: std::swap(current.left, current.right)]
        // (1) 두 수신 객체 없는 인자는 같은 Node 안의 유효한 서로 다른 int lvalue다.
        // (2) `template<class T> constexpr void swap(T&,T&)`에서 T=int가 선택된다.
        // (3) 두 non-const lvalue reference는 자식 index 값을 교환하며 소유 자원은 없다.
        // (4) 반환형 void라 결과값은 없다.
        // (5) left/right 값만 바뀌고 subtree 원소 집합·size·sum은 그대로다.
        // (6) O(1)·무할당·noexcept다. 같은 Node에 대한 병행 접근은 없으며 유효 int 객체가
        //     필요하다. 실제 load/store 수는 컴파일러·CPU·최적화에 따라 달라진다.
        std::swap(current.left, current.right);
        current.reversed = !current.reversed;
    }

    // lazy tag를 자식에 합성한 뒤 현재 node의 tag를 지운다. reverse 두 번은 항등이다.
    void push(int index) noexcept {
        if (index == kNull || !node(index).reversed) {
            return;
        }
        const int left{node(index).left};
        const int right{node(index).right};
        apply_reverse(left);
        apply_reverse(right);
        node(index).reversed = false;
    }

    // 앞 left_count개가 first, 나머지가 second가 되도록 순서를 보존해 나눈다.
    [[nodiscard]] std::pair<int, int> split(int index, int left_count) noexcept {
        if (index == kNull) {
            // [첫 생성 계약: std::pair<int,int>(int,int)]
            // (1) 반환 pair 목적 객체는 생성 전이고 두 null root 값은 유효한 int prvalue다.
            // (2) 조건부 explicit인 `template<class U1=T1, class U2=T2> constexpr
            //     pair(U1&&,U2&&)`에서 T1=T2=U1=U2=int인 forwarding overload가 선택된다.
            // (3) 두 인자는 각각 kNull int prvalue이며 tree/node 소유권을 전달하지 않는다.
            // (4) 생성자는 별도 반환값 없이 pair prvalue를 만들고 split 반환값으로 사용한다.
            // (5) 결과 first/second가 모두 -1이고 arena와 입력 인자 상태는 그대로다.
            // (6) O(1)·무할당·int 복사 무예외다. 이후 `auto [first,second]` structured binding은
            //     pair의 tuple_size/tuple_element/get 프로토콜로 두 int 값을 이름에 바인딩해 쓰며
            //     Node 자체를 복사하거나 node 수명을 늘리지 않는다. 병행 상태는 없다.
            return {int{kNull}, int{kNull}};
        }

        push(index);
        const int left{node(index).left};
        const int left_size{subtree_size(left)};
        if (left_count <= left_size) {
            // [첫 숨은 호출 계약: pair structured binding의 std::get<0/1>]
            // (1) split 반환 pair<int,int> prvalue로 수명이 시작된 숨은 pair 객체 e가 유효하고, e의
            //     first/second int subobject도 살아 있다.
            // (2) tuple_size/tuple_element가 두 원소를 정적으로 정한 뒤 각각
            //     `get<0>(pair<int,int>&&)`와 `get<1>(pair<int,int>&&)` overload가 선택된다.
            // (3) 소스에 호출이 직접 적히지는 않지만 각 숨은 free-function 호출의 유일한 인자 식 e는
            //     pair<int,int> xvalue다. int에는 소유 자원이 없으며 pair 전체를 복사하지 않는다.
            // (4) 각 호출은 해당 int subobject의 int&&를 반환하고 first/second binding이 그 subobject에
            //     바인딩된다. 이름 식 first/second 자체는 이후 lvalue로 읽힌다.
            // (5) pair 값은 바뀌지 않는다. 숨은 e는 현재 block 끝까지 살아 binding을 유효하게 하고,
            //     반환 root 정수들은 그 뒤 tree index 값으로 복사되어 pair 수명에 의존하지 않는다.
            // (6) 각 get은 O(1)·무할당·noexcept다. 유효 pair subobject라는 전제에서 예외/무효화가 없고,
            //     local 숨은 객체를 한 thread에서만 읽으므로 데이터 경쟁이 없다.
            auto [first, second]{split(left, left_count)};
            node(index).left = second;
            pull(index);
            return {first, index};
        }

        const int right{node(index).right};
        auto [first, second]{split(right, left_count - left_size - 1)};
        node(index).right = first;
        pull(index);
        return {index, second};
    }

    // first의 모든 원소가 second보다 앞선다는 전제에서 heap priority로 한 tree를 만든다.
    [[nodiscard]] int merge(int first, int second) noexcept {
        if (first == kNull) {
            return second;
        }
        if (second == kNull) {
            return first;
        }

        push(first);
        push(second);
        if (higher(first, second)) {
            node(first).right = merge(node(first).right, second);
            pull(first);
            return first;
        }

        node(second).left = merge(first, node(second).left);
        pull(second);
        return second;
    }

    [[nodiscard]] int create_node(Value value) {
        const int id{next_id_};
        ++next_id_;

        // [첫 호출 계약: vector<Node>::push_back(Node&&)]
        // (1) 수신 객체 nodes_는 capacity>=최종 n으로 reserve된 유효 vector이고 size=id다.
        // (2) `void push_back(value_type&&)`가 Node aggregate prvalue에 선택된다.
        // (3) 유일한 인자 Node prvalue는 값/sum/priority/link/size/id/tag를 모두 소유한 값이며 vector가 이동
        //     생성해 보관한다. Node는 값 멤버뿐이라 실질적으로 비트 값을 복사한다.
        // (4) 반환형 void로 별도 값은 없다.
        // (5) size가 1 증가하고 새 원소 index는 id다. 사전 reserve 덕분에 오늘 호출은 재할당하지
        //     않으므로 기존 reference가 유지된다.
        // (6) 오늘 capacity 안에서는 상각 O(1), 추가 할당 없음이다. 재할당이 없으면 기존 원소의
        //     iterator/reference는 유지되지만 과거 past-the-end iterator는 무효다. 일반적으로 capacity
        //     초과 시 O(N) 재할당/bad_alloc과 모든 iterator/reference 무효화가 가능하다. Node의 무예외
        //     이동 조건에서 예외 시 vector는 원래 상태를 보존하며 단일 thread에서만 수정한다.
        nodes_.push_back(Node{
            .value = value,
            .sum = value,
            .priority = next_priority(),
            .left = kNull,
            .right = kNull,
            .size = 1,
            .id = id,
            .reversed = false,
        });
        return id;
    }

public:
    // explicit은 정수 capacity가 암시적으로 tree로 변환되는 것을 막는다.
    explicit ImplicitTreap(int capacity, Priority seed) : priority_state_{seed} {
        // [첫 호출 계약: vector<Node>::reserve(size_type)]
        // (1) nodes_는 빈 유효 vector이고 capacity는 문제 제약상 1..200000의 int다.
        // (2) `void reserve(size_type new_cap)`가 선택된다.
        // (3) 양수 int를 size_t 값으로 변환해 전달하며 원소나 소유권 입력은 없다.
        // (4) 반환형 void라 결과는 없다.
        // (5) 성공 뒤 capacity>=요청값, size=0이며 이후 정확히 capacity회 push_back 동안 재할당이 없다.
        // (6) 현재 size에 선형(여기서는 0), 메모리 할당과 length_error/bad_alloc 가능이다. Node의
        //     무예외 이동 조건에서는 예외 시 no-effects 보장이다. capacity가 실제 늘면 기존 iterator와
        //     past-the-end iterator가 무효지만 아직 원소/reference는 없다. thread 동기화는 제공하지 않는다.
        nodes_.reserve(static_cast<std::size_t>(capacity));
    }

    void append(Value value) {
        root_ = merge(root_, create_node(value));
    }

    void reverse_range(int first_position, int last_position) noexcept {
        // 세 조각 [0,a), [a,b], (b,n)으로 분리한다. structured binding은 pair의 두 int 값을 받는다.
        auto [left, tail]{split(root_, first_position - 1)};
        auto [middle, right]{split(tail, last_position - first_position + 1)};
        apply_reverse(middle);
        root_ = merge(left, merge(middle, right));
    }

    [[nodiscard]] Value range_sum(int first_position, int last_position) noexcept {
        auto [left, tail]{split(root_, first_position - 1)};
        auto [middle, right]{split(tail, last_position - first_position + 1)};
        const Value answer{subtree_sum(middle)};
        root_ = merge(left, merge(middle, right));
        return answer;
    }
};

int main() {
    // [첫 호출 계약: ios::sync_with_stdio(false), cin.tie(nullptr), operator>>]
    // (1) 표준 stream 객체들은 프로그램 시작 때 생성되어 유효하며 아직 사용자 I/O를 하지 않았다.
    // (2) static `bool ios_base::sync_with_stdio(bool)`와 `ostream* basic_ios::tie(ostream*)`, 이어
    //     `int&` 및 std::int64_t가 존재하는 구현에서 정확히 64비트를 표현하도록 정의된 signed integer
    //     typedef의 기반 타입에 대응하는 산술 추출 overload가 선택된다. 특정 기반형으로 단정하지 않는다.
    // (3) false와 nullptr prvalue를 값으로 넘기고, 추출 대상들은 살아 있는 수정 가능한 lvalue다.
    // (4) sync는 이전 설정 bool을 반환해 버리고 tie는 이전 ostream*를 반환해 버린다. 각 >>는
    //     같은 istream&를 반환해 연쇄/상태에 쓰며 여기서는 명시 검사 없이 문제의 정상 입력을 전제한다.
    // (5) C/C++ stream 동기화가 꺼지고 cin 자동 flush 연결이 해제된다. 추출 성공 시 대상 값과
    //     입력 위치가 바뀌며 실패 시 failbit/eofbit가 설정되고 이후 값은 신뢰할 수 없다.
    // (6) 표준은 이 세 연산의 정확한 복잡도 상한을 명시하지 않는다. 구현은 보통 설정을 짧게
    //     바꾸고 추출 문자/locale 처리만큼 일하며 내부 buffering이 가능하다. 설정된 exception mask에서는
    //     ios_base::failure 가능, false 동기화 뒤 stream 동시 사용의 안전을 가정하지 않는다. 오늘은
    //     single thread이며, 설정은 어떤 표준 I/O보다 먼저 해 호출 시점의 구현 의존 함정을 피한다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int element_count{};
    int operation_count{};
    std::cin >> element_count >> operation_count;

    ImplicitTreap sequence{element_count, make_priority_seed()};
    for (int index{}; index < element_count; ++index) {
        Value value{};
        std::cin >> value;
        sequence.append(value);
    }

    for (int query{}; query < operation_count; ++query) {
        int type{};
        int first{};
        int last{};
        std::cin >> type >> first >> last;

        if (type == 1) {
            sequence.reverse_range(first, last);
        } else {
            // [첫 호출 계약: std::cout operator<<(int64_t)와 char 삽입]
            // (1) 수신 객체 cout은 유효하고 살아 있는 ostream lvalue이며 range_sum은 유효한
            //     Value prvalue를 반환한다. 출력 중 tree와 stream 수명이 끝나지 않는다.
            // (2) 산술 멤버 overload(구현의 int64_t 기반 정수 overload)와 char 비멤버 overload가 선택된다.
            // (3) 두 피연산자/입력 인자는 합 prvalue와 '\n' char prvalue이며 tree 소유권을 넘기지 않는다.
            // (4) 두 호출은 같은 ostream&를 반환해 연쇄에 사용하고 마지막 반환은 버린다.
            // (5) 출력 buffer/위치만 바뀌며 tree는 split 뒤 같은 논리 순서로 merge되어 유지된다.
            // (6) 복잡도는 숫자 자릿수와 streambuf에 따르고 내부 buffering이 가능하다. stream과
            //     입력 값 수명/관찰자는 무효화되지 않지만 failbit/badbit, 설정된 exception mask의
            //     ios_base::failure가 가능하며 thread 레코드 원자성은 없다.
            std::cout << sequence.range_sum(first, last) << '\n';
        }
    }

    return 0;
}
