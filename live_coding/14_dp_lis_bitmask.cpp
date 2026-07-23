/*
Chapter 14. LIS, Edit Distance, Bitmask DP

문제 1: 최장 증가 부분 수열 길이를 구하라.
접근: tails[len] = 길이가 len+1인 증가 수열의 가능한 최소 마지막 값.
정답 해설:
- 새 값 x가 들어갈 첫 위치 lower_bound(tails, x)를 x로 교체한다.
- tails 자체가 실제 LIS는 아닐 수 있지만 길이는 정확하다.
복잡도: O(N log N)

문제 2: 두 문자열의 편집 거리(insert/delete/replace)를 구하라.
접근: dp[i][j] = a 앞 i글자를 b 앞 j글자로 바꾸는 최소 비용.

문제 3: 작은 N의 TSP를 풀어라.
접근: dp[mask][u] = 방문 집합 mask를 방문하고 u에 있을 때 최소 비용.
복잡도: O(N^2 * 2^N), N이 20을 넘으면 어렵다.
*/

#include <algorithm>
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

int lisLength(const vector<int>& a) {
    vector<int> tails; // tails는 길이가 동적으로 늘어나는 배열이며, lower_bound를 쓰려면 항상 정렬 상태를 유지해야 한다.
    for (int x : a) {
        auto it = lower_bound(tails.begin(), tails.end(), x); // lower_bound는 정렬된 vector에서 O(log N) iterator를 반환한다.
        if (it == tails.end()) tails.push_back(x);            // 모든 tail보다 크면 LIS 길이가 늘어난다.
        else *it = x;                                         // 더 작은 tail로 바꿔 이후 확장 가능성을 키운다.
    }
    return (int)tails.size();
}

int editDistance(const string& a, const string& b) {
    int n = (int)a.size();
    int m = (int)b.size();
    vector<vector<int>> dp(n + 1, vector<int>(m + 1, 0)); // dp[i][j]는 a 앞 i글자를 b 앞 j글자로 바꾸는 비용이다.

    for (int i = 0; i <= n; ++i) dp[i][0] = i; // b가 비어 있으면 i번 삭제해야 한다.
    for (int j = 0; j <= m; ++j) dp[0][j] = j; // a가 비어 있으면 j번 삽입해야 한다.

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1]; // 마지막 문자가 같으면 추가 비용이 없다.
            } else {
                dp[i][j] = 1 + min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]}); // delete, insert, replace 중 최소다.
            }
        }
    }
    return dp[n][m];
}

int tspBitmask(const vector<vector<int>>& cost) {
    int n = (int)cost.size();
    const int INF = 1e9;
    vector<vector<int>> dp(1 << n, vector<int>(n, INF)); // 큰 2차원 vector는 메모리 사용량이 커서 N 제한을 먼저 확인해야 한다.
    dp[1][0] = 0; // 0번 도시만 방문하고 0번에 있는 시작 상태다.

    for (int mask = 0; mask < (1 << n); ++mask) {
        for (int u = 0; u < n; ++u) {
            if (dp[mask][u] == INF) continue;
            for (int v = 0; v < n; ++v) {
                if (mask & (1 << v)) continue; // 이미 방문한 도시는 다시 방문하지 않는다.
                int nextMask = mask | (1 << v);
                dp[nextMask][v] = min(dp[nextMask][v], dp[mask][u] + cost[u][v]); // u에서 v로 이동해 방문 집합을 확장한다.
            }
        }
    }

    int all = (1 << n) - 1; // 모든 도시를 방문한 mask다.
    int answer = INF;
    for (int u = 0; u < n; ++u) {
        answer = min(answer, dp[all][u] + cost[u][0]); // 마지막 도시에서 시작 도시로 돌아오는 비용을 더한다.
    }
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "[lis] " << lisLength({10, 9, 2, 5, 3, 7, 101, 18}) << '\n';
    cout << "[edit distance] " << editDistance("kitten", "sitting") << '\n';

    vector<vector<int>> cost = {
        {0, 10, 15, 20},
        {10, 0, 35, 25},
        {15, 35, 0, 30},
        {20, 25, 30, 0}
    };
    cout << "[tsp] " << tspBitmask(cost) << '\n';
    return 0;
}
