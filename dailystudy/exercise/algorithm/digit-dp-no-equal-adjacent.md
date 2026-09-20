# Digit DP로 인접한 같은 자릿수 없는 수 세기

## 정의

Digit DP(자릿수 동적 계획법)는 `0..x`처럼 매우 큰 정수 구간을 한 수씩 순회하지 않고, 숫자의 십진 자릿수를 왼쪽부터 선택하면서 **같은 접두사가 만드는 동일한 나머지 문제**를 합치는 방법이다. 오늘 대표 문제에서는 “지금까지 실제 숫자가 시작되었는가”, “직전 실제 자릿수는 무엇인가”, “아직 상한 `x`의 접두사와 같은가”를 상태로 둔다.

누적 함수 `F(x)`를 `0 <= value <= x`이면서 이웃한 실제 십진 자릿값이 모두 다른 정수의 개수라고 정의한다. 그러면 닫힌 구간 `[a,b]`의 답은 `F(b) - F(a-1)`이다. `x < 0`일 때 `F(x)=0`으로 두면 `a=0`도 별도 unsigned 언더플로 없이 처리된다.

## 적용 조건

- 상한 자체는 너무 커서 모든 수를 순회할 수 없지만 자릿수 수 `D`는 작아야 한다. `10^18`은 십진수로 최대 19자리다.
- 조건을 왼쪽 접두사와 작은 추가 상태만으로 판정할 수 있어야 한다. 오늘 조건은 직전 자릿수 하나만 기억하면 된다.
- 구간 조건은 `0..x` 누적 함수의 차로 바꿀 수 있어야 한다.
- 선행 0이 실제 표기에 포함되는지 문제 정의를 먼저 확정해야 한다. 오늘은 포함되지 않는다.

## 핵심 아이디어와 불변식

상태 `dp(position, previous, started, tight)`의 의미는 다음과 같다.

- `position`: 지금 선택할 문자 위치. `0..D`다.
- `previous`: 직전 **실제** 자릿수 `0..9`; 실제 숫자가 아직 시작되지 않았다면 sentinel `10`이다.
- `started`: 지금까지 0이 아닌 자릿수를 하나라도 골랐는가. `false`일 때 고른 0은 자릿수 맞춤용 선행 0이다.
- `tight`: 지금까지 고른 접두사가 상한 `x`의 접두사와 정확히 같은가. 참이면 현재 선택 상한은 `x[position]`, 거짓이면 9다.

재귀에 진입할 때 다음 불변식을 유지한다.

1. 이미 고른 실제 자릿수에는 같은 값이 이웃한 곳이 없다.
2. `started=false`이면 `previous=10`이고, 지금까지 고른 모든 0은 실제 표기에 포함되지 않는다.
3. `tight=true`이면 현재 접두사는 상한 접두사와 같고, `tight=false`이면 이미 더 작아서 나머지는 자유롭게 `0..9`를 고를 수 있다.
4. 한 완성 경로는 고정 길이 선행-0 표현 하나이고, 이는 정수 하나와 일대일 대응한다.

`tight=false`인 상태만 memoize하면 cache key에 구체적인 상한 접두사를 넣지 않아도 된다. `tight=true` 경로는 위치마다 최대 하나이고, 해당 `DigitCounter`의 상한에 종속적이므로 그대로 계산한다.

## 단계별 절차

1. `F(x)`가 음수면 0을 반환한다.
2. `x`를 십진 문자열로 바꾸고 `dp(0, 10, false, true)`를 시작한다.
3. `position == D`이면 1을 반환한다. `started=false`인 all-zero 경로는 정수 0이다.
4. 현재 선택 상한을 `tight ? digit_of_x : 9`로 정한다.
5. 각 `digit`을 시도한다.
   - 이미 시작했고 `digit == previous`이면 건너뛴다.
   - 아직 시작 전이고 `digit==0`이면 sentinel을 유지한다.
   - 그 밖에는 `previous=digit`, `started=true`로 넘긴다.
   - `next_tight = tight && (digit == digit_of_x)`로 갱신한다.
6. 모든 완성 개수를 더하고, `tight=false`라면 상태에 저장한다.
7. 답으로 `F(b)-F(a-1)`을 출력한다.

## 의사코드

```text
count_up_to(x):
    if x < 0: return 0
    digits = decimal_string(x)
    clear cache for this x
    return dfs(0, NONE, false, true)

dfs(pos, prev, started, tight):
    if pos == digits.length:
        return 1                 // all-zero 경로가 정수 0

    if not tight and cache has (pos, prev, started):
        return cache value

    limit = tight ? digits[pos] : 9
    answer = 0
    for digit in 0..limit:
        if started and digit == prev:
            continue
        next_started = started or digit != 0
        next_prev = next_started ? digit : NONE
        next_tight = tight and digit == limit
        answer += dfs(pos+1, next_prev, next_started, next_tight)

    if not tight:
        cache state = answer
    return answer
```

## C++ 뼈대

아래 뼈대는 핵심 상태 전이만 분리한 형태다. 실전 제출본은 [`../2026-09-21/icpc_problem.cpp`](../2026-09-21/icpc_problem.cpp)에 입력·출력 계약과 초보자 주석까지 포함되어 있다.

```cpp
class Counter {
public:
    explicit Counter(long long x) : digits_(std::to_string(x)) {}

    long long run() { return dfs(0, 10, false, true); }

private:
    long long dfs(int pos, int prev, bool started, bool tight) {
        if (pos == static_cast<int>(digits_.size())) return 1;

        const int started_index = started ? 1 : 0;
        if (!tight && seen_[pos][prev][started_index]) {
            return memo_[pos][prev][started_index];
        }

        const int bound = tight
            ? digits_[static_cast<std::size_t>(pos)] - '0'
            : 9;
        long long answer = 0;
        for (int digit = 0; digit <= bound; ++digit) {
            if (started && digit == prev) continue;
            const bool next_started = started || digit != 0;
            answer += dfs(
                pos + 1,
                next_started ? digit : 10,
                next_started,
                tight && digit == bound);
        }

        if (!tight) {
            seen_[pos][prev][started_index] = true;
            memo_[pos][prev][started_index] = answer;
        }
        return answer;
    }

    std::string digits_;
    long long memo_[20][11][2]{};
    bool seen_[20][11][2]{};
};
```

`std::to_string`이 반환한 `std::string`은 `Counter`가 소유한다. `size()`는 문자 수를 반환하고 `operator[]`은 유효한 위치의 문자 참조를 빌려 준다. 따라서 `Counter`보다 오래 문자 참조를 저장하면 안 되며, 위 코드는 참조를 즉시 값으로 읽는다. 각 호출의 더 엄격한 계약은 제출 코드의 첫 호출 바로 위에서 확인한다.

## 정확성 근거

### 보조정리 1: 생성 경로와 `[0,x]` 정수는 일대일 대응한다

각 경로는 상한과 같은 길이의 자릿수열을 정확히 하나 고른다. `tight=true`일 때 상한 자릿수보다 큰 값을 허용하지 않고, 한 번 작은 값을 고르면 이후 `0..9`를 모두 허용한다. 따라서 완성 자릿수열은 `x`를 넘지 않는다. 반대로 `[0,x]`의 각 정수는 앞에 0을 붙여 같은 길이로 만들면 정확히 한 경로가 된다.

### 보조정리 2: 전이는 정확히 인접 중복만 제거한다

`started=false`인 동안의 0은 실제 표기에서 제외되므로 `previous`를 바꾸지 않는다. 실제 표기가 시작된 뒤에는 직전 실제 자릿수 `previous`와 현재 `digit`이 같은 전이만 제거한다. 따라서 남은 경로에는 인접 중복이 없고, 인접 중복이 없는 모든 수의 각 자릿수 전이는 제거되지 않는다.

### 보조정리 3: 기저 사례는 각 유효 정수를 한 번 센다

모든 위치를 채운 경로는 보조정리 1의 정수 하나이고 보조정리 2로 유효하다. 기저가 1을 반환하므로 각 유효 정수를 한 번 센다. 어떤 비영 정수도 서로 다른 선행-0 경로를 갖지 않는다. 모든 자릿수가 0인 단 하나의 경로는 정수 0을 한 번 센다.

### 정리

위 세 보조정리와 위치에 대한 귀납으로 `F(x)`는 `[0,x]`의 모든 유효 정수만 정확히 한 번 센다. `[0,b]`에서 `[0,a-1]`을 빼면 남는 집합은 정확히 `[a,b]`이므로 알고리즘의 출력이 문제의 답이다.

## 시간·공간 복잡도

자릿수 수를 `D`라 하자. `position`은 `D+1`, `previous`는 11, `started`와 `tight`는 각각 2개이고 각 상태에서 최대 10개 전이를 본다.

- 시간: `O(D * 11 * 2 * 2 * 10)`, 보통 `O(D * 10^2)`로 쓴다.
- memo 공간: `O(D * 11 * 2)`; tight 상태를 모두 저장해도 같은 점근 차수다.
- 호출 스택: `O(D)`이며 오늘은 19번의 재귀 전이, 기저 위치를 포함해 최대 20개 활성 frame이다.
- `F`를 두 번 호출해도 상수 2만 곱해지고 차수는 같다.

## 흔한 실수

- 기저에서 `started=false`를 0으로 반환해 정수 0을 빠뜨린다.
- 선행 0을 `previous=0`으로 기록해 `7`의 고정 길이 표현 `0007`을 잘못 탈락시킨다.
- `tight && digit == upper_digit`에서 tight 조건을 빼, 이미 작은 접두사가 다시 tight로 돌아가게 한다.
- `F(a-1)`을 unsigned 타입으로 계산해 `a=0`에서 매우 큰 값으로 언더플로한다.
- 서로 다른 상한의 tight 상태를 전역 cache에서 그대로 재사용한다.
- 답과 memo를 32비트 `int`에 저장한다. 결과는 수십 경만 넘어도 쉽게 32비트를 넘는다.
- 조건이 “인접 중복 금지”인데 과거에 등장한 모든 자릿수를 금지해 전혀 다른 문제를 푼다.

## 변형

- 특정 숫자 패턴 포함/제외: KMP 또는 Aho–Corasick automaton 상태를 `previous` 대신 붙인다.
- 자릿수 합 제한: `sum` 상태를 추가한다.
- 사용한 숫자 집합 제한: 10비트 mask 상태를 추가한다.
- 정확히 `k`번 등장: 등장 횟수 상태를 추가하되 상태 폭발을 계산한다.
- 여러 질의: 상한과 무관한 non-tight 부분을 길이별로 재사용하거나 행렬/조합 전처리를 검토한다.
- 밑이 10이 아닌 표기: 같은 원리는 적용되지만 `decimal_base`뿐 아니라 sentinel, 최대 자릿수,
  memo 차원, 상한의 자릿수 분해·문자 해석을 모두 그 진법에 맞춰 바꿔야 한다.

## 오늘 문제와의 연결

- 문제: [CSES 2220 — Counting Numbers](https://cses.fi/problemset/task/2220/)
- 제약: `0 <= a <= b <= 10^18`, 최대 19자리라 작은 상태 DP가 충분하다.
- 핵심 상태: `(position, previous, started, tight)`.
- 실전 포인트: `0`의 포함, 선행 0과 실제 0의 구분, `a=0`의 `F(-1)` 처리가 정답률을 가른다.
- 직접 검증: `0..10`은 11개, `11..11`은 0개, `98..102`는 `98,101,102` 세 개다. `101`처럼 떨어진 반복은 허용되고 `100`은 끝의 `00` 때문에 제외된다.
