# Suffix Automaton으로 서로 다른 부분 문자열 세기

## 정의

Suffix Automaton(SAM)은 한 문자열의 모든 부분 문자열을 인식하는 결정적 유한 오토마톤 중 상태 수가 선형인 구조다. 각 상태는 부분 문자열 하나가 아니라, 원문에서 끝나는 위치 집합이 같은 문자열들의 **endpos 동치류**를 대표한다.

문자열 `x`의 `endpos(x)`를 원문에서 `x`가 끝나는 모든 1-based 위치의 집합이라 하자. `endpos(x)=endpos(y)`인 부분 문자열 `x,y`는 같은 SAM 상태에 대응한다.

## 적용 조건

- 한 문자열의 서로 다른 부분 문자열 개수나 길이 합을 선형 시간에 구하고 싶다.
- 문자열을 왼쪽부터 한 문자씩 온라인으로 추가하며 부분 문자열 정보를 유지하고 싶다.
- 반복 등장 횟수, 최장 반복 부분 문자열, 다른 문자열과의 공통 부분 문자열로 확장하고 싶다.
- 알파벳 크기와 메모리 제한에 맞는 전이 표현을 선택할 수 있다.

단순히 한 번의 패턴 검색만 필요하면 KMP/Z가 더 작고 쉽다. 접미사 사전순 전체 순서나 LCP 질의에는 suffix array/tree가 더 직접적일 수 있다.

## 상태가 저장하는 값

상태 `v`마다 다음을 둔다.

- `longest[v]`: 이 상태가 대표하는 문자열 중 가장 긴 길이
- `link[v]`: 같은 상태의 문자열에서 한 문자 이상 제거해 얻는 suffix 가운데 가장 긴, 다른 endpos 동치류의 상태
- `next[v][c]`: 대표 문자열 뒤에 문자 `c`를 붙였을 때 도달하는 상태

root는 빈 문자열을 대표하며 `longest[root]=0`이다. 구현에서는 `0`을 전이 부재 sentinel, root를 `1`로 두면 zero-initialized 배열을 그대로 쓸 수 있다.

## 핵심 아이디어와 불변식

### 불변식 1: 상태별 길이는 연속 구간이다

root가 아닌 상태 `v`가 대표하는 서로 다른 부분 문자열 길이는 정확히 다음 범위다.

```text
longest[link[v]] + 1, ..., longest[v]
```

같은 endpos를 가진 문자열은 가장 긴 대표 문자열의 suffix들이다. suffix link는 다른 endpos 집합으로 넘어가는 가장 긴 proper suffix이므로 그 길이 이하는 부모 동치류가 담당하고, 그보다 긴 suffix들은 `v`가 담당한다.

### 불변식 2: 마지막 상태는 현재 전체 문자열을 대표한다

문자를 하나 추가할 때 새 상태 `current`의 `longest`는 이전 마지막 상태보다 1 크다. 새 전체 문자열과 그 suffix들을 반영한 뒤 `last=current`로 갱신한다.

### 불변식 3: 전이는 결정적이다

상태와 문자 쌍마다 다음 상태는 최대 하나다. 기존 전이의 길이 조건이 맞지 않으면 전이를 두 개로 만들지 않고 **clone 상태**로 endpos 동치류를 분리한다.

### 불변식 4: suffix link 길이는 엄격히 감소한다

root가 아닌 모든 상태에서 다음이 성립한다.

```text
longest[link[v]] < longest[v]
```

따라서 suffix link를 따라가면 반드시 root를 지나 sentinel에서 끝난다.

## 한 문자 확장 절차

이전 마지막 상태를 `last`, 추가 문자를 `c`라 하자.

1. `current`를 만들고 `longest[current]=longest[last]+1`로 둔다.
2. `p=last`에서 시작해 `c` 전이가 없는 동안 `next[p][c]=current`를 쓰고 `p=link[p]`로 간다.
3. `p`가 sentinel에 도달하면 `link[current]=root`다.
4. 기존 전이 `q=next[p][c]`를 만났고 `longest[p]+1==longest[q]`이면 `link[current]=q`다.
5. 위 길이 등식이 깨지면 clone을 만든다.
   - clone은 `q`의 모든 전이와 suffix link를 복사한다.
   - `longest[clone]=longest[p]+1`로 줄인다.
   - suffix-link 조상에서 문자 `c` 전이가 정확히 `q`인 동안 clone으로 돌린다.
   - `link[q]=link[current]=clone`으로 바꾼다.
6. `last=current`로 갱신한다.

## clone이 필요한 이유

기존 `q`가 `p+c`보다 더 긴 문자열도 함께 대표하면 `q`를 current의 suffix link로 바로 연결할 때 길이가 한 단계 이어지지 않는다. 그렇다고 `longest[q]`를 줄이면 이미 `q`가 대표하던 긴 경로 정보가 깨진다.

clone은 `q`와 같은 미래 전이·suffix 행동을 복사해 endpos 역할을 공유하되, 더 짧은 길이 경계를 담당한다. 이후 `q`는 원래 긴 경로를, clone은 새로 분리된 짧은 길이 구간을 담당한다. clone 자체는 새 문자를 끝에 붙여 만든 occurrence가 아니므로 “새 terminal 상태”로 세지 않는다.

## 단계별 의사코드

```text
initialize root = 1, absent = 0
state_count = last = root

extend(c):
    current = ++state_count
    longest[current] = longest[last] + 1
    p = last

    while p != absent and next[p][c] == absent:
        next[p][c] = current
        p = link[p]

    if p == absent:
        link[current] = root
    else:
        q = next[p][c]
        if longest[p] + 1 == longest[q]:
            link[current] = q
        else:
            clone = ++state_count
            state[clone] = state[q]
            longest[clone] = longest[p] + 1

            while p != absent and next[p][c] == q:
                next[p][c] = clone
                p = link[p]

            link[q] = clone
            link[current] = clone

    last = current
```

## 서로 다른 부분 문자열 개수

root를 제외한 상태 `v`가 대표하는 새 길이 개수는 다음과 같다.

```text
contribution(v) = longest[v] - longest[link[v]]
```

따라서 정답은 다음 합이다.

```text
answer = sum(contribution(v)) for every non-root state v
```

길이가 같아도 내용이 다른 부분 문자열은 서로 다른 경로/상태 기여에 속하고, 내용이 같은 부분 문자열의 여러 occurrence는 같은 경로와 endpos 동치류로 합쳐진다.

## 컴파일 가능한 C++ 뼈대

```cpp
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

constexpr std::size_t sigma{26U};
constexpr int absent{0};
constexpr int root{1};

struct State {
    int longest{};
    int link{};
    std::array<int, sigma> next{};
};

class SuffixAutomaton {
public:
    explicit SuffixAutomaton(std::size_t length)
        : states_(length * 2U + 1U) {}

    void extend(char character) noexcept {
        const std::size_t symbol{static_cast<std::size_t>(character - 'a')};
        const int current{++count_};
        states_[current].longest = states_[last_].longest + 1;

        int cursor{last_};
        while (cursor != absent && states_[cursor].next[symbol] == absent) {
            states_[cursor].next[symbol] = current;
            cursor = states_[cursor].link;
        }

        if (cursor == absent) {
            states_[current].link = root;
        } else {
            const int target{states_[cursor].next[symbol]};
            if (states_[cursor].longest + 1 == states_[target].longest) {
                states_[current].link = target;
            } else {
                const int clone{++count_};
                states_[clone] = states_[target];
                states_[clone].longest = states_[cursor].longest + 1;
                while (cursor != absent && states_[cursor].next[symbol] == target) {
                    states_[cursor].next[symbol] = clone;
                    cursor = states_[cursor].link;
                }
                states_[target].link = clone;
                states_[current].link = clone;
            }
        }
        last_ = current;
    }

    std::int64_t distinct_count() const noexcept {
        std::int64_t answer{};
        for (int state{root + 1}; state <= count_; ++state) {
            answer += states_[state].longest - states_[states_[state].link].longest;
        }
        return answer;
    }

private:
    std::vector<State> states_;
    int count_{root};
    int last_{root};
};

int main() {
    std::string text{};
    if (!(std::cin >> text)) {
        return 0;
    }
    SuffixAutomaton automaton{text.size()};
    for (char character : text) {
        automaton.extend(character);
    }
    std::cout << automaton.distinct_count() << '\n';
}
```

## 정확성 근거

### 보조정리 1: 확장 뒤 모든 새 부분 문자열이 인식된다

새 문자 `c`로 끝나는 새 부분 문자열은 이전 전체 문자열의 어떤 suffix 뒤에 `c`를 붙인 형태다. 알고리즘은 `last`에서 suffix link를 따라가며 `c` 전이가 없던 모든 상태에 `current` 전이를 추가한다. 기존 전이를 처음 만나는 지점 이후는 이미 같은 문자열을 인식하므로 더 추가할 필요가 없다.

### 보조정리 2: clone은 언어를 바꾸지 않고 동치류만 분리한다

clone은 `q`의 전이와 suffix link를 그대로 복사하므로 clone에서 이어 읽을 수 있는 문자열 집합은 `q`와 같다. `p` 계열에서 `c`로 `q`에 들어가던 경로 중 짧은 길이 경계만 clone으로 돌리고, `q/current`의 suffix link를 clone으로 연결한다. 인식 가능한 경로를 제거하거나 새 문자 경로를 발명하지 않으면서 `longest[link[v]]<longest[v]`를 회복한다.

### 보조정리 3: 상태 기여 길이 구간은 서로 다른 부분 문자열을 정확히 한 번 센다

상태 `v`의 같은 endpos 동치류에 속하는 문자열은 가장 긴 대표 문자열의 suffix이며 길이가 연속이다. `link[v]`가 담당하는 최대 길이 이하는 부모 동치류이고, 그 다음 길이부터 `longest[v]`까지는 `v`의 동치류다. 서로 다른 동치류는 같은 부분 문자열을 동시에 가질 수 없으므로 구간별 기여가 중복되지 않는다.

### 정리

보조정리 1과 2로 완성된 SAM은 원문의 모든 부분 문자열을 정확히 인식한다. 보조정리 3으로 각 non-root 상태의 기여 `longest[v]-longest[link[v]]`를 합하면 모든 서로 다른 부분 문자열을 정확히 한 번 센다.

## 시간·공간 복잡도

알파벳 크기를 `A`라 하자.

- 문자마다 current 상태는 정확히 하나 생기고 한 확장에서 clone은 최대 하나만 생긴다. 따라서 root까지
  포함해 모든 `n>=1`에서 안전한 상한은 `2n`이다. 첫 문자에서는 clone이 생기지 않고 `n>=2`에서는
  clone이 최대 `n-2`개라 더 타이트한 상한 `1+n+(n-2)=2n-1`을 얻는다.
- 첫 suffix-link loop는 없던 전이를 만들고, redirect loop는 기존 target 전이를 더 짧은 clone 동치류로
  옮긴다. 표준 온라인 SAM의 잠재량 분석에서 전체 확장 동안 새 전이 생성과 redirect, suffix-link 이동의
  총수가 상태·전이 수에 선형으로 묶이므로 고정 알파벳에서는 상각 `O(n)`이다.
- 고정 배열 전이 복사는 clone마다 `O(A)`다.
- 전체 시간: `O(nA)`, 소문자 고정 `A=26`에서는 `O(n)`.
- 전체 공간: 상태마다 `A`개 전이를 두므로 `O(nA)`, 고정 알파벳에서는 `O(n)`.
- 최종 합산: `O(n)`.

희소하거나 매우 큰 알파벳에서 map/hash 전이를 쓰면 상태당 빈 슬롯을 줄이지만 조회·할당·상수 계수와 최악 시간 계약이 달라진다.

## 흔한 실수

1. 답을 `int`에 누적해 `n=100000`에서 오버플로한다.
2. `n=1` 예외와 1-based sentinel 슬롯을 무시하고 상태 배열을 빡빡하게 잡아 clone에서 범위를 넘는다. 오늘처럼 `2n+1` 슬롯을 잡으면 안전하다.
3. clone의 전이 또는 suffix link를 복사하지 않는다.
4. clone의 `longest`를 `longest[p]+1`로 줄이지 않는다.
5. `link[q]`만 또는 `link[current]`만 clone으로 바꾼다.
6. redirect 조건에서 `next[p][c]==q`를 확인하지 않고 모든 조상 전이를 바꾼다.
7. root와 absent sentinel을 같은 인덱스로 쓰면서 종료 조건을 섞는다.
8. 입력이 소문자라는 전제 없이 `character-'a'`를 인덱스로 쓴다.
9. vector 성장 중 State 참조를 오래 저장해 재할당 뒤 dangling시킨다.
10. 상태 수가 아니라 부분 문자열 occurrence 수를 세어 같은 내용을 중복한다.
11. clone을 새 occurrence 상태처럼 terminal count에 더한다.
12. `longest[v]-longest[link[v]]`의 64비트 변환 시점을 총합 뒤로 미룬다.

## 변형

- **서로 다른 부분 문자열 길이 합**: 상태별 등차수열 합을 더한다.
- **등장 횟수**: 각 current 상태에 1을 두고 `longest` 내림차순으로 suffix link에 누적한다. clone 초기 occurrence는 0이다.
- **최장 반복 부분 문자열**: occurrence가 2 이상인 상태의 최대 `longest`를 찾는다.
- **다른 문자열과의 최장 공통 부분 문자열**: 두 번째 문자열을 SAM 위에서 걸으며 현재 일치 길이를 유지한다.
- **k번째 사전순 부분 문자열**: 상태별 경로 개수를 DP하고 전이를 문자 순서로 선택한다.
- **여러 문자열**: 문자열 경계를 섞지 않도록 last 초기화/구분자/endpos 집계를 목적에 맞게 설계한다.

## 대회 최적화와 실행 관점

- `std::array<int,26>`은 상태별 연속 전이라 조회가 단순하고 예측 가능한 대신 상태당 104바이트 안팎의 전이 공간을 쓴다. 실제 padding과 객체 크기는 구현에 따라 확인한다.
- `2n+1` vector를 한 번에 만들면 확장 중 할당과 재할당 무효화가 없다. 대신 모든 전이를 처음 0으로 쓰는 비용이 선지불된다.
- suffix-link while에는 문자 분포에 따른 조건 분기와 비연속 상태 접근이 있다. cache와 분기 예측 결과는 입력, CPU, 배치, 컴파일러, 최적화 옵션에 따라 달라진다.
- map 전이는 공간을 줄일 수 있지만 포인터 추적·할당·비교 비용이 늘 수 있다. 측정 없이 한 구현이 항상 빠르다고 단정하지 않는다.
- 작은 `n`에서는 모든 부분 문자열 set 오라클, 최대 `n`에서는 모두 같은 문자와 무작위 문자를 별도로 검증한다.

## 오늘 문제와의 연결

- 문제: [CSES 2105 — Distinct Substrings](https://cses.fi/problemset/task/2105/)
- 입력은 길이 최대 100,000의 소문자 문자열이다.
- 고정 26 전이 배열과 안전 상한 `2n` 상태가 512MB 제한 안에 들어간다.
- 답의 최댓값은 약 50억이라 64비트 누적이 필요하다.
- 구현 파일: [`../2026-10-14/icpc_problem.cpp`](../2026-10-14/icpc_problem.cpp)
