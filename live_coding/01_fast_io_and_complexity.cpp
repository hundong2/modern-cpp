/*
Chapter 01. 빠른 입출력, 복잡도 감각, 기본 루프 패턴

문제 1: N개의 정수가 주어졌을 때 합, 최솟값, 최댓값을 구하라.
접근: 입력을 한 번만 훑으며 필요한 값을 갱신한다.
정답 해설:
- 모든 원소를 반드시 읽어야 하므로 O(N)보다 빠를 수 없다.
- 합은 int 범위를 넘을 수 있으니 long long을 기본으로 둔다.
복잡도: O(N), 메모리 O(1)

문제 2: 배열에서 연속 부분 배열의 최대 합을 구하라. 빈 배열은 허용하지 않는다.
접근: Kadane 알고리즘. i에서 끝나는 최대 합만 들고 간다.
정답 해설:
- bestEndingHere = max(a[i], bestEndingHere + a[i])
- 전체 정답은 지금까지의 max(bestEndingHere)
복잡도: O(N), 메모리 O(1)

실전 포인트:
- C++ 라이브 코딩에서는 main 첫 줄에 ios::sync_with_stdio(false), cin.tie(nullptr)를 습관화한다.
- 대략 1초 기준 O(N log N)은 N=1e6 근처까지, O(N^2)은 N=5e3 정도부터 위험하다.
*/

#include <algorithm>
#include <climits>
#include <iostream>
#include <stdexcept>
#include <vector>
using namespace std;

struct Summary {
    long long sum;
    int mn;
    int mx;
};

Summary summarize(const vector<int>& a) {
    Summary res{0, INT_MAX, INT_MIN};
    for (int x : a) {
        res.sum += x;
        res.mn = min(res.mn, x);
        res.mx = max(res.mx, x);
    }
    return res;
}

long long maxSubarraySum(const vector<int>& a) {
    if (a.empty()) throw invalid_argument("array must not be empty");

    long long bestEndingHere = a[0];
    long long best = a[0];
    for (int i = 1; i < (int)a.size(); ++i) {
        bestEndingHere = max<long long>(a[i], bestEndingHere + a[i]);
        best = max(best, bestEndingHere);
    }
    return best;
}

vector<int> readVectorFromStdin() {
    int n;
    if (!(cin >> n)) return {};

    vector<int> a(n);
    for (int& x : a) cin >> x;
    return a;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> demo = {3, -2, 5, -1, 6, -9, 4};

    Summary s = summarize(demo);
    cout << "[summary] sum=" << s.sum << " min=" << s.mn << " max=" << s.mx << '\n';
    cout << "[kadane] max subarray sum=" << maxSubarraySum(demo) << '\n';

    /*
    입력형으로 바꾸려면:
    vector<int> a = readVectorFromStdin();
    if (!a.empty()) {
        Summary s = summarize(a);
        cout << s.sum << ' ' << s.mn << ' ' << s.mx << '\n';
        cout << maxSubarraySum(a) << '\n';
    }
    */
    return 0;
}
