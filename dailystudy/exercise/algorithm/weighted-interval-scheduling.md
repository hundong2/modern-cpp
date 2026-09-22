# 가중 구간 스케줄링

## 정의

가중 구간 스케줄링(weighted interval scheduling)은 시작 시각, 종료 시각, 가치가 있는 여러 구간에서 서로 겹치지 않는 일부를 골라 가치 합을 최대화하는 문제다. 단순히 가장 빨리 끝나는 구간을 고르는 탐욕법은 **선택 개수**를 최대화할 때는 맞지만, 가치가 서로 다르면 큰 가치 하나를 버릴 수 있어 일반적으로 틀린다.

오늘 문제에서는 날짜의 양끝을 모두 프로젝트 기간에 포함한다. 따라서 앞 프로젝트의 종료일 `finish`와 다음 프로젝트의 시작일 `start`가 같아도 같은 날을 함께 사용하므로 겹친다. 호환 조건은 `finish < start`다.

## 적용 조건

- 각 후보를 하나의 연속 구간과 선택했을 때 얻는 가중치로 표현할 수 있다.
- 선택한 두 구간이 겹치지 않아야 하며, 전체 목적 함수가 선택 가중치의 합이다.
- 구간을 종료 시각 순으로 정렬했을 때 현재 구간과 호환되는 과거 후보가 접두사를 이뤄야 한다.
- 끝점의 포함 여부를 정확히 알아야 한다. 반열린 구간 `[start, finish)`라면 `finish <= next_start`, 오늘처럼 닫힌 날짜 구간이면 `finish < next_start`다.
- 가중치가 음수일 수도 있는 변형에서는 아무것도 고르는 해를 허용하는지 먼저 확인해야 한다. 오늘 보상은 모두 양수다.

## 핵심 아이디어와 불변식

프로젝트를 종료일 비감소 순으로 `P[0..n)`에 정렬하고 다음을 정의한다.

- `finish[i]`: `P[i]`의 종료일. 정렬되어 비감소한다.
- `compatible(i)`: `j < i`이면서 `finish[j] < P[i].start`인 프로젝트의 개수. 즉 호환 가능한 접두사의 길이다.
- `dp[i]`: 정렬된 첫 `i`개 프로젝트 `P[0..i)`만 고려했을 때 얻는 최대 보상. `dp[0]=0`이다.

반복 시작 시 유지할 불변식은 다음과 같다.

1. `finish[0..i)`는 정렬된 첫 `i`개의 종료일을 담고 비감소한다.
2. 모든 `0 <= k <= i`에 대해 `dp[k]`는 첫 `k`개만 사용한 최적값이다.
3. `P[i]`를 선택하는 해는 이전 선택을 `P[0..compatible(i))`에서만 가져와야 하며, 그 접두사의 최적값은 이미 `dp[compatible(i)]`에 있다.

따라서 점화식은 다음과 같다.

```text
dp[i + 1] = max(
    dp[i],                              // P[i]를 선택하지 않는다
    P[i].reward + dp[compatible(i)]     // P[i]를 선택한다
)
```

`compatible(i)`는 `finish[0..i)`에서 `P[i].start` 이상인 첫 위치를 찾는 lower-bound 이분 탐색의 반환 위치다. 그 위치 앞의 값만 시작일보다 엄격히 작다.

## 단계별 절차

1. 모든 `(start, finish, reward)`를 읽는다.
2. 프로젝트를 `finish` 오름차순으로 정렬한다. 동률은 어느 순서여도 정답에 영향이 없지만 재현성을 위해 `start`로 한 번 더 정렬할 수 있다.
3. 정렬된 종료일 배열과 `n+1`칸 DP 배열을 0으로 만든다.
4. 프로젝트를 왼쪽부터 하나씩 처리한다.
   - 현재 종료일을 종료일 배열에 기록한다.
   - 이전 구간에서 `finish < current.start`인 접두사 길이를 이분 탐색한다.
   - 현재 프로젝트를 건너뛴 값과 선택한 값을 비교해 다음 DP 칸에 저장한다.
5. `dp[n]`을 출력한다.

## 의사코드

```text
sort projects by (finish, start)
finish_days = array of n zeros
dp = array of n+1 zeros

for i = 0 .. n-1:
    finish_days[i] = projects[i].finish
    k = first index in finish_days[0..i) whose value >= projects[i].start
    skip = dp[i]
    take = projects[i].reward + dp[k]
    dp[i+1] = max(skip, take)

return dp[n]
```

이분 탐색은 다음 불변식으로 직접 구현할 수 있다.

```text
low = 0, high = i
while low < high:
    mid = low + (high-low)/2
    if finish_days[mid] < current.start:
        low = mid+1
    else:
        high = mid
return low
```

항상 `[0,low)`는 호환, `[high,i)`는 비호환이 확정되어 있다. 반복이 끝나면 두 경계가 만나는 위치가 곧 호환 접두사의 길이다.

## 컴파일 가능한 C++ 뼈대

아래 코드는 C++20 이상에서 그대로 컴파일할 수 있다. 오늘의 제출본은 [`../2026-09-23/icpc_problem.cpp`](../2026-09-23/icpc_problem.cpp)에 초보자 설명과 각 표준 호출의 엄격한 6부 계약까지 포함한다.

```cpp
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <vector>

struct Project {
    long long start{};
    long long finish{};
    long long reward{};
};

std::size_t compatible_count(
    const std::vector<long long>& finishes,
    std::size_t prefix,
    long long start) {
    std::size_t low{};
    std::size_t high{prefix};
    while (low < high) {
        const std::size_t mid{low + (high - low) / 2U};
        if (finishes[mid] < start) {
            low = mid + 1U;
        } else {
            high = mid;
        }
    }
    return low;
}

int main() {
    int n{};
    std::cin >> n;
    const std::size_t count{static_cast<std::size_t>(n)};
    std::vector<Project> projects(count);
    for (Project& project : projects) {
        std::cin >> project.start >> project.finish >> project.reward;
    }

    std::ranges::sort(projects, [](const Project& a, const Project& b) {
        return a.finish != b.finish ? a.finish < b.finish : a.start < b.start;
    });

    std::vector<long long> finishes(count, 0);
    std::vector<long long> dp(count + 1U, 0);
    for (std::size_t i{}; i < count; ++i) {
        const Project& current{projects[i]};
        finishes[i] = current.finish;
        const std::size_t k{compatible_count(finishes, i, current.start)};
        const long long take{current.reward + dp[k]};
        dp[i + 1U] = take > dp[i] ? take : dp[i];
    }
    std::cout << dp[count] << '\n';
}
```

## 정확성 근거

### 보조정리 1: 호환 가능한 이전 프로젝트는 하나의 접두사다

프로젝트가 종료일 비감소 순으로 정렬되어 있으므로 어떤 위치 `k`에서 `finish[k] >= current.start`가 처음 성립하면 모든 `j >= k`에서도 종료일이 현재 시작일 이상이다. 닫힌 날짜 구간에서는 이들이 모두 현재 프로젝트와 겹친다. 반대로 모든 `j < k`는 `finish[j] < current.start`라 함께 선택할 수 있다. 따라서 호환 후보는 정확히 `P[0..k)`다.

### 보조정리 2: 점화식은 첫 `i+1`개 프로젝트의 모든 가능성을 분할한다

첫 `i+1`개의 최적해는 `P[i]`를 선택하지 않거나 선택한다.

- 선택하지 않으면 첫 `i`개만 쓴 해이므로 보상은 최대 `dp[i]`이고, 그 값은 실제로 가능하다.
- 선택하면 보조정리 1에 따라 나머지는 첫 `compatible(i)`개에서만 선택할 수 있다. 그 최적값은 귀납 가정으로 `dp[compatible(i)]`이므로 가능한 최댓값은 `P[i].reward + dp[compatible(i)]`다.

두 경우는 서로 배타적이며 모든 해를 포함하므로 둘의 최댓값이 정확한 최적값이다.

### 정리

기저 `dp[0]=0`은 프로젝트가 없을 때 정확하다. 보조정리 2를 `i=0`부터 차례대로 적용하면 귀납적으로 모든 `dp[i]`가 첫 `i`개의 최적값이다. 따라서 마지막 `dp[n]`은 전체 프로젝트에서 얻을 수 있는 최대 보상이다.

## 시간·공간 복잡도

- 종료일 정렬: `O(n log n)` 시간.
- 각 프로젝트의 이분 탐색: 한 번에 `O(log n)`, 전체 `O(n log n)` 시간.
- DP 전이: 전체 `O(n)` 시간.
- 총 시간: `O(n log n)`.
- 프로젝트, 종료일, DP 배열: `O(n)` 공간.

`n <= 200000`, `reward <= 10^9`이므로 답은 최대 `2 * 10^14`다. 보상과 DP는 64비트 정수로 저장해야 한다.

## 흔한 실수

- 닫힌 날짜 구간인데 `finish <= start`를 호환으로 보아 같은 날 두 프로젝트를 허용한다.
- 시작일로만 정렬한 뒤 같은 점화식을 쓰면서 호환 후보가 접두사라는 성질을 잃는다.
- 이분 탐색 범위에 현재 프로젝트까지 포함해 자기 자신을 참조하는 전이를 만든다.
- `lower_bound`와 `upper_bound`의 경계 의미를 외우기만 하고 끝점 포함 규칙을 대입하지 않는다. 오늘은 첫 `finish >= start` 위치가 필요하다.
- `dp[i]`가 “i번 프로젝트를 반드시 고른 값”인지 “첫 i개의 최적값”인지 정의를 섞는다.
- 답을 `int`로 두어 큰 입력에서 오버플로한다.
- 가치 없는 일반 구간 스케줄링의 “가장 빨리 끝나는 구간을 즉시 선택” 탐욕법을 그대로 사용한다.
- 정렬 뒤 원래 입력 순서가 필요해지는 변형에서 원래 ID를 함께 저장하지 않는다.

## 변형

- 선택한 프로젝트 복원: `take > skip` 여부와 직전 DP 위치를 저장한 뒤 뒤에서 추적한다.
- 정확히 `k`개 선택: DP에 선택 개수 차원을 추가한다. 시간·공간 증가를 먼저 계산한다.
- 동시에 최대 `c`개 수행: 단순 1차원 DP가 아니라 sweep line, min-cost flow 등 용량 모델을 검토한다.
- 온라인 삽입/질의: 좌표 압축 뒤 Fenwick tree나 segment tree에 “해당 종료일까지의 최대 보상”을 저장한다.
- 가중치가 음수이고 하나 이상 반드시 선택: `dp[0]=0`만으로 빈 선택이 섞이지 않도록 불가능 sentinel을 둔다.
- 반열린 시간 구간: 호환 조건을 `finish <= start`로 바꾸고 이에 맞는 upper-bound/lower-bound 경계를 선택한다.

## 오늘 문제와의 연결

- 문제: [CSES 1140 — Projects](https://cses.fi/problemset/task/1140/)
- 공식 제약: `1 <= n <= 200000`, `1 <= a_i <= b_i <= 10^9`, `1 <= p_i <= 10^9`.
- 핵심 모델: 프로젝트를 닫힌 날짜 구간과 보상으로 보고 종료일 정렬 + 이분 탐색 + 1차원 DP를 적용한다.
- 핵심 경계: `finish == next_start`는 같은 날이 겹치므로 호환되지 않는다.
- 공식 예제에서는 `(2,4,4)`와 `(5,7,3)`을 골라 보상 `7`을 얻는다.
- 실전 검증: 단일 프로젝트, 같은 날 경계, 모두 겹침, 모두 연결 가능, 같은 종료일, 보상 합이 32비트를 넘는 사례를 확인하고 작은 `n`에서는 모든 부분집합 전수조사와 비교한다.
