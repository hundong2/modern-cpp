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

#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

bool isValidParentheses(const string& s) {
    unordered_map<char, char> match = {{')', '('}, {']', '['}, {'}', '{'}}; // hash table이라 닫는 괄호에서 필요한 여는 괄호를 평균 O(1)에 찾는다.
    stack<char> st; // stack은 LIFO adapter라 가장 최근 push한 값이 top이다. 괄호의 안쪽부터 닫히는 규칙과 맞다.

    for (char c : s) {
        if (c == '(' || c == '[' || c == '{') {
            st.push(c); // 닫는 괄호가 나올 때까지 최근 여는 괄호를 기억한다.
        } else if (match.count(c)) {
            if (st.empty() || st.top() != match[c]) return false; // 비어 있거나 종류가 다르면 즉시 실패다.
            st.pop(); // 가장 안쪽 괄호부터 짝이 맞아야 전체도 올바르다.
        }
    }
    return st.empty();
}

vector<int> nextGreaterElements(const vector<int>& a) {
    vector<int> answer(a.size(), -1); // 끝까지 못 찾은 원소는 -1로 남긴다.
    stack<int> pending;               // 값을 직접 넣지 않고 인덱스를 넣어 answer 위치를 갱신한다.

    for (int i = 0; i < (int)a.size(); ++i) {
        while (!pending.empty() && a[pending.top()] < a[i]) {
            answer[pending.top()] = a[i]; // 현재 값이 pending top의 첫 번째 오른쪽 큰 값이다.
            pending.pop();
        }
        pending.push(i); // 아직 next greater를 못 찾은 인덱스를 보관한다.
    }
    return answer;
}

vector<int> slidingWindowMaximum(const vector<int>& a, int k) {
    deque<int> dq;       // deque는 양쪽 끝 삽입/삭제가 O(1)이라 queue보다 슬라이딩 윈도우 후보 관리에 적합하다.
    vector<int> result;  // i >= k-1부터 창 하나가 완성된다.

    for (int i = 0; i < (int)a.size(); ++i) {
        while (!dq.empty() && dq.front() <= i - k) dq.pop_front(); // 창 밖 인덱스는 후보에서 제거한다.
        while (!dq.empty() && a[dq.back()] <= a[i]) dq.pop_back(); // 더 작거나 같은 값은 앞으로 최댓값이 될 수 없다.
        dq.push_back(i); // 값이 아니라 인덱스를 넣어 창 밖 여부를 판단할 수 있게 한다.
        if (i >= k - 1) result.push_back(a[dq.front()]); // deque 앞 인덱스가 현재 창의 최댓값 위치다.
    }
    return result;
}

vector<int> topKLargest(const vector<int>& a, int k) {
    priority_queue<int, vector<int>, greater<int>> minHeap; // priority_queue는 일반 queue처럼 먼저 넣은 값이 아니라 우선순위가 높은 값이 top이다.
    for (int x : a) {
        minHeap.push(x); // 일단 후보에 넣고 크기가 k를 넘으면 가장 작은 값을 제거한다.
        if ((int)minHeap.size() > k) minHeap.pop(); // k개만 유지하면 top은 현재 top-k 중 가장 작은 값이다.
    }

    vector<int> result;
    while (!minHeap.empty()) {
        result.push_back(minHeap.top());
        minHeap.pop(); // heap에서 꺼낸 순서는 오름차순이므로 아래에서 다시 내림차순 정렬한다.
    }
    sort(result.rbegin(), result.rend()); // reverse_iterator를 쓰면 별도 comparator 없이 내림차순 정렬할 수 있다.
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
