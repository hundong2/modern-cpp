# 경로 복사 영속 세그먼트 트리(Persistent Segment Tree)

## 정의

영속 자료구조(persistent data structure)는 값을 바꾼 뒤에도 **이전 버전을 계속 조회할 수 있는** 자료구조다. 경로 복사 영속 세그먼트 트리는 한 점을 갱신할 때 기존 트리를 직접 덮어쓰지 않고, 루트에서 그 잎까지의 경로에 있는 `O(log n)`개 노드만 새로 만든다. 갱신 경로 밖의 서브트리는 이전 버전과 안전하게 공유한다.

각 버전은 노드 전체를 소유하는 별도 트리가 아니라 루트 인덱스 하나로 식별한다. 노드를 수정 불가능한 값처럼 다루면 같은 노드를 여러 버전이 가리켜도 서로의 결과가 바뀌지 않는다. 이것이 구조적 공유(structural sharing)다.

오늘 구현은 포인터마다 동적 할당하는 대신 `std::vector<Node>` 하나를 **노드 arena**로 사용한다. 자식은 raw pointer가 아니라 arena 안의 정수 인덱스로 가리킨다. 이 선택은 소유권을 한 컨테이너에 모으고, 할당 횟수와 노드당 메모리 오버헤드를 줄이며, 대회 코드의 복사·파괴 규칙을 단순하게 한다.

## 적용 조건

다음 조건이면 경로 복사 영속 세그먼트 트리를 우선 검토한다.

- 같은 배열·수열의 여러 과거 버전을 보존해야 한다.
- 새 버전 만들기, 한 점 갱신, 구간 합·최솟값·최댓값 같은 결합 가능한 질의가 섞여 있다.
- 두 구간 요약을 결합하는 연산이 결합법칙을 만족하고 항등값을 정할 수 있다.
- 갱신마다 전체 배열 `O(n)`개를 복사할 수 없지만, `O(log n)`개 새 노드는 감당할 수 있다.
- 질의 시점의 버전 번호가 명시되어 있거나, 과거 상태로 되돌아가 질의해야 한다.

다음 상황에서는 다른 구조가 더 단순할 수 있다.

- 과거 버전이 필요 없고 점 갱신·구간 합만 필요하면 펜윅 트리나 일반 세그먼트 트리가 더 짧다.
- 갱신 없이 정적 구간 질의만 있다면 누적 합, sparse table 같은 정적 전처리가 적합할 수 있다.
- 모든 버전이 아니라 최신 버전 몇 개만 필요하면 복사본 수명 정책이나 rollback 자료구조가 메모리를 덜 쓸 수 있다.
- 구간 전체 갱신까지 필요하면 lazy tag도 영속적으로 복사해야 한다. 합성 순서와 노드 수 상한이 훨씬 까다로우므로 [지연 전파 세그먼트 트리](lazy-segment-tree.md)의 불변식도 함께 검토한다.

## 핵심 아이디어와 불변식

### 1. 노드는 생성 뒤 바꾸지 않는다

arena의 노드 `v`는 다음 세 값을 가진다.

- `sum`: 노드가 담당하는 구간 합
- `left`: 왼쪽 자식 노드의 arena 인덱스
- `right`: 오른쪽 자식 노드의 arena 인덱스

한 번 arena에 들어간 노드는 다시 쓰지 않는다. 갱신은 기존 노드를 복사한 새 값을 만들고 새 자식 인덱스와 합을 기록한 뒤 arena 끝에 추가한다. 따라서 과거 버전이 가리키는 노드의 의미가 보존된다.

### 2. 버전 불변식

`roots[k]`가 버전 `k`의 루트라면, 그 루트에서 도달 가능한 모든 노드의 `sum`은 **버전 `k` 배열의 해당 구간 합**과 같다.

- 초기 build가 모든 잎에 원소 값을, 내부 노드에 두 자식 합을 저장하므로 버전 1에서 성립한다.
- 한 점 갱신은 목표 잎까지의 경로만 복사한다.
- 목표를 포함하지 않는 자식은 과거 노드 인덱스를 그대로 공유하므로 그 구간 합이 변하지 않는다.
- 목표를 포함하는 자식은 재귀적으로 새 노드를 받고, 부모 합은 새 두 자식 합으로 다시 계산한다.

### 3. 공유 안전 불변식

서로 다른 두 루트가 같은 노드 인덱스를 가리킬 수 있지만, 공유 노드는 생성 뒤 수정하지 않는다. 그러므로 한 버전의 갱신이 다른 버전의 관찰 결과를 바꾸지 않는다. 복사 명령은 전체 노드를 복제하지 않고 루트 인덱스만 한 번 복사하면 된다.

### 4. 구간 질의 불변식

질의 구간 `[ql, qr]`과 현재 노드 구간 `[left, right]`의 관계는 세 경우로 나눈다.

1. 서로 겹치지 않으면 합의 항등값 `0`을 반환한다.
2. 현재 구간 전체가 질의 안에 있으면 저장된 `sum`을 반환한다.
3. 일부만 겹치면 두 자식 결과를 더한다.

세 경우가 현재 구간을 빠짐없이, 중복 없이 처리하므로 반환값은 정확한 구간 합이다.

## 단계별 절차

### 초기 트리 만들기

1. 배열 전체 구간을 루트가 담당하게 한다.
2. 구간 길이가 1이면 그 원소 값을 가진 잎 노드를 arena에 추가한다.
3. 아니면 구간을 가운데에서 나누어 왼쪽·오른쪽 자식을 먼저 만든다.
4. 두 자식 합을 더한 내부 노드를 arena에 추가한다.
5. 완성된 루트 인덱스를 `roots`의 첫 원소로 보관한다.

### 한 점 갱신

1. 바꿀 버전의 기존 루트와 목표 위치를 받는다.
2. 현재 기존 노드를 **값으로 복사**한다. arena 원소 참조를 재귀 호출 너머로 보관하지 않는다.
3. 잎이면 복사본의 합을 새 값으로 바꾸고 arena에 추가한다.
4. 내부 노드면 목표 위치가 속한 자식만 재귀 갱신한다.
5. 반대쪽 자식 인덱스는 기존 값을 그대로 둬 구조적으로 공유한다.
6. 두 자식 합으로 복사본의 합을 다시 계산하고 arena에 추가한다.
7. 반환된 새 루트로 해당 버전의 루트 인덱스를 교체한다.

### 버전 복사와 질의

1. 버전 복사는 선택한 `roots[k]` 정수 하나를 `roots` 끝에 추가한다.
2. 이후 어느 버전을 갱신하든 경로 복사가 새 루트를 만들므로 두 버전은 독립적으로 갈라진다.
3. 질의는 선택한 버전 루트에서 일반 세그먼트 트리와 같은 구간 분할을 수행한다.

## 의사코드

```text
build(left, right):
    if left == right:
        return append(Node(value = array[left]))
    mid = (left + right) / 2
    left_root  = build(left, mid)
    right_root = build(mid + 1, right)
    return append(Node(sum(left_root) + sum(right_root), left_root, right_root))

update(previous, left, right, position, value):
    next = copy(arena[previous])
    if left == right:
        next.sum = value
        return append(next)
    mid = (left + right) / 2
    if position <= mid:
        next.left = update(next.left, left, mid, position, value)
    else:
        next.right = update(next.right, mid + 1, right, position, value)
    next.sum = arena[next.left].sum + arena[next.right].sum
    return append(next)

query(node, left, right, query_left, query_right):
    if query_right < left or right < query_left:
        return 0
    if query_left <= left and right <= query_right:
        return arena[node].sum
    mid = (left + right) / 2
    return query(arena[node].left, left, mid, query_left, query_right)
         + query(arena[node].right, mid + 1, right, query_left, query_right)

copy_version(k):
    copied_root = roots[k]
    roots.push_back(copied_root)
```

## 컴파일 가능한 C++20 뼈대

아래 예제는 CSES 1737 공개 예제와 같은 버전 생성·질의·갱신 순서를 메모리 안에서 재현한다. 출력은 항상 `13`, `13`, `13`, `15` 네 줄이다.

```cpp
// <cstddef>는 컨테이너 크기와 인덱스를 표현하는 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 합의 폭을 명확히 하는 std::int64_t를 선언한다.
#include <cstdint>
// <iostream>은 표준 출력 객체 std::cout와 스트림 삽입 연산을 선언한다.
#include <iostream>
// <vector>는 배열·노드 arena·버전 루트를 연속 저장하는 std::vector를 선언한다.
#include <vector>

// Sum은 새 타입이 아니라 std::int64_t의 읽기 쉬운 별칭이다.
// n * 값의 최댓값은 2 * 10^14까지 가능하므로 32비트 int로 합을 저장하면 안 된다.
using Sum = std::int64_t;

// struct는 기본 접근이 public이라 단순 노드 레코드에 알맞다.
struct Node {
    Sum sum{};   // {}는 정수 합을 0으로 값 초기화한다.
    int left{-1};
    int right{-1};
};

class PersistentRangeSum final {
public:
    // 생성자는 반환형이 없고, 입력 배열을 const 참조로 빌려 읽을 뿐 소유하지 않는다.
    // max_queries는 전체 명령 수 q이며, 갱신 수와 복사 수 모두의 안전한 상한으로 사용한다.
    explicit PersistentRangeSum(
        const std::vector<Sum>& values,
        std::size_t max_queries)
        : size_{} {
        // [호출 계약: const vector<Sum>::size()]
        // (1) 수신 객체는 공식 제약에 따라 1개 이상 초기화된 Sum을 소유하는
        //     const std::vector<Sum> lvalue values다.
        // (2) 선택 오버로드는 size_type size() const noexcept이고 데이터 인자는 없다.
        // (3) 숨은 const this만 빌리며 원소·소유권을 넘기지 않는다.
        // (4) 반환형 size_type의 원소 수 값은 levels와 capacity 계산에 사용한다.
        // (5) 호출 뒤 values의 size, capacity, 원소와 모든 관찰자는 그대로다.
        // (6) O(1), 무할당·비무효화·noexcept다. 같은 vector를 다른 실행 흐름이
        //     동시에 변경해서는 안 된다. n<=200,000이므로 int 변환도 표현 가능하다.
        size_ = static_cast<int>(values.size());
        std::size_t levels{1};
        for (std::size_t width{1}; width < values.size(); width *= 2) {
            ++levels;
        }

        const std::size_t build_nodes{values.size() * 2 - 1};
        const std::size_t node_capacity{build_nodes + max_queries * levels};

        // [호출 계약: vector<Node>::reserve, vector<int>::reserve]
        // (1) 수신 nodes_와 roots_는 각각 기본 생성된 빈 vector lvalue이고 capacity는
        //     구현이 정한 초기값이다. node_capacity와 max_queries+1은 공식 상한 안이다.
        // (2) 각 호출은 void reserve(size_type new_cap)를 선택한다.
        // (3) 인자는 size_t 값 prvalue로 전달되며 소유권·참조를 넘기지 않는다.
        // (4) 반환형은 void라 결과값은 없고, 이후 재할당 방지가 목적이다.
        // (5) 성공하면 size는 0 그대로이고 capacity가 적어도 요청값이 된다. 기존 원소는 없다.
        // (6) 현재 size에 대해 O(size), 여기서는 O(1)이고 메모리를 할당할 수 있다.
        //     max_size 초과는 length_error, 할당 실패는 bad_alloc이 될 수 있다.
        //     실제 재할당이면 기존 관찰자가 무효화되나 지금은 관찰자가 없다. 자체 동기화는 없다.
        nodes_.reserve(node_capacity);
        roots_.reserve(max_queries + 1);

        const int initial_root{build(values, 0, size_ - 1)};

        // [호출 계약: vector<int>::push_back(const int&)]
        // (1) 수신 roots_는 size 0이고 최소 max_queries+1 capacity를 확보한 vector<int>다.
        // (2) 선택 오버로드는 void push_back(const int& value)다.
        // (3) initial_root는 유효한 arena 인덱스를 가진 const int lvalue이며 값이 복사된다.
        //     roots_는 트리 노드가 아니라 정수 인덱스만 소유한다.
        // (4) 반환형 void라 반환값은 사용하지 않는다.
        // (5) 성공 뒤 roots_.size()==1이고 버전 1의 루트가 끝 원소가 된다. 인자는 유지된다.
        // (6) 확보한 capacity 안에서는 상각 O(1), 무할당이며 기존 원소 관찰자는 유지되지만 옛
        //     past-the-end iterator는 무효화된다. 일반적으로 재할당 시 모든 roots_ 관찰자가
        //     무효화되고 bad_alloc이 가능하다.
        //     같은 vector에 대한 동시 접근은 외부에서 동기화해야 한다.
        roots_.push_back(initial_root);
    }

    // CSES의 1-based 버전 k를 복사해 목록 끝에 새 버전을 만든다.
    void copy_version(std::size_t version) {
        // [호출 계약: vector<int>::operator[] const 읽기]
        // (1) 수신 roots_는 적어도 version개 초기화된 int를 소유한다.
        // (2) 선택 오버로드는 reference operator[](size_type position)이며 수신자가
        //     non-const여도 이 식에서는 반환 int&를 읽기만 한다.
        // (3) 인자는 version-1 size_t prvalue이고 전제조건은 version>=1 및
        //     version-1<roots_.size()다. 소유권 이전은 없다.
        // (4) 반환 int&의 값을 copied_root에 복사하며 참조 자체는 보관하지 않는다.
        // (5) roots_와 원소는 바뀌지 않는다.
        // (6) O(1), 무할당이다. 범위 검사가 없으므로 잘못된 번호는 미정의 동작이다.
        //     반환 참조는 roots_ 재할당·삭제·파괴 시 무효가 되며 자체 동기화는 없다.
        const int copied_root{roots_[version - 1]};
        roots_.push_back(copied_root);
    }

    // 선택한 버전의 1-based position 하나를 value로 바꾼다.
    void assign(std::size_t version, int position, Sum value) {
        const std::size_t root_index{version - 1};
        const int previous_root{roots_[root_index]};
        const int next_root{update(previous_root, 0, size_ - 1, position - 1, value)};
        roots_[root_index] = next_root;
    }

    // [left,right]는 공개 API에서 CSES 입력과 같은 1-based 닫힌 구간이다.
    [[nodiscard]] Sum range_sum(std::size_t version, int left, int right) const {
        return query(roots_[version - 1], 0, size_ - 1, left - 1, right - 1);
    }

private:
    int size_{};

    // [생성 계약: vector 기본 생성 두 번]
    // (1) 목적 객체 nodes_와 roots_는 아직 존재하지 않는다.
    // (2) 각각 vector<Node>()와 vector<int>() 기본 생성자를 선택한다.
    // (3) 명시적 데이터 인자·외부 소유권은 없다.
    // (4) 생성자는 별도 반환값이 없고 두 멤버 객체의 수명을 시작한다.
    // (5) 성공 뒤 두 vector 모두 size 0이며 아직 Node/int 원소 수명은 시작하지 않았다.
    // (6) 일반적으로 O(1)이고 기본 생성 자체는 원소 관찰자를 만들지 않는다.
    //     allocator 정책에 따른 계약을 따르며 같은 미완성 객체의 동시 접근은 성립하지 않는다.
    std::vector<Node> nodes_{};
    std::vector<int> roots_{};

    int append(Node node) {
        // [호출 계약: vector<Node>::push_back(const Node&), vector<Node>::size()]
        // (1) 수신 nodes_는 유효한 기존 노드를 [0,size)에서 소유하며 reserve 상한 안이다.
        // (2) push_back은 void push_back(const Node&)를, size는 size_type size() const noexcept를 고른다.
        // (3) node는 완성된 Node lvalue이며 세 정수 값을 새 원소로 복사한다. 외부 자원을 빌리지 않는다.
        // (4) push_back은 void라 버리고, size()가 돌려준 새 원소 수는 마지막 인덱스 계산에 쓴다.
        // (5) 성공 뒤 node 값은 유지되고 nodes_ 끝에 같은 값의 새 노드가 생긴다.
        // (6) 확보한 capacity 안에서 push_back은 상각 O(1)이고 기존 원소 관찰자는 유지되지만 옛
        //     past-the-end iterator는 무효화된다. size는 O(1)·noexcept다. 일반적으로 재할당 시 모든
        //     Node 참조·포인터·반복자가 무효화되고 복사·할당 예외가 가능하다. 그래서 재귀 호출
        //     너머로 arena 원소 참조를 보관하지 않는다. 동기화는 없다.
        nodes_.push_back(node);
        return static_cast<int>(nodes_.size() - 1);
    }

    int build(const std::vector<Sum>& values, int left, int right) {
        if (left == right) {
            // [호출 계약: const vector<Sum>::operator[]]
            // (1) values는 size_개의 Sum을 소유하는 const vector이고 left는 [0,size_)다.
            // (2) 선택 오버로드는 const_reference operator[](size_type position) const다.
            // (3) non-negative int left를 size_t 값으로 바꿔 전달하며 소유권 이전은 없다.
            // (4) 반환 const Sum&의 값을 새 Node.sum으로 복사하고 참조는 저장하지 않는다.
            // (5) values와 원소·관찰자는 바뀌지 않는다.
            // (6) O(1), 무할당이다. 범위 검사가 없어 전제조건 위반은 미정의 동작이며,
            //     반환 참조 수명은 values 원소 수명에 묶인다. 동시 변경은 허용하지 않는다.
            return append(Node{values[static_cast<std::size_t>(left)], -1, -1});
        }

        const int middle{left + (right - left) / 2};
        const int left_root{build(values, left, middle)};
        const int right_root{build(values, middle + 1, right)};

        // [호출 계약: vector<Node>::operator[]]
        // (1) nodes_는 left_root와 right_root 위치에 완성된 자식 Node를 소유한다.
        // (2) 선택 오버로드는 reference operator[](size_type position)다.
        // (3) 각 non-negative int 인덱스를 size_type 값으로 바꿔 전달하며 허용 범위는 [0,size)다.
        // (4) 반환 Node&에서 sum 값을 읽어 부모 합 계산에 사용하고 참조는 보관하지 않는다.
        // (5) nodes_의 size/capacity/노드 값은 바뀌지 않는다.
        // (6) 각 접근 O(1), 무할당·비무효화다. 범위 검사가 없어 잘못된 인덱스는 미정의 동작이고,
        //     다른 실행 흐름의 arena 변경과 동기화 없이 겹치면 안 된다.
        const Sum sum{nodes_[static_cast<std::size_t>(left_root)].sum
                    + nodes_[static_cast<std::size_t>(right_root)].sum};
        return append(Node{sum, left_root, right_root});
    }

    int update(int previous, int left, int right, int position, Sum value) {
        // arena 원소를 참조로 잡지 않고 값 복사한다. 아래 재귀 append가 vector를 재할당하더라도 안전하다.
        Node next{nodes_[static_cast<std::size_t>(previous)]};
        if (left == right) {
            next.sum = value;
            return append(next);
        }

        const int middle{left + (right - left) / 2};
        if (position <= middle) {
            next.left = update(next.left, left, middle, position, value);
        } else {
            next.right = update(next.right, middle + 1, right, position, value);
        }

        next.sum = nodes_[static_cast<std::size_t>(next.left)].sum
                 + nodes_[static_cast<std::size_t>(next.right)].sum;
        return append(next);
    }

    [[nodiscard]] Sum query(
        int node,
        int left,
        int right,
        int query_left,
        int query_right) const {
        if (query_right < left || right < query_left) {
            return 0;
        }
        if (query_left <= left && right <= query_right) {
            return nodes_[static_cast<std::size_t>(node)].sum;
        }

        const Node current{nodes_[static_cast<std::size_t>(node)]};
        const int middle{left + (right - left) / 2};
        return query(current.left, left, middle, query_left, query_right)
             + query(current.right, middle + 1, right, query_left, query_right);
    }
};

int main() {
    // [생성 계약: vector<Sum>(initializer_list<Sum>)]
    // (1) 목적 values는 아직 존재하지 않는다.
    // (2) 선택 생성자는 vector(initializer_list<Sum>, const Allocator& = Allocator())다.
    // (3) 다섯 정수 prvalue가 Sum으로 변환되어 initializer_list의 const 원소로 제공되고
    //     vector가 값을 자기 연속 저장소에 복사해 소유한다. 기본 allocator를 쓴다.
    // (4) 생성자는 별도 반환값이 없고 values 객체와 다섯 Sum 원소 수명을 시작한다.
    // (5) 성공 뒤 size()==5이고 순서는 2,3,1,2,5다.
    // (6) 시간·공간 O(5), 저장소 할당과 bad_alloc이 가능하다. 새 객체라 기존 관찰자 무효화는 없고,
    //     이후 같은 vector의 동시 변경은 별도 동기화가 필요하다.
    const std::vector<Sum> values{2, 3, 1, 2, 5};
    PersistentRangeSum tree{values, 6};

    tree.copy_version(1); // 명령 3 1: 버전 1의 루트 인덱스만 복사해 버전 2를 만든다.

    // [호출 계약: ostream::operator<< 연쇄]
    // (1) 수신 std::cout은 프로그램이 제공한 유효한 std::ostream lvalue이고 bad 상태가 아닐 수 있다.
    // (2) 각 식은 정수용 operator<<(Sum) 뒤 문자용 operator<<(char) 오버로드를 선택한다.
    // (3) range_sum 결과 Sum prvalue는 값으로, '\n' char prvalue는 값으로 전달한다. 소유권 이전은 없다.
    // (4) 각 호출은 같은 std::ostream&를 반환해 다음 <<에 사용하며 마지막 반환 참조는 버린다.
    // (5) 성공하면 숫자의 문자 표현과 줄바꿈이 출력 버퍼에 순서대로 추가되고 tree는 바뀌지 않는다.
    // (6) 출력 문자 수에 선형이며 버퍼링·locale 처리가 가능하다. 실패하면 stream 상태 비트가 설정되고,
    //     예외 mask 설정에 따라 ios_base::failure가 날 수 있다. 컨테이너 관찰자를 무효화하지 않으며,
    //     같은 stream에 대한 여러 실행 흐름의 레코드 단위 원자성은 보장하지 않는다.
    std::cout << tree.range_sum(1, 1, 5) << '\n'; // 13
    std::cout << tree.range_sum(2, 1, 5) << '\n'; // 13

    tree.assign(2, 2, 5); // 명령 1 2 2 5: 버전 2만 [2] = 5로 갱신한다.
    std::cout << tree.range_sum(1, 1, 5) << '\n'; // 과거 버전 1은 그대로 13
    std::cout << tree.range_sum(2, 1, 5) << '\n'; // 새 버전 2는 15
}
```

예상 출력:

```text
13
13
13
15
```

## 정확성 근거

### 보조정리 1: 초기 루트는 원래 배열의 모든 구간 합을 정확히 저장한다

구간 길이에 대한 귀납법을 쓴다. 길이 1인 잎은 `values[left]`를 그대로 저장하므로 정확하다. 길이가 2 이상이면 왼쪽·오른쪽 자식은 더 짧은 구간이므로 귀납 가정에 따라 각각 정확하다. 부모는 두 겹치지 않는 자식 구간의 합을 더하므로 부모 구간 전체 합도 정확하다. 따라서 초기 루트의 모든 노드가 정확하다.

### 보조정리 2: `update`는 목표 위치만 바뀐 정확한 새 루트를 만든다

현재 구간 길이에 대한 귀납법을 쓴다. 잎에서는 그 위치의 합을 새 값으로 정하므로 정확하다. 내부 노드에서는 목표를 포함하는 자식만 재귀 갱신한다. 귀납 가정으로 그 자식은 정확한 새 상태를 나타낸다. 반대 자식은 목표를 포함하지 않으므로 기존 노드를 공유해도 값이 정확하다. 새 부모 합을 두 자식 합으로 계산하므로 현재 구간도 정확하다.

### 보조정리 3: 갱신은 다른 버전을 바꾸지 않는다

`update`는 기존 arena 노드에 대입하지 않는다. 새 노드만 arena 끝에 추가하고 새 루트를 반환한다. 따라서 다른 `roots` 원소에서 도달 가능한 기존 노드의 모든 필드는 그대로다. 공유 노드가 불변이므로 다른 버전의 모든 구간 합도 유지된다.

### 보조정리 4: `query`는 지정한 버전의 질의 구간 합을 반환한다

겹치지 않는 노드는 항등값 0을 반환한다. 완전히 포함된 노드는 버전 불변식에 따라 정확한 저장 합을 반환한다. 일부 겹친 노드는 서로 겹치지 않는 두 자식의 결과를 더한다. 재귀가 선택한 완전 포함 조각들은 질의 구간을 정확히 분할하므로 전체 반환값이 질의 합이다.

### 정리

보조정리 1로 첫 버전이 정확하다. 버전 복사는 정확한 루트 인덱스를 그대로 복사한다. 보조정리 2와 3으로 각 점 갱신은 선택한 버전에만 정확한 새 상태를 만들고 다른 버전을 보존한다. 보조정리 4로 어느 버전의 구간 질의도 정확하다. 따라서 임의 순서의 복사·갱신·질의를 모두 올바르게 처리한다.

## 시간·공간 복잡도

`n`은 배열 길이, `u`는 실제 점 갱신 횟수, `c`는 복사 명령 수다.

| 연산 | 시간 | 새 공간 | 근거 |
|---|---:|---:|---|
| 초기 build | `O(n)` | `O(n)` | `n >= 1`에서 정확히 `2n-1`개 노드를 만든다. |
| 버전 복사 | `O(1)` | `O(1)` | 루트 인덱스 하나만 복사한다. |
| 한 점 갱신 | `O(log n)` | `O(log n)` | 루트부터 잎까지 한 경로만 복사한다. |
| 구간 질의 | `O(log n)` | 재귀 스택 `O(log n)` | 표준 구간 분할에서 각 높이의 경계 노드만 방문한다. |

전체 노드 수는 `O(n + u log n)`, 루트 목록은 `O(c)`이므로 총 저장 공간은 `O(n + u log n + c)`다. CSES 1737에서는 `n,q<=200,000`이고 `u,c<=q`다. 균형 높이는 약 19단계이므로 `Node{int64_t,int,int}`를 연속 저장하면 최악 수백만 노드를 512MB 안에서 관리할 수 있다. 정확한 `sizeof(Node)`와 vector 성장 여유는 ABI·정렬·표준 라이브러리 구현에 따라 달라 측정해야 한다.

## 대회 우승을 가르는 구현 선택과 최적화

### arena + 정수 인덱스

- `new`/`shared_ptr`를 노드마다 사용하면 수백만 번 할당, 제어 블록, 참조 횟수 갱신과 메모리 파편화가 생길 수 있다.
- `vector<Node>` 하나에 append-only로 저장하면 소유자는 하나이고 자식 인덱스는 작다. 노드가 trivially copyable한 세 정수 레코드라 경로 복사 상수도 작다.
- 포인터 대신 인덱스를 저장하면 vector 재할당 뒤에도 연결이 깨지지 않는다. 단, arena 원소의 **참조·포인터·반복자**는 재할당으로 무효가 되므로 재귀 호출 전에 `Node next = nodes_[id]`처럼 값 복사한다.

### 정확한 reserve 상한

- build는 빈 배열이 아닌 경우 `2n-1`개 노드를 만든다.
- 한 점 갱신은 최대 `ceil(log2 n)+1`개 노드를 만든다.
- 입력에서 `q`를 이미 알므로 `2n-1 + q*(ceil(log2 n)+1)` 정도를 reserve하면 실행 중 arena 재할당을 피할 수 있다.
- 무조건 `4*n + q*20` 같은 식을 쓰기 전 `size_t` 곱셈 오버플로와 실제 메모리 제한을 계산한다.

### 64비트 합, 32비트 인덱스

- 원소 최댓값 `10^9`, 원소 수 `2*10^5`이므로 합은 `2*10^14`까지 간다. 합은 `std::int64_t` 또는 충분한 폭의 signed 정수로 저장한다.
- 노드 수 상한이 signed `int` 안에 든다는 것을 계산했다면 자식 인덱스는 `int`로 줄여 노드 크기를 작게 유지할 수 있다. 상한이 커지는 변형에서는 `std::size_t`나 64비트 인덱스로 올린다.

### 버전 의미를 코드에 그대로 반영

CSES 1737의 type 1은 “버전에서 파생한 새 버전을 자동 추가”하는 명령이 아니라 **기존 배열 k 자체를 갱신**한다. 그러므로 `roots[k-1] = update(roots[k-1], ...)`로 그 슬롯을 교체한다. type 3만 `roots.push_back(roots[k-1])`로 목록을 늘린다. 이 차이를 놓치면 공식 예제는 우연히 통과해도 복사 뒤 원본과 복사본을 번갈아 수정하는 테스트에서 틀린다.

### 재귀 깊이와 입출력

- 세그먼트 트리 재귀 깊이는 `O(log n)`이라 `n=200,000`에서도 일반적인 환경에서 안전하다. 길이 `n`의 편향 DFS 재귀와 다르다.
- 1초 제한에서는 iostream 동기화 해제, 출력 버퍼링, 미리 확보한 arena가 도움이 된다. 정확한 성능은 CPU, 컴파일러, 최적화 옵션과 표준 라이브러리에 따라 달라 실제 최대 입력으로 측정한다.

## 흔한 실수

1. **기존 노드를 직접 수정한다.** 공유 노드 하나를 바꾸면 여러 과거 버전이 동시에 오염된다.
2. **배열 복사를 진짜 `vector` 전체 복사로 구현한다.** type 3이 `O(n)`이 되어 최악 `O(nq)`다.
3. **모든 자식을 새로 만든다.** 목표 경로 밖 자식은 기존 인덱스를 공유해야 갱신 공간이 `O(log n)`이다.
4. **버전 복사 뒤 source를 갱신하면 copy도 바뀐다고 생각한다.** 복사 당시 루트만 공유하고, 이후 갱신은 path-copy로 갈라져야 한다.
5. **type 1마다 새 버전 슬롯을 append한다.** CSES 1737에서는 선택한 기존 버전의 root 슬롯을 교체해야 한다.
6. **합을 `int`로 저장한다.** 최대 합이 32비트 signed 범위를 넘는다.
7. **arena 원소 참조를 잡은 채 재귀 append한다.** vector가 재할당되면 참조가 댕글링된다. 값 복사 또는 충분한 reserve를 사용하되, 안전성을 reserve 하나에만 의존하지 않는다.
8. **`roots_.push_back(roots_[k])`의 내부 별칭을 무심코 쓴다.** 먼저 정수 값을 지역 변수에 복사하면 push 과정의 재할당과 평가 순서를 쉽게 추론할 수 있다.
9. **1-based 입력을 내부 0-based 구간과 섞는다.** 공개 경계에서 한 번만 변환하고 재귀 함수의 구간 규칙은 통일한다.
10. **빈 구간·부분 겹침 조건을 뒤집는다.** `qr < left || right < ql`이면 0, 완전 포함일 때만 저장 합을 바로 반환한다.
11. **노드 수 상한 없이 push한다.** 정답이어도 메모리 초과나 반복 재할당으로 시간 제한을 넘을 수 있다.
12. **노드 수를 재귀 호출 수와 혼동한다.** 질의는 새 노드를 만들지 않고, 갱신만 경로 길이만큼 만든다.

## 변형

- **빈도 영속 세그먼트 트리**: 값 좌표를 압축하고 prefix 버전마다 빈도를 하나 추가하면 두 버전의 노드 합 차이로 구간 k번째 수를 찾을 수 있다.
- **Chairman Tree**: `root[r] - root[l-1]`의 빈도 차이를 내려가며 구간 순서 통계, 빈도, 특정 값 개수를 구한다.
- **동적 좌표 영속 트리**: 매우 큰 좌표 범위에서 방문한 경로의 노드만 만든다. null 자식을 항등 노드로 해석한다.
- **영속 lazy segment tree**: 구간 갱신 때 방문 노드와 필요한 자식·lazy tag를 모두 복사한다. push가 과거 자식을 직접 수정하지 않도록 특히 주의한다.
- **부분 영속성(partial persistence)**: 과거 버전은 읽기만 하고 최신 버전만 갱신한다. 오늘 구조는 어떤 roots 슬롯도 갱신할 수 있어 문제 범위 안에서 더 유연하다.
- **완전 영속성(full persistence)**: 어느 과거 버전에서든 새 분기를 만들 수 있다. root 목록에 새 루트를 append하는 API로 표현할 수 있다.
- **rollback 세그먼트 트리**: 변경 로그를 스택에 쌓고 되돌린다. 버전 탐색 순서가 DFS처럼 통제될 때 메모리가 더 작을 수 있지만 임의 버전 질의에는 영속 root가 단순하다.

## 오늘 문제와의 연결 — CSES 1737 Range Queries and Copies

- 공식 문제: [CSES 1737 - Range Queries and Copies](https://cses.fi/problemset/task/1737/)
- 배열 길이와 질의 수는 각각 최대 `200,000`, 원소와 갱신 값은 최대 `10^9`다.
- type 1 `1 k a x`: 배열 `k`의 위치 `a`를 `x`로 바꾼다. `roots[k-1]`을 path-copy 갱신 결과로 교체한다.
- type 2 `2 k a b`: 배열 `k`의 `[a,b]` 합을 구한다. 선택한 root에서 일반 구간 질의를 한다.
- type 3 `3 k`: 배열 `k`의 복사본을 목록 끝에 추가한다. root 인덱스 하나만 append한다.

이 문제의 핵심은 “복사본을 만드는 순간 전체 배열을 복사한다”가 아니라 **불변 노드를 공유하다가 쓰는 경로만 분리하는 copy-on-write 구조**로 해석하는 것이다. build `O(n)`, type 1·2 `O(log n)`, type 3 `O(1)`이므로 전체 시간은 `O(n + q log n)`, 공간은 `O(n + u log n + q)`다.

권장 검증은 다음과 같다.

1. 공식 예제: 두 버전의 초기 합이 같고, 버전 2 갱신 뒤 버전 1은 13, 버전 2는 15인지 확인한다.
2. 단일 원소: 복사본만 9로 바꾼 뒤 원본 5, 복사본 9가 유지되는지 확인한다.
3. 복사의 복사: 버전 1→2→3으로 복사한 뒤 버전 2만 바꿔 버전 1·3이 그대로인지 확인한다.
4. 64비트: `10^9` 세 개의 전체 합이 `3,000,000,000`인지 확인한다.
5. 작은 무작위 테스트: 각 버전을 실제 `vector`로 깊은 복사하는 느린 oracle과 모든 type 2 출력을 비교한다.
6. 최대 스트레스: `n=q=200,000`에서 갱신을 집중시켜 node 상한, reserve 계산, 시간과 메모리를 측정한다.

## 스스로 설명해 볼 질문

1. 버전 복사가 루트 정수 하나만 복사해도 되는 이유를 “노드 불변성”과 “구조적 공유”라는 말로 설명한다.
2. 한 점 갱신에서 새 노드가 정확히 어느 구간들에 생기는지 길이 8 배열로 그린다.
3. `answer = left.sum + right.sum`이 유지해야 하는 불변식을 말한다.
4. arena 원소 `Node&`를 잡은 뒤 재귀 `push_back`하면 왜 위험한지 vector 재할당과 참조 수명으로 설명한다.
5. CSES type 1과 “과거 버전에서 새 버전 생성” API의 root 목록 변화가 어떻게 다른지 비교한다.
6. `n=200,000`, 모든 질의가 갱신일 때 필요한 노드 수와 바이트를 `sizeof(Node)` 측정값으로 추산한다.
