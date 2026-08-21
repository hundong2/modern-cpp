/*
문제 ID·제목: BOJ 11404 - 플로이드
출처: https://www.acmicpc.net/problem/11404 (Baekjoon Online Judge)

문제 요약:
1번부터 N번까지 번호가 붙은 도시와, 한 도시에서 다른 도시로 가는 단방향 버스가 있다. 같은 출발지와
도착지를 잇는 버스가 여러 개일 수 있다. 모든 출발 도시와 도착 도시 쌍에 대해 필요한 최소 비용을 구한다.
아래 설명은 원문의 요지를 한글로 다시 쓴 것이며 저작권이 있는 문제 원문 전체를 복제하지 않는다.

입력:
첫 줄에 도시 수 N, 둘째 줄에 버스 수 M이 주어진다. 다음 M줄에는 출발 도시 a, 도착 도시 b,
한 번 타는 비용 c가 주어진다.

출력:
N개의 줄에 N개의 값을 출력한다. i행 j열은 i에서 j로 가는 최소 비용이다. 갈 수 없는 도시 쌍은 0을
출력하고, 같은 도시로 가는 비용도 0으로 출력한다.

제약:
N은 2 이상 100 이하(경계 검증에서는 단일 도시도 안전하게 처리), M은 최대 100,000이고 각 버스 비용은
100,000 이하의 자연수다. 경로 합은 int 범위 안이지만, INF 덧셈과 확장성을 안전하게 다루려고 long long을 쓴다.

예제:
입력
5
14
1 2 2
1 3 3
1 4 1
1 5 10
2 4 2
3 4 1
3 5 1
4 5 3
3 5 10
3 1 8
1 4 2
5 1 7
3 4 2
5 2 4

출력
0 2 3 1 4
12 0 15 2 5
8 5 0 1 1
10 7 13 0 3
7 4 10 6 0
*/

// <algorithm>은 병렬 간선 중 작은 비용을 고르는 std::min 함수 템플릿을 선언한다.
#include <algorithm>
// <cstddef>는 컨테이너 인덱스와 크기에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력·출력과 고속 입출력 설정을 선언한다.
#include <iostream>
// <limits>는 long long의 표현 한계를 질의하는 std::numeric_limits를 선언한다.
#include <limits>
// <vector>는 N×N 거리 행렬을 동적으로 소유하는 std::vector를 선언한다.
#include <vector>

// 공용 알고리즘 문서: ../algorithm/floyd-warshall-all-pairs-shortest-path.md
int main() {
    // ios::sync_with_stdio(false)는 C/C++ 표준 스트림 동기화를 끄고 이전 bool 상태를 반환한다.
    // false는 유일한 데이터 인자이고 반환값은 버린다. 뒤에는 C stdio와 임의 순서로 섞지 않으며 전역 설정이 바뀐다.
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 ostream* 널 포인터를 받아 입력 전 자동 flush 연결을 해제한다.
    // 이전 ostream* 반환은 버리고 스트림 객체의 소유권은 바꾸지 않는다. 대화형 문제가 아니어서 수동 flush가 필요 없다.
    std::cin.tie(nullptr);

    int city_count{}; // int{}는 0으로 값 초기화되며 문제의 N을 저장한다.
    // operator>>(int&)는 city_count lvalue를 갱신하고 입력 위치를 전진시키며 istream&를 bool 문맥에 반환한다.
    // 입력 실패면 상태 비트만 남고 예외는 기본 비활성이다. 실패 시 더 계산하지 않고 정상 종료한다.
    if (!(std::cin >> city_count)) {
        return 0;
    }

    // numeric_limits<long long>::max()는 인자 없이 long long 최댓값을 constexpr 값으로 반환한다.
    // 4로 나눠 유한 거리 두 개를 더해도 오버플로하지 않는 충분히 큰 sentinel을 만든다.
    constexpr long long infinity{std::numeric_limits<long long>::max() / 4LL};
    // vector<long long>(count, value)는 1번 도시 인덱스를 그대로 쓰려고 N+1개의 infinity 복사 원소를 소유한다.
    // 바깥 vector(count, inner)는 안쪽 행을 N+1번 복사해 정방 행렬을 만든다. 시간·공간 O(N^2), 할당/bad_alloc 가능성이 있다.
    // 각 행은 별도 버퍼여서 바깥 vector 재할당 시 행 객체 관찰자가 무효화될 수 있으나 이후 크기를 바꾸지 않는다.
    std::vector<std::vector<long long>> distance(
        static_cast<std::size_t>(city_count + 1),
        std::vector<long long>(static_cast<std::size_t>(city_count + 1), infinity));

    // 자기 자신으로 가는 빈 경로 비용은 0이다. 대각선을 먼저 설정하면 점화식의 항등 경로가 된다.
    for (int city{1}; city <= city_count; ++city) {
        distance[static_cast<std::size_t>(city)][static_cast<std::size_t>(city)] = 0LL;
    }

    int bus_count{};
    // 추출 연산은 bus_count를 갱신하며 반환 istream&는 연쇄가 끝나 버린다. 올바른 입력을 문제 계약이 보장한다.
    std::cin >> bus_count;
    // 정확히 M개의 단방향 간선을 읽는다.
    for (int edge{}; edge < bus_count; ++edge) {
        int from{};
        int to{};
        long long cost{};
        // 세 operator>>가 각각 lvalue를 갱신하고 같은 istream&를 다음 호출에 넘긴다. 최종 참조는 버린다.
        std::cin >> from >> to >> cost;
        long long& direct{distance[static_cast<std::size_t>(from)][static_cast<std::size_t>(to)]};
        // std::min<long long>(const T&, const T&)은 direct와 cost를 빌려 작은 값의 const 참조를 반환한다.
        // 그 값을 direct에 복사 대입한다. 두 입력은 수정하지 않고 O(1), 무할당이며 같은 방향 병렬 간선의 최솟값만 남긴다.
        direct = std::min(direct, cost);
    }

    // 불변식: via-1 반복이 끝나면 distance[i][j]는 중간 도시로 1..via만 허용한 최소 비용이다.
    // 각 단계는 via를 쓰지 않는 기존 경로와 i->via->j 경로 중 작은 쪽을 선택한다.
    for (int via{1}; via <= city_count; ++via) {
        for (int from{1}; from <= city_count; ++from) {
            // i에서 via로 갈 수 없으면 어떤 j도 via를 경유할 수 없으므로 안쪽 반복 전체를 건너뛴다.
            if (distance[static_cast<std::size_t>(from)][static_cast<std::size_t>(via)] == infinity) {
                continue;
            }
            for (int to{1}; to <= city_count; ++to) {
                // via에서 j로 갈 수 없으면 sentinel과 덧셈하지 않고 이 쌍만 건너뛴다.
                if (distance[static_cast<std::size_t>(via)][static_cast<std::size_t>(to)] == infinity) {
                    continue;
                }
                const long long through{
                    distance[static_cast<std::size_t>(from)][static_cast<std::size_t>(via)] +
                    distance[static_cast<std::size_t>(via)][static_cast<std::size_t>(to)]};
                long long& best{distance[static_cast<std::size_t>(from)][static_cast<std::size_t>(to)]};
                // min은 기존 최적값과 via 경유 후보를 빌려 작은 값을 반환하고 best가 그 값을 복사한다.
                // O(1), 무할당이며 갱신 뒤 불변식이 via까지 확장된다.
                best = std::min(best, through);
            }
        }
    }

    // 행 우선 순서로 N×N 답을 출력한다. 출력 자체도 O(N^2)이다.
    for (int from{1}; from <= city_count; ++from) {
        for (int to{1}; to <= city_count; ++to) {
            const long long value{distance[static_cast<std::size_t>(from)][static_cast<std::size_t>(to)]};
            const long long printable{value == infinity ? 0LL : value};
            // operator<<는 printable과 구분 문자를 출력 버퍼에 쓰고 ostream&를 연쇄 반환한다.
            // 수신 cout 상태/출력 위치가 바뀌고 값은 유지되며 실패는 기본적으로 상태 비트에 남는다.
            std::cout << printable << (to == city_count ? '\n' : ' ');
        }
    }

    // 세 중첩 반복은 O(N^3), 거리 행렬은 O(N^2) 공간이다.
    return 0;
}
