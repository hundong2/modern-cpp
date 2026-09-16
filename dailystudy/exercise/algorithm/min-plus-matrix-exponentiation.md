# Min-plus 행렬 거듭제곱

## 정의

**Min-plus 행렬 곱**은 보통 행렬 곱의 덧셈과 곱셈을 각각 `min`과 `+`로 바꾼 연산이다. 두 `n x n` 비용 행렬 `X`, `Y`에 대해 다음과 같이 정의한다.

```text
(X ⊗ Y)[i][j] = min over p (X[i][p] + Y[p][j])
```

여기서 도달할 수 없는 상태는 충분히 큰 센티널 `INF`로 표현하고 다음 규칙을 사용한다.

```text
min(x, INF) = x
INF + x = INF
```

가중 방향 그래프의 인접 행렬 `A`를 다음처럼 만들면 `A^k`의 `(i,j)` 원소는 **i에서 j까지 정확히 k개의 간선을 사용하는 walk의 최소 비용**이 된다.

```text
A[i][j] = i -> j 간선들 중 최소 비용
간선이 없으면 A[i][j] = INF
```

이 거듭제곱은 일반 산술 행렬 곱이 아니라 min-plus 곱 `⊗`을 반복한다. 이진 지수승을 적용하면 k번 순차 곱하는 대신 `O(log k)`번의 행렬 곱으로 답을 구할 수 있다.

## 적용 조건

다음 조건에서 특히 유용하다.

- 그래프 정점 수 `n`은 대략 수십~수백으로 작지만, 정확한 간선 수 `k`는 매우 크다.
- “최대 k개”가 아니라 **정확히 k개 전이**를 거친 최솟값이 필요하다.
- 한 구간의 최적 비용 두 개를 경계 정점에서 더하고, 가능한 경계 중 최솟값을 고르는 최적 부분 구조가 성립한다.
- 동일한 전이 그래프에서 여러 단계가 반복되어 행렬의 거듭제곱으로 모델링할 수 있다.
- `O(n^3 log k)`가 시간 제한 안에 들어온다. 희소 그래프여도 몇 번 제곱하면 행렬이 조밀해질 수 있으므로 최초 간선 수 `m`만 보고 희소 최적화를 가정하면 안 된다.
- 비용 합이 선택한 정수 타입 안에 들어가며, `INF` 덧셈을 하기 전에 도달 불가능/overflow 검사를 수행한다.

간선 비용이 음수여도 **간선 수가 정확히 고정**되어 있고 정수 overflow를 막는다면 수학적 점화식은 성립한다. 음수 사이클을 무한히 반복하는 문제가 생기지 않는 이유는 사용할 간선 수가 k로 제한되기 때문이다. 다만 `INF - right` 형태의 overflow guard는 음이 아닌 비용을 전제로 하므로, 음수 비용을 지원하려면 별도의 안전한 포화 덧셈을 설계해야 한다.

반대로 다음 상황에는 다른 기법이 더 알맞을 수 있다.

- 간선 수 제한이 없고 일반 최단거리만 필요하다면 Dijkstra, Bellman-Ford, Floyd-Warshall을 검토한다.
- `k`가 작고 그래프가 희소하면 `dp[step][vertex]` 전이를 `O(km)`에 수행하는 편이 빠를 수 있다.
- 모든 쌍이 아니라 단 하나의 출발 벡터만 필요할 때도 벡터-행렬 지수승으로 상수를 줄일 여지가 있다.

## 핵심 아이디어와 불변식

### 1. 행렬 원소는 “정확한 길이”를 나타낸다

`D_t[i][j]`를 i에서 j까지 정확히 t개의 간선을 사용하는 walk의 최소 비용이라고 하자. 한 간선 인접 행렬은 정의상 `D_1=A`다. 길이 x인 앞 구간과 길이 y인 뒤 구간을 경계 정점 p에서 이어 붙이면 다음 점화식이 나온다.

```text
D_(x+y)[i][j] = min over p (D_x[i][p] + D_y[p][j])
                = (D_x ⊗ D_y)[i][j]
```

따라서 `A^2`는 정확히 2개, `A^4`는 정확히 4개, 일반적으로 `A^k`는 정확히 k개 간선을 쓴 최소 비용을 나타낸다. 이것이 “최대 k개” 최단거리와 다른 가장 중요한 의미다.

### 2. Min-plus 항등행렬

항등행렬 `I`는 다음과 같다.

```text
I[i][i] = 0
I[i][j] = INF  (i != j)
```

길이 0인 walk는 같은 정점에 머무는 경우에만 비용 0으로 존재한다. 그러므로 `I ⊗ X = X ⊗ I = X`다. 이 항등원을 사용하면 일반 이진 지수승과 똑같이 `result=I`에서 시작할 수 있고, k=0인 확장 문제도 자연스럽게 처리된다.

### 3. 결합법칙

Min-plus 곱은 결합법칙을 만족한다.

```text
(X ⊗ Y) ⊗ Z = X ⊗ (Y ⊗ Z)
```

양쪽 모두 i에서 j까지 가는 walk를 두 경계 정점으로 나누어 모든 조합의 비용 합 중 최솟값을 고른다. 괄호 위치만 다르고 후보 집합이 같으므로 결과가 같다. 결합법칙 덕분에 지수를 이진수로 분해해 곱의 순서를 바꿀 수 있다.

### 4. 이진 지수승 불변식

처음 입력 행렬을 `A`, 처음 지수를 `K`라고 할 때 반복문 시작마다 다음 불변식을 유지한다.

```text
result ⊗ power^exponent = A^K
```

- 초기에는 `result=I`, `power=A`, `exponent=K`이므로 성립한다.
- exponent가 홀수이면 `result=result⊗power`로 현재 비트를 선택한다.
- exponent를 2로 나누고 `power=power⊗power`로 다음 비트의 길이를 두 배로 만든다.
- exponent가 0이 되면 `power^0=I`이므로 `result=A^K`다.

### 5. 곱셈 루프 불변식

고정한 `(row,column)`에 대해 `pivot`을 앞에서부터 처리할 때 다음을 유지한다.

> pivot보다 작은 경계 정점만 허용했을 때의 최소 비용이 `output[row][column]`에 저장되어 있다.

새 pivot에서 두 부분 중 하나라도 `INF`이면 그 연결 후보는 존재하지 않는다. 둘 다 유한하면 안전하게 더한 뒤 현재 값보다 작을 때만 갱신한다. 모든 pivot을 처리하면 정의 그대로의 min-plus 곱 원소가 된다.

### 6. 평행 간선과 평탄 저장

같은 `a -> b` 방향 간선이 여러 개라면 한 단계 최소 비용은 그중 최솟값이다.

```text
A[a][b] = min(A[a][b], weight)
```

첫 입력을 단순 대입하면 나중에 읽은 더 비싼 간선이 앞의 싼 간선을 덮을 수 있다.

구현에서는 `vector<vector<Cost>>` 대신 `vector<Cost>` 하나에 다음 위치로 저장할 수 있다.

```text
flat_index(row, column) = row * n + column
```

한 번의 큰 할당으로 저장소가 연속되고, 행마다 별도 할당할 필요가 없다. 행렬의 차원과 평탄 인덱스 계산을 작은 class 안에 감추면 호출부에서 실수할 가능성도 줄어든다.

### 7. INF와 overflow 불변식

`INF`는 가능한 모든 정상 답보다 커야 하고, 정수 타입 최댓값보다는 여유 있게 작아야 한다. CSES 1724에서는 다음 상한을 쓸 수 있다.

```text
최대 정상 비용 = k * max_weight <= 10^9 * 10^9 = 10^18
INF = 4 * 10^18 < int64 최대값 약 9.22 * 10^18
```

그러나 `INF`를 피연산자로 직접 더하면 두 값의 합이 int64 범위를 넘을 수 있다. 다음 순서가 필요하다.

```text
if left == INF or right == INF:
    skip
if left > INF - right:
    skip  // INF로 포화될 후보
candidate = left + right
```

공식 입력의 비용은 양수이므로 `right <= INF`이고 뺄셈도 안전하다. C++의 signed integer overflow는 단순 wraparound가 아니라 미정의 동작이라는 점을 기억해야 한다.

## 단계별 절차

1. `n x n` 인접 행렬 `A`를 모두 `INF`로 초기화한다.
2. 각 방향 간선 `(a,b,c)`를 읽고 `A[a][b]=min(A[a][b],c)`로 평행 간선을 정리한다.
3. 결과 행렬 `result`를 min-plus 항등행렬로 만든다.
4. `power=A`, `exponent=k`로 둔다.
5. `exponent>0`인 동안 다음을 반복한다.
   1. 최하위 비트가 1이면 `result=result⊗power`를 계산한다.
   2. `exponent`를 2로 나눈다.
   3. 다음 비트가 남아 있으면 `power=power⊗power`를 계산한다.
6. `result[0][n-1]`이 `INF`이면 -1, 아니면 그 값을 출력한다.

행렬을 매번 새로 할당하기보다 같은 크기의 scratch 행렬 하나에 곱셈 결과를 쓴 뒤 저장소를 `swap`하면 원소별 복사와 반복 할당을 피할 수 있다. 단, scratch는 입력 행렬과 다른 객체여야 한다. 출력이 입력과 같은 행렬이면 아직 읽지 않은 원소를 덮어써 곱셈이 깨진다.

## 의사코드

```text
INF = sufficiently large finite sentinel

function multiply(X, Y):
    Z = n by n matrix filled with INF
    for i = 0 .. n-1:
        for p = 0 .. n-1:
            if X[i][p] == INF:
                continue
            for j = 0 .. n-1:
                if Y[p][j] == INF:
                    continue
                if X[i][p] > INF - Y[p][j]:
                    continue
                Z[i][j] = min(Z[i][j], X[i][p] + Y[p][j])
    return Z

function power_exact_edges(A, k):
    result = min-plus identity matrix
    power = A

    while k > 0:
        if k is odd:
            result = multiply(result, power)
        k = floor(k / 2)
        if k > 0:
            power = multiply(power, power)

    return result
```

## 컴파일 가능한 C++20 뼈대

아래 코드는 CSES 1724 형식의 입력을 받아 답을 출력하는 독립 실행 가능한 골격이다. 날짜별 제출 파일에는 초보자 설명과 각 표준 라이브러리 호출의 상세 계약이 더 들어 있다.

```cpp
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

using Cost = std::int64_t;
constexpr Cost kInf{4'000'000'000'000'000'000LL};

class Matrix {
public:
    explicit Matrix(const int n, const Cost value = kInf)
        : n_{n},
          cells_(static_cast<std::size_t>(n) * static_cast<std::size_t>(n), value) {}

    [[nodiscard]] int dimension() const noexcept { return n_; }

    [[nodiscard]] Cost& cell(const int row, const int column) noexcept {
        return cells_[static_cast<std::size_t>(row) * static_cast<std::size_t>(n_)
                      + static_cast<std::size_t>(column)];
    }

    [[nodiscard]] const Cost& cell(const int row, const int column) const noexcept {
        return cells_[static_cast<std::size_t>(row) * static_cast<std::size_t>(n_)
                      + static_cast<std::size_t>(column)];
    }

    void swap_cells(Matrix& other) noexcept { cells_.swap(other.cells_); }

private:
    int n_;
    std::vector<Cost> cells_;
};

void multiply(const Matrix& left, const Matrix& right, Matrix& output) {
    const int n{left.dimension()};
    for (int i{0}; i < n; ++i) {
        for (int j{0}; j < n; ++j) {
            output.cell(i, j) = kInf;
        }
    }

    for (int i{0}; i < n; ++i) {
        for (int p{0}; p < n; ++p) {
            const Cost x{left.cell(i, p)};
            if (x >= kInf) {
                continue;
            }
            for (int j{0}; j < n; ++j) {
                const Cost y{right.cell(p, j)};
                if (y >= kInf || x > kInf - y) {
                    continue;
                }
                const Cost candidate{x + y};
                Cost& best{output.cell(i, j)};
                if (candidate < best) {
                    best = candidate;
                }
            }
        }
    }
}

void power_exact_edges(Matrix& power, Cost exponent, Matrix& result) {
    const int n{power.dimension()};
    for (int i{0}; i < n; ++i) {
        for (int j{0}; j < n; ++j) {
            result.cell(i, j) = (i == j) ? Cost{0} : kInf;
        }
    }

    Matrix scratch{n};
    while (exponent > 0) {
        if ((exponent & 1) != 0) {
            multiply(result, power, scratch);
            result.swap_cells(scratch);
        }
        exponent /= 2;
        if (exponent > 0) {
            multiply(power, power, scratch);
            power.swap_cells(scratch);
        }
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n{};
    int m{};
    Cost k{};
    std::cin >> n >> m >> k;

    Matrix adjacency{n};
    for (int edge{}; edge < m; ++edge) {
        int from{};
        int to{};
        Cost weight{};
        std::cin >> from >> to >> weight;
        --from;
        --to;
        Cost& direct{adjacency.cell(from, to)};
        if (weight < direct) {
            direct = weight;
        }
    }

    Matrix result{n};
    power_exact_edges(adjacency, k, result);
    const Cost answer{result.cell(0, n - 1)};
    std::cout << (answer >= kInf ? Cost{-1} : answer) << '\n';
}
```

## 정확성 근거

### 보조정리 1: 인접 행렬은 정확히 한 간선을 쓰는 최소 비용을 나타낸다

각 `(i,j)` 칸에는 i에서 j로 가는 모든 직접 간선 비용의 최솟값을 저장하고, 그런 간선이 없으면 `INF`를 저장한다. 따라서 이 값은 정확히 한 간선을 사용해 i에서 j로 갈 때의 최소 비용과 같다.

### 보조정리 2: Min-plus 곱은 정확한 간선 수를 더한다

`X[i][p]`가 i에서 p까지 정확히 x개 간선을 쓰는 최소 비용이고, `Y[p][j]`가 p에서 j까지 정확히 y개 간선을 쓰는 최소 비용이라고 하자. 두 walk를 연결하면 정확히 x+y개 간선을 쓰며 비용은 두 값의 합이다. 반대로 i에서 j까지 정확히 x+y개 간선을 쓰는 모든 walk는 x번째 간선 뒤의 정점 p에서 유일하게 앞 x개와 뒤 y개 구간으로 나뉜다. 모든 p에 대해 합의 최솟값을 취하는 min-plus 곱은 정확히 x+y개 간선을 쓰는 최소 비용이다.

### 보조정리 3: 항등행렬은 정확히 0개 간선의 비용 행렬이다

0개의 간선을 사용하는 walk는 출발 정점에 그대로 머무는 경우뿐이고 비용은 0이다. 서로 다른 정점으로는 갈 수 없으므로 비용을 `INF`로 둔다. 따라서 정의한 `I`는 정확히 0개 간선의 비용 행렬이며 보조정리 2에 의해 어느 쪽에 곱해도 간선 수와 비용을 바꾸지 않는다.

### 보조정리 4: 이진 지수승 반복 불변식이 유지된다

반복 시작 시 `result ⊗ power^exponent=A^K`라고 가정한다. exponent가 홀수이면 `exponent=2q+1`이다. result에 power를 곱하고 power를 제곱한 뒤 exponent를 q로 바꾸면 다음과 같다.

```text
(result ⊗ power) ⊗ (power ⊗ power)^q
= result ⊗ power^(2q+1)
= A^K
```

짝수이면 `exponent=2q`이고 result는 그대로 둔다.

```text
result ⊗ (power ⊗ power)^q
= result ⊗ power^(2q)
= A^K
```

따라서 두 경우 모두 불변식이 유지된다.

### 정리

초기에는 보조정리 3에 따라 `result=I`, `power=A`이므로 불변식이 성립한다. 반복이 끝나면 exponent는 0이고 불변식에서 `result=A^K`를 얻는다. 보조정리 1과 2에 의해 그 `(1,n)` 원소는 1번에서 n번까지 정확히 K개 간선을 사용하는 walk의 최소 비용이다. 원소가 `INF`이면 그러한 walk가 없고, 그렇지 않으면 저장된 값이 요구한 답이다.

## 시간·공간 복잡도

- min-plus 행렬 곱 한 번은 `(row,pivot,column)`의 세 중첩 루프 때문에 `O(n^3)` 시간이다.
- 지수는 매 반복 절반이 되므로 반복 횟수와 행렬 곱 횟수는 `O(log k)`다.
- **총 시간 복잡도는 `O(n^3 log k)`**다.
- 각 행렬은 `n^2`개의 비용을 저장하고 동시에 상수 개의 행렬만 유지하므로 **총 추가 공간은 `O(n^2)`**다.
- `INF` 칸을 건너뛰는 분기는 실제 연산 수를 줄일 수 있지만 최악 시간 복잡도는 그대로다.
- CSES 최대치 `n=100`, `k=10^9`에서는 대략 수십 번의 `100^3` 스칼라 후보 검사를 하므로 의도한 범위에 들어간다.

## 흔한 실수

1. 일반 행렬 곱처럼 `sum(X[i][p] * Y[p][j])`를 계산한다. 필요한 연산은 “합 중 최소”인 `min(X+Y)`다.
2. min-plus 항등행렬의 대각선까지 `INF`로 둔다. 결과가 모든 칸 `INF`에서 시작해 갱신되지 않는다.
3. 항등행렬의 모든 칸을 0으로 둔다. 이는 0비용으로 서로 다른 정점 사이를 이동하는 가짜 간선을 만든다.
4. `A[i][i]=0`을 인접 행렬에 무조건 넣는다. 그러면 실제 self-loop가 없어도 0비용 간선을 사용해 길이를 채울 수 있어 “정확히 k개” 조건이 깨진다. 0 대각선은 결과 항등행렬에만 둔다.
5. 평행 간선을 단순 대입해 더 싼 앞 간선을 비싼 뒤 간선으로 덮는다.
6. 입력 정점 번호 1-based와 내부 인덱스 0-based를 섞는다.
7. `INF=LLONG_MAX`로 두고 `INF+cost`를 계산해 signed overflow를 일으킨다.
8. `if (left==INF)`만 검사하고 right가 `INF`인 경우를 놓친다.
9. 정상 답보다 작은 `INF`를 골라 실제 큰 비용을 도달 불가능으로 오인한다.
10. 곱셈 출력 행렬을 입력 행렬과 같은 객체로 사용한다. 아직 읽을 값이 중간 결과로 덮인다.
11. 지수 제곱과 결과 곱의 순서를 뒤섞어 현재 비트가 아니라 다음 비트의 행렬을 곱한다.
12. `O(k n^3)`으로 한 번씩 곱해 k=10^9에서 끝나지 않는다.
13. 32비트 `int`에 비용 합을 저장해 최대 10^18에서 overflow한다.
14. “정확히 k개”를 “최대 k개”로 잘못 해석하고 모든 대각선에 0비용 대기를 추가한다.
15. 도달 불가능 출력 -1과 실제 음수 최단비용을 같은 내부 값으로 표현한다. 센티널은 가능한 정상 비용 범위 밖에 둬야 한다.

## 변형

### Boolean 행렬 거듭제곱

`min`을 논리 OR, `+`를 논리 AND로 바꾸면 정확히 k개의 간선으로 도달 가능한지 구할 수 있다. 비트셋을 사용하면 Boolean 행렬 곱을 크게 가속할 수 있다.

### 일반 산술 행렬 거듭제곱

`+`와 `*`를 그대로 쓰면 정확히 k개 간선을 쓰는 walk의 개수를 셀 수 있다. 값이 커지면 나머지 연산을 함께 사용한다. CSES Graph Paths I가 대표적이다.

### Max-plus 행렬 거듭제곱

`min` 대신 `max`를 쓰면 정확히 k번 전이했을 때의 최대 점수나 최대 가중치를 구할 수 있다. 도달 불가능 센티널은 음의 무한대 역할을 해야 한다.

### 최소 k개 이하 간선

대각선에 0비용 self-loop를 추가하고 정확히 k개 간선을 계산하면 원래 그래프에서 k개 이하 간선을 쓴 답으로 padding할 수 있다. 단, 이 변환은 “머무르기”가 비용 0이고 허용된다는 의미를 의식적으로 추가하는 것이다. 원래의 정확히 k개 문제에는 적용하면 안 된다.

### 상태 확장

정점 외에 마지막 간선 종류, 사용한 쿠폰 여부, 작은 나머지 상태 등을 행렬 인덱스에 포함할 수 있다. 상태 수가 S배가 되면 행렬 차원도 S배가 되고 세제곱 비용이 커지므로 실현 가능성을 먼저 계산해야 한다.

### 출발 벡터 지수승

모든 쌍 행렬이 아니라 한 출발점의 답만 필요한 경우, 선택된 비트마다 거리 벡터와 `power`를 `O(n^2)`에 결합하고 power 제곱은 `O(n^3)`에 수행할 수 있다. 점근 최악은 여전히 제곱 때문에 `O(n^3 log k)`지만 결과 곱의 상수를 줄인다.

### 경로 복원

비용뿐 아니라 각 곱셈의 `(i,j)` 최적 pivot을 저장하면 분할 정복으로 walk를 복원할 수 있다. 그러나 k가 매우 크면 출력 경로 자체가 k개 간선을 가져야 하므로 출력 비용과 메모리 요구를 별도로 고려해야 한다.

## 오늘 문제와의 연결

[CSES 1724 Graph Paths II](https://cses.fi/problemset/task/1724/)는 `n<=100`으로 작지만 `k<=10^9`라 단계별 DP `O(km)`은 불가능하다. 오늘의 [`../2026-09-16/icpc_problem.cpp`](../2026-09-16/icpc_problem.cpp)는 다음 실전 원칙을 코드에 반영한다.

1. 평행 방향 간선은 인접 행렬 칸의 최소 비용만 남긴다.
2. min-plus 항등행렬에서 이진 지수승을 시작해 정확히 k개의 간선을 유지한다.
3. `4*10^18` 센티널과 덧셈 전 guard로 64비트 signed overflow를 막는다.
4. 행렬을 `vector<Cost>` 하나로 평탄화하고 i-pivot-j 순서로 순회한다.
5. scratch 행렬 하나의 저장소를 swap하여 반복 할당과 `O(n^2)` 원소 복사를 피한다.
6. 공식 예제뿐 아니라 평행 간선, 정확한 길이로 도달 불가, 매우 큰 홀수 k의 순환 그래프를 테스트한다.

대회장에서 구현 전에 종이에 다음 세 줄을 먼저 적으면 흔한 의미 오류를 줄일 수 있다.

```text
A^t[i][j] = exactly t edges
identity diagonal = 0, adjacency diagonal is NOT automatically 0
never add INF; guard before signed addition
```

## 직접 해보기

1. 정점 두 개와 간선 `1->2(7)`, `2->1(4)`가 있을 때 `A`, `A^2`, `A^3`를 손으로 계산한다.
2. 공식 예제에서 k=1,2,3일 때 1번에서 3번까지의 값을 단계별 DP와 행렬 곱으로 각각 구해 비교한다.
3. 같은 방향 평행 간선 비용이 10과 3일 때 단순 대입 구현이 입력 순서에 따라 왜 틀릴 수 있는지 반례를 만든다.
4. 인접 행렬의 모든 대각선을 0으로 초기화했을 때 “정확히 k개” 조건이 깨지는 가장 작은 반례를 찾는다.
5. k=0을 허용하는 변형에서 출발점과 도착점이 같을 때와 다를 때 항등행렬이 어떤 답을 주는지 설명한다.
6. 작은 무작위 그래프와 작은 k를 생성하고 `O(k n^2)` 단계 DP를 oracle로 삼아 행렬 지수승 결과를 대조한다.
7. `INF`를 `10^18`로 잘못 선택했을 때 가능한 정상 답을 도달 불가로 오인하는 최대 제약 입력을 구성한다.
8. 음수 간선을 허용하도록 포화 덧셈을 고쳐라. 두 피연산자 부호에 따라 int64 경계를 넘지 않는지 검사해야 한다.
9. Boolean semiring으로 정확히 k개 간선 도달 가능성만 계산하는 버전을 작성하고 비용 버전과 구조를 비교한다.
10. max-plus로 정확히 k개 간선의 최대 점수를 구하는 버전을 작성하고 음의 무한대 센티널 계약을 설명한다.
11. `vector<vector<Cost>>`와 평탄 `vector<Cost>` 구현을 같은 입력에서 benchmark하되, 결과 차이가 하드웨어와 최적화 옵션에 의존하는 이유를 적는다.
12. 선택된 각 min-plus 곱의 pivot 정보를 저장해 작은 k에서 실제 최적 walk를 복원하는 코드를 확장한다.
