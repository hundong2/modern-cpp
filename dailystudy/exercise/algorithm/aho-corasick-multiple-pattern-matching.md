# Aho–Corasick 다중 패턴 문자열 검색

## 정의

Aho–Corasick은 여러 패턴의 trie에 **실패 링크(failure link)** 를 더해 text를 한 번만 읽으며 모든 패턴을 찾는 문자열 automaton이다. KMP가 한 패턴의 접두사/접미사 정보를 쓰는 것처럼, Aho–Corasick은 trie의 모든 패턴 접두사 사이에서 가장 긴 가능한 suffix로 이동한다.

## 적용 조건

- 같은 text에서 패턴 수가 많아 패턴별 KMP/`find` 반복이 비싼 경우에 적합하다.
- 전체 패턴 길이만큼 전처리 메모리를 사용할 수 있어야 한다.
- 알파벳이 작고 고정이면 상태마다 완성 전이 배열을 두는 방식이 빠르다. 유니코드·큰 알파벳이면 map/hash/sparse edge의 공간 trade-off를 검토한다.
- 패턴 추가가 끝난 뒤 실패 링크를 구축하고 검색하는 정적 집합에 가장 단순하다. 동적 삽입/삭제는 automaton 재구축이나 별도 기법이 필요하다.

## 핵심 아이디어와 불변식

1. **trie 경로 불변식**: 루트에서 상태 `v`까지 간선 문자를 이은 문자열은 어떤 패턴의 접두사다.
2. **실패 링크 정의**: `fail[v]`는 `v` 문자열의 proper suffix 중 trie에 존재하는 가장 긴 문자열 상태다. 없으면 루트다.
3. **BFS 불변식**: 상태 `v`를 처리할 때 부모보다 얕은 모든 상태의 실패 링크와 완성 전이가 이미 계산되어 있다.
4. **검색 불변식**: text 접두사를 읽은 현재 상태는 그 접두사의 suffix 중 trie에 존재하는 가장 긴 문자열이다.
5. **역전파 불변식**: BFS 역순에서 `v`를 처리할 때 모든 더 깊은 실패-link 자손의 방문 정보가 이미 `v`에 모여 있다.

실패 링크는 “문자가 맞지 않을 때 버릴 수 있는 가장 긴 앞부분”이다. 완성 전이 `go[v][c]`를 미리 채우면 검색은 문자마다 배열 조회 한 번으로 다음 상태를 결정한다.

## 단계별 절차

1. 모든 패턴을 trie에 삽입하고 각 패턴의 terminal 상태를 입력 순서대로 저장한다.
2. 루트의 실제 자식을 BFS queue에 넣고 실패 링크를 루트로 둔다. 없는 루트 전이는 루트 자신으로 둔다.
3. BFS에서 상태 `v`와 문자 `c`를 본다.
   - 실제 자식 `u`가 있으면 `fail[u] = go[fail[v]][c]`로 두고 queue에 넣는다.
   - 실제 자식이 없으면 `go[v][c] = go[fail[v]][c]`로 채운다.
4. text를 한 번 훑으며 `state = go[state][c]`로 이동하고 도착 상태를 방문 표시한다.
5. BFS 순서를 거꾸로 훑으며 `visited[fail[v]] |= visited[v]`로 suffix 패턴 일치를 전파한다.
6. 각 입력 패턴의 terminal 상태 방문 여부를 출력한다.

## 의사 코드

```text
nodes = [root]
for pattern in patterns:
    state = root
    for character in pattern:
        create missing trie edge
        state = edge(state, character)
    terminal[pattern] = state

queue = root children
while queue not empty:
    v = pop queue
    order.push(v)
    for each character c:
        if real child u exists:
            fail[u] = go[fail[v]][c]
            push u
        else:
            go[v][c] = go[fail[v]][c]

state = root
for c in text:
    state = go[state][c]
    visited[state] = true

for v in reverse BFS order:
    visited[fail[v]] |= visited[v]

for each pattern:
    print visited[terminal[pattern]]
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <array>     // 고정 소문자 전이 배열을 쓴다.
#include <iostream>  // 예제 결과를 출력한다.
#include <queue>     // 실패 링크 BFS frontier다.
#include <string>    // text와 pattern을 소유한다.
#include <vector>    // 상태와 순서를 소유한다.

struct Node {
    Node() { next.fill(-1); }  // -1은 아직 실제 trie 간선이 없다는 sentinel이다.
    std::array<int, 26> next{};
    int fail{};
    bool seen{};
};

class AhoCorasick {
public:
    AhoCorasick() { nodes_.emplace_back(); }  // 상태 0은 루트다.

    int insert(const std::string& pattern) {
        int state{};
        for (const char ch : pattern) {
            const int c{ch - 'a'};
            if (nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(c)] == -1) {
                const int created{static_cast<int>(nodes_.size())};
                nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(c)] = created;
                nodes_.emplace_back();
            }
            // emplace_back 재할당 뒤에는 예전 참조를 쓰지 않고 안정적인 인덱스로 전이를 다시 읽는다.
            state = nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(c)];
        }
        return state;
    }

    void build() {
        std::queue<int> queue{};
        order_.push_back(0);
        for (int c{}; c < 26; ++c) {
            int& edge{nodes_[0].next[static_cast<std::size_t>(c)]};
            if (edge == -1) {
                edge = 0;
            } else {
                queue.push(edge);
            }
        }
        while (!queue.empty()) {
            const int state{queue.front()};
            queue.pop();
            order_.push_back(state);
            for (int c{}; c < 26; ++c) {
                int& edge{nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(c)]};
                const int fallback{nodes_[static_cast<std::size_t>(nodes_[static_cast<std::size_t>(state)].fail)]
                                       .next[static_cast<std::size_t>(c)]};
                if (edge == -1) {
                    edge = fallback;
                } else {
                    nodes_[static_cast<std::size_t>(edge)].fail = fallback;
                    queue.push(edge);
                }
            }
        }
    }

    void scan(const std::string& text) {
        int state{};
        for (const char ch : text) {
            state = nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(ch - 'a')];
            nodes_[static_cast<std::size_t>(state)].seen = true;
        }
        for (std::size_t i{order_.size()}; i > 1U; --i) {
            const int v{order_[i - 1U]};
            nodes_[static_cast<std::size_t>(nodes_[static_cast<std::size_t>(v)].fail)].seen =
                nodes_[static_cast<std::size_t>(nodes_[static_cast<std::size_t>(v)].fail)].seen ||
                nodes_[static_cast<std::size_t>(v)].seen;
        }
    }

    bool contains(int terminal) const {
        return nodes_[static_cast<std::size_t>(terminal)].seen;
    }

private:
    std::vector<Node> nodes_{};
    std::vector<int> order_{};
};

int main() {
    AhoCorasick aho{};
    const int he{aho.insert("he")};
    const int she{aho.insert("she")};
    const int his{aho.insert("his")};
    aho.build();
    aho.scan("ahishers");
    std::cout << aho.contains(he) << ' ' << aho.contains(she) << ' ' << aho.contains(his) << '\n';
}
```

예상 출력은 `1 1 1`이다. 실제 제출판은 날짜 폴더의 [`../2026-08-30/icpc_problem.cpp`](../2026-08-30/icpc_problem.cpp)에서 호출 계약 주석과 함께 확인한다.

## 정확성 근거

### 보조정리 1: 실패 링크가 정의와 일치한다

루트 자식의 가장 긴 proper suffix trie 상태는 루트다. BFS에서 부모 `v`의 실패 전이가 정확하다고 가정하면 자식 `u=v+c`의 후보 suffix는 `v`의 suffix 뒤에 `c`를 붙인 것들이다. 그중 가장 긴 trie 상태는 `go[fail[v]][c]`이므로 설정한 `fail[u]`가 정의와 일치한다.

### 보조정리 2: 검색 상태 불변식이 유지된다

현재 상태가 읽은 text 접두사의 가장 긴 trie suffix라고 하자. 다음 문자 `c`의 완성 전이는 실제 자식이 있으면 그 더 긴 suffix로, 없으면 실패 상태에서 가능한 가장 긴 `c` 전이로 간다. 따라서 전이 뒤 상태도 새 text 접두사의 가장 긴 trie suffix다.

### 보조정리 3: 역방향 전파가 모든 패턴 등장을 표시한다

text 검색에서 상태 `v`에 도착했다면 `v` 문자열이 현재 text suffix다. `fail[v]` 문자열도 그 suffix의 suffix이므로 동시에 등장한다. 실패 링크는 깊이를 줄이므로 BFS 역순으로 `seen[fail[v]] |= seen[v]`를 수행하면 깊은 상태의 모든 방문이 terminal suffix 조상까지 전달된다.

### 정리

보조정리 2로 text의 각 위치에서 가장 긴 trie suffix 상태를 빠뜨리지 않고 표시하고, 보조정리 3으로 그 상태의 모든 패턴 suffix도 표시한다. trie terminal은 해당 패턴 문자열과 일대일 대응하므로 출력은 각 패턴의 등장 여부와 정확히 일치한다.

## 시간·공간 복잡도

- 패턴 삽입: 전체 패턴 길이 `P`에 대해 `O(P)`.
- 실패 링크와 26개 완성 전이: 상태 수가 `O(P)`이므로 `O(26P)`.
- text 검색: 길이 `N`에 대해 `O(N)`.
- 역전파와 출력: `O(P+K)`.
- 공간: 상태별 26개 int 전이와 실패/표시, terminal/BFS 배열을 합쳐 `O(26P+P+K)`.

고정 알파벳 26을 상수로 보면 전체 시간은 `O(P+N+K)`, 공간은 `O(P+K)`로 쓴다.

## 흔한 실수

- trie terminal 상태에 직접 도착한 것만 보고 실패 링크 suffix 패턴을 누락한다.
- 실패 링크를 DFS나 삽입 순서로 계산해 부모의 fallback 전이가 아직 준비되지 않는다.
- 루트의 없는 전이를 -1로 남긴 채 검색에서 음수 인덱스를 사용한다.
- BFS 정순으로 방문 정보를 부모에 보내 깊은 자손 정보가 늦게 도착해 누락된다.
- `char`를 그대로 인덱스로 써 음수 또는 25 초과가 된다. 입력 알파벳 계약을 확인하고 `'a'`를 뺀다.
- 상태 포인터/참조를 저장한 채 vector가 재할당된다. reserve 상한을 확보하거나 안정적인 정수 인덱스를 저장한다.
- 패턴 중복을 별도 상태로 강제해 메모리를 낭비한다. terminal 상태를 여러 입력 행이 공유해도 된다.

## 변형과 대회 필수 연결

- 각 상태에 terminal 패턴 ID 목록을 두면 모든 출현 위치를 보고할 수 있다. 출력량 자체가 크면 복잡도에 결과 수를 더한다.
- 방문 횟수를 세고 실패 링크 역전파하면 각 패턴 출현 횟수를 구한다.
- 최초 위치/최소 위치는 상태 방문 시 text 인덱스를 저장하고 실패 트리에서 적절한 집계를 한다.
- dictionary suffix link를 두면 terminal 상태만 건너뛰며 보고할 수 있다.
- 실패 링크를 간선으로 본 **failure tree**에 Euler tour와 Fenwick/segment tree를 결합하면 동적 활성 패턴 질의를 처리할 수 있다.
- 메모리가 제한되면 완성 전이 배열 대신 sparse edge와 실패 while을 사용하지만 상수와 캐시 지역성이 달라진다.

## 오늘 문제와의 연결

CSES 2102는 text 하나에 최대 500,000개 패턴, 패턴 총길이 500,000을 허용한다. 패턴마다 text를 찾는 방식은 통과하기 어렵다. 오늘 구현은 검색 상태 방문을 표시하고 실패 링크를 BFS 역순으로 전파해 각 terminal의 존재 여부만 선형에 답한다.

## 직접 해보기와 초보자 검증

1. `he`, `she`, `hers`, `his` trie와 모든 실패 링크를 손으로 그린다.
2. text `ahishers`를 한 글자씩 읽으며 현재 상태와 동시에 매칭되는 suffix 패턴을 기록한다.
3. 역전파를 제거해 `she`가 있을 때 `he`를 놓치는 구현을 재현한다.
4. 길이 1~40의 text와 패턴 집합을 무작위 생성해 각 결과를 `string::find`와 1,000회 대조한다.
5. 완성 전이 배열의 실제 대략 메모리를 `상태 수 * 26 * sizeof(int)`로 계산하고 512 MB 제한과 비교한다.
