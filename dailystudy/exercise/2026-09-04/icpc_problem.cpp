/*
문제 ID·제목: CSES 1691 - Mail Delivery
출처: CSES Problem Set / Graph Algorithms
출처 URL: https://cses.fi/problemset/task/1691

문제 요약:
교차로 1의 우체국에서 출발해 모든 양방향 거리를 정확히 한 번씩 지나고 다시 교차로 1로 돌아오는
경로를 찾아야 한다. 아래 설명은 저작권이 있는 문제 원문을 복제하지 않고 조건을 한글로 재서술한 것이다.

입력:
첫 줄에 교차로 수 n과 거리 수 m이 주어진다. 다음 m줄의 a, b는 서로 다른 두 교차로를 잇는
양방향 거리 하나를 뜻한다. 교차로 번호는 1부터 n까지다.

출력:
조건을 만족하는 경로가 있으면 방문 순서대로 m+1개의 교차로 번호를 출력한다. 가능한 경로가 여러 개면
아무 경로나 허용된다. 그런 경로가 없으면 IMPOSSIBLE을 출력한다.

공식 제약:
2 <= n <= 100,000, 1 <= m <= 200,000, 1 <= a,b <= n.
공식 입력에서는 자기 루프와 같은 두 교차로 사이의 중복 거리가 없지만, 아래 edge-id 구현은 그런
무방향 멀티그래프에도 같은 원리로 동작한다. 시간 제한은 1초, 메모리 제한은 512MB다.

예제:
입력
6 8
1 2
1 3
2 3
2 4
2 6
3 5
3 6
4 5

가능한 출력
1 2 6 3 2 4 5 3 1
*/

// <cstddef>는 컨테이너 크기·인덱스와 같은 부호 없는 값에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력 std::cin, 표준 출력 std::cout과 스트림 설정·연산자를 선언한다.
#include <iostream>
// <vector>는 인접 목록, 간선 사용 배열, 명시적 스택과 답 경로를 소유하는 std::vector를 선언한다.
#include <vector>

// 한 인접 목록 항목은 도착 정점과 “무방향 거리 하나”를 식별하는 번호를 함께 보관한다.
// struct의 멤버는 class와 달리 기본 public이다. 이 작은 값 객체에는 불변식을 숨길 필요가 없어 struct가 알맞다.
// int{}는 0으로 값 초기화한다. 사용자 정의 생성자가 없으므로 멤버 초기화 목록이나 explicit 생성자는 없다.
// 생성자가 인자 하나로 암시적 변환을 만들 수 있었다면 explicit로 의도치 않은 변환을 막아야 한다.
struct IncidentEdge {
    int to{};
    int id{};
};

// using은 새 소유 타입을 만드는 것이 아니라 긴 템플릿 타입에 읽기 쉬운 별칭을 붙인다.
// 바깥 vector의 템플릿 인자는 std::vector<IncidentEdge>, 안쪽 vector의 템플릿 인자는 IncidentEdge다.
using Graph = std::vector<std::vector<IncidentEdge>>;

// 실패 출력을 한 함수에 모아 두 실패 분기가 정확히 같은 문자열을 내도록 한다.
void print_impossible() {
    // [호출 계약: std::ostream 문자열 삽입]
    // (1) 수신 객체는 프로그램 수명의 표준 출력 객체 std::cout, 정확한 타입은 std::ostream인 유효한 lvalue다.
    // (2) 선택 오버로드는 operator<<(basic_ostream<char>&, const char*)이며 char 특성은
    //     char_traits<char>다.
    // (3) 인자는 null로 끝나는 문자열 리터럴에서 변환된 const char* prvalue다. 비소유 읽기 포인터이고
    //     가리키는 배열은 정적 수명이며 null이 아니다.
    // (4) 반환형은 std::ostream&이고 같은 std::cout을 뜻하지만 여기서는 반환값을 버린다.
    // (5) 성공하면 문자들이 출력 버퍼에 기록되고 인자·소유권·다른 객체는 바뀌지 않는다.
    // (6) 포인터가 유효해야 한다. 비용은 문자 수와 버퍼/장치에 의존하고 별도 할당·참조 무효화는 없다.
    //     실패는 failbit/badbit에 남으며 예외 마스크가 켜졌다면 ios_base::failure가 날 수 있다. 여러 스레드의
    //     논리적 출력 묶음을 원자적으로 만드는 보장은 없지만 이 프로그램은 단일 스레드다.
    std::cout << "IMPOSSIBLE\n";
}

int main() {
    // [호출 계약: std::ios::sync_with_stdio]
    // (1) 정적 함수라 수신 인스턴스는 없고, 표준 스트림에서 아직 어떤 입출력도 하지 않은 상태다.
    // (2) 시그니처는 static bool std::ios::sync_with_stdio(bool sync = true)이며 bool 오버로드 하나다.
    // (3) 인자는 bool prvalue false이고 소유권을 전달하지 않으며 허용된 두 bool 값 중 하나다.
    // (4) 반환형 bool은 호출 전 동기화 설정을 뜻하지만 사용하지 않는다.
    // (5) C stdio와 C++ 표준 스트림의 동기화를 해제하며 std::cin/std::cout의 수명과 소유권은 유지된다.
    // (6) 첫 I/O 뒤 호출 효과는 구현에 따라 달라질 수 있어 반드시 지금 호출한다. 복잡도·할당·예외 보장은
    //     별도로 규정되지 않고, C/C++ I/O 혼용 순서나 애플리케이션 수준 스레드 안전을 보장하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::ios::sync_with_stdio(false);

    // [호출 계약: basic_ios<char>::tie setter]
    // (1) 수신 객체는 정상 상태인 std::cin lvalue이며 정확한 타입은 std::istream이다.
    // (2) 선택 시그니처는 std::ostream* tie(std::ostream* tied_stream)인 setter 오버로드다.
    // (3) nullptr prvalue가 null std::ostream*로 변환된다. 소유권을 주지 않으며 null은 허용된다.
    // (4) 반환형 std::ostream*는 이전 tie 대상의 비소유 포인터지만 사용하지 않는다.
    // (5) 호출 뒤 입력 전 자동 flush 연결이 없어지고 두 표준 스트림과 버퍼의 수명은 변하지 않는다.
    // (6) 상수 시간·무할당이며 참조를 무효화하지 않는다. 대화형 문제가 아니므로 선행 출력 flush가 필요 없다.
    //     이 설정 자체는 오류를 보고하거나 스레드 동기화를 제공하지 않으며 noexcept라고 단정할 필요도 없다.
    std::cin.tie(nullptr);

    int crossing_count{}; // 기본 타입 int를 0으로 값 초기화하며 공식 최댓값 100,000을 안전하게 담는다.
    int street_count{};   // 거리 수 200,000도 int 범위 안이다.

    // [호출 계약: std::istream 정수 추출 체인]
    // (1) 첫 수신자는 유효한 std::cin/std::istream lvalue이고, 두 번째 수신자는 첫 호출이 반환한 같은 스트림이다.
    // (2) 두 호출 모두 멤버 std::istream& operator>>(int&) 오버로드가 선택된다.
    // (3) crossing_count와 street_count는 각각 수정 가능한 int lvalue다. 비소유 참조로 전달되며 공식 입력은
    //     표현 가능 범위의 정수 두 개를 제공한다.
    // (4) 각 반환형은 같은 std::istream&이고 연쇄의 마지막 반환값은 버린다.
    // (5) 성공하면 두 변수와 스트림 읽기 위치가 갱신되고, 실패하면 대상 값과 스트림 상태는 실패 종류에 따라
    //     정해진 방식으로 바뀐다. 변수 소유권과 수명은 그대로다.
    // (6) 선행 공백을 건너뛸 수 있어 비용은 소비 문자·locale·버퍼에 의존한다. EOF/형식/범위 실패는
    //     eofbit/failbit를 설정하고 예외 마스크에 따라 ios_base::failure를 던질 수 있다. 할당·관찰자 무효화나
    //     별도 스레드 보장은 없으며 이 문제는 유효한 입력을 전제로 한다.
    std::cin >> crossing_count >> street_count;

    // [호출 계약: Graph의 std::vector count 생성자]
    // (1) 아직 존재하지 않는 정확한 타입 Graph, 즉 vector<vector<IncidentEdge>> 객체를 새로 구성한다.
    // (2) 선택 시그니처는 explicit vector(size_type count, const Allocator& = Allocator())이고, 바깥 템플릿
    //     인자는 vector<IncidentEdge>와 allocator<vector<IncidentEdge>>다.
    // (3) 인자는 crossing_count+1을 변환한 size_t prvalue다. 소유권 의미는 없고 max_size 이하이며 1기반
    //     인덱싱을 위해 0번 칸까지 만든다.
    // (4) 생성자는 반환값이 없다.
    // (5) 성공하면 count개의 빈 안쪽 vector가 adjacency에 소유되고 size는 count가 된다.
    // (6) O(n) 초기화와 바깥 저장소 할당이 필요하다. 너무 크면 length_error, 메모리 부족이면 bad_alloc이
    //     가능하다. 새 객체라 무효화할 기존 관찰자는 없고 원소는 adjacency 파괴 때까지 RAII로 정리된다.
    Graph adjacency(static_cast<std::size_t>(crossing_count) + std::size_t{1});

    for (int edge_id{}; edge_id < street_count; ++edge_id) {
        int from{};
        int to{};
        // 같은 int 추출 오버로드를 두 번 사용한다. from/to는 수정 가능한 lvalue이고 마지막 istream&는 버린다.
        // 앞서 적은 추출 계약과 동일하며 공식 입력이 1..n 범위를 보장한다.
        std::cin >> from >> to;

        // 무방향 간선 하나를 양 끝 인접 목록에 두 번 넣되 같은 id를 공유한다. 정점 방문 여부만 기록하면
        // 평행 간선이나 되돌아오는 간선을 구분하지 못하지만 edge id는 “거리 정확히 한 번”을 직접 표현한다.
        // [호출 계약: 바깥 vector::operator[]와 안쪽 vector::push_back]
        // (1) 바깥 수신자는 size가 n+1인 Graph adjacency lvalue이고, 이어지는 수신자는 선택된 유효한
        //     std::vector<IncidentEdge> lvalue다.
        // (2) 먼저 vector<vector<IncidentEdge>>::operator[](size_type)의 비const 오버로드가
        //     vector<IncidentEdge>&를 반환하고, 다음에는 void vector<IncidentEdge>::push_back(IncidentEdge&&)
        //     rvalue 오버로드가 선택된다.
        // (3) [] 인자는 [1,n]인 size_t prvalue다. push 인자는 IncidentEdge{상대 정점, edge_id} prvalue이고
        //     두 int 값만 옮기며 외부 자원 소유권은 없다.
        // (4) [] 반환 참조는 바로 push 수신자로 사용하고, push_back 반환형 void에는 사용할 값이 없다.
        // (5) 각 push 뒤 해당 안쪽 vector 크기가 1 증가하고 새 IncidentEdge를 소유한다. 바깥 vector 크기는
        //     그대로이며 입력 변수도 바뀌지 않는다.
        // (6) []는 O(1)이고 범위 검사가 없어 범위 위반은 UB지만 입력 제약이 배제한다. push는 분할 상환 O(1),
        //     재할당 시 그 안쪽 vector의 기존 반복자·포인터·참조가 모두 무효다. 지금은 관찰자를 저장하지 않는다.
        //     IncidentEdge의 이동은 예외가 없지만 할당은 bad_alloc 가능성이 있으며 외부 동시 접근은 없다.
        // IncidentEdge{...}는 prvalue이고 rvalue 참조에 바인딩된다. 단순 int 두 개의 이동/복사는 동일한 값 복사다.
        adjacency[static_cast<std::size_t>(from)].push_back(IncidentEdge{to, edge_id});
        adjacency[static_cast<std::size_t>(to)].push_back(IncidentEdge{from, edge_id});
    }

    // 무방향 오일러 회로의 필요조건은 모든 정점 차수가 짝수라는 것이다. 자기 루프라면 같은 id를 두 번
    // 저장하므로 차수에도 2가 더해진다. 공식 입력에는 자기 루프가 없지만 표현 자체는 올바르다.
    for (int vertex{1}; vertex <= crossing_count; ++vertex) {
        // [호출 계약: vector<IncidentEdge>::size]
        // (1) 수신자는 adjacency 안에 살아 있는 유효한 const 관찰 가능 vector<IncidentEdge> lvalue다.
        // (2) 시그니처는 constexpr size_type size() const noexcept다.
        // (3) 매개변수는 없다.
        // (4) 반환형 size_t는 현재 인접 항목 수, 즉 무방향 차수이며 나머지 연산에 즉시 사용한다.
        // (5) 호출 뒤 컨테이너·원소·소유권·수명은 전혀 변하지 않는다.
        // (6) O(1), 무할당, 비무효화, noexcept다. adjacency 인덱스는 앞 계약과 같이 범위 안이고 단일
        //     스레드라 동시 변경 문제가 없다.
        if (adjacency[static_cast<std::size_t>(vertex)].size() % std::size_t{2} != std::size_t{0}) {
            print_impossible();
            return 0;
        }
    }

    // 반복형 Hierholzer의 상세한 조건·불변식·정확성 증명은 다음 공용 문서를 따른다.
    // ../algorithm/eulerian-circuit-hierholzer.md

    // next_incident[v] 앞의 인접 항목들은 이미 선택했거나 반대쪽에서 사용된 간선이다.
    // [호출 계약: vector<size_t> fill 생성자]
    // (1) 정확한 타입 std::vector<std::size_t>의 새 객체를 구성하며 기존 상태는 없다.
    // (2) 선택 시그니처는 vector(size_type count, const value_type& value,
    //     const Allocator& = Allocator())이고 템플릿 인자는 size_t와 allocator<size_t>다.
    // (3) count는 n+1인 size_t prvalue, value는 const size_t&에 바인딩되는 0 prvalue다. 소유권 전달은 없고
    //     count는 max_size 이하다.
    // (4) 생성자는 반환값이 없다.
    // (5) 성공하면 n+1개의 0을 소유하며 모든 정점의 검색 커서가 첫 인접 항목을 가리킨다.
    // (6) O(n) 시간·공간, length_error/bad_alloc 가능성이 있다. 새 객체라 기존 관찰자 무효화는 없으며
    //     파괴 시 저장소가 자동 해제된다. 외부 동시 접근은 없다.
    std::vector<std::size_t> next_incident(
        static_cast<std::size_t>(crossing_count) + std::size_t{1}, std::size_t{0});

    // used[id]는 거리 id를 어느 끝에서 처음 택했는지와 무관하게 정확히 한 번만 1이 된다.
    // [호출 계약: vector<char> fill 생성자]
    // (1) 정확한 타입 std::vector<char>의 새 객체를 구성한다.
    // (2) 선택 시그니처는 vector(size_type, const char&, const Allocator& = Allocator())이며 템플릿 인자는
    //     char와 allocator<char>다.
    // (3) count는 m인 size_t prvalue, value는 const char&에 바인딩되는 char{0} prvalue다. m은 음수가 아니고
    //     max_size 이하며 자원 소유권을 전달하지 않는다.
    // (4) 생성자는 반환값이 없다.
    // (5) 성공하면 크기 m의 0 배열을 소유해 모든 거리를 미사용으로 표시한다.
    // (6) O(m) 시간·공간, length_error/bad_alloc 가능성이 있다. 새 객체이므로 기존 관찰자는 없고 char 원소는
    //     컨테이너와 같은 수명을 가진다. 별도 스레드 동기화는 없다.
    std::vector<char> used(static_cast<std::size_t>(street_count), char{0});

    // [호출 계약: vector<int> 기본 생성자 두 번]
    // (1) 정확한 타입 std::vector<int>인 vertex_stack과 reversed_route를 각각 새로 구성한다.
    // (2) 선택 시그니처는 vector()이며 템플릿 인자는 int와 allocator<int>다.
    // (3) 명시적 매개변수와 소유권 전달은 없다.
    // (4) 생성자는 반환값이 없다.
    // (5) 두 객체 모두 size 0의 유효한 소유 컨테이너가 된다.
    // (6) 상수 시간이고 표준 allocator에서 빈 객체는 원소 저장소를 요구하지 않는다. 기존 관찰자·원소는 없으며
    //     각 지역 객체의 수명 끝에 RAII로 정리된다. 생성 실패 가능성은 사용 allocator에 의존한다.
    std::vector<int> vertex_stack;
    std::vector<int> reversed_route;

    const std::size_t maximum_route_size{
        static_cast<std::size_t>(street_count) + std::size_t{1}};
    // [호출 계약: vector<int>::reserve 두 번]
    // (1) 수신자는 각각 size 0인 유효한 vector<int> lvalue이고 요청 전 관찰자는 없다.
    // (2) 시그니처는 void reserve(size_type new_capacity)다.
    // (3) 인자는 m+1인 size_t lvalue maximum_route_size에서 값으로 복사된다. max_size 이하여야 하며 입력
    //     제약상 만족한다. 원소 소유권은 전달하지 않는다.
    // (4) 반환형은 void다.
    // (5) 성공하면 size는 0, capacity는 적어도 m+1이어서 이후 int push가 재할당하지 않는다.
    // (6) 현재 size에 선형이나 빈 컨테이너에서는 원소 이동이 없다. 저장소 할당으로 length_error/bad_alloc이
    //     가능하고, 일반적으로 재할당하면 모든 반복자·참조가 무효지만 현재는 없다. 실패 시 int의 무예외 이동
    //     조건에서 원래 vector가 유지되며 스레드 동기화는 제공하지 않는다.
    vertex_stack.reserve(maximum_route_size);
    reversed_route.reserve(maximum_route_size);

    // [호출 계약: vector<int>::push_back(int&&)]
    // (1) 수신자는 size 0이고 capacity가 m+1 이상인 vertex_stack lvalue다.
    // (2) 선택 시그니처는 void push_back(value_type&& value), 즉 int&& rvalue 오버로드다.
    // (3) 인자는 우체국 번호 int prvalue 1이다. 자원 소유권이 없는 숫자이고 표현 가능하다.
    // (4) 반환형 void로 사용할 반환값이 없다.
    // (5) 성공하면 스택 크기가 1 증가하고 값 1의 수명이 컨테이너 안에서 시작된다.
    // (6) 분할 상환 O(1)이며 reserve 때문에 여기서는 할당·관찰자 무효화가 없다. 일반적으로 재할당은 모든
    //     관찰자를 무효화할 수 있고 bad_alloc 가능성이 있다. int 이동·소멸은 예외를 던지지 않으며 단일 스레드다.
    vertex_stack.push_back(1);

    // 핵심 불변식:
    // 1) used[id]가 1인 간선만 현재 walk에 들어갔으며 같은 id는 다시 선택되지 않는다.
    // 2) vertex_stack은 아직 회로에 확정하지 않은 현재 walk를 순서대로 보관한다.
    // 3) 현재 정점에 미사용 간선이 없을 때만 그 정점을 reversed_route에 확정한다.
    // 반복형이라 최악 200,000개 간선에서도 재귀 호출 스택 한도를 사용하지 않는다.
    // [호출 계약: vector<int>::empty]
    // (1) 수신자는 유효한 vertex_stack lvalue이며 루프 중 size가 0 이상이다.
    // (2) 시그니처는 constexpr bool empty() const noexcept다.
    // (3) 매개변수는 없다.
    // (4) 반환 bool을 즉시 부정해 루프 조건으로 사용한다.
    // (5) 객체·원소·capacity·소유권은 바뀌지 않는다.
    // (6) O(1), 무할당, 비무효화, noexcept이며 동시 수정은 없으므로 읽기 경쟁도 없다.
    while (!vertex_stack.empty()) {
        // [호출 계약: vector<int>::back]
        // (1) 수신자는 while 조건으로 비어 있지 않음이 증명된 vertex_stack lvalue다.
        // (2) 선택 시그니처는 reference back(), 반환형은 int&인 비const 오버로드다.
        // (3) 매개변수는 없다.
        // (4) 마지막 int의 lvalue 참조를 반환하며 즉시 city에 값 복사한다.
        // (5) 호출 자체는 상태를 바꾸지 않고 city가 독립 int를 갖는다. 따라서 뒤의 pop 뒤에도 city는 유효하다.
        // (6) O(1), 무할당·비무효화다. 빈 vector에서 호출하면 UB지만 조건이 배제한다. 반환 참조는 해당 원소
        //     제거·재할당·파괴 전까지만 유효하고, 이 코드는 참조를 저장하지 않는다. 스레드 동기화는 없다.
        const int city{vertex_stack.back()};

        // 바깥 operator[] 결과는 adjacency가 소유하는 안쪽 vector의 lvalue 참조다. 바깥 vector는 구성 뒤
        // 재할당하지 않으므로 이 참조는 루프 동안 안정적이고, 소유권은 adjacency에 남는다.
        const std::vector<IncidentEdge>& incidents{
            adjacency[static_cast<std::size_t>(city)]};

        // [호출 계약: vector<size_t>::operator[]]
        // (1) 수신자는 크기 n+1인 next_incident lvalue이며 city는 [1,n]이다.
        // (2) 선택 시그니처는 reference operator[](size_type), 반환형 std::size_t&인 비const 오버로드다.
        // (3) 인자는 city를 변환한 size_t prvalue이고 범위 안이다. 소유권 의미는 없다.
        // (4) 해당 정점 커서의 lvalue 참조를 반환해 cursor에 비소유 바인딩하며 사용한다.
        // (5) 호출만으로 vector는 변하지 않지만 이후 ++cursor가 그 원소 값을 갱신한다.
        // (6) O(1), 무할당·비무효화이고 범위 검사가 없어 잘못된 인덱스는 UB다. next_incident는 이후
        //     재할당하지 않아 cursor 수명은 안전하며 단일 스레드에서만 쓴다.
        std::size_t& cursor{next_incident[static_cast<std::size_t>(city)]};

        // 커서는 각 인접 항목을 단조 증가로 한 번만 본다. 반대쪽에서 이미 사용한 id면 건너뛴다.
        while (cursor < incidents.size()) {
            // [호출 계약: vector<IncidentEdge> const operator[]와 vector<char> operator[]]
            // (1) 첫 수신자는 size보다 작은 cursor가 증명된 const incidents lvalue, 둘째 수신자는 크기 m인
            //     used lvalue이며 candidate.id는 입력 때 부여한 [0,m) 값이다.
            // (2) 첫 시그니처는 const_reference operator[](size_type) const로 const IncidentEdge&를 반환한다.
            //     둘째는 reference vector<char>::operator[](size_type)로 char&를 반환한다.
            // (3) 두 인덱스는 각각 size_t lvalue 값과 edge id를 변환한 size_t prvalue이며 모두 범위 안이다.
            //     소유권 전달은 없다.
            // (4) 첫 참조는 candidate에 비소유 바인딩하고, 둘째 참조의 char 값은 비교에 읽는다.
            // (5) 이 두 호출은 객체를 바꾸지 않는다. incidents/used는 재할당하지 않아 참조 수명도 유지된다.
            // (6) 둘 다 O(1), 무할당·비무효화다. 범위 위반은 검사되지 않아 UB지만 불변식이 배제한다.
            //     예외를 던질 연산은 없으며 외부 동시 접근도 없다.
            const IncidentEdge& candidate{incidents[cursor]};
            if (used[static_cast<std::size_t>(candidate.id)] == char{0}) {
                break;
            }
            ++cursor;
        }

        if (cursor == incidents.size()) {
            // 미사용 간선이 없는 정점은 현재 부분 회로의 뒤쪽으로 확정한다. 이후 전체 목록을 뒤집어 읽으면
            // 작은 회로들이 올바른 위치에 이어 붙은 Hierholzer 회로가 된다.
            // [호출 계약: vector<int>::push_back(const int&)]
            // (1) 수신자는 size<=m이고 capacity>=m+1인 reversed_route lvalue다.
            // (2) 선택 시그니처는 void push_back(const value_type& value), 즉 const int& 복사 오버로드다.
            // (3) 인자는 const int lvalue city로 비소유 바인딩되며 유효한 교차로 번호다.
            // (4) 반환형 void로 사용할 반환값이 없다.
            // (5) 성공하면 답 배열 크기가 1 증가하고 city 값의 독립 복사본을 소유한다. 원본 city는 그대로다.
            // (6) 분할 상환 O(1), reserve로 실제 재할당·무효화가 없다. 일반적으로 bad_alloc과 재할당에 따른
            //     모든 관찰자 무효화가 가능하지만 int 복사는 예외를 던지지 않는다. 외부 동시 접근은 없다.
            reversed_route.push_back(city);

            // [호출 계약: vector<int>::pop_back]
            // (1) 수신자는 현재 city를 마지막에 가진 비어 있지 않은 vertex_stack lvalue다.
            // (2) 시그니처는 void pop_back()다.
            // (3) 매개변수는 없다.
            // (4) 반환형 void이며 제거 값을 반환하지 않는다. 그래서 위에서 city로 먼저 복사했다.
            // (5) 마지막 int의 수명이 끝나고 size가 1 감소하며 capacity는 유지된다.
            // (6) O(1), 무할당이다. 빈 컨테이너 호출은 UB지만 while이 배제한다. 제거 원소 참조와 기존
            //     past-the-end 반복자는 무효화된다. int 소멸은 예외가 없고 단일 스레드다.
            vertex_stack.pop_back();
            continue;
        }

        // incidents[cursor]는 lvalue다. 지역 edge로 값을 복사하면 다음 반복에서 참조 수명이나 컨테이너 상태를
        // 신경 쓰지 않아도 된다. 이 지역 객체의 수명은 블록 끝까지이며 소유 자원은 없다.
        const IncidentEdge edge{incidents[cursor]};
        ++cursor; // 선택한 인접 항목을 다시 보지 않도록 다음 위치로 이동한다.
        used[static_cast<std::size_t>(edge.id)] = char{1}; // 같은 무방향 거리의 양쪽 항목을 함께 사용 처리한다.
        vertex_stack.push_back(edge.to); // 앞서 설명한 vector<int> const lvalue 복사 push 계약과 같다.

        // 기계 실행 관점에서는 cursor/used를 메모리에서 읽고 id를 비교한 뒤 조건 분기하며, 선택 시 used를
        // 저장하고 정점 번호를 연속 vector 저장소에 쓴다. 가상 함수나 가상 간접 호출은 없다. 다만 실제
        // load/store 수, 분기 제거·예측, 함수 인라이닝은 CPU·컴파일러·최적화 옵션에 따라 달라진다.
    }

    // 짝수 차수만 검사해서는 부족하다. 1과 분리된 다른 짝수 차수 컴포넌트가 있을 수 있다. 1에서 만든
    // reversed_route가 정확히 m+1개 정점을 가질 때만 m개의 모든 edge id가 소비되었다고 결론 낸다.
    // [호출 계약: vector<int>::size]
    // (1) 수신자는 알고리즘이 끝난 유효한 reversed_route lvalue다.
    // (2) 시그니처는 constexpr size_type size() const noexcept다.
    // (3) 매개변수는 없다.
    // (4) 반환 size_t는 확정한 경로 정점 수이며 maximum_route_size와 비교해 즉시 사용한다.
    // (5) 객체·원소·capacity·소유권과 수명은 바뀌지 않는다.
    // (6) O(1), 무할당·비무효화, noexcept다. 단일 스레드라 동시 접근 문제도 없다.
    if (reversed_route.size() != maximum_route_size) {
        print_impossible();
        return 0;
    }

    // 모든 차수가 짝수이고 모든 간선을 썼으므로 역순 경로의 시작과 끝은 모두 우체국 1이다.
    // route는 소유 vector를 반환하지 않고 바로 출력하므로 이 풀이에는 RVO가 적용될 반환 객체가 없다.
    // 지역 vector들은 main 종료 때 역순으로 파괴되어 저장소를 자동 해제하며, 이것이 RAII 소유권 경계다.
    for (std::size_t index{reversed_route.size()}; index > std::size_t{0}; --index) {
        // [호출 계약: vector<int> const 관찰 operator[]]
        // (1) 수신자는 크기 maximum_route_size인 reversed_route lvalue이고 index-1은 범위 안이다.
        // (2) 이 비const 객체에서 선택되는 시그니처는 reference operator[](size_type), 반환형 int&다.
        // (3) 인자는 index-1인 size_t prvalue로 [0,size)이며 소유권 의미가 없다.
        // (4) 해당 int의 lvalue 참조가 반환되고 city 값 초기화에 복사되어 참조는 저장하지 않는다.
        // (5) 호출과 복사 뒤 vector는 변하지 않고 지역 city가 독립 값을 가진다.
        // (6) O(1), 무할당·비무효화다. 범위 위반은 UB지만 루프 조건이 배제한다. vector가 더는 변경되지 않아
        //     참조 수명도 안정적이고 외부 동시 접근은 없다.
        const int city{reversed_route[index - std::size_t{1}]};

        // [호출 계약: std::ostream 정수 삽입]
        // (1) 수신자는 유효한 std::cout/std::ostream lvalue다.
        // (2) 선택 오버로드는 멤버 std::ostream& operator<<(int)다.
        // (3) 인자는 city에서 lvalue-to-rvalue 변환된 int 값이며 소유권을 전달하지 않는다.
        // (4) 반환형은 같은 std::ostream&지만 연쇄하지 않고 버린다.
        // (5) 성공하면 십진 표현이 출력 버퍼에 추가되고 city·다른 객체는 변하지 않는다.
        // (6) 비용은 숫자 변환·locale·버퍼/장치에 의존한다. 할당·컨테이너 무효화는 없다. 실패 시 상태 비트,
        //     예외 마스크가 켜졌다면 ios_base::failure로 보고하며 출력 레코드의 스레드 원자성은 보장하지 않는다.
        std::cout << city;
        if (index != std::size_t{1}) {
            // [호출 계약: std::ostream 문자 삽입]
            // (1) 수신자는 앞 출력 뒤에도 유효한 std::cout/std::ostream lvalue다.
            // (2) 선택 오버로드는 operator<<(basic_ostream<char>&, char)다.
            // (3) 인자는 공백 char prvalue이며 소유권·수명 전달이 없다.
            // (4) 같은 std::ostream&를 반환하지만 사용하지 않는다.
            // (5) 성공하면 문자 하나가 버퍼에 추가되고 프로그램 데이터는 그대로다.
            // (6) 별도 복잡도 상한은 없고 실제 비용은 버퍼에 의존한다. 할당·참조 무효화는 없으며 실패/예외와
            //     스레드 보장은 위 정수 삽입 계약과 같다.
            std::cout << ' ';
        }
    }
    std::cout << '\n'; // 같은 char 삽입 계약으로 마지막 줄바꿈을 기록한다.

    // 정확성 요약:
    // - 각 id를 처음 만날 때만 used=1로 바꾸므로 어떤 거리도 두 번 지나지 않는다.
    // - 막힌 정점을 뒤에서부터 확정하는 것은 닫힌 부분 회로를 기존 walk에 끼워 넣는 것과 동치다.
    // - 짝수 차수와 route size==m+1 검사가 각각 닫힘과 모든 거리 사용을 보증한다.
    // 따라서 출력이 있다면 1에서 시작·종료하며 모든 무방향 간선을 정확히 한 번 지난다.
    // 각 인접 항목을 cursor가 한 번만 넘으므로 시간 O(n+m), 그래프·배열·스택·경로 공간 O(n+m)이다.
    // 대회 팁: 연결성 DFS를 따로 돌릴 필요 없이 route 길이로 남은 edge id를 검출하면 구현이 짧고 안전하다.

    return 0;
}
