# 분할 정복 최근접 점 쌍(Closest Pair of Points)

## 정의

2차원 평면의 점 집합에서 서로 다른 두 점 사이 유클리드 거리의 최솟값을 찾는 문제다. 정수 좌표 문제에서는 제곱 거리 `(x1-x2)^2 + (y1-y2)^2`를 비교하면 제곱근을 계산하지 않아도 순서가 같고 부동소수점 오차도 없다.

완전 탐색은 모든 `N(N-1)/2`쌍을 보아 `O(N²)`이다. 분할 정복은 점을 x좌표로 나누고 양쪽 내부 답과 중앙선을 가로지르는 답을 합쳐 `O(N log N)`을 달성한다.

## 적용 조건

- 점 수가 커서 모든 쌍을 비교할 수 없고 거리가 일반적인 2차원 유클리드 거리일 때 적합하다.
- 좌표 차의 제곱과 합이 선택한 정수 타입 범위에 들어오는지 먼저 계산해야 한다.
- 동일 좌표가 허용되면 답 0을 처리해야 한다.
- 고차원에서는 띠 후보 수에 대한 상수 경계가 차원에 따라 급격히 커지므로 KD-tree 등 다른 자료구조도 검토한다.
- 온라인 삽입/삭제와 매 질의가 섞이면 정적 분할 정복을 그대로 반복하기보다 동적 공간 자료구조가 필요하다.

## 핵심 아이디어와 불변식

점 배열을 처음 한 번 x 오름차순으로 정렬한다. 재귀 함수가 `[begin,end)`를 처리할 때 다음 계약을 둔다.

1. **진입 불변식**: 현재 재귀가 나뉘기 전 구간은 x정렬 순서를 갖는다.
2. 중앙 인덱스와 `middle_x`를 저장하고 왼쪽·오른쪽을 재귀 처리한다.
3. **반환 불변식**: 각 재귀가 반환하면 그 구간의 점들은 y정렬돼 있다.
4. y정렬된 두 반쪽을 선형 병합해 현재 구간도 y정렬로 만든다.
5. 두 내부 답의 최솟값을 `d`라 하면 중앙선과 x차 제곱이 `d` 미만인 점만 중앙 띠 후보로 남긴다.
6. 띠는 y정렬이므로 각 점 뒤에서 y차 제곱이 `d` 미만인 점까지만 비교한다.

폭 `2√d`인 띠를 `√d/2` 정도의 작은 상자로 나누는 패킹 논증을 쓰면, 양쪽 내부에 거리 `√d` 미만인 쌍이 없다는 전제 아래 한 점이 확인할 다음 후보는 상수 개뿐이다. 따라서 띠 검사는 선형이다.

## 단계별 절차

1. 모든 점을 `(x,y)` 사전식 순서로 정렬한다.
2. 점이 3개 이하면 모든 쌍을 비교하고 그 구간을 y순으로 정렬해 반환한다.
3. 중앙 x좌표를 저장하고 왼쪽·오른쪽 최근접 거리 제곱을 재귀로 구한다.
4. 두 반쪽의 y정렬 결과를 공유 작업 배열에 선형 병합하고 원래 배열로 복사한다.
5. `dx² < d`인 점만 y순서를 유지해 작업 배열의 띠 영역에 모은다.
6. 각 띠 점에서 뒤쪽 점을 보다가 `dy² >= d`가 되면 중단하고, 그 전 후보의 전체 거리로 `d`를 갱신한다.
7. 갱신된 최소 거리 제곱을 반환한다.

## 의사 코드

```text
sort points by (x, y)

function closest(begin, end):
    if end - begin <= 3:
        best <- minimum of every pair
        sort this range by (y, x)
        return best

    middle <- midpoint
    middle_x <- points[middle].x
    best <- min(closest(left half), closest(right half))
    merge two y-sorted halves

    strip <- points with (x - middle_x)^2 < best, preserving y order
    for each strip[i]:
        for j <- i+1 while (strip[j].y - strip[i].y)^2 < best:
            best <- min(best, squared_distance(strip[i], strip[j]))
    return best
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <algorithm> // sort와 min을 선언한다.
#include <cstddef>   // size_t를 선언한다.
#include <iostream>  // cout을 선언한다.
#include <limits>    // numeric_limits를 선언한다.
#include <vector>    // vector를 선언한다.

using Distance = long long; // 좌표 차 제곱을 담는 정수 별칭이다.

struct Point { // 단순 좌표 DTO라 멤버가 기본 public이다.
    Distance x{};
    Distance y{};
};

Distance distance2(const Point& a, const Point& b) { // 두 점을 const&로 빌린다.
    const Distance dx{a.x - b.x};
    const Distance dy{a.y - b.y};
    return dx * dx + dy * dy;
}

Distance closest(std::vector<Point>& points, std::vector<Point>& scratch,
                 std::size_t begin, std::size_t end) {
    const std::size_t count{end - begin};
    if (count <= 3U) { // 작은 구간은 완전 탐색한다.
        Distance best{std::numeric_limits<Distance>::max()};
        for (std::size_t i{begin}; i < end; ++i) {
            for (std::size_t j{i + 1U}; j < end; ++j) {
                best = std::min(best, distance2(points[i], points[j]));
            }
        }
        std::sort(points.begin() + static_cast<std::ptrdiff_t>(begin),
                  points.begin() + static_cast<std::ptrdiff_t>(end),
                  [](const Point& a, const Point& b) { return a.y < b.y; });
        return best;
    }

    const std::size_t middle{begin + count / 2U};
    const Distance middle_x{points[middle].x}; // 재귀 전 x경계를 보존한다.
    Distance best{std::min(closest(points, scratch, begin, middle),
                           closest(points, scratch, middle, end))};

    std::size_t left{begin}, right{middle}, write{begin};
    while (left < middle && right < end) { // 두 y정렬 반쪽을 병합한다.
        scratch[write++] = points[points[right].y < points[left].y ? right++ : left++];
    }
    while (left < middle) scratch[write++] = points[left++];
    while (right < end) scratch[write++] = points[right++];
    for (std::size_t i{begin}; i < end; ++i) points[i] = scratch[i];

    std::size_t strip_size{};
    for (std::size_t i{begin}; i < end; ++i) {
        const Distance dx{points[i].x - middle_x};
        if (dx * dx < best) scratch[begin + strip_size++] = points[i];
    }
    for (std::size_t i{}; i < strip_size; ++i) {
        for (std::size_t j{i + 1U}; j < strip_size; ++j) {
            const Distance dy{scratch[begin + j].y - scratch[begin + i].y};
            if (dy * dy >= best) break;
            best = std::min(best, distance2(scratch[begin + i], scratch[begin + j]));
        }
    }
    return best;
}

int main() {
    std::vector<Point> points{{0, 0}, {10, 10}, {0, 10}, {10, 0}};
    std::sort(points.begin(), points.end(), [](const Point& a, const Point& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
    std::vector<Point> scratch(points.size());
    const Distance answer{closest(points, scratch, 0U, points.size())};
    std::cout << answer << '\n'; // 100
    return answer == 100 ? 0 : 1;
}
```

점 배열과 scratch는 `main`이 소유하고 재귀는 비const 참조로 빌린다. 한 공유 배열을 모든 깊이에서 재사용하므로 재귀마다 vector를 새로 할당하지 않는다. 인덱스는 반열린 구간 `[begin,end)`로 통일한다.

## 정확성 근거

### 보조정리 1: 재귀 반환값은 같은 반쪽 내부 최근접 거리다

기저에서는 최대 세 점의 모든 쌍을 직접 비교하므로 참이다. 더 큰 구간에서는 귀납 가정에 의해 왼쪽 반환값과 오른쪽 반환값이 각각 그 반쪽의 최솟값이다. 두 값의 최솟값은 중앙선을 넘지 않는 모든 쌍 중 최솟값이다.

### 보조정리 2: 최적 교차 쌍은 중앙 띠 검사에 포함된다

현재 내부 최솟값을 `d`라 하자. 교차 쌍의 한 점이 중앙선에서 x거리 `√d` 이상이면 두 점의 x차만으로 거리 제곱이 `d` 이상이라 현재 답을 개선할 수 없다. 따라서 더 가까운 교차 쌍의 두 점은 모두 `dx² < d`인 띠에 들어간다. 또한 전체 거리 제곱이 `d` 미만이면 y차 제곱도 반드시 `d` 미만이므로 y순 반복이 중단되기 전에 비교된다.

### 보조정리 3: 반환 시 현재 구간은 y정렬이다

기저는 직접 y정렬한다. 귀납 단계에서 두 재귀 반환 구간은 각각 y정렬이고, 알고리즘이 두 정렬열을 선형 병합하므로 합친 현재 구간도 y정렬이다.

### 정리: 알고리즘은 전체 점 집합의 최근접 거리 제곱을 반환한다

임의의 점 쌍은 왼쪽 내부, 오른쪽 내부, 중앙선을 가로지르는 교차 쌍 중 하나다. 보조정리 1이 내부 쌍을 모두 포함하고 보조정리 2가 현재 답을 개선할 모든 교차 쌍을 포함한다. 세 범주의 최솟값을 계속 갱신하므로 반환값은 전체 최솟값이다.

## 시간·공간 복잡도

- 최초 x정렬은 `O(N log N)`이다.
- 각 재귀 노드는 두 y정렬 반쪽 병합과 띠 구성/검사에 `O(N)`을 쓴다.
- 점화식은 `T(N)=2T(N/2)+O(N)`이므로 `T(N)=O(N log N)`이다.
- 점 배열과 공유 scratch가 `O(N)`, 균형 재귀 스택이 `O(log N)`이다.
- 각 재귀에서 y정렬을 새로 하면 병합 단계가 `O(N log N)`이 되어 전체가 `O(N log²N)`으로 악화된다.

## 자료구조 선택

- `vector<Point>`: 연속 메모리와 random-access 반복자가 정렬·병합·인덱스 분할에 잘 맞는다.
- 공유 `vector<Point> scratch`: 병합과 띠 저장을 번갈아 맡아 반복 할당을 없앤다.
- `long long`: 현재 문제 좌표 범위의 차·제곱·합을 안전하게 담는다. 다른 문제에서는 제한을 다시 계산한다.
- 반열린 구간: 길이가 `end-begin`이고 두 반쪽이 겹치지 않아 off-by-one 오류를 줄인다.

## 흔한 실수

1. **좌우 답만 취함**: 중앙선을 가로지르는 최근접 쌍을 놓친다.
2. **재귀 뒤 `middle_x`를 읽음**: 배열이 y정렬로 바뀌어 중앙 인덱스가 원래 x경계를 뜻하지 않는다.
3. **매 단계 y정렬**: 정답은 맞아도 `O(N log²N)`이 된다.
4. **띠에서 모든 쌍 비교**: 같은 y좌표가 많을 때 `O(N²)`으로 무너진다. y차 조건으로 중단한다.
5. **`int` 제곱**: 더 큰 좌표 문제에서 곱셈이 결과 타입으로 승격되기 전에 오버플로할 수 있다.
6. **제곱근 사용**: 불필요한 부동소수점 오차와 비용을 만든다.
7. **중복 좌표 무시**: 답 0이 가능하며 조기 반환도 안전하다.
8. **비엄격 비교자**: `<=` 비교자는 strict weak ordering을 깨 `std::sort` 계약을 위반한다.

## 변형과 확장

- **스위프 라인**: x순으로 이동하며 x거리 밖 점을 제거하고 y정렬 `set`에서 후보를 찾으면 `O(N log N)`이다.
- **실제 점 쌍 복원**: 거리와 함께 두 점의 인덱스/좌표를 결과 struct에 저장한다.
- **맨해튼 거리**: 좌표 변환과 정렬/자료구조 기법이 달라지므로 같은 띠 논증을 그대로 쓰지 않는다.
- **가장 먼 점 쌍**: 볼록 껍질 뒤 rotating calipers가 대표적이다.
- **고차원 최근접 이웃**: KD-tree, ball tree, 근사 최근접 이웃을 데이터 분포·질의 수에 맞게 검토한다.

## 오늘 문제와의 연결

[`../2026-08-28/icpc_problem.cpp`](../2026-08-28/icpc_problem.cpp)는 BOJ 2261의 거리 제곱을 구한다. `closest_pair`는 반환 시 y정렬 불변식을 유지하고, `scratch`를 병합 결과와 중앙 띠에 재사용한다. CTest의 교차 띠 사례는 좌우 내부 답만 취하는 오류를, 같은 y 사례는 띠 완전 탐색으로 인한 성능 오류를 겨냥한다.

## 직접 검증

1. 여섯 점을 x순으로 적고 재귀 반환마다 배열의 y순서를 손으로 추적한다.
2. 중앙 띠를 제거하면 실패하는 최소 반례를 직접 만든다.
3. `middle_x`를 재귀 뒤 읽도록 바꾸고 잘못된 경계가 생기는 입력을 찾는다.
4. 작은 무작위 점 집합을 만들어 `O(N²)` 완전 탐색과 1,000회 이상 대조한다.
5. y좌표가 모두 같은 100,000점을 넣어 안쪽 반복이 빠르게 중단되는지 측정한다.
6. 반환값뿐 아니라 실제 최근접 점 쌍도 복원하도록 결과 struct를 확장한다.
