/*
Chapter 05. 해시맵, 카운팅, 좌표 압축

문제 1: 배열에서 합이 target인 두 수의 인덱스를 찾아라.
접근: 지금 값 x를 보며 target - x가 이전에 있었는지 unordered_map으로 확인한다.
복잡도: 평균 O(N)

문제 2: 값의 범위가 큰 배열을 0부터 시작하는 순위로 압축하라.
접근: 정렬한 복사본에서 중복 제거 후 lower_bound로 위치를 찾는다.

문제 3: 가장 많이 등장한 값을 구하라. 동률이면 작은 값을 고른다.
접근: 빈도 map을 만들고 규칙대로 갱신한다.

실전 포인트:
- unordered_map은 평균 O(1)이지만 해시 충돌 최악 O(N)이다.
- 키가 정수이고 범위가 작으면 vector count가 더 빠르고 안정적이다.
*/

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

pair<int, int> twoSumIndices(const vector<int>& a, int target) {
    unordered_map<int, int> indexOf;
    indexOf.reserve(a.size() * 2 + 1);

    for (int i = 0; i < (int)a.size(); ++i) {
        int need = target - a[i];
        auto it = indexOf.find(need);
        if (it != indexOf.end()) return {it->second, i};
        indexOf[a[i]] = i;
    }
    return {-1, -1};
}

vector<int> coordinateCompress(const vector<int>& a) {
    vector<int> values = a;
    sort(values.begin(), values.end());
    values.erase(unique(values.begin(), values.end()), values.end());

    vector<int> compressed;
    compressed.reserve(a.size());
    for (int x : a) {
        compressed.push_back((int)(lower_bound(values.begin(), values.end(), x) - values.begin()));
    }
    return compressed;
}

int mostFrequentSmallestTie(const vector<int>& a) {
    map<int, int> freq;
    for (int x : a) ++freq[x];

    int bestValue = 0;
    int bestCount = -1;
    for (auto [value, count] : freq) {
        if (count > bestCount) {
            bestCount = count;
            bestValue = value;
        }
    }
    return bestValue;
}

bool canFormPalindrome(const string& s) {
    unordered_map<char, int> freq;
    for (char c : s) ++freq[c];

    int odd = 0;
    for (const auto& entry : freq) {
        int count = entry.second;
        odd += count % 2;
    }
    return odd <= 1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> a = {10, 7, 3, 2, 7, 10, -1};
    auto [i, j] = twoSumIndices(a, 9);
    cout << "[two sum target 9] " << i << ' ' << j << '\n';

    auto compressed = coordinateCompress({100, -5, 100, 7, -5});
    cout << "[compressed]";
    for (int x : compressed) cout << ' ' << x;
    cout << '\n';

    cout << "[mode] " << mostFrequentSmallestTie(a) << '\n';
    cout << "[palindrome permutation civic] " << boolalpha << canFormPalindrome("civic") << '\n';
    return 0;
}
