/*
문제 ID·제목: CSES 2102 - Finding Patterns
출처: CSES Problem Set / String Algorithms
출처 URL: https://cses.fi/problemset/task/2102

문제 요약:
소문자 문자열 text 하나와 여러 패턴이 주어진다. 각 패턴에 대해 text의 연속 부분 문자열로 한 번이라도
등장하는지 판정한다. 패턴마다 text를 처음부터 다시 찾으면 입력 상한에서 지나치게 느리므로, 모든 패턴의
접두사를 하나의 automaton으로 합쳐 text를 한 번만 훑어야 한다. 저작권이 있는 원문 전체를 복제하지 않고
핵심 조건과 입출력을 한글로 다시 설명했다.

입력:
첫 줄에 text가 주어진다. 둘째 줄에 패턴 수 k가 주어지고, 이어지는 k개 줄에 패턴이 하나씩 주어진다.
text와 패턴은 영문 소문자 a부터 z까지만 포함한다.

출력:
입력된 각 패턴 순서대로 text에 등장하면 YES, 등장하지 않으면 NO를 한 줄에 하나씩 출력한다.

제약:
1 <= text 길이 <= 100,000, 1 <= k <= 500,000이며 모든 패턴 길이의 합은 최대 500,000이다.
아래 Aho-Corasick 구현은 전체 패턴 길이를 P, text 길이를 N이라 할 때 O(26P + N + k) 시간과
O(26P + P + k) 공간을 사용한다. 알파벳 크기 26을 상수로 보면 O(P+N+k)이다.

예제:
입력
aybabtu
3
bab
abc
ayba

출력
YES
NO
YES
*/

#include <array>     // std::array<int, 26>으로 각 상태의 고정 알파벳 전이를 연속 저장한다.
#include <cstddef>   // std::size_t로 문자열·컨테이너 크기와 역방향 인덱스를 표현한다.
#include <iostream>  // std::cin/std::cout과 스트림 연산자를 사용한다.
#include <queue>     // std::queue<int>로 실패 링크를 루트에서 가까운 순서로 구축한다.
#include <string>    // std::string이 text와 각 패턴의 문자를 소유한다.
#include <vector>    // std::vector가 trie 상태, BFS 순서, 패턴 끝 상태를 동적으로 소유한다.

// 공용 알고리즘 문서: ../algorithm/aho-corasick-multiple-pattern-matching.md
// 하나의 상태는 어떤 패턴 접두사를 이미 읽은 automaton 위치다.
struct Node {
    // 생성자는 반환형이 없다. fail은 루트 0, matched는 false로 중괄호 값 초기화된다.
    Node() {
        // array::fill(const int&)은 값 -1을 const 참조로 읽어 26개 원소에 복사하고 void를 반환한다.
        // O(26) 시간, 무할당이며 배열 크기·수명·참조 유효성은 바뀌지 않는다. -1은 아직 전이가 없다는 sentinel이다.
        next.fill(-1);
    }

    std::array<int, 26> next{};
    int fail{};
    bool matched{};
};

// class는 기본 private 접근으로 전이 완성 전 검색 같은 잘못된 호출 순서를 캡슐화한다.
class AhoCorasick {
public:
    // 생성자는 반환형이 없고 루트 Node 하나를 vector 안에 직접 만든다.
    AhoCorasick() {
        // vector::emplace_back()은 인자 없이 Node 기본 생성자를 끝 저장소에서 호출하고 Node&를 반환하지만 사용하지 않는다.
        // amortized O(1)이며 재할당 시 기존 Node 포인터·참조·반복자는 무효화된다. 인덱스만 보관하므로 재할당 뒤에도 안전하다.
        nodes_.emplace_back();
    }

    [[nodiscard]] int insert(const std::string& pattern) {
        int state{};
        // range-for의 char 값은 pattern 원소를 하나씩 복사한다. pattern은 const lvalue 참조로 빌릴 뿐 소유하지 않는다.
        for (const char ch : pattern) {
            const int letter{ch - 'a'};  // - 연산은 문자를 정수 승격해 0..25 전이 번호를 만든다.
            // vector::operator[]와 array::operator[]는 범위 검사 없이 O(1) lvalue 참조를 반환한다.
            // 입력 소문자와 유효 state 불변식이 범위를 보장하며 이 읽기는 컨테이너 크기·수명을 바꾸지 않는다.
            if (nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(letter)] == -1) {
                const int created{static_cast<int>(nodes_.size())};
                nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(letter)] = created;
                nodes_.emplace_back();
            }
            state = nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(letter)];
        }
        return state;  // 패턴 끝 상태를 값으로 반환하며 호출자는 입력 순서대로 저장한다.
    }

    void build_failure_links() {
        // queue<int>{} 기본 생성자는 빈 내부 컨테이너를 소유하며 별도 반환값이 없다. 필요 시 할당하고 함수 끝에서 원소를 파괴한다.
        std::queue<int> pending{};
        // vector::push_back(const int&)은 루트 인덱스 0을 복사해 BFS 순서 끝에 넣고 void를 반환한다.
        // amortized O(1), 재할당 시 order의 기존 참조/반복자가 무효화되며 여기서는 보관하지 않는다.
        bfs_order_.push_back(0);

        for (int letter{}; letter < 26; ++letter) {
            int& root_transition{nodes_[0].next[static_cast<std::size_t>(letter)]};
            if (root_transition == -1) {
                root_transition = 0;  // 없는 루트 전이는 다시 루트로 가도록 완성한다.
            } else {
                nodes_[static_cast<std::size_t>(root_transition)].fail = 0;
                // queue::push(const int&)는 자식 상태 값을 내부 deque 끝에 복사하고 void를 반환한다.
                // amortized O(1), pending 외부의 nodes_ 수명/참조는 바꾸지 않으며 할당 실패 시 bad_alloc이 가능하다.
                pending.push(root_transition);
            }
        }

        // queue::empty()는 bool을 O(1)에 반환하며 상태를 바꾸지 않는다. 모든 도달 상태를 한 번씩 처리하는 반복 조건이다.
        while (!pending.empty()) {
            // front()는 첫 int의 lvalue 참조를 반환한다. !empty()가 전제조건을 보장하고 값으로 복사해 pop 뒤에도 유지한다.
            const int state{pending.front()};
            // pop()은 반환값 없이 첫 원소를 제거해 queue 크기를 1 줄인다. 제거 원소 참조는 무효화된다.
            pending.pop();
            bfs_order_.push_back(state);

            for (int letter{}; letter < 26; ++letter) {
                int& transition{nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(letter)]};
                const int fallback{nodes_[static_cast<std::size_t>(nodes_[static_cast<std::size_t>(state)].fail)]
                                       .next[static_cast<std::size_t>(letter)]};
                if (transition == -1) {
                    transition = fallback;  // 없는 간선을 실패 상태의 완성 전이로 물려 검색 중 while 후퇴를 없앤다.
                } else {
                    nodes_[static_cast<std::size_t>(transition)].fail = fallback;
                    pending.push(transition);
                }
            }
        }
    }

    void scan(const std::string& text) {
        int state{};
        for (const char ch : text) {
            const int letter{ch - 'a'};
            // build 뒤 모든 전이는 0..nodes_.size()-1이다. 문자마다 O(1) 전이하고 도착 상태를 표시한다.
            state = nodes_[static_cast<std::size_t>(state)].next[static_cast<std::size_t>(letter)];
            nodes_[static_cast<std::size_t>(state)].matched = true;
        }

        // BFS 역순이면 자식의 matched를 fail 조상으로 보내기 전에 모든 더 깊은 상태 정보가 모여 있다.
        for (std::size_t index{bfs_order_.size()}; index > 1U; --index) {
            const int state_id{bfs_order_[index - 1U]};
            const int failure{nodes_[static_cast<std::size_t>(state_id)].fail};
            nodes_[static_cast<std::size_t>(failure)].matched =
                nodes_[static_cast<std::size_t>(failure)].matched || nodes_[static_cast<std::size_t>(state_id)].matched;
        }
    }

    [[nodiscard]] bool matched(int terminal_state) const {
        return nodes_[static_cast<std::size_t>(terminal_state)].matched;
    }

private:
    // vector{} 기본 생성은 size/capacity 0인 소유 컨테이너를 만들며 보통 아직 원소 저장 할당이 없다.
    // 멤버는 AhoCorasick과 함께 살고 재할당 시 포인터/참조/반복자는 무효화되지만 정수 인덱스 값은 유지된다.
    std::vector<Node> nodes_{};
    std::vector<int> bfs_order_{};
};

int main() {
    // sync_with_stdio(false)는 C/C++ 스트림 동기화 설정을 false로 바꾸고 이전 bool 설정을 반환하지만 무시한다.
    // 이후 C stdio와 섞는 순서를 보장하지 않는다. cin.tie(nullptr)는 이전 ostream*를 반환하지만 저장하지 않고 자동 flush 연결을 끊는다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // string{} 기본 생성은 빈 소유 문자열을 만들며 별도 반환값이 없다. 이후 추출이 문자 버퍼와 길이를 바꾼다.
    std::string text{};
    // operator>>(istream&, string&)은 cin 위치/상태와 text를 바꾸고 같은 istream&를 반환한다. 공백 전까지 문자를 text가 소유한다.
    std::cin >> text;

    int pattern_count{};
    std::cin >> pattern_count;

    AhoCorasick matcher{};
    // vector<int>(count)는 count개의 0으로 값 초기화된 원소를 소유한다. 음수 count는 문제 제약이 금지한다.
    // O(k) 시간·공간, 할당 실패 시 bad_alloc이며 생성 뒤 size==pattern_count다.
    std::vector<int> terminals(static_cast<std::size_t>(pattern_count));
    for (int index{}; index < pattern_count; ++index) {
        // 매 반복의 string{}은 빈 문자열을 만들고 scope 끝에서 버퍼를 정리한다. 이전 pattern과 소유권을 공유하지 않는다.
        std::string pattern{};
        std::cin >> pattern;
        terminals[static_cast<std::size_t>(index)] = matcher.insert(pattern);
    }

    matcher.build_failure_links();
    matcher.scan(text);

    for (const int terminal : terminals) {
        // 조건 연산자는 bool에 따라 두 문자열 리터럴 중 하나의 const char* 값을 고른다.
        // ostream << const char*는 null 종료 문자를 출력 버퍼에 복사하고 ostream&를 반환해 '\n' 출력과 연쇄한다.
        std::cout << (matcher.matched(terminal) ? "YES" : "NO") << '\n';
    }
    return 0;
}
