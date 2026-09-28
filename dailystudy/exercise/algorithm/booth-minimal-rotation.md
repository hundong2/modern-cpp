# Booth 알고리즘: 문자열의 사전순 최소 회전

## 정의

길이 `n>0`인 문자열 `s`의 `k`번째 회전을 다음처럼 정의한다.

```text
rotation(k) = s[k..n) + s[0..k),  0 <= k < n
```

최소 회전 문제는 모든 `rotation(k)` 가운데 사전순으로 가장 작은 문자열을 찾는 문제다. Booth 알고리즘의 최소 표현 변형은 두 후보를 비교하다가 첫 불일치에서 최소가 될 수 없는 시작점 구간을 통째로 제거해 `O(n)`에 답을 찾는다.

## 적용 조건

- 원형 문자열의 모든 시작 위치 가운데 사전순 최소 표현이 필요하다.
- `n`이 커서 `n`개 회전을 각각 `O(n)`에 만들거나 비교하는 `O(n^2)` 방식이 불가능하다.
- 문자 비교가 일관된 전체 순서를 이룬다.
- 최소 시작 위치 하나 또는 최소 회전 문자열만 필요하다.

회전 전체 순서나 서로 다른 회전 개수를 함께 구해야 한다면 suffix array, suffix automaton, 문자열 주기 알고리즘을 결합하는 편이 나을 수 있다.

## 핵심 아이디어와 불변식

`doubled = s + s`라 하면 시작점 `k<n`의 길이 `n` 연속 구간이 곧 `rotation(k)`다. 두 후보 `i`, `j`와 같은 접두사 길이 `matched`를 유지한다.

```text
0 <= i,j < n인 동안 비교한다.
doubled[i .. i+matched) == doubled[j .. j+matched)
```

다음 문자가 다르고 `doubled[i+matched] > doubled[j+matched]`라 하자. 후보 `i`가 지는 것은 자명하다. 더 중요한 점은 `i, i+1, ..., i+matched`가 모두 탈락한다는 것이다.

오프셋 `t` (`0 <= t <= matched`)를 잡으면 회전 `(i+t) % n`과 `(j+t) % n`은 처음 `matched-t`자가 같다. 그 다음 비교는 원래 첫 불일치의 두 문자로 이어져 `(i+t) % n` 쪽이 더 크다. `i`와 `j`는 서로 다른 `[0,n)` 후보이므로 두 경쟁 회전도 서로 다르다. 또한 실제 비교가 읽는 최대 위치는 `max(i,j)+matched <= 2n-2`라 길이 `2n`인 `doubled` 안이다. 따라서 `i`부터 `i+matched`까지 나타내는 회전 어느 것도 전역 최소가 될 수 없다. `i += matched+1`로 안전하게 한꺼번에 제거한다. 반대 부등호는 `j`에 대해 대칭이다.

## 단계별 절차

1. `doubled = s + s`를 만든다.
2. 후보를 `i=0`, `j=1`, 공통 길이를 `matched=0`으로 둔다.
3. 두 현재 문자가 같으면 `matched`를 1 늘린다.
4. 다르면 더 큰 문자를 가진 후보를 `matched+1`만큼 전진시킨다.
5. 전진한 후보가 다른 후보와 같아지면 한 칸 더 전진시켜 두 후보를 다르게 유지한다.
6. `matched=0`으로 재설정하고 새 후보 비교를 시작한다.
7. 한 후보가 `n` 이상이 되거나 `matched==n`이 되면 `min(i,j)`가 최소 회전 시작점이다.

`matched==n`은 두 길이 `n` 회전이 완전히 같은 주기 문자열이라는 뜻이다. 더 작은 시작 인덱스를 골라도 출력 문자열은 같다.

## 의사코드

```text
booth_min_index(s):
    n = length(s)
    doubled = s + s
    i = 0
    j = 1
    matched = 0

    while i < n and j < n and matched < n:
        left = doubled[i + matched]
        right = doubled[j + matched]

        if left == right:
            matched += 1
        else if left > right:
            i += matched + 1
            if i == j: i += 1
            matched = 0
        else:
            j += matched + 1
            if i == j: j += 1
            matched = 0

    return min(i, j)
```

## C++ 뼈대

```cpp
#include <cstddef>
#include <iostream>
#include <string>

std::size_t booth_index(const std::string& doubled, std::size_t n) noexcept {
    std::size_t first{};
    std::size_t second{1};
    std::size_t matched{};

    while (first < n && second < n && matched < n) {
        const char left{doubled[first + matched]};
        const char right{doubled[second + matched]};
        if (left == right) {
            ++matched;
            continue;
        }
        if (left > right) {
            first += matched + 1;
            if (first == second) {
                ++first;
            }
        } else {
            second += matched + 1;
            if (first == second) {
                ++second;
            }
        }
        matched = 0;
    }
    return first < second ? first : second;
}

int main() {
    std::string text{};
    if (!(std::cin >> text)) {
        return 0;
    }
    const std::size_t n{text.size()};
    std::string doubled{text};
    doubled += text;
    const std::size_t start{booth_index(doubled, n)};
    std::cout << doubled.substr(start, n) << '\n';
}
```

## 정확성 근거

### 보조정리 1: 점프 구간의 모든 후보는 탈락한다

두 후보가 `matched`자까지 같고 다음 문자에서 `i` 쪽이 더 크다고 하자. 앞 절의 `t`별 비교로 `(i+t) % n` (`0<=t<=matched`)마다 서로 다른 더 작은 경쟁 회전 `(j+t) % n`이 존재한다. `doubled`에서 필요한 최대 접근도 `2n-2` 이하다. 따라서 그 구간이 나타내는 회전에는 최소 회전 시작점이 없다. `j`가 더 큰 경우도 대칭이다.

### 보조정리 2: 최소 후보는 절대 제거되지 않는다

알고리즘은 보조정리 1로 “명시적인 더 작은 경쟁 회전”이 있는 후보만 제거한다. 전역 최소 회전에는 그보다 작은 경쟁 회전이 존재할 수 없으므로 어느 점프에서도 제거되지 않는다.

### 보조정리 3: 종료 때 남은 작은 후보가 최소다

각 불일치에서 적어도 한 후보 구간이 제거되고 포인터는 증가만 한다. 한 포인터가 `n`에 도달하면 그 포인터 쪽의 모든 시작점은 제거되었고, 다른 생존 후보가 최소다. `matched==n`이면 두 생존 회전이 같아 둘 다 최소가 될 수 있으므로 작은 인덱스를 골라도 최소 문자열을 얻는다.

### 정리

최소 후보는 보조정리 2로 항상 살아 있고 알고리즘은 유한하게 종료한다. 종료 시 보조정리 3에 따라 반환 위치의 회전이 전역 사전순 최소다.

## 시간·공간 복잡도

- 두 배 문자열 생성: `O(n)` 시간과 `O(n)` 추가 공간
- 후보 비교: 상각 `O(n)` 시간. 한 불일치 구간의 `matched`번 같은 문자 비교와 마지막 1번 불일치 비교, 즉 `matched+1`번을 그때 패배 포인터가 최소 `matched+1` 증가한 양에 부과한다. 두 포인터는 뒤로 가지 않고 종료 전 총 증가량이 각각 `O(n)`이며, 불일치 없이 `matched==n`까지 간 마지막 구간도 최대 `n`번 비교하므로 전체 비교가 `O(n)`이다.
- 정답 `substr` 생성: `O(n)` 시간과 `O(n)` 결과 공간
- 전체: `O(n)` 시간, 제출 구현의 추가 공간 `O(n)`

문자 접근에 매번 modulo를 써 두 배 문자열을 없애면 알고리즘 상태 자체는 `O(1)` 추가 공간으로 만들 수 있다. 다만 정답 문자열을 소유해 반환하면 결과 `O(n)` 공간은 여전히 필요하며, 연속 접근 구현이 더 단순하고 빠를 수 있다.

## 흔한 실수

1. 패배 후보를 한 칸만 움직여 반복 접두사에서 `O(n^2)`가 된다.
2. `matched+1`이 아니라 `matched`만 더해 같은 불일치를 다시 비교한다.
3. 갱신 뒤 `i==j`를 처리하지 않아 같은 회전을 자기 자신과 비교한다.
4. 불일치 뒤 `matched=0`을 빼먹는다.
5. `matched<n` 종료 조건을 빼 모두 같은 문자나 주기 문자열에서 범위를 넘는다.
6. 후보 포인터 자체에 `% n`을 적용해 이미 제거한 위치를 다시 살린다.
7. `doubled` 길이는 `2n`인데 `i+matched`의 상계를 증명하지 않고 `operator[]`를 쓴다.
8. 최소 문자를 찾는 것만으로 답을 고른다. 같은 최소 문자가 여러 번 나오면 뒤 접미 비교가 필요하다.
9. locale 기반 대소문자 규칙과 byte 사전순 비교를 혼동한다. 문제의 알파벳·비교 규칙을 확인한다.

## 변형

- **최대 회전**: 문자 비교 방향을 뒤집어 같은 점프 구조를 쓴다.
- **숫자 배열 최소 회전**: 문자 대신 전체 순서가 있는 원소 배열에 그대로 적용한다.
- **동치 시작점 모두 찾기**: 최소 표현을 얻은 뒤 문자열의 최소 주기를 구해 같은 회전을 만드는 시작점을 열거한다.
- **메모리 절약**: `doubled[position]` 대신 `s[position % n]`을 사용하되 후보 포인터에는 modulo를 적용하지 않는다.
- **두 원형 문자열 동치**: 길이가 같을 때 한 문자열이 다른 문자열의 두 배 안에 나타나는지 KMP 등으로 검사한다. 최소 회전 문제와 목적이 다르다.

## 대회 최적화와 실행 관점

두 배 문자열은 연속 메모리라 비교마다 인덱스 주소 계산, 문자 load, 비교, 조건 분기가 일어난다. modulo 버전은 저장을 줄이지만 나눗셈 또는 컴파일러가 선택한 나머지 계산이 추가될 수 있다. 반복 문자가 많은 입력에서 분기 패턴과 cache 효과는 달라질 수 있다. 어떤 버전이 빠른지는 CPU·컴파일러·표준 라이브러리·최적화 옵션에 따라 달라지며 특정 어셈블리나 분기 예측 결과를 단정하지 않는다.

우승권 구현에서는 다음을 별도로 점검한다.

- `n=1`, 모두 같은 문자, 짧은 주기, 최소 문자가 끝에 하나뿐인 입력
- 길이 `10^6`에서 quadratic 동작이나 재귀·과도한 회전 복사가 없는지
- 인덱스 합이 `2n-2` 이하임을 타입 범위와 함께 확인
- brute-force 모든 회전 오라클로 작은 전수·무작위 차등 테스트

## 오늘 문제와의 연결

- 문제: [CSES 1110 — Minimal Rotation](https://cses.fi/problemset/task/1110/)
- 입력은 길이 최대 `1,000,000`의 소문자 문자열 하나다.
- 모든 회전을 생성하지 않고 두 후보와 공통 접두사 길이만 갱신한다.
- 제출 구현은 `s+s`를 한 번만 만들고 최소 위치를 찾은 뒤 길이 `n` `substr`을 반환한다.
- 시간 `O(n)`, 추가 공간 `O(n)`으로 공식 제한에 맞는다.
- 구현 파일: [`../2026-09-29/icpc_problem.cpp`](../2026-09-29/icpc_problem.cpp)
