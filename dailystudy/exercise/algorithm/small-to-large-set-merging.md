# 작은 집합을 큰 집합으로 합치기

## 정의

**Small-to-large merging**은 여러 부분 문제의 컨테이너를 합칠 때, 항상 더 작은 쪽의 원소만 더 큰 쪽으로
옮기는 기법이다. 트리에서는 자식 서브트리의 집합을 부모 집합에 합쳐야 할 때 자주 사용한다. 합칠 방향을
고정하지 않으면 같은 원소가 긴 트리에서 `O(n)`번씩 이동할 수 있지만, 작은 bag만 소비하도록 만들면 한
원소가 작은 쪽에 속해 이동할 때마다 자신이 들어간 bag의 무게가 적어도 두 배가 된다.

오늘 다루는 대표 문제는 [CSES 1139 — Distinct Colors](https://cses.fi/problemset/task/1139/)다. 루트가
1인 트리에서 각 정점 `v`에 대해 `v`의 서브트리에 나타나는 서로 다른 색의 개수를 구한다. 각 서브트리의
색을 처음부터 다시 세면 경로 모양 트리에서 `O(n^2)`이므로, 자식이 이미 만든 색 집합을 부모가 재사용한다.

이 문서의 구현은 다음 두 값을 한 bag에 둔다.

- `colors`: 현재 bag이 대표하는 정점들의 서로 다른 색을 담는 `std::unordered_set<int>`
- `population`: 현재 bag이 대표하는 **원래 정점 수**

`population`을 무게로 비교하는 이유는 색 중복과 무관한 엄밀한 상각 분석을 얻기 위해서다. 서로 다른 색의
수만 비교해도 실전에서는 흔히 통과하지만, 두 집합에 겹치는 색이 많으면 “살아남은 키가 옮길 때마다 집합
크기가 정확히 두 배가 된다”는 단순한 증명은 그대로 적용되지 않는다.

## 적용 조건

다음 조건에서 특히 유용하다.

- 입력이 트리이거나, 부분 문제들이 합쳐져 더 큰 부분 문제 하나가 되는 계층 구조다.
- 각 부분 문제의 답을 집합, 맵, multiset 또는 그와 비슷한 merge 가능한 상태로 표현할 수 있다.
- 부모 답을 만들 때 자식 상태를 이후 독립적으로 보존할 필요가 없다. 이 기법은 작은 상태를 **소비**한다.
- 모든 원소를 매번 새 컨테이너에 복사하는 `O(n^2)` 구현은 느리지만, 전체 원소 수에 로그 인자를 곱한
  정도는 허용된다.
- 색 값처럼 값의 범위가 크고 좌표 압축 없이 동일성만 검사하면 되는 경우 해시 집합이 편리하다.

다음 상황에서는 다른 방법이 더 낫다.

- 자식 버전의 집합을 합친 뒤에도 질의해야 한다면 영속 자료구조나 오프라인 질의를 고려한다.
- 색 우주가 작다면 bitset의 OR가 더 단순하고 빠를 수 있다.
- 적대적인 입력에서도 결정적 시간 상한이 필요하면 `std::set` 또는 정렬 벡터 병합을 고려한다.
- 서브트리마다 합치기보다 Euler tour 구간 질의로 바꿀 수 있다면 Fenwick tree를 이용한 오프라인 풀이도
  CSES 1139를 `O(n log n)`에 해결한다.

## 핵심 아이디어와 불변식

### 1. 부모 우선 순서와 역순 후위 처리

루트 1에서 반복 탐색을 수행해 `parent`와 `order`를 만든다. DFS 스택과 FIFO worklist 중 무엇을 써도
`order`에 부모가 항상 자식보다 먼저 등장하게 하면 충분하다. 따라서 `order`를 뒤에서 앞으로 읽으면 모든
자식 bag이 부모보다 먼저 완성된다. 아래 뼈대는 명시적 `vector<int>` 스택 DFS를 쓰고, 오늘 제출 코드는
같은 불변식을 만족하는 cursor 기반 FIFO `vector<int>`를 쓴다. 둘 다 `n=200000` 경로에서 재귀 호출
스택을 넘치는 위험을 없앤다.

### 2. 완성된 bag의 의미

정점 `v` 처리가 끝났을 때 다음 불변식을 유지한다.

> `bags[v].population`은 `v` 서브트리의 정점 수이고, `bags[v].colors`는 그 서브트리에 등장하는 서로
> 다른 색의 집합이며, `answer[v] == bags[v].colors.size()`다.

처리를 시작할 때는 `v` 자신만 담아 `population=1`, `colors={color[v]}`로 둔다. 자식 bag은 역순 처리
덕분에 이미 위 불변식을 만족한다.

### 3. 작은 무게를 큰 무게로 합치기

현재까지 부모 쪽에 모은 bag을 `destination`, 다음 자식 bag을 `source`라고 하자. 두 bag의
`population`을 비교해 작은 쪽이 `source`가 되도록 소유 포인터를 교환한다. 그런 다음
`destination.colors.merge(source.colors)`로 목적지에 없던 node를 이전하고 두 `population`을 더한다.
목적지와 중복인 키는 source에 남으므로 마지막에 source bag을 파괴한다.

합치기 직전에 `source.population <= destination.population`이고, 합친 뒤의 무게는 두 값의 합이다.
따라서 source가 대표하던 모든 원래 정점은 이전보다 적어도 두 배 무거운 bag에 들어간다.

```text
new_population
= source_population + destination_population
>= 2 * source_population
```

어떤 원래 정점도 무게 `1 -> 2 -> 4 -> ... -> n`을 넘어 이동할 수 없으므로 작은 쪽에 속하는 횟수는
최대 `floor(log2(n)) + 1`이다. 한 번의 merge에서 순회하는 서로 다른 색 수는 source의 정점 수보다 클
수 없으므로, 전체 해시 삽입 시도 횟수도 `O(n log n)`이다.

### 4. 소유권 불변식

각 미처리 조상 경계에서 원래 정점의 정보는 정확히 하나의 살아 있는 bag에 속한다. 구현은
`std::unique_ptr<Bag>`을 사용한다.

- 포인터 교환은 bag 내부의 해시 노드를 복사하지 않고 어느 bag을 계속 살릴지만 바꾼다.
- source의 비중복 색 node를 이전한 뒤 `reset()`하면 소비된 해시 테이블과 남은 중복 키 저장소가 즉시
  해제된다.
- 자식의 `answer`는 자식 bag을 소비하기 전에 이미 값으로 저장했으므로, 자식 bag 자체는 보존할 필요가
  없다.

### 5. 해시 컨테이너의 보장 범위

`std::unordered_set::insert`는 해시가 고르게 분포한다는 평균적 모델에서 상각 `O(1)`이다. 이 모델에서는
삽입 시도 `O(n log n)`개를 합쳐 전체 평균 시간이 `O(n log n)`이다. 그러나 C++ 표준은 적대적 충돌이
없는 것을 보장하지 않는다. 모든 키가 같은 bucket에 몰리면 한 번의 탐색·삽입이 현재 집합 크기에 비례할
수 있다. 표준의 한 merge 최악 상한은 source 원소 수를 `S`, 호출 전 destination 원소 수를 `D`라 할 때
`O(S*D + S)`다. 전체 merge의 `S*D`는 서로 다른 두 원래 정점의 정보가 처음 만나는 사건에 충전할 수
있어 그 합이 `O(n^2)`이다. 따라서 다음을 구분해야 한다.

- **평균/기대 시간**: `O(n log n)`
- **보수적인 최악 상한**: 병적인 해시 충돌까지 포함하면 `O(n^2)`
- **알고리즘 공간**: 저장된 키 객체, 트리, 부모·순서·답 배열을 합쳐 `O(n)`

온라인 저지의 정수 색에는 기본 해시가 보통 충분하지만, 보안 경계나 해시 충돌 공격이 가능한 입력에서는
이 기대 시간에 의존하면 안 된다. `std::set`으로 바꾸면 삽입 한 번이 `O(log n)`이어서 전체
`O(n log^2 n)`이라는 결정적 상한을 얻는 대신 노드 할당과 포인터 추적 상수가 커진다.

## 단계별 절차

1. 무방향 트리와 각 정점의 색을 읽는다.
2. 루트 1에서 명시적 stack DFS 또는 FIFO worklist 탐색으로 `parent`와 부모 우선 `order`를 만든다.
3. `order`를 역순으로 순회한다.
4. 현재 정점 `v`만 담은 `population=1`, `colors={color[v]}` bag을 만든다.
5. 인접 정점 중 `parent[child] == v`인 직접 자식만 고른다.
6. 현재 bag과 자식 bag 중 `population`이 작은 쪽이 source가 되도록 포인터를 교환한다.
7. `destination.colors.merge(source.colors)`로 중복이 아닌 node를 이전하고 population을 더한다.
8. source 포인터를 `reset()`해 소비된 저장소를 해제한다.
9. 모든 자식을 합친 뒤 `answer[v] = destination.colors.size()`를 저장하고 그 bag을 `bags[v]`에 둔다.
10. 정점 번호 1부터 n까지 답을 출력한다.

## 의사코드

```text
root tree at 1
order = vertices in parent-before-child order

for v in reverse(order):
    current = Bag(colors = {color[v]}, population = 1)

    for child of v:
        source = bags[child]

        if current.population < source.population:
            swap(current, source)

        # 이제 source가 더 작거나 같다.
        current.colors.merge(source.colors)

        current.population += source.population
        destroy source

    answer[v] = current.colors.size
    bags[v] = current

print answer[1..n]
```

## 컴파일 가능한 C++20 뼈대

다음 프로그램은 CSES 1139 입력 형식을 그대로 받아 제출할 수 있는 골격이다. 날짜별 학습 코드에서는 각
표준 라이브러리 첫 호출의 수신자·오버로드·인자·반환·사후 상태·오류/수명 계약을 더 자세히 설명한다.

```cpp
#include <cstddef>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

struct Bag {
    // colors는 이 bag이 대표하는 정점들의 서로 다른 색만 소유한다.
    std::unordered_set<int> colors{};

    // population은 중복 색을 포함한 원래 정점 수이며 merge 방향과 상각 증명에 사용한다.
    int population{};
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int vertex_count{};
    if (!(std::cin >> vertex_count)) {
        return 0;
    }

    const auto size{static_cast<std::size_t>(vertex_count)};
    std::vector<int> color(size);
    for (int& value : color) {
        std::cin >> value;
    }

    std::vector<std::vector<int>> graph(size);
    for (int edge_index{}; edge_index < vertex_count - 1; ++edge_index) {
        int left{};
        int right{};
        std::cin >> left >> right;
        --left;
        --right;
        graph[static_cast<std::size_t>(left)].push_back(right);
        graph[static_cast<std::size_t>(right)].push_back(left);
    }

    // 부모가 자식보다 먼저 나오는 order를 반복 DFS로 만든다.
    std::vector<int> parent(size, -1);
    std::vector<int> order;
    std::vector<int> stack;
    order.reserve(size);
    stack.reserve(size);

    parent[0] = 0;
    stack.push_back(0);
    while (!stack.empty()) {
        const int vertex{stack.back()};
        stack.pop_back();
        order.push_back(vertex);

        for (const int next : graph[static_cast<std::size_t>(vertex)]) {
            if (next == parent[static_cast<std::size_t>(vertex)]) {
                continue;
            }
            parent[static_cast<std::size_t>(next)] = vertex;
            stack.push_back(next);
        }
    }

    std::vector<std::unique_ptr<Bag>> bags(size);
    std::vector<int> answer(size);

    // 역순에서는 모든 직접 자식의 bag이 현재 정점보다 먼저 완성되어 있다.
    for (auto position{order.rbegin()}; position != order.rend(); ++position) {
        const int vertex{*position};
        const auto vertex_index{static_cast<std::size_t>(vertex)};

        auto current{std::make_unique<Bag>()};
        current->colors.insert(color[vertex_index]);
        current->population = 1;

        for (const int child : graph[vertex_index]) {
            const auto child_index{static_cast<std::size_t>(child)};
            if (parent[child_index] != vertex) {
                continue;
            }

            // child bag은 후위 순서 때문에 반드시 존재한다.
            std::unique_ptr<Bag>& source{bags[child_index]};

            // 더 작은 population의 bag만 순회하도록 소유 포인터를 교환한다.
            if (current->population < source->population) {
                std::swap(current, source);
            }

            // 없는 key의 node 소유권만 current로 옮긴다. 중복 key는 source에 남는다.
            current->colors.merge(source->colors);
            current->population += source->population;

            // 자식 답은 이미 저장됐고 source 상태는 다시 필요하지 않으므로 즉시 파괴한다.
            source.reset();
        }

        answer[vertex_index] = static_cast<int>(current->colors.size());
        bags[vertex_index] = std::move(current);
    }

    for (int vertex{}; vertex < vertex_count; ++vertex) {
        if (vertex != 0) {
            std::cout << ' ';
        }
        std::cout << answer[static_cast<std::size_t>(vertex)];
    }
    std::cout << '\n';
}
```

## 정확성 근거

### 보조정리 1: 역순에서 자식 bag은 부모보다 먼저 완성된다

반복 DFS가 정점을 발견할 때 부모를 기록하고, 부모를 `order`에 넣은 뒤 자식을 스택에 넣는다. 트리에서
루트를 제외한 각 정점에는 부모가 하나뿐이므로 부모는 `order`에서 자식보다 앞선다. 따라서 역순에서는
모든 자식이 부모보다 먼저 처리된다.

### 보조정리 2: 한 번의 merge 뒤 destination은 두 bag의 합집합을 정확히 나타낸다

포인터 교환은 두 bag의 내용이나 의미를 바꾸지 않고 어느 쪽을 source로 순회할지만 바꾼다. source의
색 node를 destination의 `unordered_set`으로 merge하면 없던 색은 이전되고, 이미 있던 색 node는
source에 남는다. source를 파괴한 뒤 destination에 남은 색은 정확히 두 색 집합의 합집합이다. 두 population을 더하면 두 bag이
대표하던 서로 겹치지 않는 정점 집합의 총 정점 수도 정확하다.

### 보조정리 3: 정점 처리가 끝나면 bag 불변식이 성립한다

후위 순서에 대한 귀납법을 쓴다. 잎은 자신의 색 하나와 population 1로 시작하므로 불변식이 성립한다.
내부 정점 `v`에서는 귀납 가정에 의해 각 자식 bag이 그 자식 서브트리를 정확히 나타낸다. `v` 자신만
담은 bag에 모든 자식 bag을 보조정리 2의 합집합 연산으로 차례로 합치면, 최종 bag의 정점 집합은 정확히
`v` 서브트리이고 colors는 그 안의 서로 다른 색 전체다. 따라서 저장한 `colors.size()`가 정답이다.

### 정리: 출력한 모든 값은 해당 서브트리의 서로 다른 색 개수다

보조정리 1에 의해 모든 정점은 필요한 자식 상태가 준비된 뒤 처리되고, 보조정리 3에 의해 각 정점 처리가
끝날 때 그 정점의 답이 정확하다. 각 답을 정점 번호 위치에 저장하므로 마지막 출력은 1번부터 n번까지
요구된 답과 일치한다.

## 시간·공간 복잡도

- 트리 rooting과 순서 생성: `O(n)` 시간, `O(n)` 공간
- merge 중 source 쪽이 되는 횟수: 원래 정점 하나당 `O(log n)`
- 해시 삽입 시도 수: 전체 `O(n log n)`
- `unordered_set`의 평균적 해시 가정 아래 전체 시간: **평균 `O(n log n)`**
- 해시 충돌까지 포함한 보수적 최악 시간: **`O(n^2)`**
- 그래프, 배열, 살아 있는 bag의 키를 합친 알고리즘 공간: **`O(n)`**

`std::set`으로 바꾸면 같은 삽입 시도 수에 각 삽입 `O(log n)`이 곱해져 결정적
`O(n log^2 n)` 시간이 된다. 실제 성능은 할당자, bucket 수, load factor, 캐시 지역성, 표준 라이브러리
구현에 따라 달라진다.

## 흔한 실수

1. **큰 집합을 작은 집합에 넣기**: 정답은 맞아도 경로 트리에서 같은 데이터가 반복 이동해 느려진다.
2. **부모를 후위 순서보다 먼저 처리하기**: 아직 완성되지 않은 자식 bag을 합쳐 일부 색을 잃는다.
3. **부모 간선을 다시 자식으로 보기**: 무방향 인접 리스트에서 `parent[child] == vertex` 조건 없이
   합치면 조상 상태를 다시 소비하거나 null 포인터를 역참조한다.
4. **색 출현 횟수를 답으로 쓰기**: 문제는 정점 수가 아니라 서로 다른 색 수를 요구하므로 최종 답은
   `colors.size()`다.
5. **집합 크기만으로 단순 doubling을 증명하기**: 겹치는 색이 많으면 합집합 크기가 두 집합 크기의 합이
   아니다. 이 문서처럼 원래 정점 수 population을 무게로 사용하면 증명이 흔들리지 않는다.
6. **소비한 child bag을 계속 사용하기**: small-to-large 구현은 상태를 파괴적으로 합친다. child 답은 먼저
   값으로 저장하고, merge 뒤 bag 관찰자는 모두 버린다.
7. **깊은 재귀만 사용하기**: `n=200000` 경로 입력은 환경에 따라 호출 스택을 넘길 수 있다.
8. **`reserve`를 모든 작은 merge 직전에 호출하기**: 조금씩 늘어나는 예약이 불필요한 반복 rehash를 만들
   수 있다. 실제 측정과 성장 정책을 이해하지 않은 미세 최적화는 피한다.
9. **해시 평균 시간을 최악 보장으로 쓰기**: 기본 정수 해시는 빠르지만 표준은 충돌 공격에 대한 상수 시간을
   보장하지 않는다.
10. **`unordered_set::merge`가 source를 항상 비운다고 가정하기**: 목적지에 이미 있는 키는 source에 남는다.
    node handle merge를 쓰려면 남은 중복 키의 정리와 source 수명을 별도로 처리해야 한다.

## 변형

- **Ordered set 버전**: `std::set`을 사용해 해시 충돌을 제거하고 `O(n log^2 n)` 결정적 상한을 얻는다.
- **빈도 맵 병합**: `unordered_map<Key, Count>`를 합쳐 각 값의 출현 횟수, 최빈값, 합계 등을 관리한다.
- **DSU on tree(sack)**: 가장 무거운 자식의 전역 빈도 상태를 보존하고 가벼운 서브트리의 Euler 구간을
  다시 더한다. 서브트리 질의가 더 복잡하거나 상태를 컨테이너 하나로 공유할 때 유용하다.
- **Euler tour + Fenwick tree**: 같은 색의 마지막 등장 위치를 갱신하며 서브트리 구간의 서로 다른 색
  수를 오프라인으로 센다. CSES 1139에서 결정적 `O(n log n)`을 얻을 수 있다.
- **작은 색 우주의 bitset**: 색 종류가 `C`로 작으면 자식 bitset을 OR하여 `O(nC/w)`에 합친다.
- **문자열·해시 맵 통합**: 컴파일러 심볼 테이블, 설정 계층, 파일 확장자 통계처럼 계층별 distinct 키나
  빈도를 모으는 실무 집계에도 같은 소유권 패턴을 적용할 수 있다.

## 오늘 문제와의 연결

CSES 1139는 정점 수가 최대 200,000이고 색 값은 최대 `10^9`이므로, 정점마다 자신의 전체 서브트리를
새로 순회하거나 색 값 크기만큼 배열을 두는 방법은 부적절하다. 루트가 1로 고정되어 있으므로 한 번 만든
부모·후위 순서를 모든 merge에 재사용할 수 있다.

오늘 풀이에서 꼭 확인할 연결점은 다음과 같다.

- `order`의 역순은 자식 색 집합이 완성된 뒤 부모를 처리하게 하는 후위 순서다.
- 오늘 제출 코드는 `cursor`가 증가하는 FIFO `vector<int>`로 부모 우선 순서를 만들고, 아래 일반 뼈대는
  LIFO stack DFS를 쓴다. 처리 순서가 달라도 부모가 자식보다 먼저 기록된다는 필요한 불변식은 같다.
- `population`은 서로 다른 색 수가 아니라 서브트리 정점 수이며, merge 방향과 복잡도 증명을 책임진다.
- `colors.size()`만 정답으로 저장하므로 큰 색 값에 좌표 압축이 필요 없다.
- `unique_ptr` 교환은 큰 해시 테이블을 복사하지 않고 소유권만 바꾸며, source 파괴가 메모리 잔류를 막는다.
- 공식 예제의 3번 정점은 색 `{2,1}`을, 루트 1은 `{1,2,3}`을 가지므로 답은
  `3 1 2 1 1`이다.
- 제출 코드 가까이에는 이 문서의 상대 경로 `../algorithm/small-to-large-set-merging.md`를 표시해,
  구현 불변식과 평균/최악 복잡도 근거를 바로 찾을 수 있게 한다.
