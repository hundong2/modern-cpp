/*
Chapter 20. 기하, CCW, 선분 교차, 볼록 껍질, 스위프 라인

문제 1: 세 점의 방향을 판정하라.
접근: cross(b - a, c - a)의 부호를 본다.
- 양수: 반시계, 음수: 시계, 0: 일직선

문제 2: 두 선분이 교차하는지 판정하라.
접근: CCW 곱이 서로 0 이하인지 확인하고, 일직선이면 bounding box를 확인한다.

문제 3: 점들의 볼록 껍질을 구하라.
접근: Monotonic Chain. 정렬 후 lower/upper hull을 만든다.
복잡도: O(N log N)

문제 4: 여러 구간 중 동시에 겹치는 최대 개수를 구하라.
접근: 시작 이벤트 +1, 끝 이벤트 -1을 정렬한다.
주의: 닫힌 구간 [l, r]이면 같은 좌표에서 시작을 끝보다 먼저 처리한다.
*/

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>
using namespace std;

struct Point {
    long long x;
    long long y;

    bool operator<(const Point& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

Point operator-(const Point& a, const Point& b) {
    return {a.x - b.x, a.y - b.y};
}

long long cross(Point a, Point b) {
    return a.x * b.y - a.y * b.x;
}

long long ccw(Point a, Point b, Point c) {
    long long value = cross(b - a, c - a); // 외적 부호로 세 점의 회전 방향을 판정한다.
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}

bool onSegment(Point a, Point b, Point p) {
    if (ccw(a, b, p) != 0) return false; // 일직선이 아니면 선분 위에 있을 수 없다.
    return min(a.x, b.x) <= p.x && p.x <= max(a.x, b.x) &&
           min(a.y, b.y) <= p.y && p.y <= max(a.y, b.y);
}

bool segmentsIntersect(Point a, Point b, Point c, Point d) {
    long long ab1 = ccw(a, b, c);
    long long ab2 = ccw(a, b, d);
    long long cd1 = ccw(c, d, a);
    long long cd2 = ccw(c, d, b);

    if (ab1 == 0 && onSegment(a, b, c)) return true; // 끝점이 다른 선분 위에 있는 퇴화 케이스다.
    if (ab2 == 0 && onSegment(a, b, d)) return true;
    if (cd1 == 0 && onSegment(c, d, a)) return true;
    if (cd2 == 0 && onSegment(c, d, b)) return true;
    return ab1 * ab2 < 0 && cd1 * cd2 < 0;
}

long long polygonArea2(const vector<Point>& poly) {
    long long area2 = 0;
    for (int i = 0; i < (int)poly.size(); ++i) {
        Point a = poly[i];
        Point b = poly[(i + 1) % poly.size()];
        area2 += a.x * b.y - a.y * b.x; // 신발끈 공식의 한 항이다.
    }
    return llabs(area2);
}

vector<Point> convexHull(vector<Point> points) {
    sort(points.begin(), points.end());
    points.erase(unique(points.begin(), points.end()), points.end());
    if (points.size() <= 1) return points;

    vector<Point> lower, upper;
    for (Point p : points) {
        while (lower.size() >= 2 && ccw(lower[lower.size() - 2], lower.back(), p) <= 0) { // 우회전/일직선이면 convex hull 경계가 아니다.
            lower.pop_back();
        }
        lower.push_back(p);
    }

    for (int i = (int)points.size() - 1; i >= 0; --i) {
        Point p = points[i];
        while (upper.size() >= 2 && ccw(upper[upper.size() - 2], upper.back(), p) <= 0) {
            upper.pop_back();
        }
        upper.push_back(p);
    }

    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower;
}

int maxOverlappingClosedIntervals(const vector<pair<int, int>>& intervals) {
    vector<pair<int, int>> events;
    for (auto [l, r] : intervals) {
        events.push_back({l, +1}); // 구간 시작점에서는 활성 구간 수가 증가한다.
        events.push_back({r, -1}); // 닫힌 구간이라 같은 좌표에서는 시작 이벤트를 먼저 처리한다.
    }

    sort(events.begin(), events.end(), [](auto a, auto b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second > b.second;
    });

    int cur = 0;
    int best = 0;
    for (const auto& event : events) {
        cur += event.second;
        best = max(best, cur);
    }
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "[ccw] " << ccw({0, 0}, {2, 0}, {1, 1}) << '\n';
    cout << "[segments intersect] " << boolalpha
         << segmentsIntersect({0, 0}, {3, 3}, {0, 3}, {3, 0}) << '\n';

    vector<Point> poly = {{0, 0}, {4, 0}, {4, 3}, {0, 3}};
    cout << "[polygon area] " << polygonArea2(poly) / 2.0 << '\n';

    vector<Point> points = {{0, 0}, {1, 1}, {2, 0}, {2, 2}, {0, 2}, {1, 0}};
    auto hull = convexHull(points);
    cout << "[hull]";
    for (Point p : hull) cout << " (" << p.x << ',' << p.y << ')';
    cout << '\n';

    cout << "[max overlap] " << maxOverlappingClosedIntervals({{1, 4}, {2, 5}, {5, 7}, {3, 6}}) << '\n';
    return 0;
}
