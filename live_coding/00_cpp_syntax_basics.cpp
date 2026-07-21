/*
Chapter 00. C++ 라이브 코딩 문법, STL, 실수 방지 체크리스트

목표:
- 알고리즘 풀이 전에 반드시 필요한 C++17 문법과 기본 지식을 빠르게 복습한다.
- 라이브 코딩에서 말로 설명하기 좋은 표현을 주석으로 남긴다.

1. 기본 파일 구조
- 표준 헤더를 직접 include한다.
  예: <iostream>, <vector>, <algorithm>, <queue>, <unordered_map>
  GCC 전용 통합 헤더는 표준 C++이 아니고 MSVC에서 동작하지 않는다.
- using namespace std;
  예제와 코딩 테스트에서는 짧게 쓰기 위해 사용한다. 실무 코드에서는 네임스페이스 오염 때문에 제한적으로 쓴다.
- int main()은 프로그램 시작점이다. return 0은 정상 종료를 뜻한다.

2. 빠른 입출력
- ios::sync_with_stdio(false);
  C 입출력(scanf/printf)과 C++ 입출력(cin/cout)의 동기화를 끊어 cin/cout을 빠르게 만든다.
- cin.tie(nullptr);
  cin 전에 cout을 자동 flush하지 않게 한다.
- 이 설정을 썼다면 같은 프로그램에서 scanf/printf와 cin/cout을 섞지 않는 편이 안전하다.

3. 정수 타입
- int: 보통 약 -2.1e9 ~ 2.1e9
- long long: 보통 약 -9e18 ~ 9e18
- 합, 곱, 거리, 경우의 수, 누적합은 기본적으로 long long을 의심한다.
- 예: int 1e9 + int 1e9 + int 1e9는 overflow 가능성이 있다.

4. 인덱스와 범위
- C++ vector/string 인덱스는 0부터 시작한다.
- [l, r]은 양끝 포함, [l, r)는 r 제외다.
- STL iterator는 보통 [begin, end) 규칙을 쓴다.
- v.size()는 size_t라서 int와 비교할 때 경고가 날 수 있다. 코딩 테스트에서는 (int)v.size()로 맞추는 경우가 많다.

5. 값 전달, 참조 전달, const 참조
- void f(vector<int> v): 전체 vector가 복사된다. O(N) 비용이다.
- void f(vector<int>& v): 원본을 수정할 수 있고 복사 비용이 없다.
- void f(const vector<int>& v): 원본을 수정하지 않고 복사 비용도 없다. 읽기 전용 인자로 가장 자주 쓴다.

6. 자주 쓰는 STL
- vector<T>: 동적 배열. push_back O(1) amortized, 임의 접근 O(1)
- string: 문자 배열. s[i], substr, find 등을 사용한다.
- pair<T,U>, tuple: 여러 값을 묶어 반환할 때 편하다.
- stack, queue, deque: DFS 보조, BFS, 슬라이딩 윈도우에 자주 나온다.
- priority_queue<T>: 기본은 max-heap이다.
- priority_queue<T, vector<T>, greater<T>>: min-heap이다.
- set/map: 정렬된 트리 기반. 삽입/삭제/탐색 O(log N)
- unordered_set/unordered_map: 해시 기반. 평균 O(1), 최악 O(N)

7. 정렬과 comparator
- sort(v.begin(), v.end())는 오름차순이다.
- 내림차순은 sort(v.rbegin(), v.rend()) 또는 comparator를 쓴다.
- comparator는 strict weak ordering을 지켜야 한다. 같을 때 true를 반환하면 안 된다.

8. lambda
- auto f = [&](int x) { return x + base; };
- [&]는 주변 변수를 참조로 캡처한다.
- [=]는 주변 변수를 값으로 캡처한다.
- 이분 탐색의 possible 함수, DFS 내부 함수, 정렬 comparator에 자주 쓴다.

9. 구조체와 연산자
- struct는 여러 필드를 이름으로 묶을 때 좋다.
- priority_queue나 sort에서 비교 기준을 명확하게 만들 수 있다.

10. 자주 나는 실수
- 배열 범위 초과: i <= n 대신 i < n인지 확인한다.
- overflow: mid = (lo + hi) / 2 대신 lo + (hi - lo) / 2를 쓴다.
- 초기값: 최솟값 문제는 INF, 최댓값 문제는 -INF를 제대로 둔다.
- 방문 처리: BFS에서는 큐에 넣을 때 visited=true로 만드는 것이 중복 삽입을 막는다.
- 재귀 복구: backtracking 후 pop_back, used=false를 빠뜨리지 않는다.
- endl: 매번 flush하므로 느리다. 보통 '\n'을 쓴다.

11. 설명 템플릿
- "상태는 dp[i]이고, i번째까지 고려했을 때의 최적값입니다."
- "이분 탐색이 가능한 이유는 조건이 false...false, true...true 형태의 단조성을 갖기 때문입니다."
- "BFS를 쓰는 이유는 모든 간선 비용이 1이라 먼저 도착한 거리가 최단 거리이기 때문입니다."
- "해시맵을 써서 이전에 본 값의 존재 여부를 평균 O(1)에 확인합니다."
*/

#include <algorithm>
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

struct Person {
    string name;
    int score;
};

void printVector(const vector<int>& values) {
    for (int x : values) {  // range-for는 컨테이너 전체를 순회할 때 인덱스 실수를 줄인다.
        cout << x << ' ';
    }
    cout << '\n';
}

int main() {
    ios::sync_with_stdio(false);  // cin/cout만 쓸 때 입출력 병목을 줄이는 기본 설정이다.
    cin.tie(nullptr);             // 입력 전 자동 flush를 끊어 불필요한 대기 비용을 없앤다.

    vector<int> numbers = {5, 1, 4, 1, 3};
    sort(numbers.begin(), numbers.end());  // unique는 인접 중복만 제거하므로 정렬을 먼저 한다.
    cout << "[sorted] ";
    printVector(numbers);

    numbers.erase(unique(numbers.begin(), numbers.end()), numbers.end());  // erase-remove 계열 패턴이다.
    cout << "[unique] ";
    printVector(numbers);

    vector<Person> people = {
        {"alice", 90},
        {"bob", 95},
        {"chris", 90}
    };

    sort(people.begin(), people.end(), [](const Person& a, const Person& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.name < b.name;  // 점수가 같을 때만 이름 오름차순을 적용해 비교 규칙을 완성한다.
    });

    cout << "[people]\n";
    for (const Person& p : people) {
        cout << p.name << ' ' << p.score << '\n';
    }

    unordered_map<string, int> count;  // 평균 O(1) 조회가 필요할 때 map보다 먼저 검토한다.
    for (const Person& p : people) {
        ++count[p.name];
    }
    cout << "[hash lookup] alice=" << count["alice"] << '\n';

    priority_queue<int, vector<int>, greater<int>> minHeap;  // greater를 쓰면 기본 max-heap이 min-heap이 된다.
    for (int x : {7, 2, 9}) {
        minHeap.push(x);
    }
    cout << "[min heap top] " << minHeap.top() << '\n';

    long long lo = 0;
    long long hi = 1'000'000'000LL;
    long long mid = lo + (hi - lo) / 2;  // lo + hi가 overflow 나는 경우를 피하는 이분 탐색 관용구다.
    cout << "[safe mid] " << mid << '\n';

    return 0;
}
