/*
Chapter 04. 정렬, lower/upper bound, 매개변수 탐색

문제 1: 정렬된 배열에서 값 x의 등장 횟수를 구하라.
접근: lower_bound와 upper_bound의 차이.
복잡도: O(log N)

문제 2: N개의 일을 M시간 안에 끝낼 수 있는 최소 처리 속도를 구하라.
접근: 속도 v가 가능하면 v보다 큰 값도 가능하다. 즉 단조성이 있으므로 이분 탐색.
정답 해설:
- possible(v): sum(ceil(work / v)) <= M
- true가 처음 되는 최소 v를 찾는다.

문제 3: 겹치는 구간을 병합하라.
접근: 시작점 기준 정렬 후 마지막 병합 구간과 비교한다.
*/

#include <bits/stdc++.h>
using namespace std;

int countOccurrences(vector<int> a, int x) {
    sort(a.begin(), a.end());
    auto lo = lower_bound(a.begin(), a.end(), x);
    auto hi = upper_bound(a.begin(), a.end(), x);
    return (int)(hi - lo);
}

long long minimumSpeed(const vector<int>& works, long long maxHours) {
    long long lo = 1;
    long long hi = *max_element(works.begin(), works.end());

    auto possible = [&](long long speed) {
        long long hours = 0;
        for (int w : works) {
            hours += (w + speed - 1) / speed;
            if (hours > maxHours) return false;
        }
        return true;
    };

    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2;
        if (possible(mid)) hi = mid;
        else lo = mid + 1;
    }
    return lo;
}

vector<pair<int, int>> mergeIntervals(vector<pair<int, int>> intervals) {
    sort(intervals.begin(), intervals.end());
    vector<pair<int, int>> merged;

    for (auto [start, end] : intervals) {
        if (merged.empty() || merged.back().second < start) {
            merged.push_back({start, end});
        } else {
            merged.back().second = max(merged.back().second, end);
        }
    }
    return merged;
}

int lowerBoundManual(const vector<int>& a, int target) {
    int lo = 0;
    int hi = (int)a.size();
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] >= target) hi = mid;
        else lo = mid + 1;
    }
    return lo;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> a = {4, 1, 2, 2, 2, 5, 7};
    cout << "[count 2] " << countOccurrences(a, 2) << '\n';

    vector<int> works = {30, 11, 23, 4, 20};
    cout << "[minimum speed in 6 hours] " << minimumSpeed(works, 6) << '\n';

    vector<pair<int, int>> intervals = {{1, 3}, {2, 6}, {8, 10}, {9, 12}};
    auto merged = mergeIntervals(intervals);
    cout << "[merged]";
    for (auto [l, r] : merged) cout << " [" << l << ',' << r << ']';
    cout << '\n';

    vector<int> sorted = {1, 3, 3, 5, 8};
    cout << "[manual lower_bound 4] " << lowerBoundManual(sorted, 4) << '\n';
    return 0;
}
