# Manacher로 최장 팰린드롬 부분 문자열 찾기

## 정의

팰린드롬은 왼쪽에서 읽은 문자열과 오른쪽에서 읽은 문자열이 같은 문자열이다. **부분 문자열(substring)** 은 원문에서 연속한 구간이므로, 문자를 건너뛸 수 있는 부분 수열(subsequence)과 다르다.

Manacher 알고리즘은 문자열의 모든 위치를 중심으로 하는 최대 팰린드롬 반지름을 `O(n)` 시간에 계산한다. 각 중심에서 양쪽을 처음부터 다시 비교하는 대신, 이미 확인한 가장 오른쪽 팰린드롬과 그 안의 대칭 중심 정보를 재사용한다. 계산한 반지름 중 가장 긴 것을 고르면 최장 팰린드롬 부분 문자열도 선형 시간에 얻는다.

## 적용 조건

- 한 문자열에서 최장 팰린드롬 **부분 문자열** 또는 모든 중심의 최대 팰린드롬 길이가 필요할 때 적합하다.
- 문자열 길이가 커서 각 중심을 독립적으로 확장하는 최악 `O(n^2)` 풀이가 허용되지 않을 때 유용하다.
- 문자의 동등 비교가 가능하면 알파벳 종류와 무관하게 적용할 수 있다.
- 같은 문자열에 팰린드롬 질의가 많이 들어오면 반지름 배열을 전처리 결과로 재사용할 수 있다.
- 문자가 계속 추가되는 온라인 문제, 편집이 있는 동적 문자열, 팰린드롬 부분 수열 문제에는 그대로 적용할 수 없다. 각각 팰린드롬 트리(eertree), 해시·세그먼트 트리, 구간 DP 같은 다른 도구를 검토한다.

## 홀수·짝수 반지름 표현

문자열을 `s[0..n-1]`이라 하자. 구분 문자를 삽입해 홀짝을 통합할 수도 있지만, 이 문서에서는 원본 인덱스를 그대로 보존하는 두 배열을 쓴다.

### 홀수 길이: `odd[i]`

`odd[i]`는 중심 문자 `i`를 포함해 한쪽으로 몇 칸까지 팰린드롬인지 나타낸다. 항상 최소 1이다.

```text
팰린드롬 구간 = [i - odd[i] + 1, i + odd[i] - 1]
길이          = 2 * odd[i] - 1
```

예를 들어 `s = "abac"`에서 중심 `i=1`의 최대 팰린드롬은 `aba`이므로 `odd[1]=2`다.

### 짝수 길이: `even[i]`

`even[i]`는 문자 `i-1`과 `i` 사이의 틈을 중심으로 서로 맞는 문자 쌍의 수다. 빈 팰린드롬만 가능한 중심에서는 0이다.

```text
팰린드롬 구간 = [i - even[i], i + even[i] - 1]
길이          = 2 * even[i]
```

예를 들어 `s = "abba"`에서 가운데 틈을 나타내는 `i=2`에 대해 `even[2]=2`이고 구간은 `[0,3]`이다. 짝수 중심을 문자 인덱스로 착각하지 않는 것이 중요하다.

## 핵심 아이디어와 가장 오른쪽 구간 불변식

홀수 배열과 짝수 배열을 각각 왼쪽에서 오른쪽으로 계산한다. 두 순회 모두 다음 불변식을 유지한다.

> 다음 중심 `i`를 처리하기 직전, `[left, right]`는 지금까지 완성한 중심들의 최대 팰린드롬 가운데 오른쪽 끝점이 가장 큰 구간이다. `right` 바깥 문자는 아직 이 구간의 대칭성으로 확인되지 않았다.

동률이면 어느 구간을 보존해도 된다. 아직 구간이 없음을 `left=0`, `right=-1`로 나타낸다.

### 홀수 중심의 거울 복사

`i > right`이면 재사용할 구간이 없으므로 중심 문자 하나가 만드는 반지름 `1`에서 확장한다.

`i <= right`이면 `[left,right]`가 팰린드롬이므로 `i`의 거울 중심은 다음과 같다.

```text
mirror = left + right - i
```

거울 중심에서 이미 구한 반지름을 복사하되 현재 오른쪽 경계를 넘는 부분은 아직 보장할 수 없다.

```text
radius = min(odd[mirror], right - i + 1)
```

이 초기 반지름이 나타내는 구간은 대칭성으로 이미 팰린드롬임이 보장된다. 그 다음 문자 쌍부터 직접 비교한다. 새 팰린드롬이 `right`보다 멀리 가면 `[left,right]`를 새 구간으로 갱신한다.

### 짝수 중심의 거울 복사

짝수 중심 `i`는 `i-1`과 `i` 사이의 틈이다. `[left,right]` 안에서 이 틈을 반사한 중심은 홀수 공식과 1만큼 다르다.

```text
mirror = left + right - i + 1
radius = min(even[mirror], right - i + 1)
```

경계 밖이면 반지름 `0`에서 시작한다. 확장할 때 비교할 문자는 `s[i-radius-1]`와 `s[i+radius]`다. 최대 반지름 `radius`의 실제 구간은 `[i-radius, i+radius-1]`이므로, 갱신식도 `left=i-radius`, `right=i+radius-1`이다.

### 대회장에서 기억할 불변식 세 줄

1. 반지름의 `min` 안에는 **거울 반지름**과 **현재 오른쪽 경계까지의 길이**가 들어간다.
2. 재사용한 구간 안쪽은 다시 비교하지 않고, 아직 확인하지 않은 첫 문자 쌍부터 확장한다.
3. 더 오른쪽으로 나간 경우에만 `[left,right]`를 갱신한다.

## 단계별 절차

1. 길이 `n`의 `odd` 배열을 0으로 만들고 `left=0`, `right=-1`로 둔다.
2. 각 문자 인덱스 `i`를 왼쪽에서 오른쪽으로 방문한다.
3. `i > right`이면 홀수 반지름을 1로 시작한다. 아니면 거울 중심의 반지름과 `right-i+1` 중 작은 값으로 시작한다.
4. 인덱스가 문자열 안에 있고 양쪽 문자가 같은 동안 반지름을 늘린다.
5. 완성한 구간의 오른쪽 끝이 기존 `right`보다 크면 `left`와 `right`를 갱신한다.
6. 같은 절차를 `even` 배열에 적용하되 초기 반지름은 0, 거울 중심은 `left+right-i+1`, 비교 인덱스는 `i-radius-1`과 `i+radius`를 쓴다.
7. 두 배열을 순회해 `2*odd[i]-1`과 `2*even[i]` 중 최댓값을 고른다.
8. 홀수 답의 시작점은 `i-odd[i]+1`, 짝수 답의 시작점은 `i-even[i]`다.

## 의사코드

```text
function manacher_odd(s):
    odd[0..n-1] = 0
    left = 0, right = -1

    for i = 0 .. n-1:
        if i > right:
            radius = 1
        else:
            mirror = left + right - i
            radius = min(odd[mirror], right - i + 1)

        while i-radius >= 0 and i+radius < n
              and s[i-radius] == s[i+radius]:
            radius += 1

        odd[i] = radius
        if i+radius-1 > right:
            left = i-radius+1
            right = i+radius-1

    return odd

function manacher_even(s):
    even[0..n-1] = 0
    left = 0, right = -1

    for i = 0 .. n-1:
        if i > right:
            radius = 0
        else:
            mirror = left + right - i + 1
            radius = min(even[mirror], right - i + 1)

        while i-radius-1 >= 0 and i+radius < n
              and s[i-radius-1] == s[i+radius]:
            radius += 1

        even[i] = radius
        if i+radius-1 > right:
            left = i-radius
            right = i+radius-1

    return even
```

## 완전히 컴파일 가능한 C++20 뼈대

아래 프로그램은 공백이 없는 문자열 하나를 읽어 최장 팰린드롬 부분 문자열 하나를 출력한다. 최장 답이 여러 개면 순회 중 먼저 찾은 답을 유지한다.

```cpp
#include <algorithm> // std::min으로 안전하게 복사할 반지름을 제한한다.
#include <cstddef>   // std::size_t가 문자열과 vector의 인덱스를 표현한다.
#include <iostream>  // std::cin과 std::cout으로 표준 입출력을 수행한다.
#include <string>    // std::string이 입력 문자열과 결과 문자열을 소유한다.
#include <vector>    // std::vector<int>가 중심별 반지름을 연속 저장한다.

// 반환 vector의 i번째 값은 중심 i의 홀수 팰린드롬 반지름이다.
[[nodiscard]] std::vector<int> manacher_odd(const std::string& text) {
    const int size{static_cast<int>(text.size())};
    std::vector<int> radius(static_cast<std::size_t>(size), 0);

    int left{};
    int right{-1};
    for (int center{}; center < size; ++center) {
        int current{1};
        if (center <= right) {
            const int mirror{left + right - center};
            current = std::min(
                radius[static_cast<std::size_t>(mirror)], right - center + 1);
        }

        // current가 이미 보장하는 구간 바로 바깥의 문자 쌍부터 비교한다.
        while (center - current >= 0 && center + current < size
               && text[static_cast<std::size_t>(center - current)]
                      == text[static_cast<std::size_t>(center + current)]) {
            ++current;
        }
        radius[static_cast<std::size_t>(center)] = current;

        const int candidate_right{center + current - 1};
        if (candidate_right > right) {
            left = center - current + 1;
            right = candidate_right;
        }
    }
    return radius;
}

// 반환 vector의 i번째 값은 i-1과 i 사이를 중심으로 하는 문자 쌍 수다.
[[nodiscard]] std::vector<int> manacher_even(const std::string& text) {
    const int size{static_cast<int>(text.size())};
    std::vector<int> radius(static_cast<std::size_t>(size), 0);

    int left{};
    int right{-1};
    for (int center{}; center < size; ++center) {
        int current{};
        if (center <= right) {
            const int mirror{left + right - center + 1};
            current = std::min(
                radius[static_cast<std::size_t>(mirror)], right - center + 1);
        }

        while (center - current - 1 >= 0 && center + current < size
               && text[static_cast<std::size_t>(center - current - 1)]
                      == text[static_cast<std::size_t>(center + current)]) {
            ++current;
        }
        radius[static_cast<std::size_t>(center)] = current;

        const int candidate_right{center + current - 1};
        if (candidate_right > right) {
            left = center - current;
            right = candidate_right;
        }
    }
    return radius;
}

struct PalindromeRange {
    std::size_t start{};
    std::size_t length{};
};

[[nodiscard]] PalindromeRange longest_palindrome(const std::string& text) {
    if (text.empty()) {
        return {};
    }

    const std::vector<int> odd{manacher_odd(text)};
    const std::vector<int> even{manacher_even(text)};
    PalindromeRange best{0U, 1U};

    for (int center{}; center < static_cast<int>(text.size()); ++center) {
        const int odd_length{2 * odd[static_cast<std::size_t>(center)] - 1};
        if (static_cast<std::size_t>(odd_length) > best.length) {
            best.start = static_cast<std::size_t>(
                center - odd[static_cast<std::size_t>(center)] + 1);
            best.length = static_cast<std::size_t>(odd_length);
        }

        const int even_length{2 * even[static_cast<std::size_t>(center)]};
        if (static_cast<std::size_t>(even_length) > best.length) {
            best.start = static_cast<std::size_t>(
                center - even[static_cast<std::size_t>(center)]);
            best.length = static_cast<std::size_t>(even_length);
        }
    }
    return best;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string text{};
    if (!(std::cin >> text)) {
        return 0;
    }

    const PalindromeRange answer{longest_palindrome(text)};
    std::cout << text.substr(answer.start, answer.length) << '\n';
}
```

`text`가 살아 있는 동안 두 함수의 `const std::string&` 참조는 유효하다. 반지름 배열과 `substr` 결과는 각각 자기 저장소를 소유한다. `int`는 CSES의 문자열 길이 범위를 충분히 담지만, 범위가 `INT_MAX`를 넘을 수 있는 일반 라이브러리 코드라면 `std::ptrdiff_t` 같은 부호 있는 인덱스형과 명시적 범위 검사를 고려한다.

## 정확성 증명

### 보조정리 1: 거울에서 가져온 초기 반지름은 항상 유효하다

중심 `i`가 `right` 안에 있다고 하자. `[left,right]` 전체가 팰린드롬이므로 그 안에서 서로 반사되는 위치의 문자는 같다. 홀수 중심의 거울은 `left+right-i`, 짝수 틈 중심의 거울은 `left+right-i+1`이다.

거울 중심의 최대 반지름과 `i`에서 오른쪽 경계까지 포함한 길이 `right-i+1` 중 작은 반지름을 택하면, 복사한 구간은 `[left,right]` 밖으로 나가지 않는다. 따라서 그 안의 모든 대칭 문자 쌍은 이미 같은 것으로 확인되어 있으며 초기 반지름은 유효하다.

### 보조정리 2: 각 중심에 기록한 반지름은 최대다

`i > right`이면 홀수는 자명하게 유효한 중심 문자 하나, 짝수는 빈 반지름에서 시작해 양쪽 문자를 직접 비교한다.

`i <= right`이면 보조정리 1에 따라 초기 반지름까지는 유효하다. 거울 팰린드롬이 현재 경계 안에서 먼저 끝나는 경우, 그 거울 중심을 계산할 때 확인한 불일치가 대칭 위치에서도 확장을 막는다. 거울 반지름이 경계에 잘린 경우에는 경계 바깥 정보가 없으므로 직접 비교한다.

두 경우 모두 반복문은 다음 문자 쌍이 다르거나 문자열 경계에 닿을 때만 끝난다. 따라서 더 큰 반지름은 존재할 수 없고 기록한 값은 해당 중심의 최대 반지름이다.

### 보조정리 3: `[left,right]` 불변식이 유지된다

현재 중심의 최대 반지름을 구한 뒤 새 오른쪽 끝이 기존 `right` 이하이면, 기존 구간이 여전히 지금까지 가장 오른쪽에 닿는다. 새 끝이 더 크면 현재 최대 팰린드롬의 양 끝으로 갱신한다. 두 경우 모두 다음 중심을 처리하기 전에 `[left,right]`는 처리 완료한 중심 중 오른쪽 끝이 가장 큰 팰린드롬이다.

### 정리: 반환 구간은 최장 팰린드롬 부분 문자열이다

보조정리 2에 의해 `odd[i]`와 `even[i]`는 모든 홀수·짝수 중심의 최대 팰린드롬을 정확히 나타낸다. 모든 비어 있지 않은 팰린드롬은 문자 하나 또는 문자 사이 틈 하나를 유일한 중심으로 가지므로 두 배열이 모든 후보를 포함한다. 알고리즘이 그 길이를 전부 비교해 가장 긴 구간을 반환하므로 결과는 최장 팰린드롬 부분 문자열이다.

## `O(n)` 상각 시간 증명과 공간 복잡도

겉으로는 각 중심 안에 `while` 확장이 있어 제곱 시간처럼 보이지만, 비교를 두 종류로 나누면 선형 상한을 얻는다.

- 거울 반지름이 현재 경계 안에서 끝나면, 복사한 반지름 뒤의 첫 비교는 많아야 한 번 실패하고 그 중심의 확장이 끝난다.
- 복사한 반지름이 `right`에서 잘리면, 다음 성공 비교는 반드시 기존 `right` 바깥 문자를 포함하며 성공할 때마다 `right`가 최소 1 증가한다.

`right`는 순회 하나에서 `-1`부터 최대 `n-1`까지만 증가한다. 경계를 늘리는 성공 비교는 총 `O(n)`, 각 중심의 마지막 실패 비교도 총 `O(n)`이다. 나머지 구간은 거울 값으로 상수 시간에 건너뛴다. 따라서 홀수 순회와 짝수 순회가 각각 `O(n)`이고, 최장 구간을 고르는 순회까지 포함한 총 시간은 `O(n)`이다.

- `odd`, `even`: 각각 `n`개의 정수이므로 `O(n)` 공간
- 입력 문자열과 출력 부분 문자열: 각각 최악 `O(n)` 공간
- 반지름 계산 자체의 배열 외 보조 상태: `O(1)` 공간
- 전체 작업 공간: `O(n)`

문자 비교·로드·분기와 vector/string 메모리 접근의 실제 비용은 CPU, 컴파일러, 최적화 옵션, 문자 자료형에 따라 달라질 수 있다. 복잡도 증명은 특정 어셈블리 명령 수를 가정하지 않는다.

## 흔한 실수

1. 홀수 반지름을 중심 제외 거리로 이해해 길이를 `2*r+1`로 계산한다. 이 문서의 `odd`는 중심을 포함하므로 `2*r-1`이다.
2. `even[i]`의 중심을 문자 `i`라고 생각한다. 실제 중심은 `i-1`과 `i` 사이이며 구간 시작은 `i-even[i]`다.
3. 짝수 거울 인덱스에서 `+1`을 빠뜨린다. 올바른 식은 `left+right-i+1`이다.
4. 초기 반지름을 경계 길이로 자르지 않고 거울 값을 그대로 복사해, 아직 확인하지 않은 `[left,right]` 밖을 참으로 가정한다.
5. 홀수 확장을 `s[i-r]`와 `s[i+r]`가 아닌 이미 확인한 안쪽 문자부터 다시 비교해 선형 시간의 장점을 잃는다.
6. 갱신 구간의 포함 끝점과 반열린 구간을 섞는다. 이 문서의 `right`는 포함되는 마지막 인덱스다.
7. 짝수 팰린드롬을 검사하지 않아 `abba` 같은 답을 놓친다.
8. 홀수 답의 시작점 `i-r+1`과 짝수 답의 시작점 `i-r`을 서로 바꾼다.
9. 빈 문자열에서 `best={0,1}`을 그대로 사용해 범위를 벗어난 부분 문자열을 만든다.
10. 최장 답이 여러 개인데 특정 답만 정답이라고 가정해 테스트한다. 문제에서 어느 하나를 허용하면 길이와 팰린드롬 성질을 함께 검사한다.
11. 모든 중심에서 실제 부분 문자열을 만들어 비교해 `O(n^2)` 복사 비용을 추가한다. 계산 중에는 시작점과 길이만 보존한다.
12. 구분 문자를 삽입하는 통합 구현에서 입력에 등장할 수 있는 문자를 구분자로 골라 거짓 일치를 만든다.

## 변형

- **구분 문자로 통합**: 문자 사이와 양끝에 센티널을 두면 홀수·짝수 로직을 하나의 반지름 배열로 합칠 수 있다. 센티널 충돌과 원본 인덱스 복원 공식을 명확히 해야 한다.
- **팰린드롬 개수 세기**: `sum(odd[i]) + sum(even[i])`는 같은 내용이라도 위치가 다른 모든 팰린드롬 부분 문자열의 개수다. 합은 `O(n^2)`까지 커질 수 있어 64비트 정수를 쓴다.
- **특정 구간 판정**: `[l,r]`의 중심과 길이를 반지름 배열에 대입하면 그 구간이 팰린드롬인지 `O(1)`에 확인할 수 있다.
- **가장 긴 접두·접미 팰린드롬**: 각 중심의 실제 구간이 0에서 시작하거나 `n-1`에서 끝나는지 검사한다.
- **모든 서로 다른 팰린드롬**: Manacher는 중심별 최대 길이를 주지만 중복 문자열 자체를 묶어 주지는 않는다. 서로 다른 팰린드롬의 집합과 출현 정보를 온라인으로 관리하려면 eertree를 고려한다.
- **동적 질의**: 문자 갱신이 있으면 전역 반지름을 다시 계산해야 할 수 있다. 양방향 해시와 세그먼트 트리, 오프라인 처리 등 문제 조건에 맞는 대안을 검토한다.

## 오늘 문제와의 연결

[CSES 1111 Longest Palindrome](https://cses.fi/problemset/task/1111/)은 소문자 문자열에서 가장 긴 팰린드롬 부분 문자열 하나를 요구하며, 같은 최장 길이의 답이 여러 개면 어느 하나를 출력할 수 있다. 문자열 길이가 최대 `10^6`이므로 모든 중심에서 처음부터 확장하는 최악 `O(n^2)` 풀이는 안전하지 않다.

오늘의 `icpc_problem.cpp`에서는 홀수·짝수 반지름을 각각 선형 시간에 구하고, 실제 문자열 복사는 최종 답을 출력할 때 한 번만 수행한다. 제출 전에 다음 경계를 별도로 확인한다.

- 길이 1: 홀수 반지름의 최소값과 기본 답
- `aaaa...`: 거의 모든 중심이 길게 확장되는 상각 분석 스트레스
- `abba`: 짝수 중심 인덱스와 시작점 공식
- `abacaba`: 큰 오른쪽 구간 안에서 거울 반지름을 재사용하는 경우
- 답이 여러 개인 문자열: 특정 문자열이 아니라 유효한 최장 답인지 확인

## 직접 해보기와 초보자 검증

1. `s="abacaba"`의 각 중심에 대해 `odd[i]`를 손으로 구하고, 각 반지름을 실제 `[left,right]` 구간으로 바꾼다.
2. `s="abba"`에서 모든 `even[i]`를 구한다. 중심 `i=2`, 반지름 2가 구간 `[0,3]`으로 변환되는 식을 설명한다.
3. `s="aaaaaa"`를 순회하며 매 중심 직전의 `[left,right]`, `mirror`, 초기 반지름, 확장 뒤 반지름을 표로 기록한다.
4. 홀수 거울 식 `left+right-i`와 짝수 거울 식 `left+right-i+1`을 작은 인덱스 그림으로 유도한다.
5. `min(mirror_radius, right-i+1)`에서 두 번째 항을 제거하고 잘못된 값을 만드는 반례를 찾는다.
6. 완성한 `odd`와 `even`으로 임의 구간 `[l,r]`이 팰린드롬인지 `O(1)`에 판정하는 함수를 작성한다.
7. 모든 중심을 단순 확장하는 `O(n^2)` oracle을 작성하고 길이 30 이하의 무작위 문자열 수천 개에서 두 반지름 배열을 비교한다.
8. `sum(odd)+sum(even)`이 `aaa`에서 6이 되는 이유를 실제 위치별 부분 문자열과 대응한다.
9. 최장 답의 길이는 같지만 위치가 여러 개인 문자열을 만들어, 결과의 길이·원문 포함 여부·역순 동일성을 검사하는 테스트를 작성한다.
10. 구분 문자 통합 버전을 구현하고 홀수·짝수 두 배열 버전과 무작위 대조한다. 원본 시작점과 길이를 복원하는 공식을 주석으로 증명한다.
