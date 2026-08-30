# Heavy-Light Decomposition(HLD)

## 정의와 적용 조건

Heavy-Light Decomposition은 루트 트리의 간선을 **heavy**와 **light**로 나누어 임의의 트리 경로를 `O(log N)`개의 연속 구간으로 바꾸는 기법이다. 각 정점에서 서브트리 크기가 가장 큰 자식 하나로 가는 간선을 heavy로, 나머지 자식 간선을 light로 정한다. heavy 간선을 이어 만든 체인을 DFS 순서처럼 연속 인덱스로 배치하면 세그먼트 트리·펜윅 트리 같은 1차원 자료구조를 트리 경로에 적용할 수 있다.

다음 조건에서 특히 유용하다.

- 입력 그래프가 연결되고 사이클이 없는 트리다.
- 정점 또는 간선 값의 **갱신**과 두 정점 사이 경로의 합·최댓값·최솟값 질의가 섞여 있다.
- 연산이 구간 자료구조에서 결합 가능한 모노이드 성질을 가진다. 예를 들어 합의 항등값은 0, 최댓값은 값 범위에 맞는 음의 무한대 또는 양수 문제의 0이다.
- 정적 트리 구조는 바뀌지 않는다. 간선 추가·삭제까지 필요하면 link-cut tree 같은 동적 트리 기법을 검토한다.

정적 경로 질의만 있고 갱신이 없다면 LCA와 prefix 합이 더 단순할 수 있다. 서브트리 질의만 있다면 일반 Euler tour 선형화로 충분하다. HLD는 “경로를 여러 구간으로 분해해야 하는 온라인 갱신 문제”에서 빛난다.

## 핵심 아이디어와 불변식

### 1. 서브트리 크기와 heavy 자식

`size[v]`를 v를 루트로 하는 서브트리 정점 수라 하자. 각 v에서 `size`가 가장 큰 자식 하나를 `heavy[v]`로 고른다. 동률은 아무 자식이나 골라도 정확성과 복잡도가 유지된다.

light 간선 `v -> child`를 지나면 `size[child] <= size[v] / 2`다. 만약 절반보다 큰 자식이 둘이면 두 서브트리 크기의 합이 부모 서브트리보다 커져 모순이다. 따라서 루트에서 어떤 정점까지 가는 경로가 light 간선을 지날 때마다 남은 서브트리 크기가 적어도 절반으로 줄고, light 간선 수는 `O(log N)`이다.

### 2. 체인 연속성

정점 v를 배치한 직후 `heavy[v]`를 배치한다. 그러면 같은 heavy 체인의 정점 위치는 선형 배열에서 연속이다. light 자식은 새 체인의 머리 `head[child] = child`가 된다.

유지할 배열은 다음과 같다.

- `parent[v]`: 루트 방향의 바로 위 정점
- `depth[v]`: 루트에서 떨어진 간선 수
- `size[v]`: 서브트리 크기
- `heavy[v]`: 가장 큰 자식 또는 없음(0)
- `head[v]`: v가 속한 heavy 체인의 가장 위 정점
- `position[v]`: 1차원 자료구조에서 v가 차지하는 위치

### 3. 경로 분해 불변식

두 정점 u, v의 `head`가 다르면 더 깊은 head 쪽의 `[position[head[u]], position[u]]`는 아직 처리하지 않은 경로의 한 연속 부분이다. 이 구간을 질의한 뒤 `u = parent[head[u]]`로 올려도 이미 처리한 부분과 남은 부분은 겹치지 않는다.

두 head가 같아지면 남은 경로는 체인 안의 연속 구간 하나다. 정점 값 문제는 양 끝을 포함한다. **간선 값 문제**는 각 간선 값을 더 깊은 자식 위치에 저장하므로 LCA 위치를 제외한 `[position[lca] + 1, position[deeper]]`를 질의한다.

## 단계별 절차

1. 루트를 하나 정하고 DFS/BFS로 `parent`, `depth`, 순서를 구한다.
2. 역순으로 정점을 보며 `size`를 부모에 합치고 `heavy` 자식을 고른다.
3. 루트부터 heavy 자식을 먼저 방문해 `head`, `position`을 부여한다. light 자식은 새 체인으로 예약한다.
4. 정점 값이면 `position[v]`, 간선 값이면 더 깊은 끝점 `position[child]`에 값을 저장한다.
5. 한 점 갱신은 그 위치의 세그먼트 트리 값을 바꾼다.
6. 경로 질의는 head가 다를 동안 더 깊은 체인 구간을 처리하고, 마지막 같은 체인 구간을 처리한다.

## 의사 코드

```text
compute_size(v, p):
    parent[v] = p
    size[v] = 1
    for child in adjacency[v], child != p:
        depth[child] = depth[v] + 1
        compute_size(child, v)
        size[v] += size[child]
        if heavy[v] is none or size[child] > size[heavy[v]]:
            heavy[v] = child

decompose(v, chain_head):
    head[v] = chain_head
    position[v] = next_position++
    if heavy[v] exists:
        decompose(heavy[v], chain_head)
    for light child of v:
        decompose(child, child)

query_path(u, v):
    answer = identity
    while head[u] != head[v]:
        if depth[head[u]] < depth[head[v]]: swap(u, v)
        answer = combine(answer, range_query(position[head[u]], position[u]))
        u = parent[head[u]]
    if depth[u] < depth[v]: swap(u, v)
    answer = combine(answer, range_query(position[v] + EDGE_OFFSET, position[u]))
    return answer
```

`EDGE_OFFSET`은 정점 값이면 0, 간선 값을 자식 위치에 저장하면 1이다.

## 컴파일 가능한 C++ 뼈대

아래 코드는 정점 값의 경로 합을 다루는 최소 뼈대다. 실전에서는 재귀 깊이, 입력 크기, 합의 오버플로를 확인한다.

```cpp
#include <cstddef>   // std::size_t로 vector의 인덱스 타입을 명시한다.
#include <iostream>  // std::cout으로 계산 결과를 출력한다.
#include <vector>    // std::vector가 Fenwick tree 저장소를 소유한다.

// class는 기본 접근이 private이고, public API 뒤에 저장 구현을 감춘다.
class FenwickTree {
public:
    // 생성자에는 반환형이 없다. explicit은 int에서 FenwickTree로 암시 변환되는 것을 막는다.
    // vector(count, 0)는 count개의 int 0을 연속 저장소에 소유한다.
    explicit FenwickTree(int count) : tree_(static_cast<std::size_t>(count + 1), 0) {}

    // int 값 매개변수 둘은 복사된다. index 위치에 difference만큼 더한다.
    void add(int index, int difference) {
        // i & -i는 가장 낮은 1비트 값이며 다음 담당 구간으로 이동한다.
        for (int i{index + 1}; i < static_cast<int>(tree_.size()); i += i & -i) {
            // operator[]은 경계 검사 없이 int lvalue 참조를 반환하므로 반복 조건이 유효 인덱스를 보장해야 한다.
            tree_[static_cast<std::size_t>(i)] += difference;
        }
    }

    // const 멤버 함수는 tree_를 바꾸지 않고 [0, index] 합을 int 값으로 반환한다.
    [[nodiscard]] int prefix_sum(int index) const {
        // {} 값 초기화는 result를 0으로 만든다.
        int result{};
        // i -= i & -i는 현재 구간을 합친 뒤 다음 부모 구간으로 이동한다.
        for (int i{index + 1}; i > 0; i -= i & -i) {
            // vector 원소를 읽어 지역 누산값에 더하며 vector의 원소와 수명은 유지된다.
            result += tree_[static_cast<std::size_t>(i)];
        }
        return result;
    }

    // 닫힌 구간 [left, right] 합을 prefix 차이로 O(log N)에 구한다.
    [[nodiscard]] int range_sum(int left, int right) const {
        // 조건 연산자는 빈 구간이면 0, 아니면 두 prefix_sum 함수 호출의 차이를 선택한다.
        return left > right ? 0 : prefix_sum(right) - (left == 0 ? 0 : prefix_sum(left - 1));
    }

private:
    // vector<int>가 Fenwick 배열의 동적 메모리를 독점 소유한다.
    std::vector<int> tree_;
};

int main() {
    // 1-2-3이 한 heavy 체인이라고 가정한 선형 위치 예제다.
    // explicit 생성자는 중괄호 직접 초기화로 올바르게 호출한다.
    FenwickTree values{3};
    // add는 위치와 차이를 값으로 받고 tree_만 바꾼다.
    values.add(0, 4);
    values.add(1, 7);
    values.add(2, 2);
    // range_sum은 int 13을 반환하고 operator<<가 cout 버퍼에 쓴 뒤 같은 ostream&로 개행을 연쇄한다.
    std::cout << values.range_sum(0, 2) << '\n';
    // 0 반환은 정상 종료이며 지역 values가 먼저 파괴되어 vector 저장소를 해제한다.
    return 0;
}
```

이 뼈대는 선형 구간 자료구조만 보여 준다. 실제 HLD의 전처리와 경로 분할은 오늘의 [`../2026-08-31/icpc_problem.cpp`](../2026-08-31/icpc_problem.cpp)에 재귀 없는 형태로 구현되어 있다.

## 정확성 근거

### 보조정리 1: 같은 heavy 체인은 연속 구간이다

선형화에서 v 다음에 heavy[v]를 배치하고 그 과정을 반복한다. 귀납적으로 체인의 첫 정점부터 마지막 정점까지 다른 체인의 정점이 끼어들지 않으므로 위치가 연속이다.

### 보조정리 2: 경로 분해 반복은 빠짐과 중복이 없다

head가 다를 때 더 깊은 head부터 현재 정점까지는 u-v 경로 위의 연속 부분이다. 그 부분을 처리하고 u를 head의 부모로 바꾸면 처리한 간선 아래를 모두 제거하며, 남은 두 정점 사이 경로는 정확히 미처리 부분이다. 각 반복에서 이 불변식이 유지된다.

### 보조정리 3: 반복 횟수는 O(log N)이다

한 체인을 벗어나는 이동은 light 간선을 위로 건넌다. light 자식의 서브트리 크기는 부모의 절반 이하이므로 아래로 볼 때 light 간선을 한 번 지날 때 가능한 서브트리 크기가 절반 이하가 된다. 크기는 1 미만이 될 수 없으므로 경로당 light 간선 수는 O(log N)이다.

### 정리

보조정리 1로 각 처리 조각은 1차원 자료구조의 정확한 연속 구간이고, 보조정리 2로 그 조각들의 합집합은 원래 경로와 같으며 겹치지 않는다. 결합 연산이 구간 순서와 문제 계약에 맞으면 모든 구간 결과를 합친 값이 경로 답이다. 보조정리 3으로 조각 수가 O(log N)이므로 세그먼트 트리 O(log N) 질의를 결합한 경로 질의는 O(log² N)이다.

## 시간·공간 복잡도

- 부모·서브트리·체인 전처리: `O(N)`
- 세그먼트 트리 초기화: 점 갱신 N회면 `O(N log N)`, bottom-up build면 `O(N)`
- 한 점/간선 갱신: `O(log N)`
- 경로 질의: 체인 `O(log N)`개 × 구간 질의 `O(log N)` = `O(log² N)`
- 배열·인접 리스트·세그먼트 트리 공간: `O(N)`

결합 연산이 가환적이지 않으면 u 쪽과 v 쪽 조각의 방향을 따로 보존해야 한다. 문자열 합성이나 행렬 곱처럼 순서가 중요한 연산은 단순히 `answer = combine(answer, segment)`만 하면 틀릴 수 있다.

## 흔한 실수

1. 간선 값을 부모 위치에 저장해 루트 또는 LCA를 잘못 포함한다.
2. 마지막 같은 체인에서 간선 질의인데 `position[lca]`를 포함한다.
3. head가 다른데 정점 depth만 비교하고 `depth[head[u]]`와 `depth[head[v]]`를 비교하지 않는다.
4. heavy 자식 선택 전에 서브트리 크기 계산이 끝나지 않았다.
5. 최댓값 항등값을 0으로 두면서 음수 가중치도 허용하는 문제에 그대로 적용한다.
6. 재귀 DFS를 N=100,000 일자 트리에 사용하면서 실행 환경의 스택 한도를 확인하지 않는다.
7. 간선 번호를 갱신 위치와 연결하지 않아 입력 방향이 부모-자식 방향과 반대일 때 틀린 끝점을 갱신한다.
8. `O(log² N)`을 무조건 `O(log N)`이라고 적는다. 세그먼트 트리 구간 질의가 각 체인마다 한 번 더 로그 비용을 낸다.

## 변형과 대회 필수 연결

- 정점 값/간선 값: 마지막 구간의 LCA 포함 여부만 달라지는 경우가 많다.
- 경로 합·최댓값·최솟값·XOR: 알맞은 항등값과 결합 함수를 세그먼트 트리에 둔다.
- lazy segment tree 결합: 경로 전체 갱신과 경로 질의를 모두 지원할 수 있다.
- subtree + path 혼합: HLD 위치는 DFS 순서를 유지하므로 서브트리도 대개 한 연속 구간이다.
- LCA: HLD의 head 점프만으로 LCA를 구하거나 이진 리프팅과 함께 쓸 수 있다.
- 순서가 있는 연산: 정방향/역방향 세그먼트 값을 둘 다 저장하거나 양쪽 누산기를 둔다.
- 더 빠른 경로 질의: 문제 구조에 따라 세그먼트 트리 누산 최적화 또는 특수 자료구조로 `O(log N)`이 가능하지만 기본 HLD의 안전한 목표는 `O(log² N)`이다.

HLD는 ICPC에서 “트리 경로 + 온라인 갱신”을 보는 순간 후보로 떠올려야 하는 필수 도구다. 다만 먼저 prefix/LCA, Euler tour, DSU on tree처럼 더 단순한 해법이 조건을 충족하는지 확인하는 습관이 중요하다.

## 오늘 문제와의 연결

BOJ 13510은 간선 번호 갱신과 경로 최댓값을 요구한다. 오늘 구현은 각 입력 간선 번호를 더 깊은 자식으로 매핑하고, 그 자식의 HLD 위치에 가중치를 저장한다. `type 1`은 세그먼트 트리 한 점을 바꾸고, `type 2`는 두 정점 경로를 체인 구간으로 나눠 최댓값을 합친다. 마지막 구간에서 `position[lca] + 1`을 쓰는 이유를 설명할 수 있어야 풀이를 이해한 것이다.

## 직접 해보기와 초보자 검증

1. 정점 1이 2, 3, 4를 자식으로 갖고 2 아래에 5, 6이 있는 트리의 `size`와 heavy 간선을 손으로 표시한다.
2. 같은 트리에서 5에서 3으로 가는 경로가 어떤 HLD 구간으로 나뉘는지 적는다.
3. 간선 값을 자식 위치에 저장할 때 5와 2 사이 질의가 왜 `position[2] + 1`부터 시작하는지 설명한다.
4. 오늘 코드의 `+ 1`을 제거하고 CTest에 같은 정점 질의와 조상-자손 질의를 추가해 실패를 관찰한다.
5. 세그먼트 트리 `max`를 `sum`으로 바꾸고 작은 트리의 경로 합을 brute force DFS와 1,000회 비교한다.
6. N=100,000 일자 트리와 별 모양 트리를 만들어 전처리 배열이 모두 채워지고 실행 스택을 소모하지 않는지 확인한다.
7. 자료 없이 `parent`, `depth`, `size`, `heavy`, `head`, `position` 각각을 한 문장으로 정의하고 경로 질의 복잡도를 증명한다.
