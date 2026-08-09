/*
문제 ID/제목: BOJ 1149 - RGB거리
출처: https://www.acmicpc.net/problem/1149

문제 요약: 일렬로 놓인 N개의 집을 빨강, 초록, 파랑 중 하나로 칠한다. 서로 이웃한 집은
같은 색일 수 없다. 집마다 각 색의 비용이 주어질 때 모든 집을 칠하는 최소 비용을 구한다.
입력: 첫 줄에 집 수 N, 다음 N줄에 각 집의 빨강·초록·파랑 비용이 차례로 주어진다.
출력: 조건을 만족하는 최소 총비용 하나를 출력한다.
제약: 2 이상인 일반 입력뿐 아니라 구현 검증을 위해 N=1도 자연스럽게 처리한다. 공식 문제의
N은 최대 1,000이고 각 비용은 양의 정수이므로 int 범위에서 안전하다.
예제: 3 / (26,40,83), (49,60,57), (13,89,99)이면 최소 비용은 96이다.
*/

// <algorithm>은 두 값 중 작은 값을 고르는 std::min을 제공한다.
#include <algorithm>
// <array>는 크기가 3으로 고정된 연속 컨테이너 std::array를 제공한다.
#include <array>
// <iostream>은 표준 입력 std::cin과 표준 출력 std::cout을 제공한다.
#include <iostream>

// 구현 근처의 공용 알고리즘 문서: ../algorithm/dynamic-programming-state-transition.md
// using 타입 별칭으로 “색 3개의 비용/상태”라는 의미를 템플릿 타입에 부여한다.
using Costs = std::array<int, 3>;

int main() {
    // 입출력 동기화를 끄고 묶음을 해제해 많은 정수 입력의 부가 비용을 줄인다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int house_count{}; // int 변수를 중괄호로 0 초기화한다.
    std::cin >> house_count; // >> 연산자가 입력 문자를 int로 변환해 저장한다.

    Costs previous{}; // 불변식: 처리한 마지막 집의 색별 최소 누적 비용이다.
    // for 반복문은 index를 0부터 N-1까지 1씩 증가시켜 집을 한 번씩 처리한다.
    for (int index{}; index < house_count; ++index) {
        Costs cost{}; // 현재 집의 세 색 비용을 모두 0으로 값 초기화한다.
        std::cin >> cost[0] >> cost[1] >> cost[2]; // [] 연산자로 고정 배열의 원소에 접근한다.

        // 첫 집에는 이전 집 제약이 없으므로 입력 비용 자체가 각 상태의 최소 비용이다.
        if (index == 0) {
            previous = cost; // array의 대입은 세 int 값을 복사한다.
            continue; // 아래 점화식을 건너뛰고 다음 반복으로 이동한다.
        }

        // 점화식: 현재 빨강은 직전 초록/파랑 중 작은 비용 뒤에만 붙일 수 있다.
        const Costs current{
            cost[0] + std::min(previous[1], previous[2]),
            cost[1] + std::min(previous[0], previous[2]),
            cost[2] + std::min(previous[0], previous[1])};
        // current는 const lvalue라서 previous로 세 값을 복사한다. 작은 고정 배열은 소유권 이동 이점이 없다.
        previous = current;
    }

    // 중첩 min 호출은 마지막 색 세 경우 중 전체 최소를 고른다. 전체 시간 O(N), DP 추가 공간 O(1)이다.
    const int answer{std::min(previous[0], std::min(previous[1], previous[2]))};
    std::cout << answer << '\n'; // 결과와 줄바꿈을 출력한다.
    // 함수 호출·배열 로드·비교·조건 분기의 실제 기계 명령은 CPU, ABI, 컴파일러, 최적화 옵션에 따라 달라진다.
    return 0; // 운영체제에 정상 종료 상태를 반환한다.
}
