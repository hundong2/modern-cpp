/*
Chapter 03. 투 포인터, 슬라이딩 윈도우

문제 1: 양수 배열에서 합이 S 이상인 가장 짧은 연속 부분 배열 길이를 구하라.
접근: 오른쪽 포인터로 합을 키우고, 조건을 만족하면 왼쪽 포인터를 줄인다.
정답 해설:
- 모든 원소가 양수라서 왼쪽을 이동하면 합이 단조 감소한다.
- 각 포인터는 최대 N번 움직이므로 O(N)이다.

문제 2: 중복 문자가 없는 가장 긴 부분 문자열 길이를 구하라.
접근: 마지막 등장 위치를 저장하고 left를 건너뛴다.

문제 3: 고정 길이 K 구간의 최대 합을 구하라.
접근: 첫 창의 합을 만든 뒤 한 칸씩 밀며 갱신한다.
*/

#include <bits/stdc++.h>
using namespace std;

int minLengthAtLeastS(const vector<int>& a, int s) {
    int n = (int)a.size();
    int answer = INT_MAX;
    long long sum = 0;
    int left = 0;

    for (int right = 0; right < n; ++right) {
        sum += a[right];
        while (sum >= s) {
            answer = min(answer, right - left + 1);
            sum -= a[left++];
        }
    }
    return answer == INT_MAX ? 0 : answer;
}

int longestUniqueSubstring(const string& s) {
    vector<int> last(256, -1);
    int left = 0;
    int best = 0;

    for (int right = 0; right < (int)s.size(); ++right) {
        unsigned char ch = static_cast<unsigned char>(s[right]);
        if (last[ch] >= left) left = last[ch] + 1;
        last[ch] = right;
        best = max(best, right - left + 1);
    }
    return best;
}

long long maxFixedWindowSum(const vector<int>& a, int k) {
    if (k <= 0 || k > (int)a.size()) throw invalid_argument("invalid window size");

    long long cur = 0;
    for (int i = 0; i < k; ++i) cur += a[i];

    long long best = cur;
    for (int right = k; right < (int)a.size(); ++right) {
        cur += a[right] - a[right - k];
        best = max(best, cur);
    }
    return best;
}

vector<int> findAnagramStarts(const string& text, const string& pattern) {
    if (pattern.size() > text.size()) return {};

    vector<int> need(26, 0), have(26, 0), result;
    for (char c : pattern) ++need[c - 'a'];

    int k = (int)pattern.size();
    for (int i = 0; i < (int)text.size(); ++i) {
        ++have[text[i] - 'a'];
        if (i >= k) --have[text[i - k] - 'a'];
        if (i >= k - 1 && have == need) result.push_back(i - k + 1);
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> a = {2, 3, 1, 2, 4, 3};
    cout << "[min length sum >= 7] " << minLengthAtLeastS(a, 7) << '\n';
    cout << "[longest unique] " << longestUniqueSubstring("abcabcbb") << '\n';
    cout << "[max fixed window k=3] " << maxFixedWindowSum(a, 3) << '\n';

    auto starts = findAnagramStarts("cbaebabacd", "abc");
    cout << "[anagram starts]";
    for (int idx : starts) cout << ' ' << idx;
    cout << '\n';
    return 0;
}
