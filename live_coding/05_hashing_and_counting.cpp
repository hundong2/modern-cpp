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
    unordered_map<int, int> indexOf; // unordered_map은 hash table이라 "값 -> 인덱스" 조회를 평균 O(1)에 처리한다.
    indexOf.reserve(a.size() * 2 + 1); // rehash를 줄여 평균 성능을 안정화한다.

    for (int i = 0; i < (int)a.size(); ++i) {
        int need = target - a[i];
        auto it = indexOf.find(need); // 현재 값과 짝이 되는 이전 값을 찾는다.
        if (it != indexOf.end()) return {it->second, i};
        indexOf[a[i]] = i; // 같은 값이 여러 번 나오면 더 최근 인덱스로 갱신한다.
    }
    return {-1, -1};
}

vector<int> coordinateCompress(const vector<int>& a) {
    vector<int> values = a; // vector 복사는 O(N)이다. 여기서는 원본 순서 보존과 정렬 기준 생성을 위해 의도적으로 복사한다.
    sort(values.begin(), values.end());
    values.erase(unique(values.begin(), values.end()), values.end()); // 압축 좌표의 기준 목록이다.

    vector<int> compressed;
    compressed.reserve(a.size());
    for (int x : a) {
        compressed.push_back((int)(lower_bound(values.begin(), values.end(), x) - values.begin())); // 정렬된 위치가 압축값이다.
    }
    return compressed;
}

int mostFrequentSmallestTie(const vector<int>& a) {
    map<int, int> freq; // map은 red-black tree 기반이라 key 오름차순 순회가 필요할 때 unordered_map보다 적합하다.
    for (int x : a) ++freq[x]; // map의 operator[]는 없던 키를 0으로 만든 뒤 증가시킨다.

    int bestValue = 0;
    int bestCount = -1; // 빈도 0도 정답 후보가 되지 않도록 -1에서 시작한다.
    for (auto [value, count] : freq) {
        if (count > bestCount) { // map 순서상 같은 count면 먼저 나온 작은 값이 유지된다.
            bestCount = count;
            bestValue = value;
        }
    }
    return bestValue;
}

bool canFormPalindrome(const string& s) {
    unordered_map<char, int> freq; // 문자 종류가 작으면 vector<int>(256)도 가능하지만, 여기서는 일반 해시맵 예제로 둔다.
    for (char c : s) ++freq[c]; // 각 문자의 등장 횟수 parity만 중요하다.

    int odd = 0;
    for (const auto& entry : freq) {
        int count = entry.second;
        odd += count % 2; // 홀수 개수 문자가 2개 이상이면 팰린드롬 배치가 불가능하다.
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
