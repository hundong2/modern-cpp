/*
문제 ID/제목: CSES 2079 - Finding a Centroid
출처: https://cses.fi/problemset/task/2079/

문제 요약:
정점 n개인 트리에서 한 정점 c를 고른다. c를 제거했을 때 남는 모든 연결 요소의
정점 수가 floor(n/2) 이하라면 c를 센트로이드라고 한다. 조건을 만족하는 정점 하나를 찾는다.

입력:
첫 줄에 정점 수 n이 주어진다. 다음 n-1줄에는 1-based 정점 a, b를 잇는 무방향 간선이 주어진다.

출력:
가능한 센트로이드 하나의 1-based 번호를 출력한다. 답이 둘이면 어느 쪽도 허용된다.

제약:
1 <= n <= 200000, 1 <= a,b <= n. 입력은 연결되고 사이클이 없는 트리다.

예제:
입력                         출력
5                            3
1 2
2 3
3 4
3 5

공용 알고리즘 문서:
../algorithm/centroid-decomposition-fixed-distance-pairs.md

대회 핵심:
임의의 1번 정점을 임시 루트로 삼는다. 반복 DFS의 부모-선행 순서를 뒤에서부터 읽으면
subtree_size[v]를 계산할 수 있다. v를 제거해 생기는 조각은 각 DFS 자식의 부분 트리와
부모 방향의 n-subtree_size[v]뿐이다. 이들 중 최댓값이 floor(n/2) 이하인 첫 정점이 답이다.
재귀를 쓰지 않아 길이 200000인 사슬에서도 호출 스택 크기에 의존하지 않는다.
*/

// <cstddef>는 컨테이너 크기와 인덱스에 맞는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력 std::cin, 표준 출력 std::cout과 스트림 연산을 선언한다.
#include <iostream>
// <vector>는 인접 목록과 명시적 DFS 스택을 소유하는 연속 동적 배열을 선언한다.
#include <vector>

// using은 새 정수 타입을 만들지 않고 기본 타입 int에 정점 번호라는 문제 의미를 붙인다.
using Vertex = int;
// 바깥 vector의 원소 타입은 vector<Vertex>이고, 각 안쪽 vector가 한 정점의 이웃을 소유한다.
using Graph = std::vector<std::vector<Vertex>>;

// 구현 근거 문서: ../algorithm/centroid-decomposition-fixed-distance-pairs.md
// class는 기본 접근이 private이고 불변식과 동작을 한 타입에 묶는다. struct는 기본 접근이 public이다.
class TreeCentroidFinder {
public:
    // [첫 호출 계약: std::vector<std::vector<Vertex>>::size]
    // (1) 수신 객체 graph는 main이 소유하며 생성이 끝난 유효한 const Graph lvalue이고 정점별 이웃을 담는다.
    // (2) constexpr size_type vector<value_type>::size() const noexcept 오버로드가 선택된다.
    // (3) 명시적 매개변수는 없고, 숨은 수신 객체만 읽는다. 소유권을 가져오지 않는다.
    // (4) 반환형은 Graph::size_type이며 정점 수를 반환하고 int 멤버 초기화에 사용한다.
    // (5) 호출 뒤 graph의 크기·용량·원소와 모든 관찰자의 유효성은 그대로다.
    // (6) O(1), 무할당·무효화·무예외다. 같은 graph를 다른 스레드가 동시에 수정하면 안 된다.
    // explicit은 Graph 하나가 finder로 암시 변환되는 일을 막는다. 멤버 초기화 목록은 함수 본문 전에
    // graph_ 참조를 바인딩하고 node_count_ 값을 직접 초기화한다.
    explicit TreeCentroidFinder(const Graph& graph)
        : graph_{graph}, node_count_{static_cast<int>(graph.size())} {}

    // [[nodiscard]]는 반환 센트로이드를 버린 호출을 컴파일러가 진단할 수 있게 한다.
    // const 멤버 함수는 graph_가 가리키는 트리를 바꾸지 않으며 Vertex 값을 독립 결과로 반환한다.
    [[nodiscard]] Vertex find() const {
        const std::size_t node_count_size{static_cast<std::size_t>(node_count_)};

        // [첫 생성 계약: std::vector<Vertex>(size_type, const Vertex&)]
        // (1) parent와 subtree_size 수신 객체는 아직 수명 전이며 node_count_size는 [1,200000]이다.
        // (2) vector(size_type count, const T& value)에서 T=Vertex인 채움 생성자를 두 번 선택한다.
        // (3) count는 node_count_size prvalue, value는 각각 -2와 1인 int prvalue가 const int&에 잠시
        //     바인딩된다. parent의 -2는 미설정, subtree_size의 1은 자기 자신을 센다는 뜻이다.
        // (4) 생성자는 별도 반환값이 없고 두 vector가 각각 count개 int를 소유하게 한다.
        // (5) 성공 시 size=count다. 인자는 변하지 않고 두 vector의 수명은 find 반환 때까지다.
        // (6) O(count), 저장소 할당이 가능하며 실패 시 bad_alloc/length_error가 가능하다. 생성 중 예외면
        //     완성 원소를 정리한다. 아직 외부 iterator가 없고 같은 객체의 동시 접근도 없다.
        std::vector<Vertex> parent(node_count_size, Vertex{-2});
        std::vector<int> subtree_size(node_count_size, 1);

        // [첫 생성 계약: std::vector<Vertex>()]
        // (1) order와 stack 수신 객체는 아직 수명 전이다.
        // (2) vector() 기본 생성자에서 T=Vertex, Allocator=std::allocator<Vertex>가 선택된다.
        // (3) 명시적 인자가 없고 템플릿 인자 Vertex가 저장 원소 타입을 정한다.
        // (4) 생성자는 별도 반환값 없이 빈 소유 컨테이너를 만든다.
        // (5) 두 객체의 수명이 시작되고 size=0이며 어떤 정점도 아직 소유하지 않는다.
        // (6) 정확히 std::allocator<Vertex>를 쓰는 이 기본 생성은 상수 시간이고 무예외다. 구현의 내부
        //     저장소 확보 횟수까지 단정하지 않는다. 무효화할 관찰자가 없고 객체 수명은 이 블록 끝까지다.
        //     서로 다른 객체는 독립적이지만 같은 vector를 동시 변경하려면 외부 동기화가 필요하다.
        std::vector<Vertex> order{};
        std::vector<Vertex> stack{};

        // [첫 호출 계약: std::vector<Vertex>::reserve]
        // (1) 수신 order와 stack은 각각 size=0인 유효한 vector<Vertex> lvalue다.
        // (2) void reserve(size_type new_cap) 오버로드를 두 객체에 차례로 선택한다.
        // (3) new_cap=node_count_size는 전체 정점 수이며 소유권을 전달하지 않는 정수 값이다.
        // (4) 반환형은 void라 결과값은 없고, 뒤 push_back의 재할당을 피하려고 사용한다.
        // (5) 성공하면 capacity>=new_cap이고 size는 0이다. 재할당했다면 과거 iterator는 무효지만 없다.
        // (6) new_cap<=기존 capacity면 O(1), 재할당하면 현재 size에 선형이며 지금은 size=0이라 옮길
        //     원소가 없다. 저장소 할당에서 bad_alloc/length_error가 가능하고 실패 시 원래 vector를
        //     보존한다. reserve는 vector나 allocator 수명을 연장하지 않으며 같은 객체 동시 접근은 안 된다.
        order.reserve(node_count_size);
        stack.reserve(node_count_size);

        // [첫 호출 계약: std::vector<Vertex>::operator[] 비-const]
        // (1) 수신 parent는 size=node_count_size인 유효한 non-const vector<Vertex> lvalue다.
        // (2) reference operator[](size_type pos) noexcept 오버로드가 선택된다.
        // (3) pos=0인 size_type 값은 n>=1이므로 허용 범위다. 소유권 이동은 없다.
        // (4) 반환형 Vertex&를 대입의 왼쪽 피연산자로 사용해 임시 루트의 부모를 -1로 표시한다.
        // (5) parent[0]만 -1로 바뀌고 크기·용량·다른 원소·관찰자는 그대로다.
        // (6) O(1), 무할당·무효화·무예외다. 범위 밖이면 UB지만 제약으로 배제하며 동시 쓰기는 안 된다.
        parent[0] = Vertex{-1};

        // [첫 호출 계약: std::vector<Vertex>::push_back(Vertex&&)]
        // (1) 수신 stack은 size=0, capacity>=n인 유효한 vector<Vertex> lvalue다.
        // (2) void push_back(value_type&& value)에서 value_type=Vertex인 rvalue 오버로드가 선택된다.
        // (3) Vertex{0}은 0-based 루트 번호인 prvalue며 int라 별도 자원 소유권은 없다.
        // (4) 반환형은 void라 결과값은 사용하지 않는다.
        // (5) 성공하면 stack.size()가 1 늘고 마지막 원소가 0이다. 예약 덕분에 여기서는 재할당이 없고,
        //     새 원소는 호출 뒤 소멸하는 prvalue와 독립된 수명을 갖는다.
        // (6) 이 정확한 상태에서는 O(1), 무할당이며 int 이동 생성도 무예외라 예외 원인이 없다. 기존
        //     원소가 없어 그 관찰자는 없고 과거 past-the-end iterator만 무효가 된다. 같은 stack의 동시
        //     읽기·변경은 외부 동기화 없이 안전하지 않다.
        stack.push_back(Vertex{0});

        // while은 조건이 참인 동안 반복한다. 매 반복에서 stack 원소 하나를 제거하고 아직 넣지 않은
        // 자식만 추가하므로 유한하게 끝난다.
        // [첫 호출 계약: std::vector<Vertex>::empty]
        // (1) 수신 stack은 예약된 저장소를 가진 유효한 const 관찰 가능한 vector<Vertex> lvalue다.
        // (2) [[nodiscard]] constexpr bool empty() const noexcept 오버로드가 선택된다.
        // (3) 명시적 인자는 없고 소유권·수명을 바꾸지 않는다.
        // (4) 비었으면 true, 원소가 있으면 false를 반환하고 while 조건의 부정에 사용한다.
        // (5) stack의 크기·용량·원소는 변하지 않는다.
        // (6) O(1), 무할당·무효화·무예외다. 다른 스레드의 동시 변경 없이 읽어야 한다.
        while (!stack.empty()) {
            // [첫 호출 계약: std::vector<Vertex>::back]
            // (1) empty()==false가 보장한 non-empty non-const stack lvalue가 수신 객체다.
            // (2) reference back() 오버로드가 선택된다.
            // (3) 명시적 인자는 없고 마지막 원소를 빌릴 뿐 소유권 이전은 없다.
            // (4) Vertex&를 반환하고 const Vertex 지역값을 복사 초기화하는 데 즉시 사용한다.
            // (5) 호출 자체는 stack을 바꾸지 않으며 지역 vertex가 독립 int 값을 가진다.
            // (6) O(1), 무할당·무효화·무예외다. 빈 vector에서 호출하면 UB지만 앞 조건으로 배제한다.
            const Vertex vertex{stack.back()};

            // [첫 호출 계약: std::vector<Vertex>::pop_back]
            // (1) 수신 stack은 바로 위 검사로 non-empty인 유효한 vector<Vertex> lvalue다.
            // (2) constexpr void pop_back() 오버로드가 선택된다.
            // (3) 명시적 인자는 없고 마지막 int의 소유 수명을 끝낸다.
            // (4) 반환형은 void라 결과값은 없다.
            // (5) size가 1 줄고 제거 원소와 past-the-end iterator가 무효다. capacity는 변하지 않는다.
            // (6) O(1), 무할당이며 int 소멸은 무예외다. 빈 vector 호출은 UB지만 배제했고 동시 접근은 안 된다.
            stack.pop_back();

            // 같은 rvalue push_back 오버로드를 사용한다. order는 부모를 자식보다 먼저 저장한다.
            order.push_back(Vertex{vertex});

            // [첫 호출 계약: Graph::operator[] const와 const vector 반복 연산]
            // (1) graph_는 size=n인 const Graph lvalue이고 vertex는 [0,n)다. 선택된 안쪽 vector도 살아 있다.
            // (2) 바깥 const operator[](size_type)는 const vector<Vertex>&를 반환한다. range-for는 그 범위에
            //     대해 begin() const/end() const, const_iterator 비교·역참조·전위 증가를 사용한다.
            // (3) 인덱스는 vertex의 size_type 변환값이다. 반복자는 안쪽 vector를 빌리며 소유하지 않는다.
            // (4) operator[] 결과는 range-for가 사용하고, 역참조의 const Vertex&는 next 값으로 복사된다.
            // (5) graph와 인접 목록은 바뀌지 않고 반복 종료 시 숨은 iterator 지역만 소멸한다.
            // (6) 인덱싱·각 iterator 연산은 O(1), 전체 루프는 degree(vertex) 시간이다. 무할당이며 입력 뒤
            //     graph를 바꾸지 않아 iterator가 유효하다. 범위 밖 인덱스는 UB이나 DFS 불변식으로 배제한다.
            for (const Vertex next : graph_[static_cast<std::size_t>(vertex)]) {
                // 트리에는 교차 간선이 없으므로 직전 부모만 제외하면 각 정점을 정확히 한 번 발견한다.
                if (next == parent[static_cast<std::size_t>(vertex)]) {
                    continue;
                }
                parent[static_cast<std::size_t>(next)] = vertex;
                stack.push_back(Vertex{next});
            }
        }

        // [첫 호출 계약: std::vector<Vertex>::size]
        // (1) 수신 order는 DFS가 모든 연결 정점을 한 번씩 넣은 유효한 vector<Vertex> lvalue다.
        // (2) constexpr size_type size() const noexcept 오버로드가 선택된다.
        // (3) 명시적 인자는 없고 order를 읽기만 한다.
        // (4) 반환형 size_type 값은 역순·정순 루프 경계로 사용한다.
        // (5) order와 모든 관찰자의 상태는 그대로다.
        // (6) O(1), 무할당·무효화·무예외다. 입력 트리라 반환값은 node_count_size와 같다.
        const std::size_t order_size{order.size()};

        // 부모-선행 order를 역순 처리한다. 자식 크기가 모두 완성된 뒤 부모에 정확히 한 번 합쳐진다.
        for (std::size_t index{order_size}; index > 0U; --index) {
            const Vertex vertex{order[index - 1U]};
            const Vertex parent_vertex{parent[static_cast<std::size_t>(vertex)]};
            if (parent_vertex != Vertex{-1}) {
                subtree_size[static_cast<std::size_t>(parent_vertex)] +=
                    subtree_size[static_cast<std::size_t>(vertex)];
            }
        }

        // n>=1이므로 order[0]은 존재한다. 센트로이드가 반드시 존재하지만 방어적인 초기 답으로도 쓴다.
        Vertex centroid{order[0]};

        // 정확성 불변식: vertex를 제거한 조각은 자식 subtree와 부모 쪽 n-subtree[vertex]의 완전 분할이다.
        for (std::size_t index{}; index < order_size; ++index) {
            const Vertex vertex{order[index]};
            int largest_piece{node_count_ - subtree_size[static_cast<std::size_t>(vertex)]};

            for (const Vertex next : graph_[static_cast<std::size_t>(vertex)]) {
                if (parent[static_cast<std::size_t>(next)] == vertex
                    && subtree_size[static_cast<std::size_t>(next)] > largest_piece) {
                    largest_piece = subtree_size[static_cast<std::size_t>(next)];
                }
            }

            // 정수 나눗셈 node_count_/2는 양수 n에서 floor(n/2)다.
            if (largest_piece <= node_count_ / 2) {
                centroid = vertex;
                break;
            }
        }

        // 내부는 0-based지만 출력 계약은 1-based다. 값 반환은 독립 int prvalue를 호출자에 제공한다.
        return centroid + 1;
    }

private:
    // graph_는 소유하지 않는 const lvalue 참조다. finder보다 main의 graph가 오래 살아야 한다.
    const Graph& graph_;
    int node_count_{};
};

int main() {
    // [첫 호출 계약: ios_base::sync_with_stdio와 basic_ios::tie]
    // (1) 표준 스트림은 초기화된 전역 객체이며 아직 이 프로그램의 입출력을 시작하지 않았다.
    // (2) static bool ios_base::sync_with_stdio(bool sync)와 ostream* cin.tie(ostream*)가 선택된다.
    // (3) false는 C/C++ 스트림 동기화를 끄고 nullptr는 cin의 tied ostream을 없앤다. 소유권 이전은 없다.
    // (4) 첫 호출은 이전 동기화 상태 bool, 둘째는 이전 tie 포인터를 반환하지만 둘 다 의도적으로 버린다.
    // (5) 이후 C++ 스트림 입출력이 빨라질 수 있고 cin 입력마다 cout 자동 flush를 하지 않는다.
    // (6) 표준은 두 설정 함수에 별도 복잡도 상한을 두지 않으며 입력 크기에 비례하는 작업은 아니다.
    //     둘 다 noexcept 선언이 아니므로 무예외를 가정하지 않고, failbit 같은 I/O 오류값도 반환하지 않는다.
    //     tie는 포인터를 소유하거나 대상 수명을 늘리지 않으며 nullptr는 tie 순환 위험이 없다. 두 호출은
    //     stream/buffer 수명을 끝내지 않는다. 첫 I/O 전 설정하며 여러 스레드의 레코드 원자성은 별개다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Vertex vertex_count{}; // 빈 중괄호 초기화는 기본 정수 타입 int를 0으로 만든다.

    // [첫 호출 계약: istream 정수 추출과 basic_ios::operator!]
    // (1) 수신 std::cin은 유효한 std::istream lvalue이고 vertex_count는 살아 있는 int lvalue다.
    // (2) basic_istream& operator>>(int&)와 bool basic_ios::operator!() const가 차례로 선택된다.
    // (3) 인자는 vertex_count의 비소유 참조다. 입력은 공식 제약의 정수여야 한다.
    // (4) 추출은 cin 참조를 반환하고 !가 오류 상태 여부 bool을 만들어 if에서 사용한다.
    // (5) 성공하면 vertex_count에 n을 저장하고 입력 위치가 전진한다. 실패하면 fail/eof 상태가 설정되고,
    //     범위를 넘긴 숫자는 int의 해당 경계값을 저장할 수 있으므로 실패 뒤 기존 값 보존을 기대하지 않는다.
    // (6) 소비한 문자 수에 비례하고 locale/버퍼를 사용할 수 있다. 예외 mask가 해당 상태를 요구하면
    //     ios_base::failure를 던질 수 있고, 기본 mask에서는 상태 비트로 보고한다. 컨테이너 관찰자 무효화는
    //     없으며 반환 cin&은 비소유 별칭으로 정적 수명의 cin까지만 유효하다. 같은 stream의 동시 추출
    //     순서와 vertex_count 동시 접근은 외부에서 동기화해야 한다.
    if (!(std::cin >> vertex_count)) {
        return 0;
    }

    const std::size_t vertex_count_size{static_cast<std::size_t>(vertex_count)};

    // [첫 생성 계약: Graph(size_type)]
    // (1) graph 수신 객체는 아직 수명 전이고 vertex_count_size는 공식 제약상 [1,200000]이다.
    // (2) vector<vector<Vertex>>(size_type count) 크기 생성자를 선택해 count개 안쪽 vector를 기본 삽입한다.
    // (3) count는 vertex_count_size 값이며 각 안쪽 vector의 템플릿 인자는 Vertex다.
    // (4) 생성자는 별도 반환값 없이 graph가 정점별 빈 인접 목록을 소유하게 한다.
    // (5) 성공하면 graph의 수명이 시작되고 graph.size()==n이며 모든 안쪽 vector의 size는 0이다.
    // (6) n개의 빈 안쪽 vector를 기본 삽입하므로 시간 O(n), 바깥 저장소 O(n)이다. 바깥 저장소 할당에서
    //     bad_alloc/length_error가 가능하고 실패 시 이미 만든 원소를 정리해 graph 객체를 남기지 않는다.
    //     아직 iterator가 없어 무효화 문제가 없고, graph와 원소의 수명은 main 블록 끝까지다. 이후 같은
    //     graph에 대한 동시 접근·변경은 외부 동기화 없이 안전하지 않다.
    Graph graph(vertex_count_size);

    // for 초기식의 edge{}는 int를 0으로 초기화하고, 비교·증가로 정확히 n-1개 간선을 읽는다.
    for (int edge{}; edge < vertex_count - 1; ++edge) {
        Vertex from{};
        Vertex to{};
        // 위와 같은 int 추출 오버로드를 두 번 연쇄한다. 첫 반환 cin&이 다음 추출의 수신 객체가 된다.
        std::cin >> from >> to;
        --from;
        --to;

        // [첫 호출 계약: Graph::operator[] 비-const와 vector<Vertex>::push_back(const Vertex&)]
        // (1) graph는 size=n인 non-const Graph lvalue이고 from/to는 공식 입력을 0-based로 바꾼 [0,n) 값이다.
        // (2) 바깥 reference operator[](size_type) 뒤 안쪽 void push_back(const Vertex&)가 선택된다.
        // (3) 인덱스는 from/to의 size_type 값이고, 추가 값 to/from은 const int&로 잠시 빌린다. 소유권은 없다.
        // (4) operator[]는 안쪽 vector<Vertex>&를 반환해 push_back 수신으로 쓰고 push_back은 void다.
        // (5) 양방향 인접 목록에 상대 정점이 하나씩 복사되고 각 size가 1 증가한다. 안쪽 재할당 시 그
        //     목록의 기존 iterator/reference만 무효화되며 바깥 graph 원소 주소는 유지된다.
        // (6) 인덱싱 O(1), push_back amortized O(1)·재할당 시 O(degree)다. 할당 예외가 가능하며 범위 밖
        //     인덱스는 UB지만 제약으로 배제한다. 입력 구축 중 같은 목록의 동시 변경은 허용하지 않는다.
        graph[static_cast<std::size_t>(from)].push_back(to);
        graph[static_cast<std::size_t>(to)].push_back(from);
    }

    // 이름 있는 graph는 lvalue라 finder의 const Graph&에 바인딩된다. 참조는 graph 수명을 연장하지 않지만
    // 이 블록에서는 graph가 finder보다 먼저 생성되고 나중에 파괴되어 안전하다.
    const TreeCentroidFinder finder{graph};
    const Vertex answer{finder.find()};

    // [첫 호출 계약: ostream 정수·문자 삽입]
    // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 answer는 [1,n]인 const int lvalue다.
    // (2) basic_ostream& operator<<(int) 뒤 basic_ostream& operator<<(char)가 선택된다.
    // (3) answer 값과 '\n' 값을 전달하며 소유권 이전은 없다.
    // (4) 각 호출은 cout 참조를 반환해 다음 삽입에 사용하고 최종 참조는 버린다.
    // (5) 출력 버퍼에 센트로이드 번호와 줄바꿈이 추가되고 answer는 변하지 않는다.
    // (6) 정수 삽입은 생성한 숫자 문자 수 d에 O(d), 문자 삽입은 O(1)개의 문자를 처리하며 버퍼 I/O가
    //     가능하다. 실패는 상태 비트 또는 예외 마스크에 따른 ios_base::failure로 나타난다. 두 삽입은
    //     answer 수명을 바꾸거나 출력 문자를 별도 owner로 돌려주지 않는다. '\n'은 강제 flush가 아니며
    //     여러 스레드가 한 레코드 단위로 출력된다는 보장은 없다.
    std::cout << answer << '\n';

    // 정확성: 위 완전 분할의 모든 조각이 절반 이하인 정점을 출력하므로 충분하고, 센트로이드 존재
    // 정리에 따라 반드시 하나를 찾으므로 필요 조건도 만족한다.
    // 복잡도: 각 정점과 양방향 간선을 상수 번 보아 O(n) 시간, graph 외 배열·스택 O(n) 공간이다.
    // 흔한 실수: 부모 방향 크기 n-subtree[v] 누락, >= 대신 > 절반 판정, 1-based 변환 누락,
    // n=200000 사슬에 재귀 DFS 사용, 짝수 경로의 두 정답 중 하나만 정답이라고 가정하는 것이다.
    return 0;
}
