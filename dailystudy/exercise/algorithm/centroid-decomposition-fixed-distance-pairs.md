# 센트로이드 분할로 고정 거리 정점 쌍 세기

## 정의

센트로이드 분할(centroid decomposition)은 트리의 한 정점을 제거했을 때 생기는 모든 연결 성분의 크기가 원래 성분 크기의 절반 이하가 되도록 하는 정점, 즉 **센트로이드**를 반복해서 선택하는 분할 기법이다. 각 조각에 같은 과정을 적용하면 분할 단계의 깊이는 `O(log n)`이다.

이 문서에서는 가중치가 없는 트리에서 서로 다른 두 정점의 최단 거리, 즉 두 정점을 잇는 유일한 단순 경로의 간선 수가 정확히 `k`인 **순서 없는 정점 쌍**의 개수를 센다. 핵심은 각 쌍을 그 경로가 처음 통과하는 센트로이드 단계에서 정확히 한 번 세는 것이다.

## 적용 조건

- 입력 그래프가 연결되고 사이클이 없는 트리다.
- 질의 거리가 하나이며, 거리 `k`인 모든 정점 쌍의 개수만 필요하다.
- `n`이 커서 모든 정점을 시작점으로 하는 BFS/DFS의 `O(n^2)` 시간이 허용되지 않는다.
- 답이 `n(n-1)/2`까지 커질 수 있으므로 64비트 정수형이 필요하다.
- 트리가 일자 모양일 수 있어 깊이 `O(n)` 재귀 DFS가 호출 스택을 넘길 수 있다.

센트로이드 분할은 일반 그래프에는 그대로 적용할 수 없다. 트리에서는 두 정점 사이의 단순 경로가 유일하므로 “경로가 센트로이드를 지난다”라는 분류가 명확하다. 여러 거리 질의나 거리 구간 질의에는 같은 분할 틀을 쓰되 빈도 자료구조를 바꾸는 편이 좋다.

## 핵심 아이디어와 불변식

### 1. 센트로이드가 만드는 균형

현재 활성 연결 성분의 크기를 `s`라 하자. 센트로이드 `c`를 제거해 생긴 각 성분의 크기는 `s/2` 이하이다. 따라서 어떤 정점이 속한 활성 성분의 크기는 분할 한 단계마다 적어도 절반으로 줄고, 한 정점은 최대 `O(log n)`개의 단계에서만 순회된다.

### 2. 센트로이드를 지나는 쌍의 거리 식

정점 `u`, `v`가 센트로이드 `c`의 서로 다른 자식 방향에 있거나 둘 중 하나가 `c`라면 유일 경로가 `c`를 지난다.

```text
dist(u, v) = dist(u, c) + dist(v, c)
```

그러므로 지금 살펴보는 자식 성분에서 깊이 `d`인 정점마다, 앞서 처리한 영역에서 깊이 `k-d`인 정점 수를 더하면 정확히 거리 `k`인 새 쌍을 얻는다.

### 3. 자식 성분의 조회-후-등록 순서

`frequency[x]`는 다음 정점만 센다.

- 센트로이드 자체: `frequency[0] = 1`
- 현재 센트로이드의 자식 방향 중 **이미 처리를 끝낸 방향**에 있고, 센트로이드까지 거리가 `x`인 정점

한 자식 성분의 모든 깊이를 먼저 조회하고 나서 빈도에 등록한다. 등록부터 하면 같은 자식 성분 안의 두 정점도 이 단계에서 세게 된다. 그런 쌍의 경로는 센트로이드를 지나지 않으므로 잘못된 중복이며, 나중에 그 자식 성분을 분할할 때 세어야 한다.

### 4. 쌍의 유일 담당 단계

아직 함께 들어 있는 활성 성분에서 두 정점 `u`, `v`를 생각한다. 분할을 계속하면 처음으로 다음 둘 중 하나가 일어나는 단계가 유일하게 존재한다.

- 센트로이드가 `u` 또는 `v`다.
- 센트로이드를 제거했을 때 `u`, `v`가 서로 다른 성분으로 갈라진다.

그 단계의 센트로이드가 두 정점의 경로 위에 있고, 알고리즘은 그 쌍을 그때 한 번 센다. 이전 단계에서는 둘이 같은 자식 성분에 있어 세지 않았고, 이후 단계에서는 이미 서로 다른 활성 성분에 있어 다시 만날 수 없다.

### 5. 반복형 순회 불변식

구현은 긴 사슬 트리에서도 안전하도록 다음 두 작업을 명시적 `std::vector` 스택으로 수행한다.

- 활성 성분의 정점 수집, 부모 설정, 부분 트리 크기 계산
- 한 센트로이드 자식 방향의 깊이 수집

센트로이드 분할 작업 자체도 `pending_components` 스택으로 처리하므로 사용자 입력 크기에 비례하는 재귀 호출 스택을 전혀 사용하지 않는다. 명시적 스택은 힙 메모리를 쓰며 최악에도 `O(n)`칸이다.

### 6. 빈도 초기화 비용 불변식

각 센트로이드마다 `frequency[0..k]` 전체를 0으로 채우면 `k`가 큰 경우 센트로이드 수만큼 반복되어 `O(nk)`가 될 수 있다. 실제로 값이 바뀐 깊이만 `touched_depths`에 기록하고 마지막에 그 칸만 0으로 되돌린다. 한 단계의 초기화 비용이 그 단계에서 수집한 정점 수에 비례하므로 전체 시간 상한 `O(n log n)`을 보존한다.

## 단계별 절차

1. 임의의 활성 정점을 시작점으로 현재 연결 성분을 반복형 DFS로 수집한다.
2. 수집 순서를 거꾸로 보며 각 정점의 활성 부분 트리 크기를 계산한다.
3. 각 후보 정점에서 제거 뒤 가장 큰 조각의 크기를 계산하고, 그 값이 성분 크기의 절반 이하인 센트로이드를 고른다.
4. `frequency[0] = 1`로 두어 센트로이드와 다른 정점으로 이루어진 쌍도 셀 수 있게 한다.
5. 센트로이드의 아직 제거되지 않은 이웃을 하나씩 처리한다.
6. 그 이웃 방향에서 센트로이드까지의 깊이가 `k` 이하인 정점 깊이를 반복형 DFS로 모은다. `k`보다 깊으면 자손도 모두 더 깊으므로 가지치기한다.
7. 모은 각 깊이 `d`에 대해 `frequency[k-d]`를 답에 더한다.
8. 조회가 모두 끝난 뒤에야 그 자식 방향의 깊이들을 `frequency`에 등록한다.
9. 이번 센트로이드에서 건드린 빈도 칸만 0으로 복구한다.
10. 센트로이드를 제거 상태로 표시하고, 남은 각 연결 성분의 시작 정점을 작업 스택에 넣는다.
11. 작업 스택이 빌 때까지 반복한다.

## 의사코드

```text
answer = 0
removed[v] = false for every vertex v
pending_components = [any vertex]

while pending_components is not empty:
    entry = pop(pending_components)
    if removed[entry]:
        continue

    nodes, parent = iterative_traverse_active_component(entry)
    subtree_size = compute_sizes_in_reverse(nodes, parent)
    centroid = vertex whose largest remaining piece <= nodes.size / 2

    frequency[0] = 1
    touched_depths = [0]

    for each active neighbor next of centroid:
        depths = iterative_collect_depths(next, centroid, limit=k)

        # 조회가 먼저다. 같은 자식 방향 내부의 쌍은 여기서 세지 않는다.
        for d in depths:
            answer += frequency[k - d]

        # 이제 이 방향을 이후 방향의 짝 후보로 공개한다.
        for d in depths:
            if frequency[d] == 0:
                touched_depths.push(d)
            frequency[d] += 1

    for d in touched_depths:
        frequency[d] = 0

    removed[centroid] = true
    for each active neighbor next of centroid:
        pending_components.push(next)

return answer
```

## 완전한 C++20 뼈대/예제

아래 프로그램은 `n k`와 `n-1`개의 1-based 무방향 간선을 읽고 거리 `k`인 순서 없는 정점 쌍의 수를 출력한다. 활성 성분 순회, 깊이 수집, 분할 작업 관리가 모두 반복형이다.

```cpp
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

class FixedDistancePairCounter {
public:
    FixedDistancePairCounter(std::vector<std::vector<int>> graph, int target_distance)
        : graph_{std::move(graph)},
          target_distance_{target_distance},
          removed_(graph_.size(), 0U),
          parent_(graph_.size(), -1),
          subtree_size_(graph_.size(), 0),
          frequency_(graph_.size(), 0) {}

    std::int64_t count() {
        if (graph_.empty() || target_distance_ < 0
            || target_distance_ >= static_cast<int>(graph_.size())) {
            return 0;
        }

        std::int64_t answer{};
        pending_components_.push_back(0);

        while (!pending_components_.empty()) {
            const int entry{pending_components_.back()};
            pending_components_.pop_back();

            // 같은 활성 성분이 중복 등록되지는 않지만 방어적으로 제거 여부를 확인한다.
            if (removed_[static_cast<std::size_t>(entry)] != 0U) {
                continue;
            }

            const int centroid{find_centroid(entry)};
            answer += count_pairs_through(centroid);
            removed_[static_cast<std::size_t>(centroid)] = 1U;

            // 센트로이드를 제거하면 각 활성 이웃이 서로 다른 다음 성분의 시작점이다.
            for (const int next : graph_[static_cast<std::size_t>(centroid)]) {
                if (removed_[static_cast<std::size_t>(next)] == 0U) {
                    pending_components_.push_back(next);
                }
            }
        }

        return answer;
    }

private:
    struct DepthFrame {
        int vertex{};
        int parent{};
        int depth{};
    };

    int find_centroid(int entry) {
        component_order_.clear();
        vertex_stack_.clear();

        parent_[static_cast<std::size_t>(entry)] = -1;
        vertex_stack_.push_back(entry);

        // 트리이므로 직전 부모만 제외하면 활성 성분을 중복 방문하지 않는다.
        while (!vertex_stack_.empty()) {
            const int vertex{vertex_stack_.back()};
            vertex_stack_.pop_back();
            component_order_.push_back(vertex);

            for (const int next : graph_[static_cast<std::size_t>(vertex)]) {
                if (removed_[static_cast<std::size_t>(next)] != 0U
                    || next == parent_[static_cast<std::size_t>(vertex)]) {
                    continue;
                }
                parent_[static_cast<std::size_t>(next)] = vertex;
                vertex_stack_.push_back(next);
            }
        }

        for (const int vertex : component_order_) {
            subtree_size_[static_cast<std::size_t>(vertex)] = 1;
        }
        for (auto it{component_order_.rbegin()}; it != component_order_.rend(); ++it) {
            const int vertex{*it};
            const int parent{parent_[static_cast<std::size_t>(vertex)]};
            if (parent != -1) {
                subtree_size_[static_cast<std::size_t>(parent)]
                    += subtree_size_[static_cast<std::size_t>(vertex)];
            }
        }

        const int component_size{static_cast<int>(component_order_.size())};
        for (const int vertex : component_order_) {
            // 현재 임시 루트에서 부모 쪽 조각의 크기다.
            int largest_piece{component_size
                              - subtree_size_[static_cast<std::size_t>(vertex)]};

            for (const int next : graph_[static_cast<std::size_t>(vertex)]) {
                if (removed_[static_cast<std::size_t>(next)] == 0U
                    && parent_[static_cast<std::size_t>(next)] == vertex
                    && subtree_size_[static_cast<std::size_t>(next)] > largest_piece) {
                    largest_piece = subtree_size_[static_cast<std::size_t>(next)];
                }
            }

            if (largest_piece * 2 <= component_size) {
                return vertex;
            }
        }

        // 유효한 비어 있지 않은 트리 성분에는 반드시 센트로이드가 존재한다.
        return entry;
    }

    void collect_depths(int entry, int centroid) {
        collected_depths_.clear();
        depth_stack_.clear();
        depth_stack_.push_back(DepthFrame{entry, centroid, 1});

        while (!depth_stack_.empty()) {
            const DepthFrame frame{depth_stack_.back()};
            depth_stack_.pop_back();

            if (frame.depth > target_distance_) {
                continue;
            }
            collected_depths_.push_back(frame.depth);

            for (const int next : graph_[static_cast<std::size_t>(frame.vertex)]) {
                if (removed_[static_cast<std::size_t>(next)] != 0U
                    || next == frame.parent) {
                    continue;
                }
                depth_stack_.push_back(
                    DepthFrame{next, frame.vertex, frame.depth + 1});
            }
        }
    }

    std::int64_t count_pairs_through(int centroid) {
        std::int64_t pairs{};
        touched_depths_.clear();
        frequency_[0] = 1;
        touched_depths_.push_back(0);

        for (const int next : graph_[static_cast<std::size_t>(centroid)]) {
            if (removed_[static_cast<std::size_t>(next)] != 0U) {
                continue;
            }

            collect_depths(next, centroid);

            // 먼저 조회해야 같은 자식 방향 내부의 두 정점을 잘못 세지 않는다.
            for (const int depth : collected_depths_) {
                pairs += frequency_[static_cast<std::size_t>(target_distance_ - depth)];
            }

            // 조회를 끝낸 뒤 이 방향의 정점들을 다음 방향의 후보로 등록한다.
            for (const int depth : collected_depths_) {
                const std::size_t index{static_cast<std::size_t>(depth)};
                if (frequency_[index] == 0) {
                    touched_depths_.push_back(depth);
                }
                ++frequency_[index];
            }
        }

        // 전체 frequency 배열을 매번 채우지 않고 실제 변경된 칸만 복구한다.
        for (const int depth : touched_depths_) {
            frequency_[static_cast<std::size_t>(depth)] = 0;
        }
        return pairs;
    }

    std::vector<std::vector<int>> graph_;
    int target_distance_{};
    std::vector<unsigned char> removed_;
    std::vector<int> parent_;
    std::vector<int> subtree_size_;
    std::vector<int> frequency_;

    // 반복 할당을 줄이기 위해 각 작업의 버퍼를 멤버로 재사용한다.
    std::vector<int> pending_components_;
    std::vector<int> component_order_;
    std::vector<int> vertex_stack_;
    std::vector<DepthFrame> depth_stack_;
    std::vector<int> collected_depths_;
    std::vector<int> touched_depths_;
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int vertex_count{};
    int target_distance{};
    if (!(std::cin >> vertex_count >> target_distance)) {
        return 0;
    }

    std::vector<std::vector<int>> graph(static_cast<std::size_t>(vertex_count));
    for (int edge{}; edge < vertex_count - 1; ++edge) {
        int from{};
        int to{};
        std::cin >> from >> to;
        --from;
        --to;
        graph[static_cast<std::size_t>(from)].push_back(to);
        graph[static_cast<std::size_t>(to)].push_back(from);
    }

    FixedDistancePairCounter counter{std::move(graph), target_distance};
    std::cout << counter.count() << '\n';
}
```

## 정확성 증명

### 보조정리 1: 한 센트로이드 단계에서 더한 모든 쌍의 거리는 `k`다

현재 자식 방향에서 수집한 정점 `u`의 깊이를 `d`라 하자. 조회 대상 `frequency[k-d]`의 정점 `v`는 센트로이드이거나 앞서 처리한 다른 자식 방향에 있다. 따라서 `u`와 `v`의 유일 경로는 센트로이드를 지나며 길이는 `d + (k-d) = k`다. 그러므로 알고리즘은 조건을 만족하지 않는 쌍을 더하지 않는다.

### 보조정리 2: 센트로이드를 지나는 거리 `k` 쌍은 그 단계에서 정확히 한 번 더해진다

그러한 순서 없는 쌍 `{u,v}`를 잡는다. 둘 중 센트로이드가 아닌 정점들이 서로 다른 자식 방향에 있다면 그 두 방향 중 나중에 처리되는 방향의 정점을 조회할 때, 먼저 처리된 방향의 정점 깊이는 이미 `frequency`에 있다. 깊이 합이 `k`이므로 쌍이 한 번 더해진다. 한 정점이 센트로이드라면 `frequency[0]=1` 때문에 다른 정점을 처리할 때 한 번 더해진다.

한 자식 방향의 깊이를 조회하는 동안 그 방향의 정점은 아직 빈도에 없고, 조회 뒤에는 같은 방향을 다시 처리하지 않는다. 따라서 같은 단계에서 동일한 쌍을 두 번 더할 수 없다.

### 보조정리 3: 모든 거리 `k` 쌍에는 유일한 담당 센트로이드 단계가 있다

임의의 쌍 `{u,v}`를 포함하는 활성 성분을 따라 분할 과정을 본다. 둘 중 하나가 센트로이드가 되거나 둘이 서로 다른 제거 후 성분으로 처음 갈라지는 단계가 반드시 존재한다. 그 단계 전까지는 둘이 같은 자식 성분에 있으므로 조회-후-등록 규칙상 세지 않는다. 그 단계에서는 경로가 센트로이드를 지나므로 보조정리 2에 따라 정확히 한 번 센다. 그 뒤에는 두 정점이 같은 활성 성분에 다시 속하지 않으므로 다시 셀 수 없다.

### 정리

보조정리 1에 의해 더한 모든 쌍은 거리 `k`이고, 보조정리 3에 의해 거리 `k`인 모든 순서 없는 쌍은 유일한 단계에서 한 번 더해진다. 따라서 최종 누적값은 거리 `k`인 정점 쌍의 정확한 개수다.

## 시간·공간 복잡도

- 한 활성 성분에서 정점 수집, 부분 트리 크기 계산, 센트로이드 탐색, 깊이 수집에 드는 시간은 모두 그 성분 크기에 선형이다.
- 센트로이드를 제거하면 각 다음 성분 크기가 절반 이하이므로 한 정점은 최대 `O(log n)`개 성분 순회에 참여한다.
- **총 시간 복잡도는 `O(n log n)`이다.**
- 인접 리스트, 제거 표시, 부모, 부분 트리 크기, 거리 빈도, 변경 깊이 목록, 명시적 스택은 각각 `O(n)` 범위다.
- 서로 다른 분할 단계의 컴포넌트를 동시에 복제해 저장하지 않으므로 **총 공간 복잡도는 `O(n)`이다.**

`touched_depths` 없이 매 단계 `frequency` 전체를 초기화하면 위 시간 증명이 깨질 수 있다. 또한 깊이가 `k`를 넘는 순간 그 아래를 탐색하지 않아 답에 기여하지 않는 긴 가지의 상수 비용을 줄인다.

## 흔한 실수

1. 한 자식 성분의 깊이를 빈도에 먼저 넣고 조회하여, 경로가 센트로이드를 지나지 않는 같은 방향 내부 쌍까지 센다.
2. `frequency[0] = 1`을 빼서 센트로이드 자체가 한 끝점인 쌍을 놓친다.
3. 방향별 처리가 끝날 때마다 빈도를 초기화하여 서로 다른 두 방향의 쌍을 놓친다. 센트로이드의 모든 방향을 끝낸 뒤 초기화해야 한다.
4. 센트로이드를 찾을 때 임시 루트의 자식 부분 트리만 보고 부모 쪽 조각 `component_size - subtree_size[v]`를 빠뜨린다.
5. 이미 제거된 센트로이드를 활성 성분 순회나 깊이 수집에서 통과한다.
6. `frequency` 전체를 매 센트로이드마다 `O(k)`로 지워 최악 시간 복잡도를 `O(nk)`로 만든다.
7. 답을 32비트 `int`에 저장한다. 별 모양 트리에서 `k=2`이면 답이 거의 `n^2/2`다.
8. 일자 트리에 재귀 DFS를 사용해 알고리즘 복잡도는 맞지만 호출 스택 초과를 일으킨다.
9. 순서 있는 `(u,v)`와 순서 없는 `{u,v}`를 혼동해 답을 두 배로 만든다.
10. `k`보다 큰 깊이도 끝까지 수집하여 불필요한 메모리와 시간을 쓴다.

## 변형

- **거리 구간 `[a,b]` 쌍 세기**: 정확한 깊이 빈도 대신 Fenwick tree나 누적 빈도를 사용해 보완 깊이 구간의 개수를 질의한다.
- **Fixed-Length Paths II**: 각 깊이 `d`에 대해 `[k1-d, k2-d]`의 기존 정점 수를 구한다. 좌표 범위가 깊이이므로 Fenwick tree 또는 관리되는 prefix 구조를 결합할 수 있다.
- **가중 트리**: 깊이를 간선 수가 아닌 거리 합으로 저장하고, 좌표 압축한 빈도 맵이나 정렬+두 포인터를 사용한다.
- **여러 거리 질의**: 센트로이드별 거리 목록을 보존해 질의별 합성곱, 오프라인 처리, FFT/NTT 등의 적용 가능성을 검토한다.
- **동적 활성 정점 질의**: 각 정점에서 센트로이드 조상까지의 거리와 자식 방향 정보를 저장하고, 센트로이드 트리별 카운터를 갱신한다.
- **가장 가까운 색칠 정점**: 센트로이드 조상 경로에 최소 거리를 저장·조회하는 고전적인 동적 센트로이드 분할 문제로 바뀐다.
- **재귀 분할 + 반복형 DFS**: 분할 재귀 깊이는 `O(log n)`이라 보통 안전하다. 다만 이 문서의 예제는 호출 스택 의존을 완전히 제거하려고 분할도 작업 스택으로 구현했다.

## 오늘 문제와의 연결

[CSES 2080 Fixed-Length Paths I](https://cses.fi/problemset/task/2080/)는 큰 트리에서 거리가 정확히 `k`인 순서 없는 정점 쌍을 요구한다. 각 정점에서 탐색하면 `O(n^2)`이고, 일자 트리에서 재귀 DFS는 스택 위험도 있다.

오늘 구현은 센트로이드마다 자식 방향별 깊이 빈도를 조회한 뒤 등록한다. 한 정점이 `O(log n)`개 분할 단계에만 참여하므로 `O(n log n)` 시간, 재사용 배열과 명시적 스택으로 `O(n)` 공간을 지킨다. 대회장에서 다음 세 문장을 먼저 적으면 구현 방향이 흔들리지 않는다.

1. 같은 자식 방향 내부 쌍은 지금 세지 않고 더 작은 분할 단계에 맡긴다.
2. 현재 방향은 조회를 마친 뒤에만 빈도에 등록한다.
3. 각 쌍은 두 끝점이 처음 갈라지는 센트로이드에서 정확히 한 번 센다.

## 직접 해보기

1. 정점 5개의 경로 `1-2-3-4-5`에서 `k=2`일 때 각 쌍의 담당 센트로이드를 표시하고 답 `3`을 손으로 확인한다.
2. 중심 하나와 잎 `m`개인 별 모양 트리에서 `k=2`의 답이 `m(m-1)/2`인 이유와 64비트 정수가 필요한 최소 `m`을 계산한다.
3. 조회와 등록 순서를 바꿔 같은 자식 방향 쌍이 잘못 포함되는 가장 작은 반례를 만든다.
4. `touched_depths`를 제거하고 매번 전체 빈도 배열을 지우면 경로 트리와 큰 `k`에서 연산 횟수가 어떻게 변하는지 비교한다.
5. 재귀 깊이 수집을 사용한 버전과 문서의 명시적 스택 버전을 `n=200000` 경로 트리에서 비교한다.
6. 깊이 정확값 대신 구간 합을 질의하도록 Fenwick tree를 붙여 Fixed-Length Paths II 형태로 확장한다.
7. 작은 무작위 트리에서는 모든 시작점 BFS로 거리 쌍을 세는 `O(n^2)` oracle을 만들고 이 구현과 수천 건 비교한다.
8. `k=1`, `k=n-1`, 경로, 별, 균형 이진 트리를 각각 테스트하여 간선 수·지름·조합식과 답이 일치하는지 검증한다.
