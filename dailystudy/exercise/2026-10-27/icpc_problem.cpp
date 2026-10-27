/*
문제 ID·제목: CSES 1160 - Planets Queries II
공식 출처: https://cses.fi/problemset/task/1160/

문제 요약
- n개의 행성은 각각 다음 행성 하나로 가는 순간이동 장치를 가진다. 자기 자신으로 가는 장치도 가능하다.
- 각 질의 (a,b)마다 a에서 b에 도달하는 최소 순간이동 횟수를 구한다.
- 유일한 다음 행성만 따라갈 수 있으므로, 그 열에 b가 없으면 -1을 출력한다.

입력
- 첫 줄: 행성 수 n, 질의 수 q.
- 둘째 줄: i번 행성의 다음 행성 t_i를 1번부터 n번까지.
- 다음 q개 줄: 출발 a와 목적지 b.

출력
- 질의마다 최소 순간이동 횟수, 도달할 수 없으면 -1을 한 줄에 하나씩 출력한다.

공식 제약
- 1 <= n,q <= 200,000, 1 <= t_i,a,b <= n.
- 시간 제한 1초, 메모리 제한 512MB.

공식 예제
입력:
5 3
2 3 2 3 2
1 2
1 3
1 4
출력:
1
2
-1
*/

// <cstddef>는 vector의 크기와 첨자에 쓰는 부호 없는 타입 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력·출력 객체와 정수 stream 연산을 선언한다.
#include <iostream>
// <vector>는 입력 크기에 맞게 연속 저장소를 소유하는 std::vector를 선언한다.
#include <vector>

// 초보 문법 지도:
// - int는 정점 번호·거리처럼 n 이하 값을 담는 기본 정수 타입이고, 변수명 뒤 {}는 0으로 값 초기화한다.
// - advance의 반환형 int는 이동 뒤 정점 값이고, jump는 const reference 매개변수라 복사·수정 없이 빌린다.
//   pointer라면 null을 표현하고 재바인딩할 수 있지만, reference와 pointer 모두 자동 소유권을 주지 않는다.
// - if/while/for/do-while은 비교 결과로 분기·반복하고 각 {} block은 지역 객체의 수명 범위도 만든다.
// - 이 풀이는 데이터와 동작을 별도 객체로 묶을 필요가 없어 struct/class를 선언하지 않는다. 일반적으로
//   struct 기본 접근은 public, class 기본 접근은 private이며 public:/private:가 compile-time 접근을 정한다.
// - 사용자 정의 생성자가 없으므로 암시 변환을 막는 explicit과 본문 전 멤버를 만드는 멤버 초기화 목록도
//   쓰지 않는다. 이 둘이 필요한 도메인 객체 예제는 같은 날짜 main.cpp의 CancellationCoordinator에 있다.
// - using 별칭도 필요하지 않다. 반면 vector<int>의 int와 vector<vector<int>>의 안쪽 vector<int>는
//   class template 타입 인자이며 compiler가 해당 특수화의 저장·연산 타입을 정한다.

/*
핵심 알고리즘과 불변식
1. 함수형 그래프의 각 약한 연결 성분은 단 하나의 유향 사이클과 그곳으로 흐르는 역방향 트리로 이루어진다.
2. 진입 차수 0 정점을 연쇄적으로 제거하면 남은 정점은 정확히 사이클 정점이다.
3. 각 사이클에 component id와 position을 붙이고, 사이클에서 역간선 BFS를 하면 모든 트리 정점에
   같은 component id와 사이클까지의 depth가 전파된다.
4. jump[k][v]는 v에서 2^k번 이동한 정점이다. 점화식은
   jump[k][v] = jump[k-1][jump[k-1][v]]이며, depth 차이나 사이클 진입점을 O(log n)에 구한다.
5. 목적지 b가 트리에 있으면 a를 depth 차이만큼 올린 정점이 b일 때만 가능하다.
   b가 사이클에 있으면 a→진입점 거리와 position의 모듈러 차를 더한다.

대회 실전 선택
- 출발점마다 추적하면 O(nq)까지 커진다. 성분·깊이·사이클 좌표를 O(n), binary lifting을
  O(n log n)에 한 번 전처리하여 각 질의를 O(log n)에 답한다.
- 간선 방향과 반대인 reverse 목록을 두어 사이클에서 밖으로만 BFS한다. depth==-1을 미배정 표시로
  쓰면 사이클 간선을 돌아 재방문하지 않는다.
- 재귀 DFS를 쓰지 않아 200,000개 짜리 긴 꼬리에서도 호출 스택 한도와 무관하다.

값 범주·수명·소유권
- successor, reverse_edges, jump 등 이름 있는 vector는 lvalue이고 각 저장소를 직접 소유한다.
- vector[index]는 소유 vector의 원소를 빌린 lvalue 참조를 돌려준다. 이 풀이는 재할당을 건너 원소
  참조를 보존하지 않으므로 dangling이 생기지 않는다.
- 산술식·비교식의 int 결과는 prvalue, 이름 있는 변수는 lvalue다. 소유권 이전이 필요 없어
  xvalue·이동·RVO는 핵심 경로에 등장하지 않으며 모든 vector는 main 종료 시 자동 해제된다.

기계 실행 관점
- 전처리는 주로 배열 load/store, 정수 비교, 조건 분기, 첨자 추적으로 이루어진다. vector가
  연속 저장소를 쓰므로 jump의 각 level 순회는 cache locality를 기대할 수 있다. 실제 명령·분기 제거·
  prefetch 여부는 CPU, compiler, optimization option, 입력 분포에 따라 달라지므로 특정 assembly로 단정하지 않는다.
*/

// implementation 가까운 공용 문서: ../algorithm/functional-graph-cycle-peeling.md

// steps는 올라갈 간선 수, node는 출발 정점이다. const&는 jump 저장소를 복사 없이 읽고 소유하지 않는다.
[[nodiscard]] int advance(const std::vector<std::vector<int>>& jump, int node, int steps) {
    // table이 필요한 level 수만큼 크고 steps<=n이라는 전처리 불변식 때문에
    // 비트가 남아 있는 동안만 순회해도 범위 안이다.
    int level{};
    while (steps > 0) {
        if ((steps & 1) != 0) {
            // [첫 호출 계약: const vector::operator[] 두 번]
            // (1) 바깥 수신 jump는 최소 필요 level을 갖는 const vector<vector<int>> lvalue,
            //     안쪽 수신 jump[level]은 size>node인 const vector<int> lvalue다.
            // (2) 바깥은 const_reference vector<vector<int>>::operator[](size_type) const,
            //     안쪽은 const_reference vector<int>::operator[](size_type) const overload다.
            // (3) level/node는 각각 전처리·질의 불변식이 보장하는 비음수 int lvalue에서
            //     size_t prvalue로 변환되어 값 전달된다. 소유권 이전은 없다.
            // (4) 첫 []는 const vector<int>&, 둘째 []는 const int&를 반환하고 최종 int 값을
            //     node에 복사하는 데 사용한다.
            // (5) 두 vector와 원소는 변하지 않으며 node만 해당 2^level번 후속 정점으로 갱신된다.
            // (6) 각 O(1), 무할당·무효화이며 유효 첨자에서는 오류를 별도로 보고하지 않는다. 범위 밖은
            //     UB이므로 table level·1-based 정점 전제가 필수다. 참조는 jump 수명에 묶이며
            //     보존하지 않고, 동시 수정은 없다.
            node = jump[static_cast<std::size_t>(level)][static_cast<std::size_t>(node)];
        }
        steps >>= 1;
        ++level;
    }
    return node;
}

// int 반환형 0은 정상 종료 상태를 의미하고, 입력·출력은 표준 stream을 통해 처리한다.
int main() {
    // [첫 호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 수신 객체 없는 std::ios_base 정적 함수이고, 표준 C++ stream은 초기 동기화 상태다.
    // (2) static bool ios_base::sync_with_stdio(bool sync=true)의 bool 인자 overload다.
    // (3) false는 bool prvalue 값 전달이며 소유권·수명을 옮기지 않는다.
    // (4) 이전 동기화 상태 bool을 반환하지만 버린다.
    // (5) 이후 C++ stream과 C stdio가 문자 단위로 자동 동기화되지 않는다.
    // (6) 초기 I/O 전이라고 가정한다. O(1), 무할당·무효화, 예외 없음. C stdio를 섞어 순서에
    //     의존하면 안 되며, 설정 변경을 다른 thread와 동시에 하지 않는다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie(nullptr)]
    // (1) 수신 std::cin은 살아 있는 std::istream lvalue이고 초기에 std::cout과 묶여 있다.
    // (2) 선택 시그니처는 basic_ostream<char>* basic_ios<char>::tie(basic_ostream<char>* tiestr) setter다.
    // (3) nullptr는 null pointer prvalue로 값 전달되며 stream 소유권을 옮기지 않는다.
    // (4) 이전 tie 포인터를 반환하지만 버린다.
    // (5) 입력 전 std::cout 자동 flush 묶음이 해제된다. stream 객체 수명은 변하지 않는다.
    // (6) O(1), 무할당·무효화. 대화형 입력은 수동 flush가 필요하지만 이 배치 문제에는 없다.
    //     같은 stream 설정을 동시에 변경하지 않는다.
    std::cin.tie(nullptr);

    // int는 최대 200,000의 정점·질의·거리를 충분히 담고, {}는 0으로 값 초기화한다.
    int n{};
    int query_count{};
    // [첫 호출 계약: basic_istream<char>::operator>>(int&) 연쇄]
    // (1) 수신 std::cin은 입력 가능한 std::istream lvalue, n/query_count는 0인 int lvalue다.
    // (2) 각각 basic_istream<char>& basic_istream<char>::operator>>(int&) 오버로드다.
    // (3) 두 lvalue를 쓰기 가능한 int&로 빌리며 소유권은 유지된다. 공식 [1,200000]을 기대한다.
    // (4) 매 추출은 같은 istream&를 반환해 다음 추출에 쓰고 마지막 참조는 버린다.
    // (5) 성공 시 두 값과 입력 위치가 갱신되고, 실패 시 failbit가 설정된다.
    // (6) 자릿수에 선형, 보통 무할당·무효화. 형식 오류·EOF는 stream 상태로 나타나며 유효한
    //     온라인 저지 입력을 전제한다. 동시 입력은 하지 않는다.
    std::cin >> n >> query_count;

    // [첫 호출 계약: vector 생성자 군]
    // (1) 각 수신 vector는 생성 전이라 저장소·관찰자가 없다.
    // (2) vector<int>(count,value), vector<vector<int>>(count), vector<int>() overload와 기본 allocator를 쓴다.
    // (3) count=n+1은 size_t로 변환되는 [2,200001] prvalue, 0/-1은 각 원소 초기값 int prvalue다.
    //     외부 저장소나 소유권을 넘기지 않는다.
    // (4) 생성자는 반환값이 없고, 각 vector가 지정된 개수의 원소를 소유한다.
    // (5) 성공한 vector들은 독립 저장소를 갖고, 중간 실패 시 완성된 선행 객체는 자동 정리된다.
    // (6) 전체 O(n) 초기화·공간, 동적 할당. length_error/bad_alloc 가능, 아직 참조 무효화 없음,
    //     외부 thread에 게시하지 않는다.
    std::vector<int> successor(static_cast<std::size_t>(n + 1), 0);
    std::vector<int> indegree(static_cast<std::size_t>(n + 1), 0);
    std::vector<std::vector<int>> reverse_edges(static_cast<std::size_t>(n + 1));
    std::vector<int> depth(static_cast<std::size_t>(n + 1), -1);
    std::vector<int> component(static_cast<std::size_t>(n + 1), -1);
    std::vector<int> cycle_position(static_cast<std::size_t>(n + 1), -1);
    std::vector<int> cycle_length;
    std::vector<int> peeled;

    // [첫 호출 계약: vector<int>::reserve(size_type)]
    // (1) 수신 peeled와 cycle_length는 빈 vector<int> lvalue이고 원소 관찰자가 없다.
    // (2) 선택 시그니처는 void vector<int>::reserve(size_type new_capacity)다.
    // (3) n을 변환한 size_t prvalue [1,200000]을 값 전달하며 소유권 이전은 없다.
    // (4) void라 반환값은 없고 capacity를 최소 n으로 확보한다.
    // (5) size 0은 그대로이고, 재할당이 있어도 무효화될 관찰자가 없다.
    // (6) 빈 int vector에서 원소 이동 O(1), 최대 O(n) 할당. length_error/bad_alloc 가능·실패 시 상태 보존,
    //     저장소는 vector 수명까지, 동시 접근 없음.
    peeled.reserve(static_cast<std::size_t>(n));
    cycle_length.reserve(static_cast<std::size_t>(n));

    for (int planet{1}; planet <= n; ++planet) {
        int destination{};
        std::cin >> destination;

        // [첫 호출 계약: vector::operator[]과 reverse_edges[destination].push_back(planet)]
        // (1) successor/indegree/reverse_edges는 size=n+1인 각 vector lvalue, 안쪽 vector는 빈 상태일 수 있다.
        // (2) 선택 overload는 비-const reference vector<T>::operator[](size_type)와
        //     void vector<int>::push_back(const int&)다.
        // (3) planet,destination은 공식 [1,n] int lvalue에서 size_t 첨자로 변환된다. planet은 const int&로
        //     빌려 값 복사되며 소유권은 유지된다.
        // (4) []는 원소 lvalue 참조를 반환해 대입·++·push 수신자로 쓰고, push_back은 void다.
        // (5) successor[planet]이 destination, indegree[destination]이 1 증가, reverse 목록 끝에 planet이 생긴다.
        //     안쪽 vector 재할당은 그 vector의 기존 반복자를 무효화하지만 보관하지 않는다.
        // (6) []는 O(1)·무할당, push는 상각 O(1)·bad_alloc 가능. [] 범위 밖은 UB라 공식 입력이 전제다.
        //     성공한 push 후 size가 1 늘고, 해당 vector의 동시 수정은 하지 않는다.
        successor[static_cast<std::size_t>(planet)] = destination;
        ++indegree[static_cast<std::size_t>(destination)];
        reverse_edges[static_cast<std::size_t>(destination)].push_back(planet);
    }

    // 진입 차수 0은 사이클에 속할 수 없으므로 제거 순서 vector에 넣는다.
    for (int planet{1}; planet <= n; ++planet) {
        if (indegree[static_cast<std::size_t>(planet)] == 0) {
            // 위와 같은 vector<int>::push_back(const int&) 계약이다. reserve 덕에 peeled는 재할당하지 않는다.
            peeled.push_back(planet);
        }
    }

    // [첫 호출 계약: peeled.size()]
    // (1) 수신 peeled는 size<=n, capacity>=n인 const 관찰 가능 vector<int> lvalue다.
    // (2) 선택 시그니처는 constexpr size_type vector<int>::size() const noexcept다.
    // (3) 명시 인자·소유권 이전이 없다.
    // (4) 현재 원소 수 size_t를 반환해 동적 FIFO 끝 비교에 쓴다.
    // (5) vector 상태는 변하지 않고, 뒤 push_back으로 커진 size는 다음 조건에 반영된다.
    // (6) O(1), noexcept, 무할당·무효화. 동시 수정은 없다.
    for (std::size_t head{}; head < peeled.size(); ++head) {
        const int planet{peeled[head]};
        const int destination{successor[static_cast<std::size_t>(planet)]};
        if (--indegree[static_cast<std::size_t>(destination)] == 0) {
            peeled.push_back(destination);
        }
    }

    // 제거 뒤 indegree>0은 정확히 사이클 정점이다. 각 사이클에 ID와 0-based 위치를 붙인다.
    for (int planet{1}; planet <= n; ++planet) {
        if (indegree[static_cast<std::size_t>(planet)] == 0 ||
            component[static_cast<std::size_t>(planet)] != -1) {
            continue;
        }

        // size()는 현재 발견한 사이클 개수를 size_t로 돌려준다. n 이하이므로 int 변환이 안전하다.
        const int component_id{static_cast<int>(cycle_length.size())};
        int length{};
        int cursor{planet};
        do {
            component[static_cast<std::size_t>(cursor)] = component_id;
            cycle_position[static_cast<std::size_t>(cursor)] = length;
            depth[static_cast<std::size_t>(cursor)] = 0;
            ++length;
            cursor = successor[static_cast<std::size_t>(cursor)];
        } while (cursor != planet);

        // [첫 호출 계약: cycle_length.push_back(length)]
        // (1) 수신 cycle_length는 capacity>=n인 vector<int> lvalue이고 현재 size<n이다.
        // (2) void vector<int>::push_back(const int& value)다.
        // (3) length는 [1,n] int lvalue로 const&에 빌려 복사되며 소유권을 옮기지 않는다.
        // (4) void라 반환값은 없고 ID에 대응하는 길이를 끝에 저장한다.
        // (5) size가 1 늘고 새 int 수명이 시작한다. reserve 덕에 저장소·이전 원소 참조는 유지된다.
        // (6) O(1), 이 호출은 무할당, int 복사는 noexcept. 이전 end만 무효화되며 보관하지 않고,
        //     동시 접근은 없다.
        cycle_length.push_back(length);
    }

    // 다중 출발점 BFS를 vector FIFO로 구현한다. depth 0인 모든 사이클 정점이 seed다.
    std::vector<int> breadth_first;
    breadth_first.reserve(static_cast<std::size_t>(n));
    for (int planet{1}; planet <= n; ++planet) {
        if (depth[static_cast<std::size_t>(planet)] == 0) {
            breadth_first.push_back(planet);
        }
    }

    for (std::size_t head{}; head < breadth_first.size(); ++head) {
        const int current{breadth_first[head]};

        // [첫 호출 계약: range-for의 begin/end, iterator !=/*/++]
        // (1) 수신은 reverse_edges[current]의 살아 있는 vector<int> lvalue이고 순회 중 그 목록을 수정하지 않는다.
        // (2) 숨은 member begin()/end(), iterator operator!=, operator*, operator++가 선택된다.
        // (3) 명시 인자는 없고 predecessor는 역간선 원소 int 값 복사다. 소유권 이전은 없다.
        // (4) begin/end는 경계 iterator, *는 int&, ++는 갱신 iterator&, !=는 bool을 준다.
        // (5) reverse vector·원소는 변하지 않고 iterator만 전진한다.
        // (6) 각 연산 O(1), 무할당·무효화. 순회 중 해당 vector 수정은 iterator 무효화 위험이 있으므로
        //     하지 않고, 동시 접근도 없다.
        for (const int predecessor : reverse_edges[static_cast<std::size_t>(current)]) {
            if (depth[static_cast<std::size_t>(predecessor)] != -1) {
                continue;
            }
            depth[static_cast<std::size_t>(predecessor)] =
                depth[static_cast<std::size_t>(current)] + 1;
            component[static_cast<std::size_t>(predecessor)] =
                component[static_cast<std::size_t>(current)];
            breadth_first.push_back(predecessor);
        }
    }

    // n<=200000에서 2^levels>n이 되는 최소 level 수를 구한다. 이 범위의 shift는 int에서 안전하다.
    int levels{1};
    while ((1 << levels) <= n) {
        ++levels;
    }

    // [첫 호출 계약: 중첩 vector fill 생성]
    // (1) jump는 생성 전이고 참조·저장소가 없다.
    // (2) 바깥 vector<vector<int>>(levels, vector<int>(n+1,0))와 안쪽 fill 생성자를 쓴다.
    // (3) levels와 n+1은 size_t로 변환되는 양의 int 값, 0은 int prvalue다. 임시 안쪽 vector는
    //     각 level에 복사되어 독립 저장소를 만든다.
    // (4) 생성자 반환값은 없고 levels*(n+1)개 0을 소유한다.
    // (5) 완성 후 각 row는 서로 겹치지 않는다. 예외 시 완성된 임시·row는 자동 정리된다.
    // (6) O(n log n) 시간·공간, 동적 할당, length_error/bad_alloc 가능. 관찰자 없음,
    //     main 수명까지 유지, 동시 접근 없음.
    std::vector<std::vector<int>> jump(
        static_cast<std::size_t>(levels),
        std::vector<int>(static_cast<std::size_t>(n + 1), 0));

    for (int planet{1}; planet <= n; ++planet) {
        jump[0][static_cast<std::size_t>(planet)] =
            successor[static_cast<std::size_t>(planet)];
    }
    for (int level{1}; level < levels; ++level) {
        for (int planet{1}; planet <= n; ++planet) {
            const int halfway{
                jump[static_cast<std::size_t>(level - 1)][static_cast<std::size_t>(planet)]};
            jump[static_cast<std::size_t>(level)][static_cast<std::size_t>(planet)] =
                jump[static_cast<std::size_t>(level - 1)][static_cast<std::size_t>(halfway)];
        }
    }

    for (int query{}; query < query_count; ++query) {
        int start{};
        int target{};
        std::cin >> start >> target;

        int answer{-1};
        if (component[static_cast<std::size_t>(start)] ==
            component[static_cast<std::size_t>(target)]) {
            if (depth[static_cast<std::size_t>(target)] > 0) {
                // 트리 목적지는 출발지보다 사이클에 더 가까운 후속 조상일 때만 도달한다.
                if (depth[static_cast<std::size_t>(start)] >=
                    depth[static_cast<std::size_t>(target)]) {
                    const int difference{depth[static_cast<std::size_t>(start)] -
                                         depth[static_cast<std::size_t>(target)]};
                    if (advance(jump, start, difference) == target) {
                        answer = difference;
                    }
                }
            } else {
                // 사이클 목적지: 꼬리 depth만큼 먼저 올라간 뒤 순환 위치 차를 추가한다.
                const int entry{
                    advance(jump, start, depth[static_cast<std::size_t>(start)])};
                const int length{
                    cycle_length[static_cast<std::size_t>(component[static_cast<std::size_t>(start)])]};
                const int around{
                    (cycle_position[static_cast<std::size_t>(target)] -
                     cycle_position[static_cast<std::size_t>(entry)] + length) %
                    length};
                answer = depth[static_cast<std::size_t>(start)] + around;
            }
        }

        // [첫 호출 계약: basic_ostream<char>::operator<<(int), operator<<(char)]
        // (1) 수신 std::cout은 출력 가능한 std::ostream lvalue, answer는 [-1,n]의 int lvalue다.
        // (2) member basic_ostream<char>& operator<<(int)와 비멤버 char 삽입 overload다.
        // (3) answer는 값으로 읽고 '\n'은 char prvalue다. 버퍼·포인터 소유권을 넘기지 않는다.
        // (4) 두 연산은 ostream&를 반환해 연쇄에 쓰고 마지막 참조는 버린다.
        // (5) 출력 buffer에 문자가 추가되고 실패 시 stream 상태 bit가 바뀐다. answer는 변하지 않는다.
        // (6) 자릿수에 선형, buffer 할당 가능, 외부 vector 참조 무효화 없음. 기본은 I/O 오류를
        //     badbit/failbit로 기록하며, 같은 stream을 동시 사용하지 않는다.
        std::cout << answer << '\n';
    }

    return 0;
}
