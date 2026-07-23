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
    vector<long long> prefix(a.size() + 1, 0); // vector는 크기와 초기값을 지정할 수 있고, prefix[0]=0으로 경계 처리를 단순화한다.
    for (int i = 0; i < (int)a.size(); ++i) {
        prefix[i + 1] = prefix[i] + a[i]; // prefix[i+1]은 원소 i까지 포함한 합이다.
    }
    return prefix;
}

long long rangeSum(const vector<long long>& prefix, int l, int r) {
    return prefix[r + 1] - prefix[l]; // [0..r] 합에서 [0..l-1] 합을 빼면 [l..r]만 남는다.
}

vector<long long> applyRangeAdds(int n, const vector<tuple<int, int, int>>& queries) { // tuple은 l,r,v처럼 세 값짜리 레코드를 간단히 묶는다.
    vector<long long> diff(n + 1, 0); // r+1 위치를 안전하게 다루기 위해 한 칸 크게 잡는다.
    for (auto [l, r, v] : queries) {
        diff[l] += v;                    // l부터 v가 적용되기 시작한다.
        if (r + 1 < n) diff[r + 1] -= v; // r 다음 칸에서 적용 효과를 되돌린다.
    }

    vector<long long> result(n);
    long long cur = 0; // diff를 누적하면 각 위치의 실제 변화량이 된다.
    for (int i = 0; i < n; ++i) {
        cur += diff[i];
        result[i] = cur;
    }
    return result;
}

long long countSubarraysWithSumK(const vector<int>& a, long long k) {
    unordered_map<long long, long long> seen; // prefix sum 빈도는 정렬 순서가 필요 없어서 hash table이 map보다 적합하다.
    seen.reserve(a.size() * 2 + 1); // prefix sum 종류가 많아질 때 rehash 비용을 줄인다.
    seen[0] = 1;                    // 시작점부터 현재까지의 합이 k인 경우를 처리하는 가짜 prefix다.

    long long prefix = 0;
    long long answer = 0;
    for (int x : a) {
        prefix += x;
        auto it = seen.find(prefix - k); // currentPrefix - oldPrefix = k인 oldPrefix를 찾는다.
        if (it != seen.end()) answer += it->second;
        ++seen[prefix]; // 현재 prefix는 이후 원소들의 왼쪽 경계 후보가 된다.
    }
    return answer;
}

vector<vector<long long>> build2DPrefix(const vector<vector<int>>& grid) { // vector<vector<T>>는 행마다 별도 vector를 가지는 2차원 컨테이너다.
    int n = (int)grid.size();
    int m = (int)grid[0].size();
    vector<vector<long long>> ps(n + 1, vector<long long>(m + 1, 0)); // 위/왼쪽 경계 처리를 단순화하려고 1-index처럼 만든다.
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            ps[r + 1][c + 1] = grid[r][c] + ps[r][c + 1] + ps[r + 1][c] - ps[r][c]; // 겹친 좌상단 영역은 한 번 빼준다.
        }
    }
    return ps;
}

long long rectangleSum(const vector<vector<long long>>& ps, int r1, int c1, int r2, int c2) {
    return ps[r2 + 1][c2 + 1] - ps[r1][c2 + 1] - ps[r2 + 1][c1] + ps[r1][c1]; // 포함-배제 공식이다.
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
    for (long long x : after) cout << ' ' << x; // 최종 배열을 직접 출력해 diff 복원이 맞는지 확인한다.
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
