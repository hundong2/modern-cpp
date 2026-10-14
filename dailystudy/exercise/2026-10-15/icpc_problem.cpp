/*
문제 ID/제목: CSES 1196 - Flight Routes
출처: https://cses.fi/problemset/task/1196/

문제 요약:
도시 1에서 도시 n까지 갈 수 있는 route 가운데 비용이 작은 순서로 k개의 가격을 구한다. 한 route는
같은 도시나 간선을 다시 지날 수 있다. 서로 다른 route의 총비용이 같으면 그 가격도 각각 한 번씩
출력해야 한다. 저작권이 있는 원문 전체 대신 풀이에 필요한 조건만 한국어로 요약한다.

입력:
- 첫 줄에 도시 수 n, 단방향 항공편 수 m, 필요한 답의 수 k가 주어진다.
- 이어지는 m개 줄마다 출발 도시 a, 도착 도시 b, 양의 비용 c가 주어진다.
- 2 <= n <= 100,000, 1 <= m <= 200,000, 1 <= k <= 10,
  1 <= a,b <= n, 1 <= c <= 1,000,000,000이다.
- 도시 1에서 n까지 서로 다른 route가 적어도 k개 존재함이 보장된다.

출력:
중복 가격을 제거하지 않고 k개의 최소 route 비용을 비감소 순서로 한 줄에 출력한다.

예제(설명을 위한 작은 자체 예제):
입력
3 4 3
1 2 1
2 3 2
1 3 5
2 2 1
출력
3 4 5

풀이 요약:
최소 힙의 상태 하나를 실제 walk 접두사 하나로 보고, 정점마다 비용순 pop을 최대 k번 받아들인다.
도착점의 1~k번째 유효 pop 비용이 답이다. 같은 (비용, 정점) 상태도 합치지 않아 route multiplicity를
보존한다. 자세한 불변식과 증명은 ../algorithm/k-shortest-walks-best-first.md에 둔다.
*/

// <cstddef>는 컨테이너 크기와 인덱스에 쓰는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 누적 비용을 정확히 64비트로 저장할 std::int64_t를 선언한다.
#include <cstdint>
// <iostream>은 표준 입력·출력 객체와 정수 추출·삽입 연산을 선언한다.
#include <iostream>
// <queue>는 최소 후보를 관리할 std::priority_queue를 선언한다.
#include <queue>
// <vector>는 인접 리스트, 확정 횟수, 답을 소유할 std::vector를 선언한다.
#include <vector>

namespace {

// Edge는 한 단방향 간선의 도착 정점과 양의 비용을 값으로 소유한다.
// struct의 멤버는 기본 public이라 대회용 단순 데이터 묶음에 적합하다. class였다면 기본 private다.
struct Edge {
    int to{};
    std::int64_t cost{};
};

// Candidate는 발견된 특정 walk 접두사의 누적 비용과 마지막 정점을 값으로 소유한다.
// 같은 값의 Candidate도 서로 다른 route 접두사일 수 있으므로 중복 제거하지 않는다.
struct Candidate {
    std::int64_t cost{};
    int vertex{};
};

// priority_queue의 비교자는 “left의 우선순위가 right보다 낮은가?”를 답한다.
// 비용이 큰 left에 true를 주면 가장 작은 비용이 top에 오는 최소 힙이 된다.
struct CandidateGreater {
    [[nodiscard]] bool operator()(const Candidate& left,
                                  const Candidate& right) const noexcept {
        return left.cost > right.cost;
    }
};

// std::int64_t가 표현하는 최댓값이다. 덧셈 뒤 검사하면 signed overflow 자체가 이미 UB이므로,
// 반드시 current.cost + edge.cost를 계산하기 전에 MAX-edge.cost와 비교한다.
constexpr std::int64_t max_distance{9'223'372'036'854'775'807LL};

} // namespace

int main() {
    // [첫 호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 객체 수신자가 없는 정적 설정 함수이며, 아직 어느 표준 stream에서도 I/O하지 않은 상태다.
    // (2) 선택 시그니처는 static bool ios_base::sync_with_stdio(bool sync = true)다.
    // (3) false는 bool prvalue이고 소유권 의미가 없으며 C stdio와 C++ iostream의 동기화 해제를 요청한다.
    // (4) 이전 동기화 설정 bool을 반환하지만 이 코드는 사용하지 않는다.
    // (5) 이후 표준 C++ stream은 독립 버퍼링될 수 있고 cin/cout 객체와 버퍼의 소유권은 유지된다.
    // (6) 표준은 별도 점근 복잡도·noexcept를 보장하지 않는다. 첫 I/O 뒤 호출 효과는 구현 정의이므로
    //     지금 호출하며, 동기화를 끈 stream을 여러 스레드에서 동시에 사용하지 않는다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie(nullptr)]
    // (1) 수신 cin은 프로그램 수명의 유효한 istream lvalue이고 cout에 묶여 있을 수 있다.
    // (2) 선택 overload는 ostream* basic_ios::tie(ostream* tied)다.
    // (3) nullptr는 소유권 없는 null pointer prvalue이며 새로운 출력 stream을 지정하지 않는다.
    // (4) 이전 tie 대상 포인터를 반환하지만 사용하지 않는다.
    // (5) 입력 전 자동 cout flush 연결만 해제되고 두 stream과 buffer의 수명은 바뀌지 않는다.
    // (6) 표준은 별도 복잡도·noexcept를 규정하지 않는다. non-null tie 순환은 UB 위험이 있지만
    //     nullptr는 안전하고, 전역 stream 설정을 다른 스레드와 동시에 바꾸지 않는다.
    std::cin.tie(nullptr);

    // 기본 타입 int는 문제의 정점·간선·k 상한을 담는다. {}는 0으로 값 초기화해 미초기화 읽기를 막는다.
    int city_count{};
    int flight_count{};
    int answer_count{};

    // [첫 호출 계약: std::cin >> int 세 번과 basic_ios::operator!]
    // (1) 최초 수신 cin은 유효하고 good일 수 있는 istream lvalue이며, 세 int는 수정 가능한 0 lvalue다.
    // (2) basic_istream::operator>>(int&) overload를 왼쪽부터 세 번 고르고, 연쇄 결과에
    //     bool basic_ios::operator!() const를 적용한다.
    // (3) 각 인자는 non-const int lvalue 참조로 빌려 주며 외부 저장소나 소유권을 넘기지 않는다.
    // (4) 각 추출은 같은 istream&를 반환해 연쇄하고 operator!은 fail/bad 상태 여부 bool을 if가 쓴다.
    // (5) 성공하면 n,m,k를 저장하고 입력 위치를 전진한다. 실패하면 상태 비트를 세우며 즉시 종료한다.
    // (6) formatted 입력 복잡도에는 공통 점근 상한이 없고 locale/buffer 작업이 개입할 수 있다. 예외 mask가
    //     설정되면 ios_base::failure가 가능하고 부분 대입은 롤백되지 않는다. 단일 스레드에서만 읽는다.
    if (!(std::cin >> city_count >> flight_count >> answer_count)) {
        return 0;
    }

    // [첫 생성 계약: vector<vector<Edge>>(count)]
    // (1) 수신 graph는 아직 수명이 시작되지 않은 지역 vector<vector<Edge>> 객체다.
    // (2) explicit vector(size_type count) overload가 선택되고 두 vector 모두 기본 allocator를 쓴다.
    // (3) count는 n+1의 size_t prvalue다. n<=100000이라 표현 가능하고 외부 저장소를 빌리지 않는다.
    // (4) 생성자는 반환값 없이 count개의 빈 vector<Edge>를 소유하는 graph를 완성한다.
    // (5) 0번을 비워 둔 1-based 인접 리스트가 생기고 입력 정수는 변하지 않는다.
    // (6) O(n)개의 내부 vector 생성과 O(n) 저장 공간이다. 길이/할당 실패 시 예외와 정리가 일어나며
    //     성공 뒤 graph 재할당 전까지 내부 vector 객체 참조는 유효하다. scope 종료 시 중첩 vector
    //     소멸자가 모든 Edge와 저장소를 정리한다. 공유하지 않아 데이터 경쟁이 없다.
    std::vector<std::vector<Edge>> graph(
        static_cast<std::size_t>(city_count) + 1U);

    for (int edge_index{}; edge_index < flight_count; ++edge_index) {
        int from{};
        int to{};
        // long long은 표준상 최소 64비트이며 CSES의 간선 비용 상한을 충분히 담는다.
        long long input_cost{};

        // [첫 호출 계약: std::cin >> long long]
        // (1) 수신 cin은 헤더 추출에 성공한 유효한 istream lvalue이고 input_cost는 0인 long long lvalue다.
        // (2) 앞의 두 호출은 이미 설명한 operator>>(int&), 마지막은 operator>>(long long&) overload다.
        // (3) from/to/input_cost를 non-const lvalue 참조로 빌리며 소유권 이전은 없다. 공식 입력 범위다.
        // (4) 각 호출은 같은 istream&를 반환하며 여기서는 최종 참조를 버린다.
        // (5) 성공하면 세 값과 입력 위치가 바뀐다. 공식 입력을 전제해 반복 안에서 별도 실패 분기는 없다.
        // (6) formatted 입력의 복잡도/버퍼 동작은 구현과 locale에 좌우된다. 예외 mask에 따른 예외와
        //     부분 대입 가능성이 있고, 동기화 해제한 cin을 다른 스레드와 공유하지 않는다.
        std::cin >> from >> to >> input_cost;

        // [첫 호출 계약: vector::operator[]와 vector<Edge>::push_back(Edge&&)]
        // (1) graph는 크기 n+1인 vector<vector<Edge>> lvalue이고 graph[from]은 유효한 내부 vector다.
        // (2) graph에는 reference operator[](size_type), 내부 vector에는 void push_back(value_type&&)가
        //     선택되며 value_type은 Edge다.
        // (3) from은 공식 제약으로 [1,n]인 int lvalue가 size_t 값으로 변환된다. Edge{to,cost}는
        //     aggregate prvalue/xvalue로 전달되어 두 정수 값을 내부 저장소가 소유하며 외부 자원은 없다.
        // (4) operator[]는 vector<Edge>&를 반환해 push의 수신자로 쓰고 push_back은 void를 반환한다.
        // (5) 해당 출발점의 끝에 간선 하나가 추가된다. graph 크기와 다른 인접 리스트는 그대로다.
        // (6) 인덱싱 O(1), amortized push O(1)이다. from 범위가 깨지면 []는 UB다. push가 재할당하면
        //     그 내부 vector의 기존 원소 참조가 모두 무효화되고 할당 실패가 가능하지만 Edge 이동은
        //     noexcept라 강한 보장을 유지한다. 같은 graph를 병행 수정하지 않는다.
        graph[static_cast<std::size_t>(from)].push_back(
            Edge{to, static_cast<std::int64_t>(input_cost)});
    }

    // [첫 생성 계약: vector<int>(count, value)]
    // (1) accepted는 아직 생성 전인 지역 vector<int>다.
    // (2) vector(size_type count, const value_type& value) overload와 기본 allocator가 선택된다.
    // (3) count는 n+1 size_t prvalue, value는 0 int prvalue에서 만든 const 참조이며 소유권은 없다.
    // (4) 생성자는 반환값 없이 count개의 0을 소유하는 vector를 완성한다.
    // (5) 모든 정점의 유효 pop 횟수가 0이고 graph/입력은 그대로다.
    // (6) O(n) 시간·공간이며 길이/할당 예외가 가능하고 실패 시 부분 원소를 정리한다. 구조 변경을
    //     하지 않아 이후 원소 참조는 vector 수명까지 안정적이다. scope 종료 시 저장소를 해제한다.
    std::vector<int> accepted(
        static_cast<std::size_t>(city_count) + 1U, 0);

    // [첫 생성 계약: priority_queue<Candidate, vector<Candidate>, CandidateGreater>()]
    // (1) frontier는 아직 생성 전인 adaptor 객체다.
    // (2) 기본 생성자가 stateless CandidateGreater와 빈 vector<Candidate>를 내부 생성한다.
    // (3) 명시 인자는 없고 세 template 인자는 값 타입, 소유 컨테이너, 비교자다. 외부 저장소는 없다.
    // (4) 생성자는 반환값 없이 empty()==true인 최소 힙 adaptor를 완성한다.
    // (5) graph와 accepted는 변하지 않고 아직 Candidate를 소유하지 않는다.
    // (6) O(1), 원소 무할당이며 기본 allocator/비교자 생성은 이 타입들에서 무예외다. 이후 scope
    //     종료 시 내부 vector가 모든 Candidate 저장소를 해제하고 관찰자를 무효화한다. 공유하지 않는다.
    std::priority_queue<Candidate,
                        std::vector<Candidate>,
                        CandidateGreater> frontier{};

    // [첫 생성 계약: vector<int64_t>()]
    // (1) answers는 아직 생성 전인 지역 vector<std::int64_t>다.
    // (2) 기본 allocator를 쓰는 vector() 기본 생성자가 선택된다.
    // (3) 명시 인자가 없고 외부 저장소·소유권을 받지 않는다.
    // (4) 반환값 없이 size()==0, capacity() 구현 허용값인 유효한 빈 vector를 만든다.
    // (5) 다른 객체는 변하지 않고 아직 원소/관찰자가 없다.
    // (6) O(1)이며 기본 allocator에서는 noexcept다. scope 종료 시 소유 저장소를 해제한다.
    std::vector<std::int64_t> answers{};

    // [첫 호출 계약: vector<int64_t>::reserve(count)]
    // (1) 수신 answers는 유효한 빈 vector lvalue다.
    // (2) void reserve(size_type new_capacity) 멤버가 선택된다.
    // (3) answer_count는 [1,10]인 int lvalue에서 size_t prvalue로 변환되며 소유권 의미가 없다.
    // (4) 반환형 void라 반환값이 없다.
    // (5) capacity가 적어도 k가 되고 size는 0이다. 뒤의 k번 push에서 재할당하지 않는다.
    // (6) O(k) 이하 저장 확보이며 length_error/bad_alloc이 가능하다. 실패 시 answers는 변하지 않는다.
    //     new_capacity가 기존 capacity보다 커 실제 재할당될 때만 기존 관찰자가 모두 무효화되고,
    //     그렇지 않으면 아무 관찰자도 무효화되지 않는다. 지금은 빈 vector라 원소 관찰자가 없다.
    //     다른 스레드와 공유하지 않는다.
    answers.reserve(static_cast<std::size_t>(answer_count));

    // [첫 호출 계약: priority_queue::push(Candidate&&)와 숨은 비교자 호출]
    // (1) 수신 frontier는 빈 priority_queue lvalue이고 내부 vector와 CandidateGreater가 유효하다.
    // (2) void push(value_type&&)가 선택되고 내부 heap 정렬 중 필요하면
    //     bool CandidateGreater::operator()(const Candidate&, const Candidate&) const noexcept를 쓴다.
    // (3) Candidate{0,1}은 빈 walk를 나타내는 prvalue/xvalue다. push가 값을 소유하며 외부 자원은 없다.
    //     비교 인자는 내부 Candidate lvalue를 const 참조로 빌리고 수정하지 않는다.
    // (4) push는 void, 비교자는 “왼쪽 비용이 더 커 우선순위가 낮다”는 bool을 내부에서만 사용한다.
    // (5) frontier가 시작 후보 하나를 소유하고 top은 최소 비용 후보가 된다. 다른 객체는 그대로다.
    // (6) amortized O(log(size+1)), 첫 삽입은 O(1) 비교다. 내부 vector 재할당은 그 저장소의 참조를
    //     무효화하고 bad_alloc 가능성이 있으나 Candidate 이동·비교는 무예외라 실패 시 기존 힙은
    //     유지된다. comparator는 strict weak ordering이며 frontier를 병행 접근하지 않는다.
    frontier.push(Candidate{0, 1});

    // 불변식: frontier에는 아직 처리하지 않은 실제 walk 접두사가 비용순 heap으로 들어 있다.
    // answers는 도착점에서 이미 받아들인 비용을 비감소 순서로 보관한다.
    // [첫 호출 계약: priority_queue::empty()와 vector<int64_t>::size()]
    // (1) frontier와 answers는 유효한 const 관찰 가능 lvalue이고 현재 각각 후보/답을 소유한다.
    // (2) bool priority_queue::empty() const와 size_type vector::size() const noexcept가 선택된다.
    // (3) 두 호출 모두 데이터 인자가 없고 저장소를 빌리거나 소유권을 옮기지 않는다.
    // (4) empty는 후보 없음 bool, size는 답 개수 size_t prvalue를 반환해 && 조건이 사용한다.
    // (5) 두 컨테이너의 원소·크기·capacity는 변하지 않는다.
    // (6) 둘 다 O(1)·무할당·무효화 없음이다. vector::size는 noexcept이고 empty도 상수 시간이며
    //     이 구현의 무예외 underlying empty를 전달한다. 동시 수정 없이 단일 스레드에서 관찰한다.
    while (!frontier.empty()
           && answers.size() < static_cast<std::size_t>(answer_count)) {
        // [첫 호출 계약: priority_queue::top()과 pop()]
        // (1) while 조건으로 비어 있지 않은 frontier lvalue가 최소 Candidate를 소유한다.
        // (2) const_reference top() const 뒤 void pop()이 선택된다. pop은 내부 pop_heap과
        //     vector::pop_back을 사용하며 CandidateGreater를 0회 이상 호출한다.
        // (3) 명시 인자는 없다. top의 const Candidate&를 current 값으로 먼저 복사해 소유한다.
        // (4) top은 최소 후보 const 참조를 반환해 복사 초기화에 쓰고 pop 반환값은 없다.
        // (5) current는 독립 값을 보존하고 frontier는 최소 원소 하나를 제거한 유효한 heap이다.
        // (6) top O(1), pop O(log size), 무할당이다. 빈 heap의 top/pop은 전제조건 위반이지만 while이
        //     막는다. pop 뒤 top 참조는 무효라 저장하지 않았다. Candidate 비교/이동은 noexcept이고
        //     다른 스레드가 frontier를 접근하지 않는다.
        const Candidate current{frontier.top()};
        frontier.pop();

        const std::size_t vertex_index{
            static_cast<std::size_t>(current.vertex)};

        // [첫 호출 계약: vector<int>::operator[](index)]
        // (1) 수신 accepted는 크기 n+1인 수정 가능한 vector<int> lvalue다.
        // (2) reference operator[](size_type pos) overload가 선택된다.
        // (3) vertex_index는 생성된 후보 불변식으로 [1,n]인 size_t 값이며 소유권 의미가 없다.
        // (4) int&를 반환해 비교하고, 유효 후보이면 바로 아래 전위 증가의 수정 대상으로 쓴다.
        // (5) 읽기만 한 식은 변화가 없고 ++ 식은 해당 원소만 1 늘린다. 크기/capacity는 그대로다.
        // (6) O(1)·무할당이며 []는 범위를 검사하지 않아 범위 위반 시 UB다. 입력/간선 불변식이
        //     범위를 보장한다. 구조 변경이 없어 다른 원소 참조도 유지되며 병행 쓰기는 없다.
        if (accepted[vertex_index] >= answer_count) {
            continue;
        }
        ++accepted[vertex_index];

        if (current.vertex == city_count) {
            // [첫 호출 계약: vector<int64_t>::push_back(const int64_t&)]
            // (1) answers는 size<k이고 reserve(k)로 충분한 capacity를 가진 vector lvalue다.
            // (2) current.cost가 const lvalue라 void push_back(const value_type&) overload가 선택된다.
            // (3) 인자는 유효한 비음수 int64_t lvalue이며 값을 복사할 뿐 Candidate 소유권은 그대로다.
            // (4) 반환형 void라 반환값은 없다.
            // (5) 도착 비용이 끝에 하나 추가되고 이미 pop 순서라 비감소 순서를 유지한다.
            // (6) 예약 범위에서 O(1), 무할당·무효화 없음이다. 일반적으로 capacity 초과면 재할당과
            //     bad_alloc 가능성이 있으나 여기서는 k를 예약했다. 같은 vector 병행 접근은 없다.
            answers.push_back(current.cost);
            if (answers.size() == static_cast<std::size_t>(answer_count)) {
                break;
            }
        }

        // graph의 비const operator[] 계약은 입력 시 설명했다. 같은 범위 불변식으로 O(1)에 내부
        // vector<Edge>&를 얻고 const 참조에 바인딩한다. 소유권은 graph에 남는다.
        const std::vector<Edge>& outgoing{graph[vertex_index]};

        // 인덱스 반복을 써 range-for의 숨은 begin/end/*/++ 호출을 만들지 않는다.
        // [첫 호출 계약: const vector<Edge>::size()와 operator[](index)]
        // (1) 수신 outgoing은 살아 있는 graph 원소를 가리키는 const vector<Edge> lvalue다.
        // (2) size_type size() const noexcept와 const_reference operator[](size_type) const를 고른다.
        // (3) size에는 인자가 없고 edge_index는 loop 조건으로 [0,size())인 size_t 값이다.
        // (4) size는 간선 수 prvalue, []는 const Edge&를 반환하며 둘 다 즉시 읽기에 사용한다.
        // (5) graph, outgoing, 인덱스와 간선은 변하지 않는다.
        // (6) 둘 다 O(1)·무할당이다. size는 noexcept, []는 범위 검사를 하지 않아 범위 위반 시 UB지만
        //     loop 조건이 막는다. graph를 구조 변경하지 않아 참조 수명이 유지되고 병행 수정도 없다.
        for (std::size_t edge_index{};
             edge_index < outgoing.size();
             ++edge_index) {
            const Edge& edge{outgoing[edge_index]};
            const std::size_t next_index{static_cast<std::size_t>(edge.to)};

            // 이미 이웃의 k개 접두사가 확정됐다면 더 늦은 접두사는 전체 앞 k개 답에 필요 없다.
            if (accepted[next_index] >= answer_count) {
                continue;
            }

            // edge.cost는 양수다. 뺄셈 결과도 int64_t 범위이며 이 비교를 통과한 뒤에만 덧셈한다.
            if (current.cost > max_distance - edge.cost) {
                continue;
            }

            // frontier.push와 숨은 comparator/vector 계약은 시작 후보 삽입 때와 같다. 이번에는 기존
            // 원소와 O(log size) 비교하고 필요 시 내부 vector를 재할당한다. 동일 비용도 합치지 않는다.
            frontier.push(Candidate{
                current.cost + edge.cost,
                edge.to});
        }
    }

    // 구현 가까운 대표 문서: ../algorithm/k-shortest-walks-best-first.md
    // 공식 조건에서 answers.size()==k이고 이미 비감소 순서다.
    // [첫 호출 계약: vector<int64_t>::operator[]와 cout << long long << char]
    // (1) answers는 k개 값을 소유한 수정 가능 vector lvalue지만 읽기만 하고, cout은 유효한 ostream이다.
    // (2) answers에는 reference operator[](size_type), 출력에는 member operator<<(long long)과
    //     비멤버 operator<<(basic_ostream&, char)가 차례로 선택된다.
    // (3) index는 [0,size())의 size_t 값이다. 원소를 long long prvalue로 값 변환하고 separator는
    //     ' ' 또는 '\n' char prvalue다. 어떤 인자도 소유권을 옮기지 않는다.
    // (4) []는 int64_t&를 반환해 읽고 각 삽입은 같은 ostream&를 반환해 연쇄한다. 최종 참조는 버린다.
    // (5) 답 숫자와 구분 문자가 기록되며 실패하면 cout 상태 비트가 바뀔 수 있다. answers는 그대로다.
    // (6) []는 O(1)·무할당이고 범위 위반은 UB지만 loop가 막는다. 출력 복잡도는 문자 수/장치에
    //     좌우되며 예외 mask에 따른 I/O 예외와 부분 출력이 가능하다. 동기화 해제한 cout을 공유하지
    //     않고, scope 종료 시 모든 표준 컨테이너가 소유 메모리를 정리한다.
    for (std::size_t index{}; index < answers.size(); ++index) {
        const char separator{
            index + 1U == answers.size() ? '\n' : ' '};
        std::cout << static_cast<long long>(answers[index]) << separator;
    }

    // 실행 관점에서는 heap 원소의 load/store, 비용 비교, 조건 분기, 연속 vector 접근이 주가 된다.
    // 실제 명령·인라이닝·할당·분기 예측은 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라
    // 달라지므로 특정 어셈블리 형태나 상수 시간을 단정하지 않는다.
    return 0;
}
