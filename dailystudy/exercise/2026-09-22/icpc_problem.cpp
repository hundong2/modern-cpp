/*
CSES 1680 — Longest Flight Route
출처: https://cses.fi/problemset/task/1680/

문제 요약:
도시 1에서 도시 n까지 단방향 비행편을 이어 가되, 방문하는 도시 수가 최대인 경로를
찾는다. 비행 네트워크에는 방향 사이클이 없으므로 모든 도시에 위상 순서를 매길 수 있다.
최장 경로가 여러 개라면 그중 아무 것이나 출력해도 된다.

입력:
첫 줄에 도시 수 n과 비행편 수 m이 주어지고, 이어지는 m개 줄마다 단방향 간선 a -> b가
주어진다.

출력:
경로가 있으면 첫 줄에 방문 도시 수, 둘째 줄에 도시 1부터 n까지의 경로를 출력한다.
경로가 없으면 IMPOSSIBLE을 출력한다.

제약:
2 <= n <= 100000, 1 <= m <= 200000, 1 <= a,b <= n, 입력 그래프는 DAG이다.
시간 제한 1초, 메모리 제한 512MB이다.

예제 입력:
5 5
1 2
2 5
1 3
3 4
4 5

예제 출력(가능한 답 하나):
4
1 3 4 5
*/

#include <cstddef>  // std::size_t: 컨테이너 크기와 첨자를 표현하는 부호 없는 정수 타입이다.
#include <iostream> // std::cin/std::cout, 스트림 설정과 >>/<< 입출력 연산을 제공한다.
#include <queue>    // std::queue: 진입 차수가 0이 된 도시를 FIFO 순서로 보관한다.
#include <vector>   // std::vector: 인접 리스트, DP, 직전 도시, 복원 경로를 동적 배열로 소유한다.

// using은 새 타입을 만드는 것이 아니라 긴 기존 타입에 읽기 쉬운 별칭을 붙인다.
// 바깥 vector의 템플릿 인자는 std::vector<int>, 안쪽 vector의 템플릿 인자는 int다.
using City = int;
using Graph = std::vector<std::vector<City>>;

namespace {
// constexpr 객체는 컴파일 시간 상수로 사용할 수 있다. -1은 유효한 도시 수와 겹치지 않는다.
constexpr int unreachable{-1};
} // namespace

/*
초보자 문법 지도:
- int는 여기서 도시 번호·간선 수·경로 길이에 충분한 기본 정수 타입이고, std::size_t는 첨자용이다.
- T name{} 중괄호 초기화는 숫자를 0으로 만들며 축소 변환을 막는다. const는 초기화 뒤 재대입을 금지한다.
- main의 반환형 int는 운영체제에 종료 상태를 돌려주며, 이 제출본은 명령행 매개변수를 쓰지 않아 ()가 비어 있다.
- if/for/while은 각각 조건 분기/횟수 반복/조건 반복이다. 각 중괄호가 변수의 유효 범위와 수명을 정한다.
- const std::vector<int>&는 복사·소유권 이전 없이 기존 배열을 읽기만 하는 lvalue 참조다. 포인터와 달리
  정상적인 참조는 null이 아니며 다른 객체로 다시 바인딩되지 않는다. nullptr는 아래 tie 호출에서만
  "연결할 스트림 없음"을 나타내는 포인터 값이다.
- struct와 class는 모두 사용자 정의 타입을 선언한다. 기본 접근만 struct는 public, class는 private이다.
  생성자가 있는 class라면 멤버 초기화 목록으로 멤버를 먼저 구성하고, 단일 인자 생성자에는 explicit을
  붙여 뜻밖의 암시 변환을 막는 것이 보통이다. 이 대회 풀이는 별도 객체가 필요 없어 평평한 main을 쓴다.
*/

// 대표 알고리즘 문서: ../algorithm/topological-sort.md
int main() {
    // [첫 호출 계약: std::ios_base::sync_with_stdio]
    // (1) 수신 객체가 없는 정적 함수이며, 첫 입출력 전 C/C++ 표준 스트림 동기화 상태를 설정한다.
    // (2) static bool std::ios_base::sync_with_stdio(bool sync = true)에서 bool 오버로드를 선택한다.
    // (3) false는 bool prvalue이며 버퍼나 소유권을 넘기지 않고 동기화 해제를 요청한다.
    // (4) 이전 동기화 상태 bool을 반환하지만 이 코드는 의도적으로 사용하지 않는다.
    // (5) 이후 C++ 스트림이 C stdio와 독립 버퍼링될 수 있고 기존 표준 스트림 객체의 수명은 유지된다.
    // (6) 표준은 일반 복잡도·할당·예외 상한을 정하지 않는다. 반드시 첫 I/O 전에 호출하고 C/C++ I/O를
    //     섞지 않는다. 설정 중 다른 스레드가 같은 표준 스트림을 사용하지 않는 단일 스레드 제출이다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::basic_ios<char>::tie]
    // (1) 수신 std::cin은 프로그램 수명 동안 유효한 std::istream lvalue이며 출력 스트림에 묶일 수 있다.
    // (2) std::basic_ostream<char>* tie(std::basic_ostream<char>* tiestr) 비-const 오버로드를 선택한다.
    // (3) nullptr 포인터 prvalue는 새 연결이 없다는 뜻이고 어느 스트림의 소유권도 이전하지 않는다.
    // (4) 이전 연결 스트림의 비소유 포인터를 반환하지만 여기서는 버린다.
    // (5) std::cin의 자동 flush 연결만 해제되고 std::cin/std::cout 및 버퍼의 수명은 유지된다.
    // (6) 표준은 일반 복잡도·할당 상한을 정하지 않는다. tie 순환은 미정의 동작 위험이 있으나 nullptr는
    //     순환을 만들지 않는다. 오류는 스트림 상태/예외 설정에 따르며 동시 재설정은 외부 동기화가 필요하다.
    std::cin.tie(nullptr);

    int city_count{};
    int flight_count{};
    // [첫 호출 계약: std::istream 정수 추출 operator>>]
    // (1) 수신 std::cin은 읽을 수 있는 std::istream lvalue이고 두 int 객체는 0으로 초기화되어 살아 있다.
    // (2) basic_istream<char>::operator>>(int&) 정수 오버로드를 두 번 연쇄해 선택한다.
    // (3) city_count와 flight_count는 쓰기 가능한 int lvalue 참조로 전달되며 소유권 이동은 없다.
    // (4) 각 호출은 같은 std::istream&를 반환해 다음 추출의 수신자로 쓰고 마지막 반환 참조는 버린다.
    // (5) 성공하면 두 정수와 입력 위치가 바뀌고, 실패하면 failbit/eofbit 등의 상태가 설정될 수 있다.
    // (6) 표준은 일반 복잡도·할당 상한을 정하지 않는다. 형식·범위 오류는 상태 비트(예외 mask가 켜지면
    //     예외)로 보고된다. 변수 참조는 무효화되지 않으며 같은 스트림의 동시 읽기는 외부 동기화가 필요하다.
    std::cin >> city_count >> flight_count;

    // city_count는 최대 100000이어서 +1도 int 범위지만, 컨테이너 크기 타입으로 명시 변환한다.
    const std::size_t storage_size{static_cast<std::size_t>(city_count) + 1U};

    // [첫 호출 계약: std::vector(count)와 중첩 원소 기본 생성]
    // (1) 아직 graph 객체/저장소가 없으며 Graph는 std::vector<std::vector<int>> 정확한 타입이다.
    // (2) outer의 explicit vector(size_type count, const Allocator& = Allocator())를 선택하고 각 원소인
    //     std::vector<int>에는 기본 생성자 vector()가 적용된다. 템플릿 인자는 각각 vector<int>, int다.
    // (3) storage_size는 [3,100001]의 size_t lvalue 값을 복사해 전달하며 외부 저장소 소유권은 없다.
    // (4) 생성자는 반환값이 없고, 완성된 graph 객체 자체가 결과다.
    // (5) graph는 storage_size개의 빈 내부 vector와 바깥 연속 저장소를 독점 소유한다.
    // (6) 시간/공간 O(n), 길이 초과 시 length_error, 할당 실패 시 bad_alloc 가능하며 실패 시 이미 만든
    //     원소는 정리된다. 아직 관찰자·참조가 없어 무효화 대상이 없고 graph 수명은 main 블록 끝까지다.
    Graph graph(storage_size);

    // [첫 호출 계약: std::vector(count, value)]
    // (1) 각 std::vector<int> 객체는 아직 없고 storage_size는 유효하며 채울 int 값 0이 살아 있다.
    // (2) vector(size_type count, const int& value, const Allocator& = Allocator()) 오버로드를 선택한다.
    // (3) 두 인자로 storage_size 값과 int prvalue 0을 받는다. 0은 호출 동안 const int&에 바인딩되고
    //     소유권은 없다.
    // (4) 생성자는 반환값 없이 완성된 vector 객체를 만든다.
    // (5) indegree가 storage_size개의 독립된 0을 소유한다. 같은 오버로드로 best/predecessor도 구성한다.
    // (6) 시간/공간 O(n), length_error/bad_alloc 가능, int 복사는 예외를 던지지 않아 실패 시 효과가 남지
    //     않는다. 생성 중 외부 참조 무효화는 없고 세 vector의 원소 수명은 각 vector 수명에 묶인다.
    std::vector<int> indegree(storage_size, 0);
    std::vector<int> best(storage_size, unreachable);
    std::vector<int> predecessor(storage_size, unreachable);

    for (int edge{}; edge < flight_count; ++edge) {
        int from{};
        int to{};
        // 앞에서 계약을 설명한 같은 int 추출 오버로드다. 공식 입력이므로 성공을 전제로 한다.
        std::cin >> from >> to;

        // [첫 호출 계약: 바깥 vector::operator[]와 안쪽 vector::push_back]
        // (1) graph는 storage_size개의 내부 std::vector<int>를 소유하고 from은 [1,city_count]다.
        // (2) 먼저 std::vector<std::vector<int>>::operator[](size_type)의 비-const 오버로드, 이어서 선택된
        //     내부 함수 void std::vector<int>::push_back(const int&)를 호출한다.
        // (3) from을 변환한 size_t prvalue는 유효 첨자다. to는 int lvalue로 빌려 복사되며 소유권 이전과
        //     xvalue 이동은 없다.
        // (4) operator[]의 vector<int>&는 즉시 push_back 수신자로 쓰이고, push_back은 값을 반환하지 않는다.
        // (5) graph[from] 끝에 to 복사본이 생긴다. 용량이 모자라면 그 내부 vector 저장소만 재할당된다.
        // (6) 첨자는 O(1), push_back은 분할 상환 O(1)·재할당 시 현재 길이에 선형이다. 유효 첨자가
        //     전제이며 위반은 미정의 동작이다. 재할당 시 해당 내부 vector의 모든 반복자/참조/포인터가,
        //     아니면 과거 end만 무효화된다. bad_alloc 가능, int에서 강한 예외 보장, 동시 쓰기는 금지다.
        graph[static_cast<std::size_t>(from)].push_back(to);

        // [첫 호출 계약: std::vector<int>::operator[] 비-const]
        // (1) indegree는 storage_size개의 int를 소유하고 to는 [1,city_count]라 유효한 위치를 가리킨다.
        // (2) reference std::vector<int>::operator[](size_type pos) 비-const 오버로드를 선택한다.
        // (3) to에서 만든 size_t prvalue는 값으로 전달되며 소유권·별도 저장소를 요구하지 않는다.
        // (4) 해당 int의 lvalue 참조를 반환하고 전위 ++가 그 값을 직접 1 증가시킨다.
        // (5) 그 원소만 변하며 vector 크기/용량/주소와 다른 인자들은 그대로다.
        // (6) O(1), 무할당이며 유효 범위에서는 예외가 없다. 범위 검사를 하지 않아 잘못된 첨자는 미정의
        //     동작이다. 구조 변경이 없어 참조 무효화가 없고 같은 원소의 동시 수정은 데이터 경쟁이다.
        ++indegree[static_cast<std::size_t>(to)];
    }

    // [첫 호출 계약: std::queue 기본 생성자]
    // (1) ready와 기반 컨테이너가 아직 없으며 정확한 타입은 std::queue<int>다.
    // (2) queue() 기본 생성자를 선택해 기본 기반 컨테이너 std::deque<int>를 값 초기화한다.
    // (3) 명시 인자나 이전 저장소, 소유권 이동이 없다.
    // (4) 생성자는 반환값이 없고 완성된 빈 FIFO 어댑터가 결과다.
    // (5) ready.empty()가 참인 큐가 생기며 원소 수명은 아직 시작하지 않는다.
    // (6) 기반 컨테이너 기본 생성 비용/할당/예외 보장을 따르며 보통 상수 시간이다. 실패하면 객체가
    //     생기지 않는다. 참조 무효화 대상은 없고 ready 수명은 main 블록 끝까지며 단일 스레드에서 쓴다.
    std::queue<int> ready{};

    for (int city{1}; city <= city_count; ++city) {
        // 모든 진입 차수 0 도시를 넣어야 시작점에서 못 가는 선행 도시도 제거된다.
        if (indegree[static_cast<std::size_t>(city)] == 0) {
            // [첫 호출 계약: std::queue<int>::push]
            // (1) ready는 유효한 queue<int> lvalue이고 city는 살아 있는 [1,city_count] int다.
            // (2) void push(const value_type&)에서 value_type=int인 const lvalue 오버로드를 선택한다.
            // (3) city lvalue를 읽기 전용으로 빌려 int 복사본을 넣으며 호출자 소유권은 그대로다.
            // (4) 반환값은 없다.
            // (5) 성공하면 큐 뒤에 도시 하나가 생겨 크기가 1 늘고 city는 변하지 않는다.
            // (6) 기반 deque의 push_back을 따라 보통 O(1)이며 bad_alloc 가능, 실패 시 int에서 효과가 없다.
            //     보관 중인 반복자는 무효화될 수 있으나 queue는 이를 노출하지 않고, 기존 원소 참조는
            //     유지된다. 같은 queue의 동시 접근은 외부 동기화가 필요하다.
            ready.push(city);
        }
    }

    // 도시 수를 DP 값으로 삼는다. 시작 도시 하나를 방문했으므로 1이다.
    best[1U] = 1;

    // 핵심 불변식: current를 꺼낼 때 모든 선행 간선의 시작 도시는 이미 처리되었으므로 best[current]는
    // 더는 개선되지 않는 최종값이다. 도달 불가능한 current도 간선 제거에는 반드시 참여한다.
    while (
        // [첫 호출 계약: std::queue<int>::empty]
        // (1) 수신 ready는 살아 있는 queue<int> lvalue이며 원소가 0개 이상이다.
        // (2) bool empty() const 오버로드를 인자 없이 선택한다.
        // (3) 매개변수·소유권 이동이 없다.
        // (4) 원소가 없으면 true인 bool prvalue를 반환해 !와 while 조건에 사용한다.
        // (5) 큐와 원소, 관찰자는 변하지 않는다.
        // (6) 기반 deque에서 O(1), 무할당·무효화·무예외다. 동시 쓰기 중 읽기는 허용하지 않는다.
        !ready.empty()) {
        // [첫 호출 계약: std::queue<int>::front]
        // (1) while 조건으로 ready가 비어 있지 않은 유효 queue<int> lvalue임이 보장된다.
        // (2) reference front() 비-const 오버로드를 인자 없이 선택한다.
        // (3) 매개변수와 소유권 이동이 없다.
        // (4) 맨 앞 int의 lvalue 참조를 반환하고 그 값을 current에 복사한 뒤 참조는 보관하지 않는다.
        // (5) ready는 변하지 않고 current라는 독립 int가 생긴다.
        // (6) O(1), 무할당이며 비어 있지 않음이 전제다. 빈 큐 호출은 미정의 동작이다. 참조는 해당
        //     원소가 pop될 때 무효화되므로 먼저 값 복사하며, 동시 pop은 외부 동기화 없이는 금지다.
        const int current{ready.front()};

        // [첫 호출 계약: std::queue<int>::pop]
        // (1) ready는 비어 있지 않고 front 값은 이미 current에 복사되어 더는 빌린 참조가 없다.
        // (2) void pop()을 인자 없이 선택하며 기반 deque의 pop_front를 수행한다.
        // (3) 매개변수·소유권 이전이 없고 제거할 원소는 큐가 소유한다.
        // (4) 반환값이 없으므로 제거한 값을 얻으려면 앞처럼 먼저 front를 읽어야 한다.
        // (5) 첫 원소 수명이 끝나고 큐 크기가 1 감소하며 current 복사본은 그대로다.
        // (6) O(1), 이 int 파괴는 무예외·무할당이다. 비어 있지 않음이 전제이며 제거 원소의 참조만
        //     무효화된다. 같은 queue를 다른 스레드가 동시에 접근하지 않는다.
        ready.pop();

        // graph의 outer operator[]가 돌려준 lvalue에 const lvalue 참조를 묶는다. 복사하지 않고 빌리며,
        // 바깥 graph를 크기 변경하지 않으므로 outgoing은 이번 반복 동안 유효하다.
        const std::vector<int>& outgoing{graph[static_cast<std::size_t>(current)]};

        for (std::size_t edge_index{};
             edge_index <
                 // [첫 호출 계약: std::vector<int>::size]
                 // (1) outgoing은 살아 있는 내부 vector<int>에 묶인 const lvalue이고 처리 중 크기가 안 변한다.
                 // (2) size_type size() const noexcept를 인자 없이 선택한다.
                 // (3) 매개변수와 소유권 이동이 없다.
                 // (4) 원소 수 size_t prvalue를 반환해 edge_index와 비교하며 값은 저장하지 않는다.
                 // (5) vector/원소/관찰자는 모두 변하지 않는다.
                 // (6) O(1), 무할당·무효화·무예외다. 같은 vector의 동시 구조 변경은 금지다.
                 outgoing.size();
             ++edge_index) {
            // [첫 호출 계약: std::vector<int>::operator[] const]
            // (1) outgoing은 const vector<int> lvalue이고 edge_index < outgoing.size()가 반복 조건으로 보장된다.
            // (2) const_reference operator[](size_type pos) const 오버로드를 선택한다.
            // (3) edge_index size_t lvalue의 값을 복사해 전달하고 저장소 소유권은 빌리지 않는다.
            // (4) const int&를 반환하며 값을 next에 복사하고 반환 참조는 저장하지 않는다.
            // (5) outgoing과 원소는 변하지 않고 독립된 const int next가 생긴다.
            // (6) O(1), 무할당이며 범위 안에서는 예외가 없다. 범위 밖이면 미정의 동작이다. 구조 변경이
            //     없어 참조 무효화가 없고 같은 원소의 동시 수정은 허용하지 않는다.
            const int next{outgoing[edge_index]};

            // unreachable을 먼저 검사해 sentinel에 1을 더한 가짜 경로로 완화하지 않는다.
            if (best[static_cast<std::size_t>(current)] != unreachable &&
                best[static_cast<std::size_t>(current)] + 1 >
                    best[static_cast<std::size_t>(next)]) {
                best[static_cast<std::size_t>(next)] =
                    best[static_cast<std::size_t>(current)] + 1;
                // 엄격한 개선을 만든 간선만 저장하면 직전 도시 사슬이 실제 최장 경로를 이룬다.
                predecessor[static_cast<std::size_t>(next)] = current;
            }

            if (--indegree[static_cast<std::size_t>(next)] == 0) {
                ready.push(next);
            }
        }
    }

    const int target{city_count};
    if (best[static_cast<std::size_t>(target)] == unreachable) {
        // [첫 호출 계약: ostream C 문자열 삽입 operator<<]
        // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 문자열 리터럴은 정적 수명의 char 배열이다.
        // (2) 비멤버 operator<<(std::basic_ostream<char>&, const char*) 오버로드를 선택한다.
        // (3) 배열이 변환된 const char* prvalue는 NUL 종료 문자열을 빌리며 소유권을 넘기지 않는다.
        // (4) 같은 ostream&를 반환하지만 사용하지 않는다.
        // (5) 성공하면 출력 버퍼/위치가 바뀌고 리터럴은 변하지 않는다.
        // (6) 표준은 일반 복잡도·할당 상한을 정하지 않는다. 유효한 NUL 종료 포인터가 전제다. 실패는 상태
        //     비트 또는 exception mask의 예외로 나타나며, 복합 출력을 다른 스레드와 섞지 않는다.
        std::cout << "IMPOSSIBLE\n";
        return 0;
    }

    // [첫 호출 계약: std::vector<int> 기본 생성자]
    // (1) reversed_route 객체는 아직 없고 초기 원소도 없다.
    // (2) vector() 기본 생성자를 선택하며 allocator 템플릿 인자는 기본 int 할당자다.
    // (3) 명시 인자와 이전 저장소의 소유권 이전이 없다.
    // (4) 생성자는 반환값 없이 빈 vector 객체를 완성한다.
    // (5) size()==0인 경로 버퍼가 생기고 원소 수명은 아직 시작하지 않는다.
    // (6) 상수 시간이며 기본 allocator에서 무예외다. 원소용 동적 저장소는 아직 필요 없고 무효화할
    //     관찰자도 없다. 객체 수명은 main 블록 끝까지며 단일 스레드에서 수정한다.
    std::vector<int> reversed_route{};

    // predecessor[target]부터 -1까지 따라가면 n -> ... -> 1의 역순이다.
    for (int city{target}; city != unreachable;
         city = predecessor[static_cast<std::size_t>(city)]) {
        // 앞에서 설명한 vector<int>::push_back(const int&)와 같은 오버로드다.
        reversed_route.push_back(city);
    }

    const std::size_t route_size{
        // 앞에서 설명한 size() const noexcept와 같은 호출이며 반환값을 출력 반복 경계로 보관한다.
        reversed_route.size()};

    // [첫 호출 계약: ostream int 및 char 삽입 operator<<]
    // (1) std::cout은 출력 가능한 ostream lvalue, best[target]은 유효 int lvalue, '\n'은 char prvalue다.
    // (2) 멤버 basic_ostream<char>::operator<<(int) 뒤 비멤버 operator<<(ostream&, char)를 선택한다.
    // (3) 경로 길이 값을 복사해 서식화하고 문자 값을 출력하며 어떠한 버퍼 소유권도 이전하지 않는다.
    // (4) 첫 ostream& 반환은 다음 호출의 수신자로 쓰고 마지막 ostream&는 버린다.
    // (5) 출력 버퍼/상태가 바뀌고 best와 경로는 그대로다.
    // (6) 표준은 일반 복잡도·할당 상한을 정하지 않는다. 실패는 상태 비트 또는 설정된 예외로 나타난다.
    //     참조/수명 무효화는 없고 같은 stream의 복합 출력에는 외부 스레드 동기화가 필요하다.
    std::cout << best[static_cast<std::size_t>(target)] << '\n';

    for (std::size_t offset{}; offset < route_size; ++offset) {
        // route_size >= 1이고 offset < route_size라 뺄셈은 언더플로하지 않는다.
        const std::size_t index{route_size - 1U - offset};
        std::cout << reversed_route[index]
                  << (offset + 1U == route_size ? '\n' : ' ');
    }

    /*
    정확성·우승 노하우:
    1. Kahn 불변식상 도시를 큐에서 꺼낼 때 모든 들어오는 간선의 시작점이 처리되었다. 따라서 그때
       best[city]는 1에서 city까지 가능한 경로 중 최대 도시 수로 확정된다.
    2. 도달 가능한 current의 모든 간선에 best[next] = max(best[next], best[current]+1)을 적용하므로
       위상 순서 귀납법으로 DP가 모든 경로와 그중 최댓값을 빠짐없이 고려한다.
    3. 엄격히 개선한 current를 predecessor[next]에 저장했으므로 target에서 거꾸로 따라간 각 쌍은
       실제 간선이고, 그 사슬 길이는 best[target]이다. 뒤에서 앞으로 출력하면 1 -> n 최장 경로다.
    4. 시작점에서 도달 불가능한 정점도 Kahn 큐에 넣어 간선을 제거해야 한다. 전체 indegree를 세고
       시작점만 큐에 넣으면, 도달 가능한 정점으로 들어오는 "도달 불가능한 선행 간선" 때문에 멈춘다.
    5. 각 정점/간선을 상수 번씩 처리하므로 시간 O(n+m), 그래프·DP·큐·경로 공간 O(n+m)이다.

    값 범주·수명·소유권:
    graph[...]와 queue::front() 결과는 lvalue 참조이고, outgoing은 그 lvalue에 묶인 const lvalue
    참조다. city_count를 size_t로 바꾼 식과 산술 결과는 prvalue다. 명시적 std::move가 없어 xvalue나
    소유권 이전은 없으며 각 vector/queue가 저장소를 독점 소유한다. 지역 객체는 main 끝에서 생성의
    역순으로 파괴된다. 이 코드는 객체를 값으로 반환하지 않으므로 RVO가 개입할 식은 없고, 별도 함수가
    지역 vector를 반환하도록 리팩터링하면 컴파일러는 NRVO를 적용할 수 있다.

    기계 실행 관점:
    indegree/best/predecessor 접근은 대체로 주소 계산 뒤 load/store, 비교와 if/while은 조건 분기,
    queue의 front/pop/push는 기반 컨테이너 접근, 인접 리스트 순회는 연속 int 읽기로 구현될 수 있다.
    가상 함수 호출은 사용하지 않아 가상 간접 호출이 필요하지 않지만, 실제 명령·분기 제거·인라이닝·
    메모리 배치는 CPU, ABI, 표준 라이브러리 구현, 컴파일러와 최적화 옵션에 따라 달라진다.
    */

    return 0;
}
