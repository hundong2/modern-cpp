/*
문제 ID/제목: BOJ 1654 - 랜선 자르기
출처: https://www.acmicpc.net/problem/1654

문제 요약: 이미 가진 K개의 랜선을 모두 같은 정수 길이로 잘라 N개 이상의 랜선을 만들려 한다.
잘라서 남는 조각은 버릴 수 있다. 만들 수 있는 랜선 길이 가운데 가장 큰 값을 구한다.
입력: 첫 줄에 K와 N, 다음 K줄에 각 랜선의 양의 길이가 주어진다.
출력: N개 이상 만들 수 있는 동일 길이의 최댓값을 출력한다.
제약: 1 <= K <= 10,000, K <= N <= 1,000,000, 각 길이는 2^31-1 이하의 자연수다.
예제: K=4, N=11이고 길이가 802, 743, 457, 539이면 정답은 200이다.
*/

// <algorithm>은 범위의 최댓값을 찾는 std::max_element를 제공한다.
#include <algorithm>
// <iostream>은 표준 입력 std::cin과 출력 std::cout을 제공한다.
#include <iostream>
// <vector>는 입력 길이를 연속 메모리에 저장하는 동적 배열을 제공한다.
#include <vector>

// 알고리즘 설명: ../algorithm/binary-search-on-answer.md
// bool 반환형은 주어진 길이가 가능한지를 참/거짓으로 답한다.
bool can_make(const std::vector<long long>& cables, long long required, long long length) {
    long long made{0}; // long long은 최대 백만 개 이상의 합산에서도 정수 범위를 넉넉히 확보한다.
    for (const long long cable : cables) { // 범위 for가 모든 랜선을 한 번씩 읽는다: O(K).
        made += cable / length; // 정수 나눗셈 /는 이 랜선에서 얻는 완전한 조각 수를 계산한다.
        if (made >= required) { // 필요한 수를 채우면 더 더해도 판정은 변하지 않는다.
            return true; // 조기 반환으로 불필요한 반복을 생략한다.
        }
    }
    return false; // 끝까지 합쳐도 부족하므로 이 길이는 불가능하다.
}

int main() {
    std::ios::sync_with_stdio(false); // C와 C++ 스트림 동기화를 꺼 대량 입력을 빠르게 한다.
    std::cin.tie(nullptr); // 입력 전 자동 출력 flush 연결을 끊는다. nullptr은 아무 객체도 가리키지 않는다.

    int cable_count{}; // int 기본 타입 변수이며 {}로 0 초기화한다.
    long long required{};
    std::cin >> cable_count >> required; // >> 연산자가 공백 기준 정수를 변수에 저장한다.

    std::vector<long long> cables(static_cast<std::size_t>(cable_count)); // 명시적 캐스트로 벡터 크기 타입을 맞춘다.
    for (long long& cable : cables) { // 비const lvalue 참조로 각 벡터 원소에 직접 입력한다.
        std::cin >> cable;
    }

    long long low{1}; // 길이 0은 나눗셈이 불가능하므로 후보 최솟값은 1이다.
    long long high{*std::max_element(cables.begin(), cables.end())}; // 반복자 범위에서 가장 긴 값의 참조를 *로 읽는다.
    long long answer{0}; // 불변식: 지금까지 검사해 가능했던 길이 중 최댓값이다.

    // 알고리즘 설명: ../algorithm/binary-search-on-answer.md
    while (low <= high) { // 닫힌 구간 [low, high]가 비지 않은 동안 O(log M)회 반복한다.
        const long long mid{low + (high - low) / 2}; // 덧셈 오버플로 위험을 줄인 중간값 계산이다.
        if (can_make(cables, required, mid)) { // 함수 호출로 단조 결정 조건을 검사한다.
            answer = mid; // 가능한 후보를 기록한다.
            low = mid + 1; // 더 큰 가능한 길이를 찾기 위해 오른쪽 절반만 남긴다.
        } else {
            high = mid - 1; // 불가능한 mid 이상은 모두 불가능하므로 왼쪽 절반만 남긴다.
        }
    }

    std::cout << answer << '\n'; // 가능한 최대 길이와 개행을 출력한다.
    // 비교·조건 분기·함수 호출·로드·저장의 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return 0; // 정상 종료 상태를 운영체제에 반환한다.
}
