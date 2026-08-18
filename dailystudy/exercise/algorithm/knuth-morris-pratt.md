# Knuth–Morris–Pratt 문자열 검색(KMP)

## 정의

KMP는 텍스트 `T`에서 패턴 `P`가 시작하는 모든 위치를 `O(|T|+|P|)`에 찾는 문자열 검색 알고리즘이다. 불일치가 생겼을 때 이미 같다고 확인한 문자 정보를 버리지 않고, 패턴의 접두사이면서 현재 접미사인 가장 긴 문자열로 이동한다. 이 이동 정보를 담는 배열을 접두 함수, 실패 함수, `pi` 배열 등으로 부른다.

`prefix[i]`는 `P[0..i]`의 **접두사이면서 접미사인 가장 긴 진부분 문자열의 길이**다. 진부분이어야 전체 문자열 자기 자신을 답으로 세어 무한히 같은 상태로 이동하는 일을 피한다.

## 적용 조건

- 하나의 긴 텍스트에서 패턴 하나를 선형 시간에 찾을 때 적합하다.
- 같은 패턴으로 여러 텍스트를 검색하면 접두 함수 전처리를 재사용할 수 있다.
- 일치가 겹칠 수 있고 모든 시작 위치가 필요할 때도 그대로 쓸 수 있다.
- 단순 비교가 `O(|T||P|)`까지 커질 수 있는 반복 문자열 입력에서 특히 중요하다.
- 패턴이 매우 많으면 Aho–Corasick, 접미 배열·접미 자동자·해시 등이 더 맞을 수 있다.

## 핵심 아이디어와 불변식

전처리 중 `matched`는 현재 `P[0..i-1]`의 접두사이자 접미사인 후보 길이다. 검색 중에는 다음 불변식을 유지한다.

> 텍스트에서 방금 끝난 길이 `matched`의 접미사는 패턴 `P[0..matched-1]`와 같다.

다음 문자가 다르면 지금까지 맞은 접두사 안에서 다시 후보가 될 수 있는 가장 긴 경계 길이 `prefix[matched-1]`로 간다. 그보다 긴 후보는 이미 불일치를 포함하므로 답이 될 수 없다. 텍스트 인덱스는 뒤로 가지 않는다.

완전 일치 뒤에도 `matched = prefix[matched-1]`로 이동한다. 완전 일치한 패턴의 접미사와 접두사가 겹칠 수 있기 때문이다. 예를 들어 `AAAAA`에서 `AAA`는 1, 2, 3 위치에 모두 나타난다.

## 단계별 절차

1. 패턴 길이만큼 0인 `prefix` 배열을 만든다.
2. 패턴의 두 번째 문자부터 보며 `matched`와 현재 문자를 비교한다.
3. 다르면 `matched`가 0이 될 때까지 또는 문자가 같아질 때까지 `prefix[matched-1]`로 후퇴한다.
4. 같으면 `matched`를 1 늘리고 `prefix[i]`에 기록한다.
5. 텍스트를 왼쪽에서 오른쪽으로 한 번 훑으며 같은 실패 이동 규칙을 적용한다.
6. `matched == |P|`이면 시작 위치를 기록하고 `prefix[matched-1]`로 이동해 겹치는 다음 일치를 준비한다.

## 의사 코드

```text
prefix[0..m-1] = 0
matched = 0
for i = 1 .. m-1:
    while matched > 0 and P[i] != P[matched]:
        matched = prefix[matched-1]
    if P[i] == P[matched]:
        matched += 1
    prefix[i] = matched

matched = 0
for i = 0 .. n-1:
    while matched > 0 and T[i] != P[matched]:
        matched = prefix[matched-1]
    if T[i] == P[matched]:
        matched += 1
    if matched == m:
        report i-m+1
        matched = prefix[matched-1]
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <iostream> // std::cout으로 검증 결과를 출력한다.
#include <string>   // std::string이 텍스트와 패턴 문자를 소유한다.
#include <vector>   // std::vector<int>가 접두 함수와 위치를 소유한다.

// const string&는 패턴을 복사하지 않고 함수 호출 동안 읽기만 한다.
[[nodiscard]] std::vector<int> make_prefix(const std::string& pattern) {
    // pattern.size()개 정수를 0으로 초기화한다. 할당 공간과 초기화 시간은 O(|P|)이다.
    std::vector<int> prefix(pattern.size(), 0);
    int matched{}; // 지금까지 유지한 가장 긴 경계 길이를 0으로 초기화한다.

    // 첫 문자의 진부분 접두사·접미사는 없으므로 index 1부터 계산한다.
    for (int index{1}; index < static_cast<int>(pattern.size()); ++index) {
        // 불일치할 때 가능한 다음으로 긴 경계로만 이동한다.
        while (matched > 0 && pattern[index] != pattern[matched]) {
            matched = prefix[matched - 1];
        }
        if (pattern[index] == pattern[matched]) {
            ++matched; // 현재 문자까지 같으므로 경계가 한 글자 길어진다.
        }
        prefix[index] = matched;
    }
    return prefix; // vector 소유권을 값으로 반환한다.
}

[[nodiscard]] std::vector<int> kmp(const std::string& text, const std::string& pattern) {
    std::vector<int> result{};
    if (pattern.empty()) { // 이 뼈대에서는 빈 패턴의 일치 정의를 빈 결과로 정한다.
        return result;
    }

    const std::vector<int> prefix{make_prefix(pattern)};
    int matched{};
    for (int index{}; index < static_cast<int>(text.size()); ++index) {
        while (matched > 0 && text[index] != pattern[matched]) {
            matched = prefix[matched - 1];
        }
        if (text[index] == pattern[matched]) {
            ++matched;
        }
        if (matched == static_cast<int>(pattern.size())) {
            // 0-based 시작 위치를 저장한다.
            result.push_back(index - matched + 1);
            // 겹치는 일치를 위해 완전 일치 문자열의 가장 긴 경계로 돌아간다.
            matched = prefix[matched - 1];
        }
    }
    return result;
}

int main() {
    const std::vector<int> found{kmp("AAAAA", "AAA")};
    for (const int position : found) {
        std::cout << position << ' '; // 예상: 0 1 2
    }
    std::cout << '\n';
    return found.size() == 3 ? 0 : 1;
}
```

## 정확성 근거

### 접두 함수의 정확성

`i` 이전까지 `matched`가 가능한 가장 긴 경계 길이라고 가정한다. `P[i]`와 `P[matched]`가 같으면 그 경계 양 끝에 같은 문자를 붙인 길이 `matched+1`이 유효하고, 기존 후보가 가장 길었으므로 새 후보도 가장 길다. 다르면 현재 후보는 불가능하다. 가능한 다음 후보는 현재 경계 문자열 자체의 접두사이자 접미사여야 하므로 `prefix[matched-1]`과 그 연쇄 안에 모두 들어 있다. 반복이 끝나 같은 문자를 붙이거나 0을 기록하므로 귀납적으로 모든 `prefix[i]`가 정확하다.

### 검색의 정확성

검색 시작마다 직전 텍스트 접미사 길이 `matched`가 패턴 접두사와 같다는 불변식을 둔다. 다음 문자가 다르면 접두 함수가 열거하는 후보만이 이미 본 문자와 모순되지 않는다. 같으면 일치 길이를 늘린다. 길이가 패턴 전체와 같아지는 바로 그때 끝 위치가 `i`인 일치가 존재하며 시작 위치는 `i-|P|+1`이다. 더 긴 후보를 건너뛰지 않으므로 모든 일치를 찾고, 전체 길이가 같을 때만 보고하므로 거짓 일치를 출력하지 않는다.

## 시간·공간 복잡도

- 접두 함수: 시간 `O(|P|)`, 공간 `O(|P|)`
- 검색: 시간 `O(|T|)`, 접두 함수 외 작업 공간 `O(1)`
- 전체: 시간 `O(|T|+|P|)`, 작업 공간 `O(|P|)`
- 모든 위치를 vector에 저장하면 일치 수 `K`만큼 추가 `O(K)` 공간이 든다.

선형 시간인 이유는 `index`가 항상 앞으로 한 칸씩 가고, `matched`가 늘어난 총량보다 더 많이 감소할 수 없기 때문이다. 각 while이 중첩되어 보여도 전체 후퇴 횟수는 선형이다.

## 흔한 실수

- 불일치 때 `matched=0`으로만 만들어 반복 패턴에서 불필요한 비교를 다시 한다.
- `prefix[matched]`로 이동해 상태가 줄지 않거나 범위를 벗어난다. 올바른 식은 `prefix[matched-1]`이다.
- 완전 일치 뒤 0으로 초기화해 겹치는 일치를 놓친다.
- 0-based 내부 위치와 문제의 1-based 출력을 변환하면서 1만큼 틀린다.
- 공백이 있는 텍스트를 `operator>>`로 읽어 첫 단어만 검색한다.
- 빈 패턴 정책을 정하지 않고 `pattern[0]`에 접근한다.
- `int`와 `size_t` 비교의 부호 변환 경고를 무시한다. 문제 범위를 확인하고 변환 위치를 명시한다.

## 변형

- 접두 함수 마지막 값으로 문자열의 경계와 최소 주기를 구한다.
- 패턴을 구분 문자와 함께 `P + '#' + T`로 이어 접두 함수만으로 일치 위치를 찾을 수 있다.
- 온라인 스트림에서 상태 `matched`만 유지하면 텍스트 전체를 저장하지 않고도 검색할 수 있다.
- 여러 패턴을 동시에 찾을 때는 접두사의 트라이와 실패 링크를 합친 Aho–Corasick으로 확장한다.

## 오늘 문제와의 연결

[BOJ 1786 찾기](https://www.acmicpc.net/problem/1786)는 최대 백만 글자의 텍스트와 패턴을 다룬다. `icpc_problem.cpp`는 줄 전체 입력으로 공백을 보존하고 접두 함수를 한 번 만든 뒤 텍스트를 한 번 순회한다. 발견 즉시 1-based 시작 위치를 vector에 넣고, 완전 일치 뒤 실패 이동으로 겹치는 답까지 보존한다.

## 직접 해보기와 초보자 검증

1. `P=ABABAC`의 `prefix` 배열을 손으로 계산하고 각 원소가 나타내는 접두사·접미사를 적는다.
2. `T=AAAAA`, `P=AAA`에서 매 문자 뒤 `matched`와 보고 위치를 표로 만든다.
3. while 안의 대입을 `matched=0`으로 바꾸고 비교 횟수가 커지는 입력을 만든다.
4. 결과를 저장하지 않고 발견 즉시 함수 객체에 전달하는 버전을 작성해 추가 공간을 `O(|P|)`로 만든다.
5. `abababab`의 최소 주기를 `n-prefix[n-1]` 후보와 나머지 검사로 구하고 정답 2를 확인한다.
