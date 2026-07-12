/*
Chapter 13. 격자 DP, 0/1 배낭, 무한 배낭

문제 1: 장애물이 있는 격자에서 오른쪽/아래로만 이동해 도착하는 경로 수를 구하라.
접근: dp[r][c] = 위에서 오는 경우 + 왼쪽에서 오는 경우.

문제 2: 각 칸 비용이 있을 때 시작점에서 끝점까지 최소 경로 합을 구하라.
접근: 현재 칸으로 들어오는 마지막 이동은 위 또는 왼쪽뿐이다.

문제 3: 0/1 배낭. 각 물건은 한 번만 쓸 수 있고, 무게 제한 W에서 최대 가치를 구하라.
접근: 물건을 바깥 loop, 무게를 큰 값에서 작은 값으로 순회한다.
정답 해설:
- 큰 값에서 작은 값으로 가야 같은 물건을 한 번만 사용한다.

문제 4: 무한 배낭. 각 물건을 여러 번 쓸 수 있다.
접근: 무게를 작은 값에서 큰 값으로 순회한다.
*/

#include <algorithm>
#include <climits>
#include <iostream>
#include <vector>
using namespace std;

long long uniquePathsWithObstacles(const vector<vector<int>>& blocked) {
    int n = (int)blocked.size();
    int m = (int)blocked[0].size();
    vector<vector<long long>> dp(n, vector<long long>(m, 0));
    if (blocked[0][0]) return 0;
    dp[0][0] = 1;

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            if (blocked[r][c]) {
                dp[r][c] = 0;
                continue;
            }
            if (r > 0) dp[r][c] += dp[r - 1][c];
            if (c > 0) dp[r][c] += dp[r][c - 1];
        }
    }
    return dp[n - 1][m - 1];
}

int minPathSum(const vector<vector<int>>& cost) {
    int n = (int)cost.size();
    int m = (int)cost[0].size();
    vector<vector<int>> dp(n, vector<int>(m, INT_MAX / 4));
    dp[0][0] = cost[0][0];

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            if (r > 0) dp[r][c] = min(dp[r][c], dp[r - 1][c] + cost[r][c]);
            if (c > 0) dp[r][c] = min(dp[r][c], dp[r][c - 1] + cost[r][c]);
        }
    }
    return dp[n - 1][m - 1];
}

int zeroOneKnapsack(const vector<int>& weight, const vector<int>& value, int capacity) {
    vector<int> dp(capacity + 1, 0);
    for (int i = 0; i < (int)weight.size(); ++i) {
        for (int w = capacity; w >= weight[i]; --w) {
            dp[w] = max(dp[w], dp[w - weight[i]] + value[i]);
        }
    }
    return dp[capacity];
}

int unboundedKnapsack(const vector<int>& weight, const vector<int>& value, int capacity) {
    vector<int> dp(capacity + 1, 0);
    for (int i = 0; i < (int)weight.size(); ++i) {
        for (int w = weight[i]; w <= capacity; ++w) {
            dp[w] = max(dp[w], dp[w - weight[i]] + value[i]);
        }
    }
    return dp[capacity];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<vector<int>> blocked = {
        {0, 0, 0},
        {0, 1, 0},
        {0, 0, 0}
    };
    cout << "[unique paths] " << uniquePathsWithObstacles(blocked) << '\n';

    vector<vector<int>> cost = {
        {1, 3, 1},
        {1, 5, 1},
        {4, 2, 1}
    };
    cout << "[min path sum] " << minPathSum(cost) << '\n';

    vector<int> w = {2, 3, 4, 5};
    vector<int> v = {3, 4, 5, 6};
    cout << "[0/1 knapsack] " << zeroOneKnapsack(w, v, 5) << '\n';
    cout << "[unbounded knapsack] " << unboundedKnapsack(w, v, 7) << '\n';
    return 0;
}
