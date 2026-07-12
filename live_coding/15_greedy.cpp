/*
Chapter 15. Greedy

문제 1: 회의 시간이 여러 개 있을 때 겹치지 않게 가장 많이 선택하라.
접근: 끝나는 시간이 빠른 회의부터 고른다.
정답 해설:
- 가장 빨리 끝나는 회의를 고르면 남은 시간 공간이 최대화된다.
- 어떤 최적해도 첫 회의를 가장 빨리 끝나는 회의로 교체해도 개수가 줄지 않는다.
복잡도: O(N log N)

문제 2: 배열에서 각 위치의 점프 길이가 주어질 때 마지막까지 도달 가능한가?
접근: 현재까지 도달 가능한 가장 먼 위치를 갱신한다.

문제 3: 파일/로프를 두 개씩 합치는 비용의 총합을 최소화하라.
접근: 매번 가장 작은 두 개를 먼저 합친다. priority_queue min-heap.
*/

#include <algorithm>
#include <climits>
#include <functional>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

int maxNonOverlappingMeetings(vector<pair<int, int>> meetings) {
    sort(meetings.begin(), meetings.end(), [](auto a, auto b) {
        if (a.second != b.second) return a.second < b.second;
        return a.first < b.first;
    });

    int count = 0;
    int currentEnd = INT_MIN;
    for (auto [start, end] : meetings) {
        if (start >= currentEnd) {
            ++count;
            currentEnd = end;
        }
    }
    return count;
}

bool canJumpToEnd(const vector<int>& jump) {
    int farthest = 0;
    for (int i = 0; i < (int)jump.size(); ++i) {
        if (i > farthest) return false;
        farthest = max(farthest, i + jump[i]);
    }
    return true;
}

long long minMergeCost(const vector<int>& sizes) {
    priority_queue<long long, vector<long long>, greater<long long>> pq(sizes.begin(), sizes.end());
    long long cost = 0;

    while (pq.size() > 1) {
        long long a = pq.top();
        pq.pop();
        long long b = pq.top();
        pq.pop();
        cost += a + b;
        pq.push(a + b);
    }
    return cost;
}

int eraseOverlapIntervals(vector<pair<int, int>> intervals) {
    sort(intervals.begin(), intervals.end(), [](auto a, auto b) {
        return a.second < b.second;
    });

    int kept = 0;
    int end = INT_MIN;
    for (auto [l, r] : intervals) {
        if (l >= end) {
            ++kept;
            end = r;
        }
    }
    return (int)intervals.size() - kept;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<pair<int, int>> meetings = {{1, 4}, {2, 3}, {3, 5}, {6, 8}, {5, 7}};
    cout << "[max meetings] " << maxNonOverlappingMeetings(meetings) << '\n';
    cout << "[can jump] " << boolalpha << canJumpToEnd({2, 3, 1, 1, 4}) << '\n';
    cout << "[merge cost] " << minMergeCost({10, 20, 30}) << '\n';
    cout << "[erase overlap] " << eraseOverlapIntervals({{1, 2}, {2, 3}, {3, 4}, {1, 3}}) << '\n';
    return 0;
}
