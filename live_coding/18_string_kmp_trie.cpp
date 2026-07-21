/*
Chapter 18. KMP, Trie, 문자열 탐색

문제 1: text 안에서 pattern이 시작하는 모든 위치를 구하라.
접근: KMP. prefix function은 pattern[0..i]의 접두사이자 접미사인 최대 길이를 저장한다.
정답 해설:
- 불일치가 나면 이미 맞춘 prefix 정보를 이용해 비교 위치를 되돌린다.
- text 포인터는 절대 뒤로 가지 않아 O(N + M).

문제 2: 단어 사전을 만들고 단어 존재/접두사 존재 여부를 빠르게 확인하라.
접근: Trie. 각 노드는 다음 문자로 가는 간선과 terminal 여부를 가진다.
복잡도: 단어 길이 O(L)

문제 3: 문자열의 각 위치에서 시작하는 접두사 매칭 길이를 구하라.
접근: Z-function.
*/

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

vector<int> prefixFunction(const string& s) {
    vector<int> pi(s.size(), 0);
    for (int i = 1; i < (int)s.size(); ++i) {
        int j = pi[i - 1];
        while (j > 0 && s[i] != s[j]) j = pi[j - 1]; // 실패하면 다음 가능한 접두사 길이로 되돌아간다.
        if (s[i] == s[j]) ++j;                       // 한 글자 더 맞으면 border 길이가 증가한다.
        pi[i] = j;
    }
    return pi;
}

vector<int> kmpSearch(const string& text, const string& pattern) {
    vector<int> result;
    if (pattern.empty()) return result;

    vector<int> pi = prefixFunction(pattern);
    int matched = 0;
    for (int i = 0; i < (int)text.size(); ++i) {
        while (matched > 0 && text[i] != pattern[matched]) matched = pi[matched - 1]; // text 인덱스는 되돌리지 않는다.
        if (text[i] == pattern[matched]) ++matched;
        if (matched == (int)pattern.size()) {
            result.push_back(i - matched + 1);
            matched = pi[matched - 1]; // 겹치는 매칭을 찾기 위해 다음 border에서 이어간다.
        }
    }
    return result;
}

class Trie {
public:
    Trie() {
        nodes.push_back(Node{});
    }

    void insert(const string& word) {
        int cur = 0;
        for (char ch : word) {
            int idx = ch - 'a';
            if (nodes[cur].next[idx] == -1) {
                nodes[cur].next[idx] = (int)nodes.size(); // 없는 간선이면 새 노드를 만든다.
                nodes.push_back(Node{});
            }
            cur = nodes[cur].next[idx];
            ++nodes[cur].prefixCount;
        }
        nodes[cur].terminal = true;
    }

    bool contains(const string& word) const {
        int node = walk(word);
        return node != -1 && nodes[node].terminal;
    }

    bool startsWith(const string& prefix) const {
        return walk(prefix) != -1;
    }

private:
    struct Node {
        array<int, 26> next;
        bool terminal = false;
        int prefixCount = 0;

        Node() {
            next.fill(-1);
        }
    };

    vector<Node> nodes;

    int walk(const string& s) const {
        int cur = 0;
        for (char ch : s) {
            int idx = ch - 'a';
            if (idx < 0 || idx >= 26 || nodes[cur].next[idx] == -1) return -1;
            cur = nodes[cur].next[idx];
        }
        return cur;
    }
};

vector<int> zFunction(const string& s) {
    int n = (int)s.size();
    vector<int> z(n, 0);
    int left = 0;
    int right = 0;
    for (int i = 1; i < n; ++i) {
        if (i <= right) z[i] = min(right - i + 1, z[i - left]); // 기존 Z-box 안의 정보를 재사용한다.
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) ++z[i];  // box 밖은 실제 문자 비교로 확장한다.
        if (i + z[i] - 1 > right) {
            left = i;
            right = i + z[i] - 1;
        }
    }
    return z;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    auto positions = kmpSearch("ababcabcabababd", "ababd");
    cout << "[kmp positions]";
    for (int p : positions) cout << ' ' << p;
    cout << '\n';

    Trie trie;
    trie.insert("code");
    trie.insert("coder");
    trie.insert("coding");
    cout << "[trie contains code] " << boolalpha << trie.contains("code") << '\n';
    cout << "[trie starts cod] " << trie.startsWith("cod") << '\n';

    auto z = zFunction("aabcaabxaaaz");
    cout << "[z]";
    for (int x : z) cout << ' ' << x;
    cout << '\n';
    return 0;
}
