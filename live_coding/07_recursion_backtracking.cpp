/*
Chapter 07. 재귀, 백트래킹, pruning

문제 1: 주어진 수들로 만들 수 있는 순열을 모두 출력하라.
접근: used 배열과 path를 사용해 깊이 우선으로 선택한다.

문제 2: 합이 target인 부분집합이 존재하는지 판정하라.
접근: 각 원소를 고르거나 건너뛰는 재귀. 정렬 후 남은 합으로 pruning 가능.

문제 3: N-Queens 해의 개수를 구하라.
접근: row마다 queen을 하나 놓고 column/diagonal 충돌을 체크한다.
복잡도: 최악 O(N!), pruning으로 실제 탐색을 줄인다.

실전 포인트:
- 재귀 함수 인자는 "현재 위치, 현재 상태, 정답 후보"만 남겨 단순하게 만든다.
- 되돌리기(pop, used=false)를 빠뜨리는 실수가 가장 흔하다.
*/

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

void permuteDfs(const vector<int>& nums, vector<int>& path, vector<bool>& used, vector<vector<int>>& result) {
    if (path.size() == nums.size()) {
        result.push_back(path); // path는 이후 pop으로 바뀌므로 현재 완성본을 복사해 저장한다.
        return;
    }

    for (int i = 0; i < (int)nums.size(); ++i) {
        if (used[i]) continue;
        used[i] = true;       // 현재 깊이에서 nums[i]를 선택한다.
        path.push_back(nums[i]);
        permuteDfs(nums, path, used, result);
        path.pop_back();      // 다음 후보를 위해 선택을 되돌린다.
        used[i] = false;
    }
}

vector<vector<int>> permutations(vector<int> nums) {
    sort(nums.begin(), nums.end()); // 출력 순서를 안정적으로 만들기 위한 정렬이다.
    vector<vector<int>> result;
    vector<int> path;                    // 현재까지 선택한 순열 prefix다.
    vector<bool> used(nums.size(), false); // vector<bool>은 bit 압축 특수화다. 메모리는 아끼지만 일반 bool&처럼 동작하지 않는 점을 알아둔다.
    permuteDfs(nums, path, used, result);
    return result;
}

bool subsetSumDfs(const vector<int>& a, int idx, int target, int current) {
    if (current == target) return true;
    if (idx == (int)a.size()) return false;
    if (current > target) return false; // 양수 배열로 정렬했기 때문에 target 초과는 더 볼 필요가 없다.

    if (subsetSumDfs(a, idx + 1, target, current + a[idx])) return true; // 현재 원소를 고르는 경우다.
    return subsetSumDfs(a, idx + 1, target, current);                    // 현재 원소를 건너뛰는 경우다.
}

bool hasSubsetSum(vector<int> a, int target) {
    sort(a.begin(), a.end()); // pruning 조건 current > target이 잘 작동하도록 양수 작은 값부터 본다.
    return subsetSumDfs(a, 0, target, 0);
}

int countNQueensDfs(int n, int row, vector<bool>& col, vector<bool>& diag1, vector<bool>& diag2) {
    if (row == n) return 1;

    int count = 0;
    for (int c = 0; c < n; ++c) {
        int d1 = row - c + n - 1; // 음수가 될 수 있는 대각선 번호를 0 이상으로 보정한다.
        int d2 = row + c;         // 다른 방향 대각선은 row+col이 같으면 충돌한다.
        if (col[c] || diag1[d1] || diag2[d2]) continue; // 열과 두 대각선 중 하나라도 막히면 놓을 수 없다.

        col[c] = diag1[d1] = diag2[d2] = true;
        count += countNQueensDfs(n, row + 1, col, diag1, diag2);
        col[c] = diag1[d1] = diag2[d2] = false; // 백트래킹 복구가 없으면 다음 열 탐색이 오염된다.
    }
    return count;
}

int countNQueens(int n) {
    vector<bool> col(n, false), diag1(2 * n - 1, false), diag2(2 * n - 1, false); // 열/대각선 점유 상태를 O(1)에 확인한다.
    return countNQueensDfs(n, 0, col, diag1, diag2);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    auto perms = permutations({1, 2, 3});
    cout << "[permutation count] " << perms.size() << '\n';

    cout << "[subset sum 9] " << boolalpha << hasSubsetSum({3, 34, 4, 12, 5, 2}, 9) << '\n';
    cout << "[n queens 8] " << countNQueens(8) << '\n';
    return 0;
}
