/*
Chapter 12. 1차원 DP 기본

문제 1: 계단을 1칸 또는 2칸씩 오를 때 N번째 계단까지 가는 방법 수를 구하라.
접근: dp[i] = dp[i - 1] + dp[i - 2]

문제 2: 동전들로 amount를 만드는 최소 동전 수를 구하라.
접근: dp[x] = min(dp[x], dp[x - coin] + 1)
정답 해설:
- 각 금액 x에 대해 마지막으로 사용한 동전을 하나 가정한다.
- 불가능 상태는 큰 값 INF로 둔다.

문제 3: 인접한 집을 동시에 털 수 없을 때 최대 금액을 구하라.
접근: dp[i] = max(dp[i - 1], dp[i - 2] + money[i])

DP 실전 질문:
1. 상태가 무엇인가?
2. 마지막 선택은 무엇인가?
3. 초기값과 불가능 상태는 어떻게 둘 것인가?
4. 답은 어느 상태에 있는가?
*/

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

long long climbStairs(int n) {
    if (n <= 1) return 1;
    vector<long long> dp(n + 1, 0);
    dp[0] = 1;
    dp[1] = 1;
    for (int i = 2; i <= n; ++i) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }
    return dp[n];
}

int minCoins(const vector<int>& coins, int amount) {
    const int INF = 1e9;
    vector<int> dp(amount + 1, INF);
    dp[0] = 0;

    for (int x = 1; x <= amount; ++x) {
        for (int coin : coins) {
            if (x >= coin && dp[x - coin] != INF) {
                dp[x] = min(dp[x], dp[x - coin] + 1);
            }
        }
    }
    return dp[amount] == INF ? -1 : dp[amount];
}

int houseRobber(const vector<int>& money) {
    int prev2 = 0;
    int prev1 = 0;
    for (int x : money) {
        int cur = max(prev1, prev2 + x);
        prev2 = prev1;
        prev1 = cur;
    }
    return prev1;
}

int maxSubarrayDp(const vector<int>& a) {
    int bestEnding = a[0];
    int best = a[0];
    for (int i = 1; i < (int)a.size(); ++i) {
        bestEnding = max(a[i], bestEnding + a[i]);
        best = max(best, bestEnding);
    }
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "[climb 5] " << climbStairs(5) << '\n';
    cout << "[min coins 11] " << minCoins({1, 2, 5}, 11) << '\n';
    cout << "[house robber] " << houseRobber({2, 7, 9, 3, 1}) << '\n';
    cout << "[max subarray dp] " << maxSubarrayDp({-2, 1, -3, 4, -1, 2, 1, -5, 4}) << '\n';
    return 0;
}
