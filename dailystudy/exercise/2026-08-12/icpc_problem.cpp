/*
문제 ID/제목: BOJ 1806 - 부분합
출처: https://www.acmicpc.net/problem/1806

문제 요약: 길이 N인 양의 정수 수열에서 연속한 일부 원소의 합이 S 이상인 구간 중
길이가 가장 짧은 것을 찾는다. 그런 구간이 없으면 0을 출력한다.
입력: 첫 줄에 N과 S, 둘째 줄에 N개의 양의 정수가 주어진다.
출력: 합이 S 이상인 가장 짧은 연속 부분 수열의 길이, 불가능하면 0을 출력한다.
제약: 10 <= N < 100,000, 0 < S <= 100,000,000, 각 원소는 10,000 이하의 자연수다.
예제: 입력 `10 15 / 5 1 3 5 10 7 4 9 2 8`의 출력은 `2`다. 5와 10 같은 길이 2 구간이 조건을 만족한다.
*/

// <algorithm>은 더 작은 값을 고르는 std::min을 제공한다.
#include <algorithm>
// <iostream>은 빠른 표준 입력과 출력을 제공한다.
#include <iostream>
// <limits>는 int 타입의 최댓값을 제공한다.
#include <limits>
// <vector>는 입력 수열을 연속 메모리에 소유하는 가변 길이 컨테이너다.
#include <vector>

int main() {
    std::ios::sync_with_stdio(false); // C와 C++ 스트림 동기화를 꺼 입력 비용을 줄인다.
    std::cin.tie(nullptr); // nullptr 포인터 값으로 자동 출력 flush 연결을 해제한다.

    int n{};              // int 변수 n을 중괄호로 0 초기화한다.
    long long target{};   // 합은 int 범위를 넘을 수 있어 long long을 사용한다.
    std::cin >> n >> target; // >> 연산자가 공백으로 구분된 두 값을 읽는다.

    std::vector<int> values(static_cast<std::size_t>(n)); // 템플릿 인자 int인 vector가 N개 원소를 소유한다.
    for (int& value : values) { // 비-const lvalue 참조로 각 원소에 입력을 직접 저장한다.
        std::cin >> value;
    }

    // 알고리즘 문서: ../algorithm/two-pointers-sliding-window.md
    int left{}; // 반열린 현재 창 [left, right]의 왼쪽 끝 인덱스다.
    long long sum{}; // 불변식: 아래 반복 시작 시 sum은 아직 제거하지 않은 창 원소의 합이다.
    int best{std::numeric_limits<int>::max()}; // 아직 답이 없음을 int 최댓값 센티널로 표현한다.

    // 모든 수가 양수이므로 right를 늘리면 합이 증가하고 left를 늘리면 감소한다.
    for (int right{}; right < n; ++right) { // 각 원소를 오른쪽 끝으로 정확히 한 번 추가한다.
        sum += values[static_cast<std::size_t>(right)]; // +=는 현재 합에 새 원소를 누적한다.

        // 합이 충분한 동안 왼쪽을 줄이면, 현재 right에서 가능한 최소 길이를 찾는다.
        while (sum >= target) { // 비교 결과에 따라 반복 조건 분기가 일어난다.
            best = std::min(best, right - left + 1); // 함수 템플릿 호출로 지금까지의 최솟값을 갱신한다.
            sum -= values[static_cast<std::size_t>(left)]; // 왼쪽 원소를 창의 합에서 제거한다.
            ++left; // 전위 증가 연산자로 왼쪽 포인터를 한 칸 옮긴다.
        }
    }

    // 두 포인터는 각각 최대 N번 이동하므로 시간 O(N), 입력 벡터 공간 O(N), 추가 공간 O(1)이다.
    // 로드·덧셈·비교·조건 분기의 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    std::cout << (best == std::numeric_limits<int>::max() ? 0 : best) << '\n';
    return 0;
}
