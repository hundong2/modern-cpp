# 접두사 트라이(Prefix Trie)

## 정의

트라이는 문자열의 각 문자에 해당하는 간선을 따라 내려가며 **접두사를 공유하는 문자열을 하나의 경로로 합치는** 루트 트리다. 루트에서 어떤 노드까지의 경로가 접두사 하나를 뜻하고, `terminal` 표시는 그 노드에서 실제 문자열 하나가 끝남을 뜻한다. 오늘처럼 알파벳이 숫자 `0..9`로 작고 고정되어 있으면 노드마다 자식 인덱스 10개를 두어 다음 상태를 `O(1)`에 찾을 수 있다.

## 적용 조건

- 문자열의 정확한 존재 여부나 접두사 존재 여부를 여러 번 질의할 때 적합하다.
- 자동 완성, 사전 검색, 라우팅 경로, IP 비트 접두사, XOR 최댓값처럼 앞부분 공유가 핵심일 때 쓴다.
- 전체 문자열을 정렬한 뒤 인접 원소만 비교하는 방법도 가능하다. 한 번의 일괄 판정이면 정렬이 간단할 수 있고, 삽입·질의가 온라인으로 섞이면 트라이가 자연스럽다.
- 문자 집합이 크고 노드가 희소하면 고정 배열은 메모리를 낭비한다. `map`, `unordered_map`, 압축 간선(radix tree) 등을 검토한다.
- 단일 패턴을 텍스트에서 찾는 문제는 KMP, 여러 패턴의 모든 출현은 트라이에 실패 링크를 더한 Aho–Corasick이 더 알맞다.

## 핵심 아이디어와 불변식

삽입 중 현재 노드 `current`에 대해 다음 불변식을 유지한다.

> 루트에서 `current`까지의 경로는 지금까지 읽은 전화번호 접두사와 정확히 같고, 그 경로에 필요한 모든 노드는 트라이에 존재한다.

접두사 충돌은 두 방향으로 생긴다.

1. 다음 문자를 읽기 전에 `current.terminal == true`이면 기존의 더 짧은 문자열이 새 문자열의 접두사다.
2. 새 문자열을 모두 읽은 노드에 자식이 있으면 새 문자열이 기존의 더 긴 문자열의 접두사다.

끝 노드가 이미 `terminal`이면 중복 문자열이며 서로 접두사로 취급한다. 이 세 검사를 모두 해야 입력 순서와 무관하게 판정할 수 있다.

오늘 구현은 노드 포인터 대신 `vector<TrieNode>`의 **인덱스**를 간선에 저장한다. `push_back` 재할당이 포인터·참조·반복자를 무효화해도 정수 인덱스는 같은 논리 노드를 계속 가리킨다. 인덱스 `0`은 루트이면서 자식 배열에서는 “간선 없음” sentinel이므로 새 자식의 실제 인덱스는 항상 1 이상이다.

## 단계별 절차

1. 루트 노드 하나를 만든다. 모든 자식은 없음, `terminal=false`다.
2. 새 문자열의 첫 문자부터 현재 노드에서 해당 자식 간선을 찾는다.
3. 문자를 더 읽기 전에 현재 노드가 terminal이면 즉시 접두사 충돌을 보고한다.
4. 자식이 없으면 노드 pool 끝에 새 노드를 추가하고 부모 간선을 그 인덱스로 연결한다.
5. 자식 노드로 이동하고 모든 문자에 대해 2~4를 반복한다.
6. 끝 노드가 이미 terminal이면 중복 충돌이다.
7. 끝 노드에 자식이 하나라도 있으면 새 문자열이 기존 문자열의 접두사이므로 충돌이다.
8. 충돌이 없을 때만 끝 노드를 terminal로 표시한다.

## 의사 코드

```text
insert(word):
    current = root
    for character in word:
        if terminal[current]:
            return inconsistent
        slot = alphabet_index(character)
        if child[current][slot] is absent:
            child[current][slot] = create_node()
        current = child[current][slot]

    if terminal[current]:
        return inconsistent
    if current has any child:
        return inconsistent
    terminal[current] = true
    return consistent
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <array>   // 숫자 자식 10개를 고정 저장한다.
#include <iostream>
#include <string>
#include <vector>

struct Node {
    std::array<int, 10> child{}; // 0은 간선 없음이다.
    bool terminal{};
};

bool insert(std::vector<Node>& trie, const std::string& word) {
    std::size_t current{};
    for (const char digit : word) {
        if (trie[current].terminal) {
            return false; // 기존 짧은 문자열이 접두사다.
        }
        const std::size_t slot{static_cast<std::size_t>(digit - '0')};
        int next{trie[current].child[slot]};
        if (next == 0) {
            trie.push_back(Node{}); // 인덱스를 쓰므로 재할당에도 논리 연결이 안전하다.
            next = static_cast<int>(trie.size() - 1U);
            trie[current].child[slot] = next;
        }
        current = static_cast<std::size_t>(next);
    }
    if (trie[current].terminal) {
        return false; // 중복 문자열이다.
    }
    for (const int next : trie[current].child) {
        if (next != 0) {
            return false; // 새 문자열이 기존 긴 문자열의 접두사다.
        }
    }
    trie[current].terminal = true;
    return true;
}

int main() {
    std::vector<Node> trie{Node{}};
    const bool ok1{insert(trie, "911")};
    const bool ok2{insert(trie, "9112")};
    std::cout << ok1 << ' ' << ok2 << '\n'; // 예상: 1 0
    return ok1 && !ok2 ? 0 : 1;
}
```

## 정확성 근거

문자 반복의 불변식에 의해 매 단계 `current` 경로는 읽은 접두사와 정확히 같다. 반복 도중 terminal을 만나면 그 경로 자체가 이미 저장된 완전한 문자열이므로 기존 문자열이 새 문자열의 접두사인 것이 필요충분하다. 간선이 없을 때만 새 노드를 만들므로 이동 뒤에도 불변식이 유지된다.

모든 문자를 읽은 뒤 terminal이면 같은 문자열이 이미 있다. terminal이 아니지만 자식이 있다면 그 자식 아래 terminal까지 이어지는 기존 문자열이 있고, 현재 새 문자열은 그 문자열의 접두사다. 반대로 terminal도 아니고 자식도 없다면 기존 문자열은 현재 경로에서 끝나지도, 더 이어지지도 않는다. 반복 중 terminal도 만나지 않았으므로 어느 방향의 접두사 관계도 없다. 따라서 함수가 충돌이라고 보고할 때와 실제 접두사 충돌이 있을 때가 정확히 일치한다.

## 시간·공간 복잡도

전화번호 길이를 `L`, 모든 번호 길이 합을 `S`, 문자 집합 크기를 `A=10`이라 하자.

- 번호 하나 삽입: 경로 탐색 `O(L)` + 끝 노드 자식 확인 `O(A)`, 숫자 알파벳에서는 `O(L)`로 본다.
- 전체: 시간 `O(S)`, 공간은 최악에 문자마다 노드 하나라 `O(SA)` 정수 슬롯, `A=10` 고정이면 `O(S)`다.
- 각 간선 조회는 배열 인덱싱 `O(1)`이고, 노드 추가는 vector의 상각 `O(1)`이다.

정렬 대안은 문자열 비교 비용을 포함해 보통 `O(S log N)` 상한으로 설명하거나 비교 모델에 따라 세밀히 분석한다. 구현이 짧고 메모리가 적을 수 있어 대회에서는 두 방법 모두 알아야 한다.

## 흔한 실수

- 반복 도중 terminal만 검사하고 끝 노드의 자식을 검사하지 않아 긴 번호가 먼저 들어온 경우를 놓친다.
- 끝 노드의 자식만 검사하고 기존 짧은 번호가 먼저 들어온 경우를 놓친다.
- 중복 번호의 terminal 검사를 빼서 같은 번호 두 개를 YES로 처리한다.
- `vector` 원소의 포인터나 참조를 잡은 채 `push_back`한 뒤 재할당으로 댕글링된 관찰자를 사용한다.
- `'0'`을 빼지 않고 문자 코드 자체를 자식 배열 인덱스로 쓴다.
- 큰 알파벳에도 모든 노드에 거대한 고정 배열을 두어 메모리 제한을 넘긴다.
- 0을 “없음”으로 쓰면서 새 노드 인덱스도 0부터 저장해 루트와 sentinel을 구분하지 못한다.

## 변형

- 각 노드에 통과 문자열 수를 저장하면 접두사 빈도와 자동 완성 후보 수를 구할 수 있다.
- `0/1` 비트 트라이는 정수의 최대·최소 XOR 쌍과 온라인 XOR 질의를 푼다.
- 간선 문자열을 압축한 radix tree/Patricia trie는 한 갈래 노드 연쇄의 메모리를 줄인다.
- terminal에 패턴 ID 목록을 두고 실패 링크를 추가하면 Aho–Corasick 다중 패턴 검색이 된다.
- 삭제는 경로별 사용 횟수를 감소시키고 더는 쓰지 않는 노드를 회수하거나 free list로 재사용한다.

## 오늘 문제와의 연결

[BOJ 5052 전화번호 목록](https://www.acmicpc.net/problem/5052)은 최대 10,000개의 짧은 숫자 문자열에서 접두사 충돌을 판정한다. `icpc_problem.cpp`는 정렬 없이 입력 순서대로 삽입하며 두 방향의 terminal/자식 검사를 모두 수행한다. 모든 번호의 길이 합에 선형이라 케이스가 많아도 안정적이다.

대회에서 트라이는 KMP, Z 함수, 접미 자료구조와 함께 문자열 필수 도구다. 특히 “문자열 집합에 삽입하면서 접두사 질의”, “여러 이진수와 XOR 최적화”, “사전순 자동 완성”이라는 신호를 보면 먼저 후보로 떠올려야 한다.

## 직접 해보기와 초보자 검증

1. `911`, `97625999`, `91125426`을 차례로 넣고 각 문자 뒤 `current`, terminal, 새 노드 인덱스를 표로 적는다.
2. 입력 순서를 `12345`, `123`과 `123`, `12345` 두 가지로 바꾸어 어느 검사가 각각 충돌을 찾는지 설명한다.
3. 중복 `42`, `42`에서 끝 노드 terminal 검사를 제거했을 때 왜 오답인지 보인다.
4. 자식 배열 대신 `std::map<char,int>`를 사용하고 시간·노드당 메모리·캐시 지역성을 비교한다.
5. 모든 번호를 정렬한 뒤 인접 문자열만 검사하는 풀이를 작성하고, 인접 비교만으로 충분한 이유를 사전순으로 증명한다.
6. 위 C++ 뼈대를 높은 경고 수준으로 컴파일하고 예상 `1 0`을 확인한다.
