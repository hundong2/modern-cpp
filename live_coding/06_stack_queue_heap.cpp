/*
Chapter 06. 스택, 큐, 덱, 힙

문제 1: 괄호 문자열이 올바른지 검사하라.
접근: 여는 괄호는 push, 닫는 괄호는 top과 매칭.
복잡도: O(N)

문제 2: 각 원소의 오른쪽에서 처음 만나는 더 큰 값을 구하라.
접근: 값이 아직 해결되지 않은 인덱스를 스택에 유지한다.
정답 해설:
- 새 값 a[i]가 stack top의 값보다 크면, 그 인덱스의 next greater는 a[i]다.
- 각 인덱스는 push/pop 한 번씩만 되므로 O(N).

문제 3: 길이 K 슬라이딩 윈도우마다 최댓값을 구하라.
접근: 덱에 후보 인덱스를 값 내림차순으로 유지한다.
*/

#include <bits/stdc++.h>
using namespace std;

bool isValidParentheses(const string& s) {
    unordered_map<char, char> match = {{')', '('}, {']', '['}, {'}', '{'}};
    stack<char> st;

    for (char c : s) {
        if (c == '(' || c == '[' || c == '{') {
            st.push(c);
        } else if (match.count(c)) {
            if (st.empty() || st.top() != match[c]) return false;
            st.pop();
        }
    }
    return st.empty();
}

vector<int> nextGreaterElements(const vector<int>& a) {
    vector<int> answer(a.size(), -1);
    stack<int> pending;

    for (int i = 0; i < (int)a.size(); ++i) {
        while (!pending.empty() && a[pending.top()] < a[i]) {
            answer[pending.top()] = a[i];
            pending.pop();
        }
        pending.push(i);
    }
    return answer;
}

vector<int> slidingWindowMaximum(const vector<int>& a, int k) {
    deque<int> dq;
    vector<int> result;

    for (int i = 0; i < (int)a.size(); ++i) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        while (!dq.empty() && a[dq.back()] <= a[i]) dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1) result.push_back(a[dq.front()]);
    }
    return result;
}

vector<int> topKLargest(const vector<int>& a, int k) {
    priority_queue<int, vector<int>, greater<int>> minHeap;
    for (int x : a) {
        minHeap.push(x);
        if ((int)minHeap.size() > k) minHeap.pop();
    }

    vector<int> result;
    while (!minHeap.empty()) {
        result.push_back(minHeap.top());
        minHeap.pop();
    }
    sort(result.rbegin(), result.rend());
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "[valid parentheses] " << boolalpha << isValidParentheses("{[()]}") << '\n';

    vector<int> a = {2, 1, 2, 4, 3};
    auto nge = nextGreaterElements(a);
    cout << "[next greater]";
    for (int x : nge) cout << ' ' << x;
    cout << '\n';

    vector<int> b = {1, 3, -1, -3, 5, 3, 6, 7};
    auto mx = slidingWindowMaximum(b, 3);
    cout << "[window max]";
    for (int x : mx) cout << ' ' << x;
    cout << '\n';

    auto top = topKLargest(b, 3);
    cout << "[top 3]";
    for (int x : top) cout << ' ' << x;
    cout << '\n';
    return 0;
}
