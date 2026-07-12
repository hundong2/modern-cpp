/*
Chapter 02. 누적합, 차분 배열, prefix hash

문제 1: 배열에서 구간 [l, r] 합을 여러 번 구하라.
접근: prefix[i] = a[0] + ... + a[i - 1]
정답 해설: sum(l, r) = prefix[r + 1] - prefix[l]
복잡도: 전처리 O(N), 질의 O(1)

문제 2: N칸 배열에 range add 쿼리 [l, r] += v가 여러 번 온다. 최종 배열을 구하라.
접근: diff[l] += v, diff[r + 1] -= v 후 누적합.
복잡도: O(N + Q)

문제 3: 합이 K인 부분 배열 개수를 구하라.
접근: prefixSum[j] - prefixSum[i] = K 이므로, 이전 prefixSum 중 current - K의 개수를 센다.
복잡도: O(N), 메모리 O(N)
*/

#include <iostream>
#include <tuple>
#include <unordered_map>
#include <vector>
using namespace std;

vector<long long> buildPrefix(const vector<int>& a) {
    vector<long long> prefix(a.size() + 1, 0);
    for (int i = 0; i < (int)a.size(); ++i) {
        prefix[i + 1] = prefix[i] + a[i];
    }
    return prefix;
}

long long rangeSum(const vector<long long>& prefix, int l, int r) {
    return prefix[r + 1] - prefix[l];
}

vector<long long> applyRangeAdds(int n, const vector<tuple<int, int, int>>& queries) {
    vector<long long> diff(n + 1, 0);
    for (auto [l, r, v] : queries) {
        diff[l] += v;
        if (r + 1 < n) diff[r + 1] -= v;
    }

    vector<long long> result(n);
    long long cur = 0;
    for (int i = 0; i < n; ++i) {
        cur += diff[i];
        result[i] = cur;
    }
    return result;
}

long long countSubarraysWithSumK(const vector<int>& a, long long k) {
    unordered_map<long long, long long> seen;
    seen.reserve(a.size() * 2 + 1);
    seen[0] = 1;

    long long prefix = 0;
    long long answer = 0;
    for (int x : a) {
        prefix += x;
        auto it = seen.find(prefix - k);
        if (it != seen.end()) answer += it->second;
        ++seen[prefix];
    }
    return answer;
}

vector<vector<long long>> build2DPrefix(const vector<vector<int>>& grid) {
    int n = (int)grid.size();
    int m = (int)grid[0].size();
    vector<vector<long long>> ps(n + 1, vector<long long>(m + 1, 0));
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            ps[r + 1][c + 1] = grid[r][c] + ps[r][c + 1] + ps[r + 1][c] - ps[r][c];
        }
    }
    return ps;
}

long long rectangleSum(const vector<vector<long long>>& ps, int r1, int c1, int r2, int c2) {
    return ps[r2 + 1][c2 + 1] - ps[r1][c2 + 1] - ps[r2 + 1][c1] + ps[r1][c1];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> a = {2, -1, 3, 5, -2, 4};
    auto prefix = buildPrefix(a);
    cout << "[range sum 1..3] " << rangeSum(prefix, 1, 3) << '\n';

    vector<tuple<int, int, int>> updates = {
        {0, 2, 5},
        {1, 4, 3},
        {3, 5, -2}
    };
    auto after = applyRangeAdds(6, updates);
    cout << "[range add result]";
    for (long long x : after) cout << ' ' << x;
    cout << '\n';

    cout << "[subarrays sum 5] " << countSubarraysWithSumK(a, 5) << '\n';

    vector<vector<int>> grid = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    auto ps = build2DPrefix(grid);
    cout << "[rect (1,1)..(2,2)] " << rectangleSum(ps, 1, 1, 2, 2) << '\n';
    return 0;
}
