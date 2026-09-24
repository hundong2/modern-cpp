# Meet-in-the-middle 부분집합 합

## 정의

**Meet-in-the-middle(MITM)**은 전체 선택 공간을 서로 독립적으로 합칠 수 있는 두 절반으로 나눈 뒤, 각 절반의 가능한 결과를 열거하고 두 결과 집합을 빠른 탐색으로 결합하는 기법이다. 완전 탐색이 `O(2^n)`일 때 절반 크기를 약 `n/2`로 줄여 `O(2^(n/2))` 규모의 두 목록으로 바꾼다.

부분집합 합 계수 문제에서는 원소 위치 집합을 왼쪽 `L`과 오른쪽 `R`로 나눈다. 모든 전체 부분집합 `S`는 `(S ∩ L, S ∩ R)`이라는 유일한 쌍으로 표현된다. 따라서 왼쪽 부분집합 합이 `a`라면 오른쪽에서 `target-a`인 부분집합의 **개수**를 더하면 된다.

이름이 비슷한 양방향 탐색과는 목적이 다르다. 그래프의 시작·도착에서 탐색 전선을 좁히는 bidirectional search가 아니라, 여기서는 독립 선택 공간의 결과를 두 표로 구체화한 뒤 join한다.

## 적용 조건

다음 조건에서 특히 유용하다.

- `n`이 30~50 정도라 다항 시간 DP는 상태 값 범위 때문에 어렵고 `2^n`은 너무 크다.
- 전체 선택이 두 부분의 선택으로 독립 분해되고, 전체 결과를 두 부분 결과의 합·곱·비교 등으로 결합할 수 있다.
- 절반 결과 `2^(n/2)`개를 메모리에 저장할 수 있다.
- 합의 값 범위가 매우 커서 `O(n * target)` 배낭 DP를 쓸 수 없다.

다음 상황에서는 다른 방법이 더 낫다.

- `target`이 작으면 0/1 배낭 DP 또는 bitset이 더 빠르고 메모리도 예측 가능하다.
- `n`이 60 이상이면 `2^(n/2)` 목록도 너무 커질 수 있다.
- 선택이 두 절반 사이의 복잡한 제약으로 얽혀 부분 결과만으로 결합 가능성을 판정할 수 없다.
- 값이 모두 양수이고 해 하나만 찾는 작은 목표라면 가지치기 DFS가 실제 입력에서 더 단순할 수 있지만, 최악 시간 보장은 별개다.

## 핵심 아이디어와 불변식

### 절반 합 열거 불변식

처음 목록은 빈 부분집합의 합 하나인 `[0]`이다. 현재 값 `v`를 처리하기 직전 목록에 앞의 `k`개 값으로 만들 수 있는 모든 부분집합 합이 정확히 한 번씩 들어 있다고 가정한다.

- 기존 합 `s`를 그대로 두면 `v`를 고르지 않은 부분집합이다.
- 기존 합마다 `s+v`를 새로 붙이면 `v`를 고른 부분집합이다.

두 부류는 `v` 포함 여부가 달라 겹치지 않고, 앞의 `k+1`개 값의 모든 부분집합은 둘 중 정확히 하나다. 따라서 새 목록도 모든 부분집합을 정확히 한 번 나타낸다. **합 값이 같더라도 위치 선택이 다르면 별도 원소로 남겨야 한다.** 이것이 경우의 수 문제에서 중복 제거를 하면 안 되는 이유다.

구현에서는 append 전 `previous_size`를 저장하고 `[0, previous_size)`만 읽는다. 같은 단계에서 방금 붙인 원소까지 다시 읽으면 현재 값 `v`를 두 번 이상 고르는 잘못된 상태가 만들어진다.

### 결합 불변식

오른쪽 합 목록을 정렬한다. 왼쪽 합 `a` 하나를 고정하면 전체 합이 `target`인 쌍은 오른쪽 합이 정확히 `target-a`인 경우뿐이다. 정렬된 목록의 `equal_range`는 그 값과 같은 반열린 구간 `[lower, upper)`를 반환하며, 구간 크기가 오른쪽 부분집합의 개수다.

왼쪽 목록에도 같은 합이 여러 번 있을 수 있다. 왼쪽 원소 각각에 대해 오른쪽 중복 개수를 다시 더해야 두 절반의 실제 부분집합 쌍 수, 즉 곱셈 효과가 자연스럽게 계산된다.

## 단계별 절차

1. 원소를 앞쪽 `floor(n/2)`개와 나머지로 나눈다.
2. 각 절반에 대해 `[0]`에서 시작하는 doubling 방식으로 모든 부분집합 합을 열거한다.
3. 오른쪽 합 목록을 오름차순 정렬한다.
4. 정답을 64비트 0으로 둔다.
5. 각 왼쪽 합 `a`에 대해 `needed = target-a`를 계산한다.
6. 오른쪽 목록에서 `needed`의 `equal_range`를 구해 구간 크기를 답에 더한다.
7. 모든 왼쪽 합을 처리한 답을 반환한다.

양쪽을 모두 정렬한 뒤 같은 합을 묶어 투 포인터로 결합할 수도 있다. 그 변형은 검색 단계가 선형이지만 왼쪽도 정렬해야 한다. 한쪽 정렬+이진 탐색은 구현과 증명이 단순하고 CSES 1628 범위에서 충분하다.

## 의사코드

```text
enumerate(values[first:last)):
    sums = [0]
    for value in values[first:last):
        previous_size = size(sums)
        for i = 0 .. previous_size-1:
            append sums[i] + value to sums
    return sums

left  = enumerate(first half)
right = enumerate(second half)
sort(right)

answer = 0
for left_sum in left:
    needed = target - left_sum
    [lo, hi) = equal_range(right, needed)
    answer += hi - lo
return answer
```

## 컴파일 가능한 C++23 뼈대

```cpp
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <ranges>
#include <vector>

std::vector<long long> enumerate(
    const std::vector<long long>& values,
    std::size_t first,
    std::size_t last) {
    std::vector<long long> sums;
    sums.reserve(std::size_t{1} << (last - first));
    sums.push_back(0);

    for (std::size_t position = first; position < last; ++position) {
        const std::size_t previous_size = sums.size();
        for (std::size_t index = 0; index < previous_size; ++index) {
            sums.push_back(sums[index] + values[position]);
        }
    }
    return sums;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n{};
    long long target{};
    std::cin >> n >> target;

    std::vector<long long> values(static_cast<std::size_t>(n));
    for (long long& value : values) {
        std::cin >> value;
    }

    const std::size_t middle = values.size() / 2;
    const auto left = enumerate(values, 0, middle);
    auto right = enumerate(values, middle, values.size());
    std::ranges::sort(right);

    long long answer{};
    for (const long long left_sum : left) {
        const auto matches = std::ranges::equal_range(right, target - left_sum);
        answer += static_cast<long long>(matches.size());
    }
    std::cout << answer << '\n';
}
```

## 정확성 근거

### 보조정리 1: 절반 열거는 각 부분집합을 정확히 한 번 만든다

처리한 원소 수에 대한 귀납법을 쓴다. 0개를 처리한 초기 목록 `[0]`은 빈 부분집합 하나와 일대일 대응한다. `k`개까지 성립한다고 하자. 다음 값 `v`를 처리하면 기존 목록은 `v`를 고르지 않은 부분집합과 일대일 대응하고, 각 기존 합에 `v`를 더한 새 목록 부분은 `v`를 고른 부분집합과 일대일 대응한다. `v` 포함 여부가 다르므로 두 집합은 선택 관점에서 서로소이고 모든 부분집합을 포함한다. 따라서 `k+1`개에서도 성립한다.

### 보조정리 2: 전체 부분집합과 두 절반 부분집합 쌍은 일대일 대응한다

전체 인덱스 집합을 서로소인 `L`, `R`로 나눴다. 임의의 전체 부분집합 `S`는 `(S∩L, S∩R)`로 유일하게 분해된다. 반대로 왼쪽 부분집합 `A`와 오른쪽 부분집합 `B`의 합집합 `A∪B`는 유일한 전체 부분집합이다. 두 사상은 서로 역이므로 일대일 대응이다.

### 보조정리 3: 한 왼쪽 부분집합에 더하는 값이 정확하다

왼쪽 합이 `a`로 고정되면 전체 합 조건은 `a+b=target`, 즉 오른쪽 합 `b=target-a`와 동치다. 정렬된 오른쪽 목록에서 `equal_range`가 반환한 구간에는 이 값과 같은 원소가 모두, 그리고 그것만 들어 있다. 보조정리 1에 의해 각 원소는 오른쪽 부분집합 하나이므로 구간 크기가 결합 가능한 오른쪽 부분집합 수다.

### 정리

알고리즘은 각 왼쪽 부분집합에 대해 보조정리 3의 정확한 오른쪽 짝 수를 더한다. 보조정리 2에 의해 모든 전체 부분집합은 정확히 한 왼쪽·오른쪽 쌍에 대응하므로 빠지거나 두 번 세는 해가 없다. 따라서 출력은 합이 `target`인 전체 부분집합의 정확한 개수다.

## 시간·공간 복잡도

왼쪽 크기를 `nL`, 오른쪽 크기를 `nR`, `L=2^nL`, `R=2^nR`라 하자.

- 절반 합 열거: 각 단계에서 목록 크기만큼 append하고 전체가 기하급수 합이므로 `O(L+R)` 시간.
- 오른쪽 정렬: `O(R log R)` 시간.
- 왼쪽별 이진 탐색: `O(L log R)` 시간.
- 총 시간: `O(L + R log R + L log R)`, 균등 분할이면 `O(n * 2^(n/2))`.
- 추가 공간: 두 합 목록 `O(L+R)`, 균등 분할이면 `O(2^(n/2))`.

CSES의 `n<=40`에서는 각 목록이 최대 `2^20=1,048,576`개다. `long long` 두 목록만 보면 약 16 MiB이며 컨테이너·정렬 호출 스택 오버헤드를 더해도 512 MiB 제한 안이다.

## 대회 우승에 필요한 구현 판단

### 1. 같은 합을 제거하지 않는다

문제는 가능한 합의 종류가 아니라 부분집합의 **개수**를 묻는다. `sort` 뒤 `unique`를 적용하면 다른 인덱스 선택을 합 하나로 합쳐 오답이 된다. 값이 네 개 모두 1이고 목표가 2라면 답은 합 값 하나가 아니라 `C(4,2)=6`이다.

### 2. 합과 답을 모두 64비트로 둔다

절반 합은 최대 `20*10^9=2*10^10`, 전체 합은 최대 `4*10^10`이다. 답의 일반 상계는 `2^40`이다. `target-left_sum`은 음수가 될 수도 있으므로 unsigned를 쓰면 underflow가 검색 키를 큰 양수로 바꾼다. 합·보수·답을 signed 64비트로 통일한다.

### 3. shift의 왼쪽 피연산자 타입을 먼저 넓힌다

`1 << width`의 `1`은 int다. 더 큰 일반화에서 width가 int 비트 수 이상이면 잘못된 shift가 된다. `std::size_t{1} << width`처럼 저장 크기 타입을 먼저 정하고, 반드시 `width < numeric_limits<size_t>::digits`를 보장한다. 오늘은 `width<=20`이라 안전하다.

### 4. append 중 iterator를 보관하지 않는다

`push_back`은 capacity가 부족하면 vector 저장소를 재할당해 기존 iterator·참조를 모두 무효화한다. 오늘은 `reserve`로 전체 크기를 확보하지만, 정확성 자체를 capacity에 암묵적으로 기대지 않도록 정수 인덱스와 고정한 `previous_size`를 쓴다. `for (auto sum : sums) sums.push_back(...)`는 range-for의 끝과 iterator가 append와 충돌하므로 피한다.

### 5. 빈 부분집합 합 0을 포함한다

해가 한쪽 절반에만 있을 수 있다. 반대쪽이 아무것도 고르지 않는 경우를 표현하려면 두 목록 모두 0에서 시작해야 한다. `n=1`인 경우 한 절반이 비어도 같은 원칙으로 동작한다.

### 6. 한쪽만 정렬해도 충분하다

각 왼쪽 합에 이진 탐색을 할 것이므로 오른쪽만 정렬하면 된다. 양쪽을 정렬하는 것은 투 포인터/group-count 변형을 쓸 때 의미가 있다. 불필요한 왼쪽 정렬은 정답을 깨지는 않지만 비용과 코드만 늘린다.

## 흔한 실수

1. `2^40`을 직접 열거해 시간 초과가 난다.
2. 부분집합 합 목록에서 같은 합을 `unique`로 제거한다.
3. 답이나 합을 32비트 `int`에 저장한다.
4. `target-left_sum`을 unsigned로 계산해 음수 보수가 wraparound한다.
5. `previous_size`를 append 루프 안에서 계속 다시 읽어 현재 원소를 여러 번 고른다.
6. `lower_bound` 하나의 존재 여부만 더해 중복 오른쪽 부분집합을 1개로 센다.
7. 빈 부분집합 합 0을 넣지 않아 한쪽만 사용하는 해를 잃는다.
8. 정렬하지 않은 목록에 `equal_range`를 호출해 partition 전제조건을 깬다.
9. `1 << half_size`를 int로 계산한다.
10. 문제의 "같은 값"과 "같은 위치 선택"을 혼동한다.

## 변형

- **존재 여부**: `equal_range` 크기 대신 `binary_search`로 한 쌍 존재만 확인한다.
- **가장 가까운 합**: 한쪽 합을 정렬하고 각 반대쪽 합마다 `lower_bound` 주변 후보를 검사한다.
- **합이 X 이하인 쌍 수**: 정렬된 양쪽 목록을 투 포인터로 훑거나 각 왼쪽 합에 `upper_bound`를 쓴다.
- **중복 그룹 투 포인터**: 양쪽을 정렬한 뒤 왼쪽 작은 값과 오른쪽 큰 값을 움직이고, 합이 맞으면 양쪽 같은 값의 빈도 곱을 더한다.
- **선택 복원**: 합과 함께 절반 bitmask를 저장한다. 메모리가 늘고 같은 합 중 어느 해를 복원할지 정책이 필요하다.
- **최소 차이 분할**: 총합 절반에 가장 가까운 부분집합 합을 찾도록 결합 조건을 바꾼다.
- **4-SUM**: 두 수 쌍 합 목록 두 개를 만들고 보수 합을 탐색하는 것도 같은 공간 분할 관점이다.
- **Schroeppel–Shamir 계열**: 메모리가 병목일 때 네 묶음과 heap을 사용해 공간을 더 줄이는 고급 변형이 있다.

## 오늘 문제와의 연결

2026-09-25의 [CSES 1628 — Meet in the Middle](https://cses.fi/problemset/task/1628/)은 `n<=40`이고 값·목표 범위가 커서 목표 합 크기에 비례하는 DP보다 MITM이 자연스럽다. [`../2026-09-25/icpc_problem.cpp`](../2026-09-25/icpc_problem.cpp)는 doubling 불변식으로 두 합 목록을 만들고, 오른쪽만 정렬한 뒤 `std::ranges::equal_range`의 구간 크기를 더한다.

공식 예제뿐 아니라 네 개의 1에서 목표 2인 중복 계수, 한 원소/빈 절반, 큰 합, 그리고 40개의 1에서 목표 20인 `137846528820`을 검증해야 합 값과 답의 64비트 요구를 함께 확인할 수 있다.

## 직접 검증

1. `[1,2,3]`을 왼쪽 `[1]`, 오른쪽 `[2,3]`으로 나눠 두 합 목록과 목표 3의 모든 결합을 적는다.
2. `previous_size`를 매 반복 조건에서 `sums.size()`로 바꾸면 왜 루프가 끝나지 않거나 같은 원소를 중복 선택하는지 설명한다.
3. `[1,1,1,1]`, 목표 2에서 왼쪽 합 1과 오른쪽 합 1의 multiplicity 곱을 계산한다.
4. 오른쪽 정렬을 지운 뒤 `equal_range`를 쓰는 것이 단순 오답을 넘어 어떤 라이브러리 전제조건을 깨는지 말한다.
5. 전체 원소가 40개일 때 `2^20` 목록 두 개와 `2^40` 전수 조사의 크기 차이를 수치로 비교한다.
6. 값에 음수가 허용돼도 정확성 증명이 유지되는지, 어떤 양수 전용 가지치기만 사용할 수 없게 되는지 구분한다.
7. 양쪽 정렬+투 포인터 빈도 묶기 버전을 작성하고 이진 탐색 버전과 복잡도를 비교한다.
