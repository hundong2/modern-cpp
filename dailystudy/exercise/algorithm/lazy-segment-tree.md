# 지연 전파 세그먼트 트리(Lazy Segment Tree)

## 정의

세그먼트 트리는 배열의 연속 구간 정보를 이진 트리의 노드에 저장하는 자료구조다. 각 잎은 원소 하나를, 각 내부 노드는 두 자식 구간을 합친 결과를 나타낸다. 합, 최솟값, 최댓값, 1의 개수처럼 두 인접 구간의 답을 빠르게 결합할 수 있을 때 구간 질의를 `O(log N)`에 처리한다.

지연 전파는 **구간 전체 변경**도 `O(log N)`에 처리하기 위한 확장이다. 갱신 구간에 완전히 포함되는 노드를 만나면 모든 잎을 즉시 바꾸지 않는다. 그 노드의 요약 값만 갱신하고 자식에 나중에 적용할 연산을 `lazy`에 기록한다. 자식의 일부를 실제로 방문해야 할 때만 `push`로 연산을 내려보낸다.

## 적용 조건

- 배열이 고정된 인덱스 순서를 가지며 연속 구간 질의와 변경이 반복된다.
- 두 자식의 요약을 결합해 부모 요약을 `O(1)`에 만들 수 있다. 예: 합은 `left_sum + right_sum`이다.
- 구간 전체에 같은 연산을 적용했을 때 노드 요약을 구간 길이와 현재 요약만으로 빠르게 바꿀 수 있다.
- 여러 미처리 연산을 하나의 지연 값으로 합성할 수 있다. 합성 순서가 교환 가능할 필요는 없지만 정확히 정의해야 한다.
- 명령 사이에 즉시 답해야 하므로 차분 배열처럼 모든 갱신을 모은 뒤 한 번에 복원하는 오프라인 방법만으로 부족하다.

점 갱신과 구간 합만 필요하면 펜윅 트리가 더 짧고 상수도 작을 수 있다. 구간 갱신과 구간 질의가 모두 필요하거나 합성 연산이 복잡하면 지연 전파 세그먼트 트리가 자연스럽다.

## 핵심 아이디어와 불변식

구간 `[left, right]`를 담당하는 노드 `node`에 대해 다음을 유지한다.

1. `tree[node]`는 `lazy[node]`가 아직 자식에 전달되지 않았더라도 **현재 논리 배열의 해당 구간 요약과 항상 같다**.
2. `lazy[node]`는 `node`의 두 자식 요약에 아직 반영하지 않은 합성 연산이다.
3. `lazy[node]`가 항등 연산이면 자식에 내려보낼 미처리 작업이 없다.
4. 일부 구간을 방문하기 전 `push(node)`를 수행하면 두 자식의 요약과 지연 값도 현재 논리 배열에 맞게 갱신된다.
5. 자식 재귀 호출이 끝난 뒤 `pull(node)` 또는 직접 결합을 수행하면 `tree[node] = combine(tree[left_child], tree[right_child])`가 복구된다.

가장 중요한 점은 부모 요약은 즉시 최신이라는 사실이다. 자식만 잠시 뒤처질 수 있다. 그래서 질의 구간이 부모 구간을 완전히 포함하면 `push` 없이 부모 값을 바로 반환할 수 있다. 일부만 포함해 자식으로 내려갈 때는 반드시 먼저 `push`한다.

## 연산 합성

`compose(existing, incoming)`은 기존에 미룬 연산 뒤에 새 연산을 적용한 결과여야 한다. 함수 합성처럼 순서가 중요하다.

범위 덧셈에서는 `existing + incoming`으로 충분하다. UVa 11402처럼 0 대입, 1 대입, 반전이 함께 있으면 다음 규칙을 쓴다.

- 새 `set_zero` 또는 `set_one`은 이전 연산을 덮어쓴다.
- 새 `invert`는 `none -> invert`, `invert -> none`이다.
- 새 `invert`는 `set_zero -> set_one`, `set_one -> set_zero`로 바뀐다.
- `set_zero` 뒤 `invert`는 `set_one`이지만, `invert` 뒤 `set_zero`는 `set_zero`다. 즉 합성은 일반적으로 교환법칙을 만족하지 않는다.

## 단계별 절차

### 1. 빌드

1. 잎 노드는 원소 하나의 요약 값으로 초기화한다.
2. 내부 노드는 왼쪽과 오른쪽 자식을 재귀적으로 만든다.
3. 두 자식 요약을 결합해 부모 값을 만든다.
4. 모든 지연 값은 항등 연산으로 시작한다.

### 2. 노드 전체 적용 `apply`

1. 연산과 구간 길이를 사용해 `tree[node]`를 즉시 바꾼다.
2. 기존 `lazy[node]` 뒤에 새 연산이 오도록 합성한다.
3. 자식으로는 아직 내려가지 않는다.

범위 덧셈과 구간 합이라면 길이가 `len`일 때 `tree[node] += delta * len`이고 `lazy[node] += delta`다. 이 곱셈을 빼먹으면 구간 합이 아니라 원소 하나만 바꾼 셈이 된다.

### 3. 전파 `push`

1. 현재 지연 값이 항등 연산이면 끝낸다.
2. 왼쪽 자식과 오른쪽 자식에 `apply`한다.
3. 부모의 지연 값을 항등 연산으로 되돌린다.

### 4. 구간 갱신

1. 현재 구간과 목표 구간이 겹치지 않으면 반환한다.
2. 현재 구간이 목표 구간에 완전히 포함되면 `apply`하고 반환한다.
3. 일부만 겹치면 `push`한 뒤 두 자식을 재귀 갱신한다.
4. 두 자식 요약을 결합해 부모를 다시 계산한다.

### 5. 구간 질의

1. 겹치지 않으면 결합 연산의 항등원을 반환한다. 합의 항등원은 0이다.
2. 완전히 포함되면 `tree[node]`를 반환한다.
3. 일부만 겹치면 `push`하고 두 자식 질의를 결합한다.

## 의사 코드

```text
apply(node, left, right, operation):
    tree[node] = operation이 전체 구간 요약에 미치는 결과
    lazy[node] = compose(lazy[node], operation)

push(node, left, right):
    if lazy[node]가 항등 연산 or left == right: return
    middle = left + (right - left) / 2
    apply(left_child, left, middle, lazy[node])
    apply(right_child, middle + 1, right, lazy[node])
    lazy[node] = 항등 연산

update(node, left, right, query_left, query_right, operation):
    if 두 구간이 겹치지 않음: return
    if 현재 구간이 목표에 완전히 포함됨:
        apply(node, left, right, operation)
        return
    push(node, left, right)
    update(left_child, ...)
    update(right_child, ...)
    tree[node] = combine(tree[left_child], tree[right_child])

query(node, left, right, query_left, query_right):
    if 두 구간이 겹치지 않음: return identity
    if 현재 구간이 목표에 완전히 포함됨: return tree[node]
    push(node, left, right)
    return combine(query(left_child, ...), query(right_child, ...))
```

## 컴파일 가능한 C++ 뼈대

다음 예제는 구간 덧셈과 구간 합을 구현한다. 오늘 문제의 대입·반전도 `apply`와 지연 연산 합성 규칙만 바꾸면 같은 탐색 구조를 재사용한다.

```cpp
// <iostream>은 실행 결과를 표준 출력으로 보여 주는 std::cout을 제공한다.
#include <iostream>
// <vector>는 원본 배열, 트리 합, 지연 덧셈 값을 소유한다.
#include <vector>

// long long은 여러 원소의 합과 누적 덧셈이 int 범위를 넘을 때를 대비한다.
using Value = long long;

// class의 기본 접근은 private이며 public 함수로만 자료구조 상태를 바꾸게 한다.
class RangeAddSumTree {
public:
    // 생성자는 반환형이 없고 explicit은 vector가 트리로 암시 변환되는 것을 막는다.
    explicit RangeAddSumTree(const std::vector<Value>& values)
        // size_는 배열 길이를 재귀 인덱스에 쓰기 쉬운 int로 보관한다.
        : size_{static_cast<int>(values.size())},
          // 4*N은 재귀 세그먼트 트리 노드 수의 안전한 상한이다.
          tree_(values.size() * 4U, 0),
          // 덧셈의 항등원 0으로 모든 지연 값을 초기화한다.
          lazy_(values.size() * 4U, 0) {
        // 빈 배열이면 [0,-1] 구간을 만들지 않는다.
        if (size_ > 0) {
            build(1, 0, size_ - 1, values);
        }
    }

    // 닫힌 구간 [left,right]의 모든 값에 delta를 더한다.
    void add(int left, int right, Value delta) {
        update(1, 0, size_ - 1, left, right, delta);
    }

    // [[nodiscard]]는 구간 합 결과를 실수로 버릴 때 경고할 수 있게 한다.
    [[nodiscard]] Value sum(int left, int right) {
        return query(1, 0, size_ - 1, left, right);
    }

private:
    // 잎에서 값을 읽고 내부 노드에서 두 자식 합을 만드는 O(N) 빌드다.
    void build(int node, int left, int right,
               const std::vector<Value>& values) {
        // left==right이면 원소 하나를 담당하는 잎이다.
        if (left == right) {
            // [] 연산자는 vector의 해당 인덱스 원소에 접근한다.
            tree_[node] = values[static_cast<std::size_t>(left)];
            return;
        }

        // 이 중간점 공식은 left+right가 큰 경우의 덧셈 오버플로를 피한다.
        const int middle{left + (right - left) / 2};
        // node*2는 왼쪽 자식, node*2+1은 오른쪽 자식 인덱스다.
        build(node * 2, left, middle, values);
        build(node * 2 + 1, middle + 1, right, values);
        // 부모 구간 합은 두 자식 구간 합의 + 결합이다.
        tree_[node] = tree_[node * 2] + tree_[node * 2 + 1];
    }

    // 현재 노드 전체에 delta 덧셈을 즉시 반영한다.
    void apply(int node, int left, int right, Value delta) {
        // 닫힌 구간 길이는 right-left+1이고 각 원소가 delta만큼 증가한다.
        const Value length{static_cast<Value>(right - left + 1)};
        tree_[node] += delta * length;
        // 덧셈 연산의 합성은 기존 값과 새 delta를 더하는 것이다.
        lazy_[node] += delta;
    }

    // 부모의 미처리 덧셈을 두 자식에 전달한다.
    void push(int node, int left, int right) {
        // 지연 값 0은 항등 연산이고 잎은 자식이 없으므로 끝낸다.
        if (lazy_[node] == 0 || left == right) {
            return;
        }

        const int middle{left + (right - left) / 2};
        // 두 자식은 각자 담당 길이에 맞춰 같은 delta를 적용한다.
        apply(node * 2, left, middle, lazy_[node]);
        apply(node * 2 + 1, middle + 1, right, lazy_[node]);
        // 부모의 지연 작업을 모두 넘겼으므로 항등원으로 되돌린다.
        lazy_[node] = 0;
    }

    // 목표 닫힌 구간에 delta를 더하는 재귀 함수다.
    void update(int node, int left, int right,
                int query_left, int query_right, Value delta) {
        // query가 현재 구간의 왼쪽 또는 오른쪽에 완전히 벗어나면 겹치지 않는다.
        if (query_right < left || right < query_left) {
            return;
        }
        // 현재 구간이 query 안에 완전히 들어가면 잎까지 내려가지 않는다.
        if (query_left <= left && right <= query_right) {
            apply(node, left, right, delta);
            return;
        }

        // 자식 일부를 바꾸기 전에 부모의 미처리 연산을 내려 자식 값을 최신으로 만든다.
        push(node, left, right);
        const int middle{left + (right - left) / 2};
        // 겹치는 자식만 실제 일을 하며 겹치지 않는 쪽은 조기 반환한다.
        update(node * 2, left, middle, query_left, query_right, delta);
        update(node * 2 + 1, middle + 1, right, query_left, query_right, delta);
        // 자식 갱신 뒤 부모 합 불변식을 복구한다.
        tree_[node] = tree_[node * 2] + tree_[node * 2 + 1];
    }

    // 목표 닫힌 구간의 합을 반환하는 재귀 함수다.
    [[nodiscard]] Value query(int node, int left, int right,
                              int query_left, int query_right) {
        // 겹치지 않는 구간은 합의 항등원 0을 반환한다.
        if (query_right < left || right < query_left) {
            return 0;
        }
        // 완전 포함이면 부모 요약이 이미 최신이므로 바로 반환한다.
        if (query_left <= left && right <= query_right) {
            return tree_[node];
        }

        // 일부 포함이면 자식 합을 읽기 전에 미처리 덧셈을 내려보낸다.
        push(node, left, right);
        const int middle{left + (right - left) / 2};
        // 두 자식의 겹치는 부분 합을 + 연산자로 결합한다.
        return query(node * 2, left, middle, query_left, query_right)
             + query(node * 2 + 1, middle + 1, right, query_left, query_right);
    }

    // class의 private 멤버는 외부가 불변식을 깨뜨리는 직접 수정을 하지 못하게 한다.
    int size_{};
    std::vector<Value> tree_{};
    std::vector<Value> lazy_{};
};

// main은 작은 배열에 갱신과 질의를 실행하는 초보자 검증 진입 함수다.
int main() {
    // vector<Value>{...} prvalue가 const 참조 매개변수에 바인딩되는 동안 생성자 호출까지 살아 있다.
    RangeAddSumTree tree{std::vector<Value>{1, 2, 3, 4}};
    // 인덱스 1..3에 5를 더하면 배열은 [1,7,8,9]가 된다.
    tree.add(1, 3, 5);
    // 전체 합은 25이고 부분 구간 1..2의 합은 15다.
    std::cout << tree.sum(0, 3) << '\n';
    std::cout << tree.sum(1, 2) << '\n';
    // 0은 정상 종료를 뜻하는 int prvalue다.
    return 0;
}
```

예상 출력은 다음과 같다.

```text
25
15
```

## 정확성 근거

### 빌드의 정확성

구간 길이에 대한 귀납법을 쓴다. 길이 1인 잎은 원본 원소를 그대로 저장하므로 정확하다. 길이 2 이상인 구간에서 귀납 가정으로 두 자식 요약이 정확하다. 부모는 문제에 맞는 결합 연산으로 두 자식을 합치므로 부모 요약도 정확하다. 따라서 루트를 포함한 모든 노드가 정확하다.

### 완전 포함 갱신의 정확성

`apply`는 노드 담당 구간의 모든 원소에 같은 연산을 했을 때의 요약을 직접 계산한다. 그러므로 부모 `tree[node]`는 즉시 논리 배열과 일치한다. 자식은 아직 바뀌지 않았지만 `lazy[node]`가 나중에 적용할 정확한 합성 연산을 저장한다. 핵심 불변식이 보존된다.

### 전파의 정확성

`push`는 부모가 보관한 연산을 두 자식 전체에 각각 `apply`한다. 두 자식 구간은 겹치지 않고 합치면 부모 구간 전체이므로 모든 원소가 정확히 한 번 같은 연산을 받는다. 이후 부모 지연 값을 항등원으로 바꿔 중복 적용을 막는다. 부모 요약은 이미 정확했고 자식 요약도 정확해진다.

### 구간 갱신의 정확성

겹치지 않는 노드는 실제 목표 원소를 포함하지 않으므로 바꾸지 않는 것이 맞다. 완전 포함 노드는 `apply`의 정확성으로 맞다. 일부 포함 노드는 `push` 뒤 두 자식에 같은 논리를 재귀 적용한다. 두 자식이 목표 구간과 겹치는 모든 원소를 빠짐없이, 중복 없이 나누며, 마지막 결합으로 부모 요약을 복구한다. 따라서 갱신 뒤 전체 트리 불변식이 유지된다.

### 구간 질의의 정확성

겹치지 않는 구간은 항등원을 반환해 결과에 영향을 주지 않는다. 완전 포함 구간은 불변식에 따라 최신 요약을 반환한다. 일부 포함 구간은 `push`로 자식을 최신화한 뒤 두 자식 결과를 결합한다. 표준 구간들이 목표 구간을 빠짐없이 겹치지 않게 덮으므로 최종 답이 정확하다.

## 시간·공간 복잡도

- 빌드: 각 노드를 한 번 방문하므로 `O(N)` 시간
- 구간 갱신: 목표 경계당 트리 높이만큼 내려가고 완전 포함 노드는 멈추므로 `O(log N)` 시간
- 구간 질의: 목표 구간을 `O(log N)`개의 표준 구간으로 분해하므로 `O(log N)` 시간
- Q개 명령 전체: `O(N + Q log N)` 시간
- 트리와 지연 배열: 노드 수가 `O(N)`이므로 `O(N)` 공간
- 재귀 호출 스택: 균형 이진 트리 높이인 `O(log N)` 공간

여러 구간으로 복잡하게 쪼개져도 한 번의 연속 구간 질의가 방문하는 노드는 `O(log N)`개다. 단, 서로 떨어진 임의의 많은 구간을 한 명령으로 묶는 다른 문제는 별도 분석이 필요하다.

## 흔한 실수

- 닫힌 구간 길이를 `right-left`로 계산한다. 양끝을 포함하면 `right-left+1`이다.
- 완전 포함 갱신에서 `tree[node]`만 바꾸고 `lazy[node]`를 기록하지 않는다. 나중에 자식 일부를 읽으면 옛 값이 나온다.
- 일부 포함 질의나 갱신 전에 `push`하지 않는다.
- `push` 뒤 부모 지연 값을 항등원으로 초기화하지 않아 같은 연산을 여러 번 적용한다.
- 새 지연 연산을 기존 값 위에 올바른 순서로 합성하지 않는다. 대입과 반전은 순서를 바꿀 수 없다.
- 범위 덧셈의 구간 합에서 `delta * length`가 아니라 `delta`만 더한다.
- 합이 커질 수 있는데 `int`를 사용한다. 입력 범위와 최대 누적합을 계산해 `long long`을 고른다.
- 0 기반과 1 기반 인덱스를 섞거나 닫힌 구간 `[l,r]`과 반열린 구간 `[l,r)`을 섞는다.
- 트리 배열을 너무 작게 잡아 자식 인덱스가 범위를 넘는다. 재귀 구현에서는 보통 `4*N`을 사용한다.
- `min` 질의의 겹치지 않는 반환값을 0으로 둔다. 결합 연산에 맞는 항등원 또는 충분히 큰 값이 필요하다.

## 변형

- **구간 덧셈 + 구간 합**: 지연 값은 덧셈 수치 하나이며 합성은 덧셈이다.
- **구간 대입 + 구간 합**: 대입 여부와 대입 값을 함께 저장한다. 새 대입은 이전 대입을 덮어쓴다.
- **구간 대입/덧셈 + 구간 합**: 대입과 덧셈의 우선순위 및 합성 순서를 명시해야 한다.
- **구간 반전 + 1 개수**: 구간 합을 `length-sum`으로 바꾸고 반전 두 번을 항등원으로 합성한다.
- **구간 affine 변환**: `x -> a*x+b`를 저장하고 함수 합성 순서로 지연 값을 결합한다. 모듈러 합 문제에서 자주 나온다.
- **구간 최솟값/최댓값**: 연산이 노드 요약에 미치는 영향을 `O(1)`에 계산할 수 있어야 한다.
- **동적 세그먼트 트리**: 좌표 범위가 매우 크고 실제 접근 위치가 적을 때 방문 노드만 동적으로 만든다.
- **영속 세그먼트 트리**: 갱신 때 바뀐 경로만 복사해 과거 버전을 보존한다. 지연 전파와 함께 쓰면 수명과 복사 규칙이 더 복잡하다.
- **반복형 세그먼트 트리**: 점 갱신/구간 질의는 짧아지지만 복잡한 지연 전파는 재귀형이 이해하기 쉬운 경우가 많다.

## 오늘 문제와의 연결

2026-08-17의 [UVa 11402 Ahoy, Pirates!](../2026-08-17/icpc_problem.cpp)는 각 노드에 1의 개수와 네 지연 상태 `none`, `set_zero`, `set_one`, `invert`를 저장한다.

- `F`: 해당 노드 합을 구간 길이로 만들고 지연 상태를 `set_one`으로 덮어쓴다.
- `E`: 해당 노드 합을 0으로 만들고 지연 상태를 `set_zero`로 덮어쓴다.
- `I`: 합을 `length-sum`으로 바꾸고 기존 지연 상태를 반전 규칙에 따라 합성한다.
- `S`: 완전 포함 노드의 합을 즉시 사용하고, 일부 포함일 때만 `push`한다.

단순 구간 합보다 어려운 부분은 트리 탐색이 아니라 **연산 합성 표를 정확히 만드는 일**이다. 합성 함수를 독립된 `compose`로 두면 16가지 상태 조합을 표나 단위 테스트로 검증할 수 있고 대회 중 디버깅도 쉬워진다.

## 직접 해보기와 초보자 검증

1. 길이 8 배열에서 구간 `[2,6]`이 어떤 세그먼트 트리 표준 구간으로 분해되는지 트리를 그린다.
2. 범위 덧셈 예제에서 `[1,3] += 5` 뒤 루트·두 자식의 `tree`와 `lazy` 값을 손으로 적는다.
3. `push`를 지운 뒤 부분 질의가 틀리는 최소 반례를 만든다.
4. `set_zero`, `set_one`, `invert`, `none`의 4×4 합성 표를 작성한다. 행을 기존 연산, 열을 새 연산으로 정하고 순서를 바꾸지 않는다.
5. 오늘 ICPC 코드에서 `set_zero -> invert -> invert`가 최종적으로 `set_zero`가 되는지 `compose` 호출을 따라간다.
6. C++ 뼈대를 직접 컴파일해 `25`, `15`가 나오는지 확인하고 `[0,0] += -1` 테스트를 추가한다.
7. 구간 합 타입을 `int`로 바꿨을 때 `N=100000`, 각 값과 갱신이 `10^9`이면 어떤 최댓값에서 오버플로하는지 계산한다.
8. 재귀 함수 네 개 `apply`, `push`, `update`, `query`를 보지 않고 다시 쓰고 각 함수가 보존하는 불변식을 한 문장으로 붙인다.
