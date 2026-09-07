# Hamiltonian 경로를 세는 부분집합 비트마스크 DP

## 정의

부분집합 비트마스크 DP는 원소 수가 작을 때 집합을 정수의 비트로 표현하고, “지금까지 선택한 집합”을 상태에 넣는 동적 계획법이다. 정점 `v`가 집합에 들어 있으면 `mask & (1 << v)`가 0이 아니다. 이 표현은 집합 복사·포함 검사·원소 제거를 상수 시간 비트 연산으로 바꾼다.

Hamiltonian 경로는 그래프의 모든 정점을 정확히 한 번 방문하는 경로다. 일반 그래프에서 Hamiltonian 경로 존재 판정은 다항 시간 풀이가 알려져 있지 않지만, `n <= 20` 정도라면 `2^n`개의 부분집합을 열거하는 DP가 실전적인 정확 해법이 된다.

오늘 다루는 고정 시작점 `s`, 고정 도착점 `t`의 경로 수 상태는 다음과 같다.

```text
dp[mask][v]
= s에서 출발해 mask에 포함된 정점을 각각 정확히 한 번 방문하고
  v에서 끝나는 경로 수
```

## 적용 조건

- 정점 수가 보통 `18~22` 이하라 `n * 2^n` 상태를 시간·메모리 제한 안에 둘 수 있다.
- 방문 순서 전체가 중요하고 같은 정점을 다시 방문하면 안 된다.
- 상태에 방문 집합과 마지막 정점만 알면 다음 전이를 결정할 수 있다.
- 경로 수, 최소 비용, 가능 여부처럼 부분 경로 결과를 결합할 수 있다.
- 방향/무방향 그래프 모두 가능하지만 간선 방향에 맞는 predecessor 또는 successor 목록을 써야 한다.

`n=30` 이상에서는 단순 `O(n2^n)` 공간도 대개 너무 크다. meet-in-the-middle, 포함배제, 구조가 있는 그래프의 별도 DP나 근사 알고리즘을 검토한다.

## 핵심 아이디어와 불변식

### 상태 불변식

처리가 끝난 `dp[mask][v]`는 다음 조건을 모두 만족하는 경로만 센다.

1. 첫 정점은 `s`다.
2. 방문한 정점 집합은 정확히 `mask`다.
3. 각 정점은 한 번만 방문한다.
4. 마지막 정점은 `v`다.
5. 저장값은 요구 modulus로 나눈 나머지다.

기저 상태는 `dp[1 << s][s] = 1`이다. 아직 간선을 타지 않았지만 시작점 하나를 방문한 길이 0 경로를 한 가지로 센다.

### 마지막 간선 점화식

`v` 직전 정점을 `u`라 하자. `v`를 지운 이전 집합은 `previous = mask ^ (1 << v)`이고 마지막 간선은 `u -> v`여야 한다.

```text
dp[mask][v]
  = sum(dp[previous][u])
    for every input edge u -> v with u in previous
```

공식 문제 설명은 중복 연결의 식별 방식을 따로 명시하지 않는다. 오늘 구현은 입력의 각 간선 occurrence를 별개 항공편 선택으로 보는 명시적 multigraph 의미를 택한다. 따라서 같은 `u -> v`가 두 번 있다면 predecessor 목록에도 두 번 남겨 각각 더한다. 정점열만 경로 정체성으로 보는 변형이라면 먼저 중복 간선을 제거해야 한다.

### 우승권 가지치기

고정 시작점과 도착점이 있으면 다음 상태는 정답에 절대 기여하지 않는다.

- 시작점 비트가 없는 `mask`: 경로가 `s`에서 출발할 수 없다.
- 기저 외에 마지막 정점이 다시 `s`인 상태: 시작점을 끝에 붙이면 같은 도시를 두 번 방문하므로 유효한 Hamiltonian 부분 경로가 아니다. 구현은 마지막 도시 반복을 `v=1`부터 시작해 이 상태를 만들지 않는다.
- `mask`가 전체 집합이 아닌데 도착점 `t`를 이미 포함한 상태: `t`에서 끝난 뒤 다른 정점을 방문하면 최종 도착점이 `t`가 아니고, `t`로 다시 돌아오면 중복 방문이다.
- 전체 집합에서 마지막 정점이 `t`가 아닌 상태: 최종 답에 필요 없다.

이 가지치기는 답을 바꾸지 않으면서 실제 mask 수를 대략 `2^(n-2)` 규모로 줄인다.

## 단계별 절차

1. 도시 번호를 `0..n-1`로 바꾼다.
2. 각 도착 도시 `v`에 대해 들어오는 간선의 출발 도시 `u`를 `incoming[v]`에 저장한다.
3. `2^n * n`개의 DP 칸을 0으로 초기화한다.
4. `dp[1][0] = 1`로 시작점 기저를 둔다.
5. mask를 작은 정수부터 순회한다. 원소 하나를 지운 `previous`는 항상 mask보다 작으므로 이미 계산되어 있다.
6. mask에 포함된 마지막 도시 `v`마다 `incoming[v]`를 순회해 이전 상태를 더한다.
7. 매 덧셈 직후 modulus로 줄여 overflow와 불필요한 큰 정수를 피한다.
8. `dp[(1<<n)-1][n-1]`를 출력한다.

## 의사코드

```text
incoming = reverse adjacency lists
dp[1 << start][start] = 1

for mask in [1, 2^n):
    if start not in mask:
        continue
    if destination in mask and mask is not full:
        continue

    for v in [1, n):
        if v not in mask:
            continue
        if mask is full and v is not destination:
            continue

        previous = mask without v
        for u in incoming[v]:
            if u in previous:
                dp[mask][v] += dp[previous][u]
                dp[mask][v] %= MOD

answer = dp[full][destination]
```

## C++20 뼈대

아래 코드는 한 개의 연속 `vector<int>`에 DP를 평탄화한다. `vector<vector<int>>` DP보다 할당 수가 적고, mask가 증가할 때 가까운 칸을 읽는 지역성이 명확하다.

```cpp
#include <cstddef>
#include <iostream>
#include <vector>

using Mask = unsigned long long;
constexpr int mod{1'000'000'007};

std::size_t index_of(Mask mask, int city, int n) {
    return static_cast<std::size_t>(mask) * static_cast<std::size_t>(n)
           + static_cast<std::size_t>(city);
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n{};
    int m{};
    std::cin >> n >> m;

    // incoming[v]는 u -> v인 모든 입력 간선의 u를 보관한다.
    std::vector<std::vector<int>> incoming(static_cast<std::size_t>(n));
    for (int edge{}; edge < m; ++edge) {
        int u{};
        int v{};
        std::cin >> u >> v;
        --u;
        --v;
        incoming[static_cast<std::size_t>(v)].push_back(u);
    }

    const Mask state_count{Mask{1} << static_cast<unsigned int>(n)};
    const Mask full{state_count - Mask{1}};
    const Mask destination_bit{Mask{1} << static_cast<unsigned int>(n - 1)};
    std::vector<int> dp(state_count * static_cast<std::size_t>(n), 0);
    dp[index_of(Mask{1}, 0, n)] = 1;

    for (Mask mask{1}; mask < state_count; ++mask) {
        if ((mask & Mask{1}) == Mask{0}) {
            continue;
        }
        if ((mask & destination_bit) != Mask{0} && mask != full) {
            continue;
        }

        for (int v{1}; v < n; ++v) {
            const Mask v_bit{Mask{1} << static_cast<unsigned int>(v)};
            if ((mask & v_bit) == Mask{0}) {
                continue;
            }
            if (mask == full && v != n - 1) {
                continue;
            }

            const Mask previous{mask ^ v_bit};
            int ways{};
            const std::vector<int>& predecessors{incoming[static_cast<std::size_t>(v)]};
            for (std::size_t i{}; i < predecessors.size(); ++i) {
                const int u{predecessors[i]};
                if ((previous & (Mask{1} << static_cast<unsigned int>(u))) == Mask{0}) {
                    continue;
                }
                ways += dp[index_of(previous, u, n)];
                if (ways >= mod) {
                    ways -= mod;
                }
            }
            dp[index_of(mask, v, n)] = ways;
        }
    }

    std::cout << dp[index_of(full, n - 1, n)] << '\n';
}
```

## 정확성 근거

### 보조정리 1: 모든 DP 전이가 유효한 경로를 만든다

`dp[previous][u]`가 세는 경로는 귀납 가정에 따라 `previous`의 도시를 정확히 한 번 방문하고 `u`에서 끝난다. `v`는 `previous`에 없고 입력 간선 `u -> v`가 있으므로 그 간선을 붙이면 어떤 도시도 중복하지 않고 방문 집합이 `mask`이며 `v`에서 끝나는 유효 경로가 된다.

### 보조정리 2: 모든 유효 경로가 전이에 포함된다

`mask`의 도시를 방문하고 `v`에서 끝나는 임의의 유효 경로를 잡는다. `v` 바로 앞 도시는 유일한 어떤 `u`이고 마지막 간선은 입력의 `u -> v`다. 마지막 도시 `v`와 그 간선을 제거하면 정확히 `previous`를 방문하고 `u`에서 끝나는 경로가 되므로 귀납 가정에 의해 `dp[previous][u]`에 포함된다.

### 보조정리 3: 중복 집계가 없다

한 경로의 마지막 입력 간선 위치는 하나뿐이다. 마지막 도시를 제거해 얻는 `(previous,u)`와 선택한 `u -> v` 간선 occurrence가 유일하므로 서로 다른 전이가 같은 경로를 만들지 않는다. 병렬 간선 occurrence를 서로 다른 선택으로 보는 경우에는 의도대로 각각 한 번 센다.

### 정리

기저는 시작점만 방문한 유일한 길이 0 경로를 정확히 센다. 보조정리 1~3과 방문 도시 수에 대한 귀납으로 모든 계산 상태가 정의와 정확히 일치한다. 전체 mask와 마지막 도시 `t`를 고른 칸은 모든 도시를 정확히 한 번 방문해 `s`에서 `t`로 가는 경로만, 그리고 그 모두를 세므로 출력이 정답이다.

도착점을 일찍 포함한 경로는 이후 `t`를 다시 방문하지 않고서는 `t`에서 끝날 수 없다. Hamiltonian 조건은 중복 방문을 금지하므로 그 상태를 건너뛰는 가지치기도 정답을 제거하지 않는다.

## 시간·공간 복잡도

- 상태 수: `n * 2^n`
- mask/마지막 도시 검사: `O(n2^n)`
- 들어오는 간선 순회: 각 mask 층에서 모든 indegree의 합이 `m`이므로 상한 `O(m2^n)`
- 총 시간: `O(2^n(n+m))`, 조밀 그래프에서는 `O(n^2 2^n)`
- DP 공간: `O(n2^n)`
- 그래프 공간: `O(n+m)`

`n=20`이면 DP int 칸은 `20 * 1,048,576`개, 바이트 수는 약 80 MiB다. 이 계산을 대회 전에 해야 `vector<vector<int>>`의 추가 할당·overhead나 더 넓은 정수 타입 선택이 메모리 제한을 넘기는지 판단할 수 있다.

## 수치·구현 안전성

- `1 << n`에서 왼쪽 피연산자가 32-bit signed `int`면 더 큰 변형에서 overflow/shift 문제가 생길 수 있다. 표현 폭을 명시한 오늘 별칭의 `Mask{1}`을 이동하고 shift count가 Mask의 value-bit 수보다 작은지 확인한다.
- 오늘 `Mask`인 `unsigned long long`은 표준상 적어도 64 value bits라 `n<=20` shift count를 담는다. 컨테이너 인덱스로 바꿀 때는 공식 최대 `20*2^20`칸이 대상 플랫폼의 `size_t`와 메모리에 들어간다는 전제를 사용한다.
- 각 DP 값과 누적값을 항상 `[0,MOD)`로 유지한다. 두 값을 더한 최대 `2*MOD-2`는 32-bit signed int 최댓값보다 작아서 한 번 더하고 한 번 빼는 구현이 안전하다.
- `state_count * n`과 평탄화 인덱스는 `std::size_t`로 계산한다. `int`로 셀 수를 계산하지 않는다.
- 입력 실패 뒤 초기값을 사용하지 않는다는 온라인 저지의 유효 입력 전제에 기대며, 실무 입력 경계라면 스트림 상태를 검사한다.

## 흔한 실수

1. `dp[mask][v]`의 시작점 조건을 빼서 아무 도시에서나 시작하는 경로까지 센다.
2. `previous`에서 `v` 비트를 제거하지 않아 같은 도시를 여러 번 방문한다.
3. 방향 그래프인데 `v -> u`와 `u -> v`를 뒤집는다. predecessor 목록은 반드시 `u -> v`의 `u`다.
4. 도착점이 포함된 중간 mask를 계산한 뒤 그 상태에서 계속 진행해 잘못된 경로를 더한다.
5. mask를 재귀적으로 탐색하면서 memo의 “미계산”과 실제 답 0을 구분하지 않는다.
6. modulus 적용을 너무 늦게 해 signed overflow를 일으킨다.
7. `O(n^2 2^n)` 시간만 보고 `O(n2^n)` 메모리 바이트를 계산하지 않는다.
8. 인접 행렬을 쓰면서 병렬 간선 occurrence를 bool 하나로 합쳐 버린다.
9. 전체 mask에서 도착점이 아닌 마지막 정점 상태까지 불필요하게 계산한다.

## 변형

- **가능 여부**: 정수 count 대신 bool/bitset 상태로 바꾼다.
- **최소 비용 Hamiltonian 경로/TSP**: 합 대신 `min`, 간선 비용을 더하고 도달 불가를 INF로 둔다.
- **Hamiltonian cycle**: 시작점을 고정하고 full 상태의 마지막 정점에서 시작점으로 돌아가는 간선을 검사한다.
- **모든 끝점 답**: 도착점 조기 가지치기를 제거하고 `dp[full][v]`를 모두 읽는다.
- **경로 복원**: 선택된 predecessor를 별도 저장하거나 답 상태에서 점화식을 역추적한다.
- **여러 query**: 시작점 고정 여부와 그래프 크기에 따라 DP 재사용 가능성을 검토한다. 시작점마다 그대로 반복하면 비용이 n배다.
- **meet-in-the-middle**: 정점 수가 더 크고 경로 구조 결합이 가능하면 집합을 둘로 나누는 방법을 검토한다.

## 오늘 문제와의 연결

[CSES 1690 Hamiltonian Flights](https://cses.fi/problemset/task/1690/)는 `n<=20`이 비트마스크 DP 신호다. 1번 도시와 n번 도시가 고정되어 있어 시작 비트 없는 상태와 도착 비트가 너무 일찍 들어간 상태를 제거할 수 있다. `icpc_problem.cpp`는 들어오는 간선 목록과 평탄한 DP를 사용해 `O(2^n(n+m))`에 구현한다.

대회에서는 다음 세 문장을 코드 전에 적을 수 있어야 한다.

1. `dp[mask][v]`가 세는 대상은 무엇인가?
2. 마지막 간선을 제거했을 때 이전 상태가 왜 유일한가?
3. 셀 개수와 바이트 수가 제한 안에 드는가?

이 세 답이 분명하면 점화식 방향, 중복 집계, 메모리 초과 실수를 크게 줄일 수 있다.

## 직접 해보기

1. `n=4` 완전 방향 그래프에서 시작·도착을 고정한 답이 `(n-2)! = 2`임을 DP 표로 확인한다.
2. 도착점 조기 가지치기를 제거해도 답은 같지만 계산 상태가 어떻게 늘어나는지 센다.
3. 병렬 간선을 하나로 압축하려면 `(u,v)`별 multiplicity를 점화식에 어떻게 곱할지 작성한다.
4. `vector<vector<int>> dp`와 평탄한 `vector<int>`의 할당 수·주소 계산·가독성을 비교한다.
5. 정답 하나를 실제 도시 순서로 복원하도록 predecessor 저장을 추가한다.
