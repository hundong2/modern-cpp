/*
문제 ID/제목: BOJ 11053 - 가장 긴 증가하는 부분 수열
출처: https://www.acmicpc.net/problem/11053
문제 요약: 수열에서 일부 원소를 원래 순서대로 골라 값이 엄격히 증가하도록 만들 때 가능한 최대 길이를 구한다.
입력: 첫 줄에 수열 길이 N, 둘째 줄에 N개의 정수 A_i가 주어진다.
출력: 가장 긴 증가하는 부분 수열(LIS)의 길이 하나를 출력한다.
제약: 1 <= N <= 1,000, 1 <= A_i <= 1,000. 같은 값은 엄격한 증가 관계가 아니다.
예제: 입력이 6 / 10 20 10 30 20 50 이면 10,20,30,50을 고를 수 있으므로 출력은 4다.
*/

// <algorithm>은 최댓값을 갱신하는 std::max를 제공한다.
#include <algorithm>
// <iostream>은 입력 std::cin과 출력 std::cout을 제공한다.
#include <iostream>
// <vector>는 수열과 DP 표를 연속 메모리에 소유한다.
#include <vector>

int main() {
    // 알고리즘 지식 문서: ../algorithm/longest-increasing-subsequence.md
    // 입출력 동기화를 끄고 묶음을 해제하여 온라인 저지의 많은 정수 입출력을 빠르게 한다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr); // nullptr은 아무 출력 스트림도 가리키지 않는 포인터 값이다.

    int n{}; // 기본 타입 int를 0으로 중괄호 초기화한다.
    std::cin >> n; // >> 연산자는 표준 입력에서 값을 읽어 n에 저장한다.
    std::vector<int> values(static_cast<std::size_t>(n)); // N개 int를 소유하며 각 원소를 0으로 초기화한다.
    // 참조 반복 변수 int&는 각 vector 원소 lvalue에 바인딩되어 입력으로 원소를 직접 수정한다.
    for (int& value : values) {
        std::cin >> value;
    }

    // dp[i]는 i에서 끝나는 LIS 길이이며 자기 자신만 고르는 길이 1로 초기화한다.
    std::vector<int> dp(static_cast<std::size_t>(n), 1);
    int answer{1}; // 지금까지 계산한 모든 dp 값의 최댓값이라는 불변식을 유지한다.

    // 바깥 반복문은 i보다 작은 상태가 모두 확정된 순서로 DP를 계산한다.
    for (int i{0}; i < n; ++i) {
        // 안쪽 반복문은 i 앞의 모든 후보 j를 검사한다. 전체 시간 복잡도는 O(N^2)이다.
        for (int j{0}; j < i; ++j) {
            // 엄격한 증가 조건을 만족할 때만 j에서 끝난 부분 수열 뒤에 values[i]를 붙인다.
            if (values[static_cast<std::size_t>(j)] < values[static_cast<std::size_t>(i)]) {
                // 점화식 dp[i] = max(dp[i], dp[j] + 1)로 최선의 이전 끝점을 선택한다.
                dp[static_cast<std::size_t>(i)] = std::max(dp[static_cast<std::size_t>(i)], dp[static_cast<std::size_t>(j)] + 1);
            }
        }
        answer = std::max(answer, dp[static_cast<std::size_t>(i)]); // 최종 답 후보를 갱신한다.
    }

    std::cout << answer << '\n'; // 추가 공간은 values와 dp 때문에 O(N)이다.
    // 위 연산은 메모리 로드·비교·조건 분기·함수 호출로 번역될 수 있으나 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return 0;
}
