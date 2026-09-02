# 2-SAT: implication graph와 강결합 요소

## 정의

2-SAT은 각 절(clause)에 리터럴이 정확히 두 개 이하인 논리식

```text
(a OR b) AND (c OR NOT d) AND ...
```

의 모든 절을 참으로 만드는 불리언 변수 배정을 찾거나, 그런 배정이 없음을 판정하는 문제다. 리터럴은 변수 `x` 또는 부정 `NOT x`다. 일반 SAT는 NP-complete이지만 2-SAT은 implication graph의 강결합 요소(SCC)를 이용해 선형 시간에 풀 수 있다.

## 적용 조건

- 각 제약을 두 리터럴의 OR로 표현할 수 있을 때 적용한다.
- 변수 수와 절 수가 커서 `2^V` 전수 탐색을 할 수 없을 때 적합하다.
- 단순 가능 여부뿐 아니라 실제 true/false 배정도 필요할 때 쓴다.
- 한 절에 리터럴이 세 개 이상인 일반 SAT를 임의로 두 개씩 나누면 동치가 보존되지 않는다.
- 가중치를 최대화하는 Max-2-SAT은 다른 문제이며 이 선형 알고리즘으로 직접 풀리지 않는다.

## 리터럴 번호와 절의 변환

변수 `x_i`마다 두 정점을 둔다.

```text
node(x_i)     = 2*i
node(NOT x_i) = 2*i + 1
negate(v)     = v XOR 1
```

짝수·홀수를 한 쌍으로 두면 최하위 비트 하나를 뒤집는 `v ^ 1`로 부정을 얻는다. 가장 중요한 변환은 다음 동치다.

```text
(a OR b)
<=> (NOT a -> b) AND (NOT b -> a)
```

절이 거짓인 유일한 경우는 `a=false`이면서 `b=false`일 때다. 따라서 `a`가 거짓이면 `b`가 참이어야 하고, `b`가 거짓이면 `a`가 참이어야 한다. 이 두 implication을 방향 간선으로 저장한다.

자주 쓰는 제약 변환은 다음과 같다.

| 원하는 제약 | 2-SAT 절 또는 implication |
|---|---|
| `a`를 반드시 참 | `(a OR a)` |
| `a`를 반드시 거짓 | `(NOT a OR NOT a)` |
| `a -> b` | `(NOT a OR b)` |
| `a`와 `b` 중 적어도 하나 | `(a OR b)` |
| 둘을 동시에 참으로 두지 않음 | `(NOT a OR NOT b)` |
| 정확히 하나만 참(XOR) | `(a OR b) AND (NOT a OR NOT b)` |
| 두 값이 같음 | `(NOT a OR b) AND (a OR NOT b)` |

## 핵심 아이디어와 불변식

implication graph에서 간선 `u -> v`는 “리터럴 `u`가 참이면 `v`도 반드시 참”이라는 뜻이다. 경로 전체에도 이 의미가 전이된다.

### 모순 판정 불변식

어떤 변수 `x`에 대해 `x`와 `NOT x`가 같은 SCC라면 두 방향 경로가 모두 있다.

- `x=true`로 두면 경로 `x -> NOT x` 때문에 부정도 참이어야 한다.
- `x=false`, 즉 `NOT x=true`로 두면 반대 경로 때문에 `x`도 참이어야 한다.

따라서 해가 없다. 반대로 모든 변수 쌍이 서로 다른 SCC라면 SCC 압축 DAG의 역위상 순서를 이용해 모순 없는 배정을 항상 만들 수 있다.

### SCC 순서 불변식

오늘의 코사라주 구현은 다음 순서다.

1. 원래 그래프 DFS의 **종료 순서**를 만든다.
2. 그 순서를 뒤에서 읽으며 역그래프 DFS를 한다.
3. 새 SCC를 만날 때마다 `component_id`를 0부터 증가시킨다.

이 번호는 원래 SCC 압축 DAG의 source에서 sink 방향으로 증가한다. 변수의 positive SCC와 negative SCC 중 번호가 더 큰 쪽, 즉 더 뒤의 sink 쪽 리터럴을 참으로 선택한다. 구현 순서를 반대로 만들었다면 부등호도 반대가 되므로 공식을 외우지 말고 SCC 번호의 위상 방향을 먼저 확인해야 한다.

첫 DFS에서 유지할 세부 불변식도 중요하다.

- 정점은 명시적 스택에 넣는 순간 방문 처리해 중복 프레임을 막는다.
- 프레임의 `next_edge`보다 앞선 outgoing edge는 모두 이미 처리됐다.
- 정점은 마지막 outgoing edge를 처리한 뒤에만 `finish_order`에 들어간다.
- 두 번째 DFS에서는 `component[v] != -1`인 정점이 정확히 한 SCC에 이미 배정됐다.

## 단계별 절차

1. 변수 `m`개를 `2m`개 리터럴 정점으로 바꾼다.
2. 각 절 `(a OR b)`마다 `NOT a -> b`, `NOT b -> a`를 원래 그래프에 추가하고 반대 간선을 역그래프에 추가한다.
3. 원래 그래프 전체에서 DFS 종료 순서를 구한다.
4. 종료 순서를 역순으로 보며 역그래프 DFS를 실행하고 SCC 번호를 붙인다.
5. 어떤 `x`와 `NOT x`의 SCC 번호가 같으면 `IMPOSSIBLE`이다.
6. 모두 다르면 SCC 위상 방향에 맞춰 둘 중 하나를 참으로 선택한다.
7. 만든 배정을 원래 절에 대입해 디버그 빌드나 무작위 검증에서 다시 확인한다.

## 의사코드

```text
for each clause (a OR b):
    graph[NOT a].add(b)
    graph[NOT b].add(a)
    reversed[b].add(NOT a)
    reversed[a].add(NOT b)

finish_order = DFS_POSTORDER(graph)
component = SCC_ON_REVERSED_GRAPH(reverse(finish_order))

for variable x:
    if component[x] == component[NOT x]:
        return IMPOSSIBLE
    answer[x] = component[x] > component[NOT x]
return answer
```

## 컴파일 가능한 C++ 뼈대

아래 예제는 깊은 implication chain에서도 호출 스택에 의존하지 않는 반복형 코사라주다. 실제 제출 코드는 입력 파싱과 출력만 덧붙이면 된다.

```cpp
// <cstddef>는 vector 인덱스 타입 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 예제 결과 출력 객체를 선언한다.
#include <iostream>
// <vector>는 그래프와 상태 배열을 소유한다.
#include <vector>

using Graph = std::vector<std::vector<int>>;

struct Frame {
    int vertex{};
    std::size_t next{};
};

int negate(int literal) {
    return literal ^ 1;
}

void add_clause(Graph& graph, Graph& reversed, int left, int right) {
    const int not_left{negate(left)};
    const int not_right{negate(right)};
    // operator[]은 범위가 증명된 원소 참조를 반환하고 push_back은 implication 목적지를 복사한다.
    graph[static_cast<std::size_t>(not_left)].push_back(right);
    graph[static_cast<std::size_t>(not_right)].push_back(left);
    reversed[static_cast<std::size_t>(right)].push_back(not_left);
    reversed[static_cast<std::size_t>(left)].push_back(not_right);
}

std::vector<int> solve_2sat(int variable_count, const Graph& graph, const Graph& reversed) {
    const int node_count{variable_count * 2};
    // char 상태 배열을 0으로 채우고, 종료 순서와 프레임 스택은 이후 원소를 소유한다.
    std::vector<char> visited(static_cast<std::size_t>(node_count), char{0});
    std::vector<int> order;
    std::vector<Frame> frames;
    // reserve는 size를 바꾸지 않고 최악 깊이의 재할당을 미리 막는다.
    order.reserve(static_cast<std::size_t>(node_count));
    frames.reserve(static_cast<std::size_t>(node_count));

    for (int start{}; start < node_count; ++start) {
        if (visited[static_cast<std::size_t>(start)] != 0) {
            continue;
        }
        visited[static_cast<std::size_t>(start)] = 1;
        frames.push_back(Frame{start, std::size_t{0}});
        while (!frames.empty()) {
            // back 반환 참조는 push/pop 전까지만 사용한다.
            Frame& frame{frames.back()};
            const auto& edges{graph[static_cast<std::size_t>(frame.vertex)]};
            if (frame.next < edges.size()) {
                const int next{edges[frame.next++]};
                if (visited[static_cast<std::size_t>(next)] == 0) {
                    visited[static_cast<std::size_t>(next)] = 1;
                    frames.push_back(Frame{next, std::size_t{0}});
                }
                continue;
            }
            order.push_back(frame.vertex);
            // 제거 뒤에는 frame 참조를 다시 사용하지 않는다.
            frames.pop_back();
        }
    }

    // -1은 아직 어느 SCC에도 배정되지 않았다는 sentinel이다.
    std::vector<int> component(static_cast<std::size_t>(node_count), -1);
    std::vector<int> stack;
    stack.reserve(static_cast<std::size_t>(node_count));
    int component_id{};
    for (std::size_t i{order.size()}; i > 0; --i) {
        const int start{order[i - 1]};
        if (component[static_cast<std::size_t>(start)] != -1) {
            continue;
        }
        component[static_cast<std::size_t>(start)] = component_id;
        stack.push_back(start);
        while (!stack.empty()) {
            const int vertex{stack.back()};
            stack.pop_back();
            const auto& edges{reversed[static_cast<std::size_t>(vertex)]};
            for (std::size_t edge{}; edge < edges.size(); ++edge) {
                const int next{edges[edge]};
                if (component[static_cast<std::size_t>(next)] == -1) {
                    component[static_cast<std::size_t>(next)] = component_id;
                    stack.push_back(next);
                }
            }
        }
        ++component_id;
    }

    // answer[i]는 0/1이며, 빈 vector는 모순을 뜻하도록 예제 API를 정했다.
    std::vector<int> answer(static_cast<std::size_t>(variable_count), 0);
    for (int variable{}; variable < variable_count; ++variable) {
        const int positive{variable * 2};
        const int negative{negate(positive)};
        if (component[static_cast<std::size_t>(positive)] ==
            component[static_cast<std::size_t>(negative)]) {
            return {};
        }
        answer[static_cast<std::size_t>(variable)] =
            component[static_cast<std::size_t>(positive)] >
                    component[static_cast<std::size_t>(negative)]
                ? 1
                : 0;
    }
    return answer;
}

int main() {
    // x0을 참, x1을 거짓으로 강제하는 (x0 OR x0) AND (!x1 OR !x1) 예다.
    Graph graph(4);
    Graph reversed(4);
    add_clause(graph, reversed, 0, 0);
    add_clause(graph, reversed, 3, 3);
    const auto answer{solve_2sat(2, graph, reversed)};
    // size가 2임을 확인한 뒤 두 int 값을 출력한다. 예상 결과는 1 0이다.
    if (answer.size() != 2) {
        return 1;
    }
    std::cout << answer[0] << ' ' << answer[1] << '\n';
}
```

## 정확성 근거

### 보조정리 1: 절 변환이 동치다

`(a OR b)`가 참이라고 하자. `NOT a`가 참이면 `a`는 거짓이므로 `b`가 반드시 참이고, 따라서 `NOT a -> b`가 성립한다. 대칭적으로 `NOT b -> a`도 성립한다. 반대로 두 implication이 참인데 `a`와 `b`가 모두 거짓이라고 가정하면 `NOT a -> b`가 참에서 거짓으로 가므로 모순이다. 따라서 두 implication과 원래 절은 동치다.

### 보조정리 2: 같은 SCC의 변수와 부정은 모순이다

`x`와 `NOT x`가 같은 SCC면 양방향 경로가 있다. `x=true`는 첫 경로를 따라 `NOT x=true`를 강제하고, `NOT x=true`도 반대 경로를 따라 `x=true`를 강제한다. 어느 초기 선택도 변수와 부정을 동시에 참으로 만들므로 만족 배정이 없다.

### 보조정리 3: 서로 다른 SCC면 구성한 배정이 모든 implication을 만족한다

SCC 번호가 source에서 sink 방향으로 증가하고, 번호가 더 큰 쪽 리터럴을 참으로 선택한다고 하자. 참인 `u`에서 거짓인 `v`로 implication이 있다고 가정하면 다음 네 부등식이 생긴다.

```text
component(u) > component(NOT u)       // u는 참
component(u) <= component(v)          // u -> v
component(v) < component(NOT v)       // v는 거짓, 따라서 NOT v가 참
component(NOT v) <= component(NOT u)  // contrapositive NOT v -> NOT u
```

이를 잇으면 `component(u) <= ... <= component(NOT u)`인데 첫 줄은 그 반대이므로 모순이다. 따라서 참 리터럴에서 거짓 리터럴로 가는 implication은 없고 모든 implication을 만족한다.

### 정리

같은 SCC 쌍이 발견되면 보조정리 2로 불가능 판정이 옳다. 그런 쌍이 없으면 보조정리 3의 배정이 모든 implication을 만족하고, 보조정리 1에 의해 모든 원래 절도 만족한다. 따라서 알고리즘은 가능 여부와 실제 배정을 모두 정확히 구한다.

## 시간·공간 복잡도

변수 수를 `M`, 절 수를 `N`이라 하자.

- 리터럴 정점: `2M`
- implication 간선: 절마다 2개이므로 `2N`
- 그래프·역그래프 구성: `O(M+N)` 시간, `O(M+N)` 공간
- 코사라주 두 번의 전체 DFS: `O(M+N)` 시간
- 모순 검사와 답 구성: `O(M)` 시간
- 전체: `O(M+N)` 시간, `O(M+N)` 공간

재귀 DFS도 점근 복잡도는 같지만 일자형 implication graph에서 호출 깊이가 `O(M)`이다. 온라인 저지의 작은 호출 스택에서는 정점 200,000개가 위험하므로 오늘 코드는 heap 저장소를 쓰는 명시적 프레임으로 바꾼다.

## 대회에서 반드시 챙길 우승 노하우

- `a OR b`에서 간선 방향을 외우기보다 “a가 거짓이면 누가 반드시 참인가?”로 다시 유도한다.
- literal mapping을 한 함수에 가두고 부정을 `^ 1`로 통일한다. 양수/음수 인덱스 공식을 여러 곳에 복사하면 가장 흔한 오답이 난다.
- SCC 번호의 대소 공식은 구현마다 반대일 수 있다. 단위 절 `(x OR x)` 하나를 넣고 결과가 true인지 즉시 확인한다.
- 가능 여부만 묻더라도 `x`와 `NOT x`의 SCC 비교까지는 동일하다. 배정이 필요 없으면 출력 단계만 생략한다.
- 절 수가 큰데 `set`으로 중복 간선을 제거하면 오히려 `O(N log N)`과 큰 상수가 생긴다. 중복 간선이 있어도 SCC 선형성은 유지되므로 보통 vector에 그대로 둔다.
- 디버그 빌드에서는 얻은 답을 모든 원래 절에 다시 대입해 검사한다. 이 `O(N)` 검산은 mapping·부등호 오류를 빠르게 잡는다.
- 무작위 작은 식은 `2^M` brute force와 비교한다. `M<=15`만 되어도 수천 건 회귀 검증에 충분하다.
- 입력이 “최대 하나”, “정확히 하나”, implication, 동치 중 무엇인지 먼저 논리식으로 적고 그래프 코드는 마지막에 쓴다.

## 흔한 실수

- `(a OR b)`를 `a -> b`, `b -> a`로 잘못 바꾼다.
- 역그래프에 원래 방향을 그대로 넣는다.
- 첫 DFS에서 발견 순간 order에 넣어 종료 순서가 아닌 방문 순서를 만든다.
- 정점 쌍을 `2*i`/`2*i+1`로 만들고도 부정을 `-node`처럼 다른 규칙으로 계산한다.
- `component[x] == component[NOT x]`를 변수 하나라도 놓치거나, 반대로 다르다는 사실만 확인하고 답 부호를 임의로 정한다.
- 깊이 200,000 재귀가 로컬에서는 되었다는 이유로 저지 호출 스택에서도 안전하다고 가정한다.
- `vector<bool>`의 원소가 실제 `bool&`가 아니라 proxy일 수 있다는 점을 잊고 주소나 참조를 저장한다. 오늘은 단순한 `vector<char>`를 쓴다.

## 변형

- **조건부 강제**: `a -> b`를 한 절 `(NOT a OR b)`로 넣는다.
- **At-most-one**: 원소가 둘이면 절 하나지만, 많은 원소의 모든 쌍을 직접 만들면 제곱 간선이 된다. 보조 변수를 이용한 순차 인코딩을 고려한다.
- **사전순 최소 해**: 일반적인 SCC 배정 하나만으로는 보장되지 않는다. 변수를 차례로 강제하며 가능성을 재검사하거나 문제 구조를 이용한다.
- **매개변수 최적화**: 임계값 이하 제약을 2-SAT으로 만들 수 있고 가능성이 단조라면 바깥에서 이분 탐색한다.
- **SCC 구현 교체**: 타잔 알고리즘은 역그래프가 필요 없지만 low-link와 스택 포함 여부 불변식이 더 섬세하다.

## 오늘 문제와의 연결

2026-09-03의 CSES 1684 `Giant Pizza`에서 각 토핑은 불리언 변수다. `+ x`는 `x`, `- x`는 `NOT x`이고 한 사람의 두 소망은 정확히 하나의 2-SAT 절이다. `icpc_problem.cpp`는 `2m` 정점과 `2n` 간선을 만든 뒤 반복형 코사라주로 SCC를 구한다. `m,n<=100,000`이므로 `O(m+n)` 알고리즘과 명시적 DFS 스택이 1초 제한에서 핵심이다.

SCC 자체의 종료 순서와 역그래프 정확성은 [코사라주 SCC 대표 문서](strongly-connected-components-kosaraju.md)도 함께 참고한다.

## 직접 해보기와 초보자 검증

1. `(x OR y)`의 네 진리값 조합에서 두 implication이 정확히 같은 결과인지 표로 확인한다.
2. `(x OR x) AND (NOT x OR NOT x)`의 그래프를 그리고 두 리터럴이 같은 SCC가 되는 경로를 찾는다.
3. `x=2*i`, `NOT x=2*i+1`일 때 `node ^ 1`이 양방향 모두 맞는 이유를 이진수로 설명한다.
4. 첫 DFS의 `order.push_back`을 발견 시점으로 옮겨 SCC가 틀리는 작은 반례를 찾는다.
5. component 번호 부등호를 반대로 바꾸고 단위 절 테스트가 잡는지 확인한다.
6. 변수 12개 이하의 무작위 절을 만들고 모든 `2^12` 배정과 결과를 비교한다.
