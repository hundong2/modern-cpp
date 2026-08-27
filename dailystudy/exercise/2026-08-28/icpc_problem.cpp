/*
문제 요약
- 문제 ID·제목: BOJ 2261 「가장 가까운 두 점」
- 출처 URL: https://www.acmicpc.net/problem/2261
- 2차원 정수 좌표 점 N개 중 유클리드 거리가 가장 짧은 서로 다른 두 점을 찾는다.
- 실제 거리의 제곱을 출력하므로 제곱근이나 부동소수점 계산은 필요 없다.

입력
- 첫 줄에 점의 수 N이 주어진다.
- 이어지는 N줄마다 한 점의 정수 좌표 x, y가 주어진다.

출력
- 서로 다른 두 점 사이 거리 제곱 (x1-x2)^2 + (y1-y2)^2의 최솟값 한 개를 출력한다.

제약
- 2 <= N <= 100,000.
- 각 좌표의 절댓값은 10,000 이하이며 같은 좌표의 점이 여러 번 나올 수도 있다.
- 완전 탐색 O(N^2)은 비교 횟수가 너무 커서 통과할 수 없다.

예제
- 입력: 네 점 (0,0), (10,10), (0,10), (10,0).
- 출력: 100. 정사각형의 이웃한 꼭짓점 사이 거리 10의 제곱이다.
*/

// <algorithm>은 x/y 정렬 std::sort와 작은 값 선택 std::min을 선언한다.
#include <algorithm>
// <cstddef>는 배열 크기와 반열린 구간 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 온라인 저지 표준 입력 std::cin과 표준 출력 std::cout을 선언한다.
#include <iostream>
// <limits>는 64비트 정수의 안전한 초기 상한을 주는 std::numeric_limits를 선언한다.
#include <limits>
// <vector>는 점과 병합 작업 공간을 연속 메모리에 소유하는 std::vector를 선언한다.
#include <vector>

// 좌표 차와 제곱 합을 안전하게 담는 부호 있는 64비트 이상 정수 별칭이다.
using Distance = long long;

// struct 기본 public 접근은 단순 좌표 DTO에 적합하다.
struct Point {
    Distance x{}; // x축 정수 좌표를 0으로 값 초기화한다.
    Distance y{}; // y축 정수 좌표를 0으로 값 초기화한다.
};

// 두 const 참조는 점을 복사하지 않고 읽고 거리 제곱 값을 반환한다.
[[nodiscard]] Distance squared_distance(const Point& left, const Point& right) noexcept {
    // 뺄셈은 64비트 값으로 수행되어 좌표 차를 보존한다.
    const Distance dx{left.x - right.x};
    const Distance dy{left.y - right.y};
    // 곱셈과 덧셈으로 유클리드 거리 제곱을 계산하며 제곱근 오차를 피한다.
    return dx * dx + dy * dy;
}

// 공용 알고리즘 문서: ../algorithm/divide-and-conquer-closest-pair.md
// 불변식: 함수 진입 시 [begin,end)는 원래 x정렬 순서이고, 반환 시 같은 점들이 y정렬되어 있다.
// points와 scratch는 호출자가 소유하며 비const 참조로 빌려 제자리 정렬과 O(N) 작업 공간 재사용을 허용한다.
[[nodiscard]] Distance closest_pair(std::vector<Point>& points,
                                    std::vector<Point>& scratch,
                                    std::size_t begin,
                                    std::size_t end) {
    // size_t 뺄셈은 begin<=end 전제에서 부분 구간 점 수를 구한다.
    const std::size_t count{end - begin};

    // 점이 최대 3개면 모든 쌍 비교가 상수 횟수라 재귀를 멈춘다.
    if (count <= 3U) {
        // numeric_limits<Distance>::max()는 가능한 거리보다 큰 Distance 최댓값을 반환하며 상태를 바꾸지 않는다.
        Distance best{std::numeric_limits<Distance>::max()};
        for (std::size_t first{begin}; first < end; ++first) {
            for (std::size_t second{first + 1U}; second < end; ++second) {
                // std::min<Distance>(a,b)는 두 const Distance&를 읽고 더 작은 값의 const&를 반환한다.
                // 반환 참조를 즉시 값 대입해 저장하며 입력과 컨테이너는 유지되고 시간 O(1), 할당·예외는 없다.
                best = std::min(best, squared_distance(points[first], points[second]));
            }
        }

        // sort(first,last,comparison)의 수신 객체는 없고 두 random-access 반복자가 [begin,end)를 지정한다.
        // 람다 prvalue는 두 Point const&를 받아 strict weak ordering bool을 반환한다. 선택 템플릿은 Point 반복자/람다 타입이다.
        // 호출 뒤 해당 구간만 y,x 순으로 재배열되고 반복자·참조는 같은 원소 객체 위치 의미가 바뀐다. O(count log count), 추가 공간 구현 의존이다.
        // static_cast<ptrdiff_t>는 부호 없는 인덱스를 반복자 차이 타입으로 명시 변환하며 현재 N 범위는 표현 가능하다.
        std::sort(points.begin() + static_cast<std::ptrdiff_t>(begin),
                  points.begin() + static_cast<std::ptrdiff_t>(end),
                  [](const Point& left, const Point& right) {
                      return left.y < right.y || (left.y == right.y && left.x < right.x);
                  });
        return best;
    }

    // 반열린 구간 중앙을 고르면 [begin,middle), [middle,end) 두 부분 문제가 된다.
    const std::size_t middle{begin + count / 2U};
    // 재귀가 points를 y순으로 바꾸기 전에 x정렬 중앙 경계 좌표를 값 복사해 보존한다.
    const Distance middle_x{points[middle].x};

    // 두 재귀 호출은 서로 겹치지 않는 반쪽의 최솟값을 반환한다. std::min은 값으로 best를 초기화한다.
    Distance best{std::min(closest_pair(points, scratch, begin, middle),
                           closest_pair(points, scratch, middle, end))};

    // left/right는 이미 y정렬된 두 반쪽의 현재 위치, write는 scratch 출력 위치다.
    std::size_t left{begin};
    std::size_t right{middle};
    std::size_t write{begin};
    // 두 반복문 조건은 범위 밖 operator[] 접근을 막는다.
    while (left < middle && right < end) {
        // 오른쪽 점의 y가 작으면 오른쪽을, 아니면 왼쪽을 골라 안정적으로 병합한다.
        if (points[right].y < points[left].y) {
            scratch[write] = points[right];
            ++right;
        } else {
            scratch[write] = points[left];
            ++left;
        }
        ++write;
    }
    while (left < middle) {
        scratch[write] = points[left];
        ++left;
        ++write;
    }
    while (right < end) {
        scratch[write] = points[right];
        ++right;
        ++write;
    }
    // 병합 결과를 points에 되돌려 상위 호출이 이 구간을 y정렬됐다고 믿게 한다.
    for (std::size_t index{begin}; index < end; ++index) {
        points[index] = scratch[index];
    }

    // scratch의 같은 구간을 중앙선에서 거리 sqrt(best) 미만인 띠 점 저장소로 재사용한다.
    std::size_t strip_size{};
    for (std::size_t index{begin}; index < end; ++index) {
        const Distance dx{points[index].x - middle_x};
        if (dx * dx < best) {
            scratch[begin + strip_size] = points[index];
            ++strip_size;
        }
    }

    // 띠는 이미 y정렬이다. y 차 제곱이 best 이상이면 뒤 점은 모두 더 멀어 반복을 끝낸다.
    for (std::size_t first{}; first < strip_size; ++first) {
        for (std::size_t second{first + 1U}; second < strip_size; ++second) {
            const Distance dy{scratch[begin + second].y - scratch[begin + first].y};
            if (dy * dy >= best) {
                break;
            }
            // 평면 패킹 성질상 각 점에서 y가 가까운 상수 개 후보만 남아 전체 병합 단계가 O(count)다.
            best = std::min(best,
                            squared_distance(scratch[begin + first], scratch[begin + second]));
            // 중복 좌표로 0을 얻으면 더 작은 값이 없으므로 현재 호출을 즉시 끝낼 수 있다.
            if (best == 0) {
                return 0;
            }
        }
    }

    return best;
}

int main() {
    // sync_with_stdio(false)는 C/C++ 표준 스트림 동기화 이전 bool 상태를 반환하지만 사용하지 않는다.
    // 호출 뒤 C stdio와 iostream을 섞지 않는 전제에서 입출력 오버헤드를 줄이며 소유권·수명은 바뀌지 않는다.
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 입력 전 자동 flush 대상 ostream*를 nullptr로 바꾸고 이전 포인터는 무시한다.
    // 호출 뒤 대화형 출력은 직접 flush해야 하지만 오늘은 일괄 입출력이라 안전하다.
    std::cin.tie(nullptr);

    std::size_t point_count{};
    // operator>>(size_t&)는 cin lvalue와 출력 대상 lvalue를 받아 값을 쓰고 cin&를 반환한다.
    // 반환을 bool 문맥에서 검사하며 실패하면 point_count를 신뢰하지 않고 종료한다. 성공 뒤 입력 위치가 전진한다.
    if (!(std::cin >> point_count)) {
        return 0;
    }

    // vector(count)는 point_count개의 Point를 값 초기화해 소유한다. 생성자는 반환형이 없고 O(N) 시간·공간이다.
    // 할당 실패 시 bad_alloc, 성공 뒤 points.size()==point_count이고 재할당 전까지 원소 참조가 유효하다.
    std::vector<Point> points(point_count);
    for (Point& point : points) {
        // 두 추출 호출은 x,y lvalue에 순서대로 쓰고 같은 cin& 반환을 연쇄하며 point의 소유권은 유지된다.
        std::cin >> point.x >> point.y;
    }

    // sort는 전체 points를 x 우선, y 차선으로 재배열한다. begin/end는 반열린 반복자 쌍이며 반환형 void다.
    // 비교자는 strict weak ordering을 보장한다. O(N log N), 원소 주소/순서 관찰은 바뀌지만 vector 용량은 유지된다.
    std::sort(points.begin(), points.end(), [](const Point& left, const Point& right) {
        return left.x < right.x || (left.x == right.x && left.y < right.y);
    });

    // scratch는 같은 수의 Point를 한 번만 할당해 모든 재귀 깊이에서 비겹치게 재사용한다.
    std::vector<Point> scratch(point_count);
    // closest_pair는 두 vector lvalue를 비소유 참조로 받아 내부 순서를 바꾸고 Distance 최솟값을 반환한다.
    const Distance answer{closest_pair(points, scratch, 0U, point_count)};
    // operator<<는 answer를 cout 버퍼에 쓰고 cout&를 반환해 개행을 연쇄하며 반환 참조는 무시한다.
    std::cout << answer << '\n';
    return 0;
}
