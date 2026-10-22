# DAG의 역위상 순서와 워드 비트셋으로 구하는 전이 폐쇄

## 정의

방향 그래프의 **전이 폐쇄(transitive closure)** 는 각 정점 `u`에서 도달 가능한 모든 정점의 집합을
구하는 것이다. 이 문서에서는 방향 비순환 그래프(DAG)의 위상 순서를 먼저 구한 뒤, 순서를 거꾸로
훑으며 도달 집합을 합친다. 집합은 정점마다 불리언 배열을 따로 두는 대신 64개 정점을 하나의
`std::uint64_t` 워드에 담는 **word-packed bitset** 으로 표현한다.

정점 수를 `n`, 64비트 워드 수를

```text
W = ceil(n / 64)
```

라고 하자. `reach[u][k]`의 `b`번째 비트가 1이면 정점 `u`에서 정점 `64k+b`로 갈 수 있다는 뜻이다.
길이 0인 경로도 도달로 세므로 언제나 `u` 자신의 비트부터 1로 둔다.

## 적용 조건

- 입력이 DAG이거나, 먼저 강한 연결 요소(SCC)를 압축해 DAG로 바꿀 수 있어야 한다.
- 모든 시작점의 도달 가능 정점 집합 또는 그 개수가 필요하다.
- `n x n` 불리언 행렬은 부담스럽지만 `n * ceil(n/64)`개의 64비트 워드는 메모리 제한에 들어간다.
- 간선을 따라 비트셋 OR을 수행하는 `mW` 규모의 워드 연산이 시간 제한 안에 들어간다.
- 정점 번호가 조밀한 `0..n-1` 범위라 비트 위치로 직접 대응시킬 수 있다.

순환이 있는 원 그래프를 위상 순서 한 번만 거꾸로 훑는 것으로는 충분하지 않다. 아직 처리되지 않은
정점의 정보가 순환을 돌아 다시 필요하기 때문이다. 그런 경우에는 SCC 압축 DAG를 사용하거나, 크기가
작을 때 비트셋 Floyd-Warshall 같은 다른 방법을 택한다.

## 핵심 아이디어와 불변식

DAG의 간선 `u -> v`가 있으면 어떤 위상 순서에서도 `u`가 `v`보다 먼저 나온다. 따라서 위상 순서를
**역순**으로 처리하면 `u`를 만나는 시점에는 모든 직접 후속 정점 `v`의 도달 집합이 완성되어 있다.

다음 점화식이 핵심이다.

```text
R(u) = {u} union (union over every edge u -> v of R(v))
```

여기서 `R(u)`는 `u`에서 도달 가능한 정점의 집합이다. 비트셋에서는 집합의 합집합이 워드별 비트 OR
한 번으로 바뀐다.

역위상 순회를 하는 동안 다음 불변식을 유지한다.

1. 아직 처리하지 않은 정점의 비트셋에는 적어도 자기 자신만 표시되어 있다.
2. 현재 정점 `u`에서 나가는 모든 간선 `u -> v`의 `reach[v]`는 이미 정확히 완성되어 있다.
3. 처리 완료한 모든 정점 `x`에 대해 `reach[x]`는 정확히 `R(x)`다.
4. 같은 정점이 여러 경로로 도달되어도 OR은 멱등적이므로 해당 비트는 한 번만 센다.

## 단계별 절차

1. 간선을 인접 리스트에 저장하고 각 정점의 진입 차수를 센다.
2. Kahn 알고리즘으로 위상 순서 `order`를 만든다.
3. `order.size() != n`이면 입력에 순환이 있으므로 이 알고리즘의 전제가 깨졌다고 판정한다.
4. `W = (n + 63) / 64`를 계산하고 `n * W`개의 64비트 워드를 0으로 초기화한다.
5. 모든 정점 `u`에 대해 `reach[u]`에서 `u`에 해당하는 비트를 켠다.
6. 위상 순서를 뒤에서 앞으로 순회한다.
7. 각 간선 `u -> v`마다 `reach[u][k] |= reach[v][k]`를 모든 워드 `k`에 수행한다.
8. 각 행의 워드에 population count를 적용해 1인 비트 수를 합산한다.

평탄한 한 개의 배열에서 정점 `u`의 행 시작 위치는 `u * W`다. `vector<vector<uint64_t>>`보다 할당
횟수가 적고, 연속 워드 순회가 분명하며, 행마다 생기는 컨테이너 객체 오버헤드도 피할 수 있다.

## 의사코드

```text
graph, indegree를 입력으로 만든다
order = Kahn 위상 정렬(graph, indegree)
if order의 길이 != n:
    DAG 전제 위반을 보고한다

W = ceil(n / 64)
reach = n행 W열의 0인 64비트 워드 행렬

for u = 0 .. n-1:
    reach[u][u / 64]의 (u % 64)번째 비트를 켠다

for u in reverse(order):
    for v in graph[u]:
        for k = 0 .. W-1:
            reach[u][k] = reach[u][k] OR reach[v][k]

for u = 0 .. n-1:
    answer[u] = sum(popcount(reach[u][k])) for k = 0 .. W-1
```

## 컴파일 가능한 C++23 뼈대

아래 코드는 정점을 입력에서 `1..n`, 내부에서 `0..n-1`로 사용한다. 온라인 저지의 유효한 입력을
전제로 하며, DAG가 아니면 비정상 입력으로 보고 종료한다.

```cpp
#include <bit>      // std::popcount: 64비트 워드에서 1인 비트의 수를 센다.
#include <cstddef>  // std::size_t: 컨테이너 크기와 인덱스를 표현한다.
#include <cstdint>  // std::uint64_t: 정확히 64비트인 부호 없는 워드 타입이다.
#include <iostream> // std::cin, std::cout: 표준 입력과 출력을 담당한다.
#include <queue>    // std::queue: Kahn 위상 정렬의 준비 정점 큐다.
#include <vector>   // std::vector: 인접 리스트와 연속 비트 행렬을 소유한다.

// 여러 곳에서 같은 워드 타입을 일관되게 쓰기 위한 별칭이다.
using Word = std::uint64_t;

int main() {
    // C 표준 입출력과의 동기화를 끊고 불필요한 자동 flush 연결을 제거한다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n{};
    int m{};
    std::cin >> n >> m;

    // graph[u]는 u에서 직접 갈 수 있는 정점을 소유하고, indegree는 Kahn 알고리즘에서 감소한다.
    std::vector<std::vector<int>> graph(static_cast<std::size_t>(n));
    std::vector<int> indegree(static_cast<std::size_t>(n), 0);

    for (int edge{}; edge < m; ++edge) {
        int u{};
        int v{};
        std::cin >> u >> v;
        --u;
        --v;
        graph[static_cast<std::size_t>(u)].push_back(v);
        ++indegree[static_cast<std::size_t>(v)];
    }

    // 진입 차수가 0인 정점만 큐에 들어간다는 Kahn 알고리즘의 불변식을 세운다.
    std::queue<int> ready{};
    for (int u{}; u < n; ++u) {
        if (indegree[static_cast<std::size_t>(u)] == 0) {
            ready.push(u);
        }
    }

    std::vector<int> order{};
    order.reserve(static_cast<std::size_t>(n));
    while (!ready.empty()) {
        const int u{ready.front()};
        ready.pop();
        order.push_back(u);

        for (const int v : graph[static_cast<std::size_t>(u)]) {
            --indegree[static_cast<std::size_t>(v)];
            if (indegree[static_cast<std::size_t>(v)] == 0) {
                ready.push(v);
            }
        }
    }

    // 모든 정점이 나오지 않았다면 순환이 있으므로 역위상 DP를 적용할 수 없다.
    if (order.size() != static_cast<std::size_t>(n)) {
        return 1;
    }

    constexpr std::size_t bits_per_word{64U};
    const std::size_t vertex_count{static_cast<std::size_t>(n)};
    const std::size_t words{(vertex_count + bits_per_word - 1U) / bits_per_word};

    // reach[u * words + k]는 정점 u의 도달 집합 중 k번째 64비트 블록이다.
    std::vector<Word> reach(vertex_count * words, Word{0});
    for (std::size_t u{}; u < vertex_count; ++u) {
        reach[u * words + u / bits_per_word] |= Word{1} << (u % bits_per_word);
    }

    // 역위상 순서에서는 모든 후속 정점의 행이 이미 완성되어 있으므로 그대로 OR할 수 있다.
    for (std::size_t position{order.size()}; position > 0U; --position) {
        const std::size_t u{static_cast<std::size_t>(order[position - 1U])};
        const std::size_t u_base{u * words};

        for (const int next : graph[u]) {
            const std::size_t v{static_cast<std::size_t>(next)};
            const std::size_t v_base{v * words};
            for (std::size_t k{}; k < words; ++k) {
                reach[u_base + k] |= reach[v_base + k];
            }
        }
    }

    // 완성된 각 행에서 켜진 비트 수가 해당 시작점에서 도달 가능한 정점 수다.
    for (std::size_t u{}; u < vertex_count; ++u) {
        int count{};
        const std::size_t base{u * words};
        for (std::size_t k{}; k < words; ++k) {
            count += std::popcount(reach[base + k]);
        }
        std::cout << count << (u + 1U == vertex_count ? '\n' : ' ');
    }
}
```

## 정확성 정리와 증명

### 보조정리 1: 역위상 처리 시 모든 직접 후속 정점의 답은 완성되어 있다

간선 `u -> v`가 있으면 위상 순서에서 `u`는 `v`보다 앞선다. 위상 순서를 거꾸로 순회하므로 `v`를
처리한 뒤에 `u`를 처리한다. 따라서 `u`를 처리할 때 모든 직접 후속 정점 `v`의 `reach[v]`는 이미
완성되어 있다.

### 보조정리 2: 점화식이 `u`에서 도달 가능한 정점을 빠짐없이 포함한다

`u` 자신은 길이 0인 경로로 도달 가능하므로 `{u}`에 포함된다. `u`와 다른 임의의 도달 가능 정점
`x`를 잡으면, `u`에서 `x`로 가는 경로의 첫 간선은 어떤 `u -> v`다. 남은 경로에 의해 `x`는
`R(v)`에 들어가므로 모든 후속 정점의 집합을 합치면 `x`도 포함된다.

### 보조정리 3: 점화식은 도달 불가능한 정점을 넣지 않는다

`{u}`에서 온 비트는 실제로 `u`를 나타낸다. 어떤 `R(v)`에서 온 정점 `x`는 실제 경로 `v => x`가
있다. 여기에 실제 간선 `u -> v`를 앞에 붙이면 `u => x` 경로가 생긴다. 따라서 합집합으로 추가되는
모든 정점은 `u`에서 실제로 도달 가능하다.

### 정리

역위상 순서에서 가장 먼저 처리되는 sink는 나가는 간선이 없으므로 자기 자신만 정확히 가진다.
처리 순서에 대한 귀납 가정으로 모든 후속 정점의 행이 정확하다고 하자. 보조정리 2와 3에 의해
점화식으로 만든 현재 행도 정확하다. 보조정리 1이 이 귀납 가정을 모든 정점에서 성립시킨다. 따라서
알고리즘 종료 후 `reach[u]`는 모든 `u`에 대해 정확한 도달 집합이고, 그 population count는 요구한
도달 가능 정점 수다.

서로 다른 경로가 같은 정점에 도달해도 동일한 비트를 OR할 뿐이다. 따라서 경로의 수가 아니라 서로
다른 도달 정점 수를 정확히 센다.

## 시간·공간 복잡도와 메모리 계산

`W = ceil(n/64)`라 두자.

- 위상 정렬은 `O(n+m)`이다.
- 자기 비트 초기화는 `O(n)`이고, 최종 popcount는 `O(nW)` 워드 연산이다.
- 각 간선에서 길이 `W`인 행을 OR하므로 `O(mW)` 워드 연산이다.
- 따라서 총 시간은 **`O((n+m)ceil(n/64))`** 이다.
- 도달 행렬은 `O(nW)`, 인접 리스트와 위상 정렬 자료는 `O(n+m)`이므로 총 공간은
  **`O(nceil(n/64) + n + m)`** 이다.

이 복잡도는 한 번의 64비트 OR 또는 popcount를 한 워드 연산으로 세는 word-RAM 관점이다. 실제 속도는
CPU의 popcount 지원, 컴파일러 최적화, 캐시와 메모리 대역폭에 따라 달라진다.

`n = 50,000`이면 `W = ceil(50,000/64) = 782`다. 비트 행렬만 계산하면

```text
50,000 * 782 * 8 = 312,800,000 bytes
312,800,000 / 1,048,576 = 약 298.3 MiB
```

가 필요하다. 여기에 인접 리스트의 정수 저장 공간, 각 내부 `vector` 객체, 위상 순서, 진입 차수와
할당자 오버헤드가 더해진다. 따라서 “비트는 312.5 MiB쯤”처럼 십진 MB와 이진 MiB를 섞지 말고,
제한과 같은 단위로 전체 예산을 미리 계산해야 한다. 평탄 배열은 행마다 별도 할당을 하지 않아 이
예산을 더 예측하기 쉽다.

## 흔한 실수

1. **위상 순방향으로 합치기**: `reach[v]`가 아직 완성되지 않은 상태에서 읽게 된다. 반드시 역순으로
   처리한다.
2. **자기 자신을 빼먹기**: 문제에서 길이 0인 경로를 도달로 세는지 확인하고, 오늘 형태에서는 자기
   비트를 먼저 켠다.
3. **워드 수 내림 계산**: `n / 64`가 아니라 `(n + 63) / 64`다.
4. **부호 있는 1 이동**: `1 << bit`는 `bit >= 31`에서 잘못될 수 있다. `Word{1} << bit`처럼 폭과
   부호를 명시한다.
5. **평탄 배열 크기를 `int`로 곱하기**: `n * W`가 `int` 범위를 넘을 수 있으므로 곱셈 전에
   `std::size_t`로 바꾼다.
6. **행 별칭 계산 오류**: `u_base = u * W`, `v_base = v * W`를 분리해 출발 행과 도착 행을 뒤섞지
   않는다.
7. **DAG 검증 누락**: 문제에서 DAG를 보장하지 않는다면 위상 순서 길이를 반드시 검사한다.
8. **마지막 워드의 여분 비트**: 처음부터 전부 0이고 실제 정점 비트만 켜면 여분 비트는 계속 0이다.
   임의로 보수를 취하는 변형에서는 마지막 워드를 마스킹해야 한다.
9. **메모리 단위 착각**: MB와 MiB, 컨테이너 오버헤드, 인접 리스트까지 포함해 제한을 확인한다.
10. **병렬 간선의 불필요한 반복**: 정답은 OR의 멱등성 때문에 맞지만 동일한 행 OR을 반복한다. 병렬
    간선이 매우 많다면 정렬·중복 제거 비용과 절감되는 `W` 워드 연산을 비교한다.

## 변형

- **도달 여부 질의**: 전처리 후 `(reach[u * W + v/64] >> (v%64)) & 1`로 `O(1)`에 답한다.
- **도달 목록 복원**: 켜진 워드를 순회하며 `countr_zero`와 `word &= word-1`로 set bit만 열거한다.
- **SCC가 있는 그래프**: SCC를 한 정점으로 압축하고 DAG에서 같은 알고리즘을 수행한다. 원래 정점 수를
  묻는다면 각 SCC 비트에 SCC 크기라는 가중치를 두고, 최종 set bit들의 가중치 합을 계산한다.
- **희소 도달 집합**: 실제 도달 관계가 매우 희소하면 정렬된 벡터의 small-to-large 합치기나 압축 집합이
  비트 행렬보다 메모리를 적게 쓸 수 있다.
- **조밀하고 작은 일반 그래프**: 각 행을 비트셋으로 둔 Warshall 변형은 중간 정점마다 조건부 행 OR을
  수행할 수 있다.
- **온라인 간선 추가**: 이 정적 DP는 갱신 전파 비용이 크다. 추가 순서가 제한되지 않으면 동적 전이
  폐쇄 전용 자료구조나 배치 재계산을 검토한다.

## 대회 우승을 가르는 구현 노하우

- 코딩 전에 `W`, `n*W*8`, 예상 `m*W`를 숫자로 직접 계산한다. 이 문제에서는 알고리즘 이름보다
  메모리 제한과 약 1억 회 규모의 단순 워드 연산을 견디는지가 먼저다.
- `reach`를 평탄화하고 가장 안쪽 루프에서 연속 주소를 증가시키면 캐시·프리패처가 이해하기 쉬운 접근이
  된다. 다만 실제 기계 명령과 벡터화 여부는 CPU, 컴파일러, 옵션에 따라 달라진다.
- `std::uint64_t`는 비트 위치와 메모리 산정이 정확하다. `unsigned long`처럼 플랫폼에 따라 폭이 달라질
  수 있는 타입에 기대지 않는다.
- OR 루프에서는 행 시작 오프셋을 한 번만 계산한다. 반복되는 곱셈을 줄일 수 있고 코드 검토 때도 어느
  행을 읽고 쓰는지 분명하다.
- 답의 개수만 필요해도 처리 중 매번 popcount하지 않는다. 모든 합집합이 끝난 뒤 행마다 한 번 세어
  중간 결과를 반복해서 세는 비용을 피한다.
- 간선 방향이 `u -> v`일 때 정보는 `v`에서 `u`로 합쳐진다는 문장을 코드 옆에 적는다. 전이 폐쇄에서
  가장 흔한 WA 원인은 그래프 방향과 DP 정보 흐름을 같은 것으로 착각하는 것이다.
- 시간 초과가 의심되면 먼저 전체 워드 OR 횟수와 메모리 대역폭을 본다. 복잡한 자료구조로 바꾸기 전에
  릴리스 최적화 빌드, 연속 저장, 불필요한 간선 중복이 없는지 확인한다.
- 입력이 DAG라고 보장되어도 개발·로컬 검증 버전에서는 위상 순서 크기를 검사하면 방향 입력 오류와
  전제 위반을 즉시 잡을 수 있다.

## 오늘 문제와의 연결

[CSES 2138 Reachable Nodes](https://cses.fi/problemset/task/2138/)에서는 DAG의 각 정점에서 도달 가능한
서로 다른 정점 수를 출력한다. 각 정점의 답을 DFS로 따로 구하면 같은 하위 그래프를 지나치게 많이
탐색한다. 반대로 역위상 순서에서는 후속 정점의 완성된 도달 집합을 재사용할 수 있고, 64개 정점을 한
워드로 합쳐 `O((n+m)ceil(n/64))` 워드 연산으로 모든 시작점의 답을 함께 만든다.

이 문제에서 기억할 한 줄은 다음과 같다.

```text
간선은 u -> v로 가지만, 완성된 도달 정보는 역위상 순서에서 v의 행을 u의 행으로 OR한다.
```
