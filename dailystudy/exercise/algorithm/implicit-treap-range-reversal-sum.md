# 암시적 트립: 구간 뒤집기와 구간 합

## 정의

암시적 트립(implicit treap)은 **중위 순회 순서가 배열 순서**가 되도록 만든 randomized binary search tree다. 일반 BST처럼 외부 key를 저장하지 않고, 각 node의 왼쪽 subtree 크기로 현재 0-based 위치를 계산한다. 동시에 각 node에 pseudo-random priority를 주고 max-heap 불변식을 유지한다.

- 순서 불변식: `inorder(tree)`가 현재 sequence와 같다.
- 크기 불변식: `size(u) = 1 + size(left(u)) + size(right(u))`.
- 집계 불변식: `sum(u) = value(u) + sum(left(u)) + sum(right(u))`.
- heap 불변식: 부모 priority가 두 자식보다 높다. priority가 같을 수 있다면 고유 id로 tie-break한다.
- lazy 뒤집기 불변식: `reversed(u)`가 참이면 `u`의 논리 sequence는 이미 뒤집힌 것으로 취급한다. 두 자식 pointer는 즉시 맞바꾸되, 자식 내부 뒤집기는 tag를 내려보낼 때까지 미룬다.

오늘 구현은 node를 개별 `new`하지 않고 `std::vector<Node>` arena에 한 번 모아 소유한다. 자식은 pointer 대신 정수 index로 표현한다. `reserve(n)` 뒤 정확히 `n`개 node만 추가하므로 구축 뒤 reference 무효화가 없고, tree 재배치는 index 값만 바꾼다.

## 적용 조건

다음 조건에 특히 알맞다.

1. sequence 중간을 자주 자르거나 붙인다.
2. 구간 전체에 적용되는 연산을 lazy tag로 합성할 수 있다.
3. 구간 합·최솟값·최댓값처럼 자식 집계에서 부모 집계를 `O(1)`에 다시 만들 수 있다.
4. 각 연산의 **기대** `O(log n)`이면 충분하고, deterministic worst-case `O(log n)`가 반드시 필요한 환경은 아니다.

단순 Fenwick tree는 합은 빠르지만 순서 뒤집기를 직접 표현하기 어렵다. 일반 lazy segment tree는 고정 index 구간의 값 갱신에는 강하지만 sequence를 split/merge하는 편집 연산에는 암시적 트립이 더 자연스럽다. 적대 입력에도 확정적인 최악 `O(log n)`가 필요하면 균형 기준이 결정적인 rope/B-tree 계열 또는 문제에 맞춘 segment tree를 검토한다.

## 핵심 연산과 불변식

### `pull(u)`

두 자식의 `size`와 `sum`이 정확하다는 전제에서 `u`의 값을 다시 계산한다.

```text
size(u) = 1 + size(left(u)) + size(right(u))
sum(u)  = value(u) + sum(left(u)) + sum(right(u))
```

link를 바꾸는 `split`/`merge`가 재귀에서 돌아올 때 반드시 호출한다. lazy reverse는 순서만 바꾸고 원소 multiset과 합은 바꾸지 않으므로 reverse tag를 붙이는 순간 `size`와 `sum`을 다시 계산할 필요는 없다.

### `apply_reverse(u)`와 `push(u)`

`apply_reverse(u)`는 `left(u)`와 `right(u)`를 맞바꾸고 `reversed(u)`를 XOR처럼 토글한다. 같은 구간을 두 번 뒤집으면 원래 순서이므로 bool 토글로 합성할 수 있다.

`push(u)`는 tag가 있을 때 두 자식 각각에 `apply_reverse`를 적용하고 `reversed(u)=false`로 만든다. 자식 방향을 보고 내려가는 `split`과 `merge`는 반드시 먼저 `push`해야 한다. 그렇지 않으면 물리 pointer와 논리 순서가 달라 잘못된 위치를 자른다.

### `split(root, k)`

현재 순서의 앞 `k`개를 `A`, 나머지를 `B`로 나누어 `(A,B)`를 반환한다.

```text
split(u, k):
    if u is null: return (null, null)
    push(u)
    L = size(left(u))
    if k <= L:
        (a, b) = split(left(u), k)
        left(u) = b
        pull(u)
        return (a, u)
    else:
        (a, b) = split(right(u), k - L - 1)
        right(u) = a
        pull(u)
        return (u, b)
```

재귀 한 단계마다 한쪽 자식으로만 내려가므로 비용은 tree 높이에 비례한다.

### `merge(A, B)`

`A`의 모든 원소가 `B`의 모든 원소보다 앞선다는 전제에서 두 sequence를 합친다. `higher(a,b)`, 즉 `(priority,id)` 사전식 key가 더 큰 root를 새 root로 정하고 반대 tree를 경계 쪽 자식과 재귀적으로 합친다. `id`는 priority 충돌 때도 strict total order를 만드는 tie-breaker다.

```text
merge(a, b):
    if a is null: return b
    if b is null: return a
    push(a); push(b)
    if higher(a, b):  // (priority(a), id(a)) > (priority(b), id(b))
        right(a) = merge(right(a), b)
        pull(a)
        return a
    else:
        left(b) = merge(a, left(b))
        pull(b)
        return b
```

`merge`의 위치 전제는 매우 중요하다. 서로 순서가 섞인 두 tree를 합치면 heap은 유지될 수 있어도 sequence 의미는 깨진다.

## 구간 연산 절차

1-based 닫힌 구간 `[a,b]`를 다음 세 조각으로 분리한다.

```text
(left, tail)    = split(root, a - 1)
(middle, right) = split(tail, b - a + 1)
```

- 뒤집기: `apply_reverse(middle)`.
- 합 질의: `answer = sum(middle)`.
- 복구: `root = merge(left, merge(middle, right))`.

질의도 tree link를 일시적으로 바꾸지만 마지막 merge 뒤 논리 sequence는 질의 전과 같다. 예외가 없는 정수/index 구현에서는 이 분해-관찰-복구를 한 함수 안에 묶어 중간 상태가 외부에 보이지 않게 한다.

## 컴파일 가능한 C++23 뼈대

아래 코드는 핵심 자료구조만 독립 실행해, `[2,1,3,4]`의 `[2,4]` 합과 전체 뒤집기 뒤 같은 합을 검사한다.

```cpp
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

// 정확한 폭의 합/우선순위 타입을 짧은 별칭으로 읽는다. 별칭은 새 타입을 만들지 않는다.
using i64 = std::int64_t;
using u64 = std::uint64_t;
constexpr int nil = -1;

// 외부에서 주입한 seed state를 섞어 pseudo-random priority를 만든다.
constexpr u64 mix(u64 x) noexcept {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30U)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27U)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31U);
}

struct Node {
    i64 value{};
    i64 sum{};
    u64 priority{};
    int left{nil};
    int right{nil};
    int size{1};
    int id{};
    bool reverse{};
};

class Treap {
    // vector 하나가 모든 Node 객체 수명을 소유하고 link는 index 값만 저장한다.
    std::vector<Node> nodes_;
    int root_{nil};
    u64 priority_state_{};

    u64 next_priority() noexcept {
        priority_state_ += 0x9e3779b97f4a7c15ULL;
        return mix(priority_state_);
    }

    // 호출자는 x가 nil이 아니고 vector 범위 안이라는 전제조건을 먼저 세운다.
    Node& node(int x) { return nodes_[static_cast<std::size_t>(x)]; }
    const Node& node(int x) const { return nodes_[static_cast<std::size_t>(x)]; }
    int size(int x) const { return x == nil ? 0 : node(x).size; }
    i64 sum(int x) const { return x == nil ? 0 : node(x).sum; }

    // 자식 link를 바꾼 뒤 size/sum 불변식을 다시 만든다.
    void pull(int x) {
        node(x).size = 1 + size(node(x).left) + size(node(x).right);
        node(x).sum = node(x).value + sum(node(x).left) + sum(node(x).right);
    }
    // reverse는 원소 multiset과 합을 바꾸지 않으므로 child swap과 tag toggle만 한다.
    void flip(int x) {
        if (x == nil) return;
        std::swap(node(x).left, node(x).right);
        node(x).reverse = !node(x).reverse;
    }
    // 논리 방향을 읽고 내려가기 전에 pending tag를 두 자식에 합성한다.
    void push(int x) {
        if (x == nil || !node(x).reverse) return;
        flip(node(x).left);
        flip(node(x).right);
        node(x).reverse = false;
    }
    bool higher(int a, int b) const {
        if (node(a).priority != node(b).priority) {
            return node(a).priority > node(b).priority;
        }
        return node(a).id > node(b).id;
    }
    // 앞 k개와 나머지를 반환한다. k는 0..size(x)여야 한다.
    std::pair<int, int> split(int x, int k) {
        if (x == nil) return {nil, nil};
        push(x);
        const int left_size = size(node(x).left);
        if (k <= left_size) {
            auto [a, b] = split(node(x).left, k);
            node(x).left = b;
            pull(x);
            return {a, x};
        }
        auto [a, b] = split(node(x).right, k - left_size - 1);
        node(x).right = a;
        pull(x);
        return {x, b};
    }
    // a의 모든 원소가 b보다 앞선다는 위치 전제조건 아래 합친다.
    int merge(int a, int b) {
        if (a == nil) return b;
        if (b == nil) return a;
        push(a); push(b);
        if (higher(a, b)) {
            node(a).right = merge(node(a).right, b);
            pull(a);
            return a;
        }
        node(b).left = merge(a, node(b).left);
        pull(b);
        return b;
    }

public:
    // 제출에서는 고정 입력과 독립인 실행별 seed를 전달해야 기대 높이 분석이 성립한다.
    explicit Treap(std::vector<i64> values, u64 seed) : priority_state_{seed} {
        // reserve 뒤 values.size()개만 push하므로 구축 중 Node reference 재할당 위험을 없앤다.
        nodes_.reserve(values.size());
        // const 값 복사는 원본 vector를 바꾸지 않는다.
        for (const i64 value : values) {
            const int id = static_cast<int>(nodes_.size());
            nodes_.push_back(Node{value, value, next_priority(), nil, nil, 1, id, false});
            root_ = merge(root_, id);
        }
    }
    void reverse(int left, int right) {
        // 1-based [left,right]를 prefix/middle/suffix로 정확히 격리한다.
        auto [a, tail] = split(root_, left - 1);
        auto [b, c] = split(tail, right - left + 1);
        flip(b);
        root_ = merge(a, merge(b, c));
    }
    i64 query(int left, int right) {
        auto [a, tail] = split(root_, left - 1);
        auto [b, c] = split(tail, right - left + 1);
        const i64 answer = sum(b);
        root_ = merge(a, merge(b, c));
        return answer;
    }
};

int main() {
    // 중괄호 초기화는 narrowing을 막고 임시 vector를 explicit 생성자에 전달한다.
    // 문서 self-test만 재현 가능한 고정 seed를 쓴다. 적대 입력을 받는 제출용 정책과 구별한다.
    Treap sequence{{2, 1, 3, 4}, 0x6a09e667f3bcc909ULL};
    assert(sequence.query(2, 4) == 8);
    sequence.reverse(1, 4);
    assert(sequence.query(2, 4) == 6);
}
```

## 정확성 근거

### 보조정리 1: `apply_reverse(u)`는 `u`의 논리 sequence를 정확히 뒤집는다

원래 중위 순서가 `L, value(u), R`라 하자. 전체 역순은 `reverse(R), value(u), reverse(L)`이다. 자식 pointer를 맞바꾸면 바깥 순서가 `R, value(u), L`이 되고, 두 자식 tag를 나중에 토글하면 각각 내부 순서도 뒤집힌다. tag가 이미 참이었다면 다시 토글해 두 번 뒤집기를 취소한다. 따라서 실제 하강 시점과 무관하게 논리 sequence는 정확하다.

### 보조정리 2: `split(u,k)`는 앞 `k`개와 나머지를 반환한다

tree 높이에 대한 귀납법을 쓴다. 빈 tree에서는 자명하다. `push(u)` 뒤 물리 자식 방향과 논리 순서가 일치한다. 왼쪽 크기를 `L`이라 하자.

- `k <= L`: 경계가 왼쪽 subtree 안에 있다. 귀납 가정으로 왼쪽을 정확히 둘로 나눈 뒤 뒤쪽 조각을 `u.left`로 연결하면 반환 첫 tree가 정확히 앞 `k`개이고 `u` 쪽은 나머지다.
- `k > L`: 왼쪽 전체와 `u`를 앞쪽에 포함해야 한다. 오른쪽에서 `k-L-1`개를 정확히 분리해 앞 조각을 `u.right`로 연결하면 원하는 경계가 된다.

각 경우 `pull`이 바뀐 link의 크기와 합을 복구하므로 모든 불변식이 유지된다.

### 보조정리 3: `merge(A,B)`는 연결 순서와 heap 불변식을 유지한다

모든 `A` 원소가 모든 `B` 원소보다 앞선다고 하자. 더 높은 `(priority,id)` root가 `A`에 있으면 `A.left`와 root는 그대로 앞에 있고, `A.right`와 `B`만 재귀 merge해 root 오른쪽에 둔다. 반대 경우도 대칭이다. 선택한 root key가 strict total order에서 반대 root key보다 높고 기존 각 tree도 같은 key의 max-heap이므로 heap 불변식이 유지된다. 귀납적으로 경계 쪽 merge 순서도 유지된다. `pull`이 aggregate를 복구한다.

### 정리: 각 질의와 뒤집기 결과가 정확하다

보조정리 2로 정확히 `[1,a-1]`, `[a,b]`, `[b+1,n]` 세 조각을 얻는다. 합 질의는 집계 불변식의 `sum(middle)`이므로 정확하다. 뒤집기는 보조정리 1로 middle만 정확히 역순으로 만든다. 보조정리 3을 두 번 적용하면 세 조각을 원래 경계 순서대로 합치며, 따라서 목표 연산 뒤 sequence와 모든 aggregate가 정확하다.

## 시간·공간 복잡도

무작위에 가까운 독립 priority에서 treap의 기대 높이는 `O(log n)`이다.

- 초기 구축: 뒤에 하나씩 merge하면 기대 `O(n log n)`. 선형 Cartesian-tree 구축도 가능하다.
- split 또는 merge: 기대 `O(log n)`.
- 구간 reverse 또는 sum: split 2회 + merge 2회이므로 기대 `O(log n)`.
- 전체: 오늘 구현은 기대 `O((n+m) log n)` 시간, `O(n)` node arena 공간.
- 재귀 stack: 기대 `O(log n)`, priority 순서가 병적으로 단조로우면 최악 `O(n)`.

`O(log n)`은 deterministic worst-case가 아니라 **고정 입력과 독립인 seed가 만드는 priority 분포**에 대한 기대값이다. 공개된 고정 seed나 `priority = mix(id)`는 test 작성자가 reverse로 priority 순서를 재배열할 수 있어 이 전제를 만족하지 않는다. 제출에서는 실행마다 입력 밖 entropy/시각을 seed에 섞고 최대 크기에서 stack 여유를 확인한다. seed를 관찰할 수 있는 더 강한 adversarial 모델이나 확정 보장이 필요하면 deterministic balanced sequence tree를 선택한다.

## 흔한 실수

1. `split(root,a)`처럼 1-based 경계를 그대로 넘겨 한 칸 밀린다. 앞 조각 크기는 `a-1`이다.
2. `push` 전에 왼쪽 크기나 자식 방향을 읽는다. reverse tag 뒤 물리 방향이 논리 순서와 다를 수 있다.
3. link를 바꾸고 `pull`을 빼먹어 다음 split의 위치와 sum이 동시에 망가진다.
4. reverse 때 자식만 swap하고 flag를 토글하지 않아 자식 내부 순서가 뒤집히지 않는다.
5. `sum`을 32비트 `int`로 둔다. 오늘 최댓값은 `2*10^14`다.
6. priority 충돌을 무시한 비교로 구현 의미가 불명확해진다. 고유 id까지 비교해 전순서를 만든다.
7. arena를 reserve하지 않은 채 Node reference를 잡고 `push_back`한다. 재할당 뒤 reference가 댕글링된다.
8. 합 질의 뒤 세 조각을 merge하지 않아 다음 연산이 잘린 tree만 본다.
9. 기대 복잡도를 확정 최악 복잡도처럼 문서화한다.
10. 재귀 깊이 위험을 입력 크기와 priority 생성 방식으로 검증하지 않는다.
11. 공개된 고정 `mix(id)`를 독립 무작위 priority로 착각한다. reverse만으로도 알려진 priority를 단조 순서로 재배열할 수 있다.

## 변형

- 구간 add/assign: lazy tag 합성 순서와 sum 갱신식을 추가한다. assign과 add는 일반적으로 가환하지 않는다.
- 구간 min/max: subtree aggregate를 바꾸되 빈 subtree 항등원을 정확히 둔다.
- cut-and-paste: 두 번 split해 middle을 떼고 다른 위치에서 다시 split/merge한다.
- 문자열 reverse/hash: 정방향·역방향 rolling hash를 함께 보관하면 palindrome 질의를 지원할 수 있다.
- 순서 통계: `size(left)`로 k번째 원소, 삽입, 삭제를 기대 `O(log n)`에 처리한다.
- 영속 implicit treap: 변경 경로 node만 복사해 과거 버전을 보존한다. lazy tag와 공유 node를 직접 수정하지 않도록 copy-on-write가 필요하다.

## 대회 실전 노하우

- 먼저 `split` 경계를 크기 관점으로 종이에 적고, `[a,b]` 길이 `b-a+1`을 별도 변수로 둔다.
- debug 빌드에서는 각 node의 `size/sum/heap`과 inorder 길이를 재귀 검증하는 함수를 둔다.
- 작은 `n` 무작위 연산을 실제 `std::vector`의 `reverse`/누적 합 oracle과 수천 번 비교한다.
- 최대 `n,m`에서 runtime뿐 아니라 tree 최대 깊이, arena byte 수, 64비트 합을 확인한다.
- pseudo-random priority를 쓰는 이유와 seed 정책을 주석으로 남겨 재현성 있는 실패 분석을 가능하게 한다.
- 값이 아니라 위치가 암시 key이므로, tree rotation/split/merge 뒤 외부가 node index를 "현재 위치"로 기억해서는 안 된다.

## 오늘 문제와의 연결

오늘의 [`../2026-10-26/icpc_problem.cpp`](../2026-10-26/icpc_problem.cpp)는 CSES 2074 `Reversals and Sums`를 index arena 암시적 트립으로 푼다.

- `split(root,a-1)`과 `split(tail,b-a+1)`이 정확한 query 구간을 격리한다.
- `apply_reverse(middle)`은 두 자식 swap과 bool tag 토글만 해 `O(1)`에 lazy 표시한다.
- `subtree_sum(middle)`은 64비트 aggregate를 읽는다.
- `merge(left,merge(middle,right))`가 다음 연산 전에 논리 sequence를 복구한다.
- `random_device`가 주는 bit와 단조 시계 tick을 섞은 실행별 seed로 고정 입력과 priority를 분리하고, 고유 id tie-breaker로 충돌에도 엄격한 heap 순서를 만든다. 표준은 `random_device`의 실제 비결정성을 강제하지 않으므로 clock fallback은 보강책이지 암호학적 보장은 아니다.

문제 출처: [CSES 2074 — Reversals and Sums](https://cses.fi/problemset/task/2074/)
