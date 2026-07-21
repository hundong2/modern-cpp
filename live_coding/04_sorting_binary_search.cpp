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

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

int countOccurrences(vector<int> a, int x) {
    sort(a.begin(), a.end());
    auto lo = lower_bound(a.begin(), a.end(), x); // x 이상이 처음 나오는 위치다.
    auto hi = upper_bound(a.begin(), a.end(), x); // x 초과가 처음 나오는 위치다.
    return (int)(hi - lo);
}

long long minimumSpeed(const vector<int>& works, long long maxHours) {
    long long lo = 1;
    long long hi = *max_element(works.begin(), works.end());

    auto possible = [&](long long speed) {
        long long hours = 0;
        for (int w : works) {
            hours += (w + speed - 1) / speed; // 정수 나눗셈에서 올림을 구현하는 관용식이다.
            if (hours > maxHours) return false;
        }
        return true;
    };

    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2; // overflow 안전한 중간값 계산이다.
        if (possible(mid)) hi = mid;        // 가능한 속도면 더 작은 속도도 가능한지 왼쪽을 본다.
        else lo = mid + 1;                  // 불가능하면 속도를 반드시 키워야 한다.
    }
    return lo;
}

vector<pair<int, int>> mergeIntervals(vector<pair<int, int>> intervals) {
    sort(intervals.begin(), intervals.end());
    vector<pair<int, int>> merged;

    for (auto [start, end] : intervals) {
        if (merged.empty() || merged.back().second < start) { // 마지막 구간과 겹치지 않으면 새 구간을 연다.
            merged.push_back({start, end});
        } else {
            merged.back().second = max(merged.back().second, end); // 겹치면 끝점만 확장한다.
        }
    }
    return merged;
}

int lowerBoundManual(const vector<int>& a, int target) {
    int lo = 0;
    int hi = (int)a.size();
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] >= target) hi = mid; // mid도 답 후보이므로 버리지 않는다.
        else lo = mid + 1;              // target보다 작으면 mid 이하가 모두 답이 아니다.
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
