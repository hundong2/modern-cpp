/*
문제 ID·제목: CSES 1133 - Tree Distances II
출처: https://cses.fi/problemset/task/1133/

문제 요약
- 1번부터 n번까지 번호가 붙은 정점과 n-1개의 무방향 간선이 주어진다.
- 그래프는 연결되어 있고 사이클이 없는 트리다.
- 각 정점 v마다 v에서 모든 정점까지의 최단거리 합을 구한다. 간선 하나를 지날 때 거리는 1이다.

입력
- 첫 줄에 정점 수 n이 주어진다.
- 다음 n-1줄마다 서로 연결된 두 정점 a, b가 주어진다.

출력
- 1번 정점부터 n번 정점까지, 각 정점을 출발점으로 삼았을 때의 거리 합 n개를 한 줄에 출력한다.

공식 제약
- 1 <= n <= 200,000
- 1 <= a, b <= n
- 시간 제한 1초, 메모리 제한 512MB

공식 예제
- 입력:
  5
  1 2
  1 3
  3 4
  3 5
- 출력:
  6 9 5 8 8
- 예를 들어 3번 정점의 거리 합은 1까지 1, 2까지 2, 자신까지 0, 4와 5까지 각각 1이므로 5다.
*/

// <cstddef>는 컨테이너 원소 수와 인덱스를 나타내는 부호 없는 표준 타입 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 거리 합을 정확히 64비트 부호 있는 정수로 저장할 std::int64_t를 선언한다.
#include <cstdint>
// <iostream>은 표준 입력 std::cin, 표준 출력 std::cout과 스트림 연산을 선언한다.
#include <iostream>
// <vector>는 인접 리스트, 명시적 DFS 스택, 전처리 배열을 동적 연속 저장하는 std::vector를 선언한다.
#include <vector>

// using은 새 정수 타입을 만드는 문법이 아니라 기존 타입에 읽기 쉬운 별칭을 붙이는 문법이다.
// 거리 합의 최댓값은 경로 트리에서 n(n-1)/2로 약 2*10^10이므로 32비트 int가 아닌 64비트가 필요하다.
using Distance = std::int64_t;

// 첫 템플릿 인자 int는 안쪽 vector의 원소 타입이고, 바깥 vector의 원소는 정점별 인접 리스트다.
// 각 vector는 자기 동적 저장소를 독점 소유하며 객체 수명이 끝날 때 그 저장소를 자동 해제한다.
using Adjacency = std::vector<int>;
using Graph = std::vector<Adjacency>;

/*
값 범주·복사/이동·수명·소유권
- solver, graph_, parent_처럼 이름이 있는 객체 표현식은 lvalue다. vertex_count, from 같은 이름 있는
  const int도 수정 불가능할 뿐 값 범주는 lvalue다.
- Distance{0}, 0, vertex_count - 1 같은 순수 계산 결과는 prvalue다. 컨테이너 생성자와 push_back은
  필요한 값을 자기 저장소의 원소로 복사해 소유한다.
- 이 구현에는 이동 변환 유틸리티 호출이 없으므로 의도적으로 만든 xvalue도 없다. 정점 번호는 작은 int라 이동과 복사가
  같은 값 복사이고, 알고리즘의 핵심은 객체 이동이 아니라 인덱스로 소유 컨테이너를 안전하게 참조하는 것이다.
- solve()은 answer_의 const lvalue 참조를 돌려주므로 큰 vector 복사와 이동이 없다. 이 참조는 solver보다
  오래 살 수 없지만 main에서는 solver가 참조보다 먼저 만들어지고 나중에 파괴되므로 사용 기간 내내 유효하다.
- 참조 반환에는 RVO가 적용되지 않는다. vector를 값으로 반환하는 설계였다면 C++17의 복사 생략 또는 이동을
  기대할 수 있지만, 여기서는 소유권을 solver에 그대로 두는 수명 계약을 명시적으로 선택했다.
*/

// 구현 가까운 공용 알고리즘 문서: ../algorithm/tree-rerooting-distance-sums.md
// class와 struct는 모두 사용자 정의 타입이지만 class의 기본 접근은 private, struct의 기본 접근은 public이다.
// 이 타입은 graph_와 DP 배열의 불변식을 외부에서 깨지 못하게 감추고 add_edge/solve만 공개하므로 class가 알맞다.
class TreeDistanceSums {
public:
    // 생성자는 반환형을 쓰지 않으며 새 객체의 수명을 시작한다. explicit은 int가 필요한 자리에
    // TreeDistanceSums가 암시적으로 만들어지는 뜻밖의 변환을 막는다.
    // const 값 매개변수는 호출자의 int를 복사해 받으며 생성자 안에서 실수로 바꾸지 않겠다는 의도를 나타낸다.
    // 멤버 초기화 목록은 생성자 본문 실행 전에 멤버를 선언 순서대로 직접 초기화한다.
    // vertex_count_의 중괄호 초기화는 축소 변환을 막고, vector fill 생성자는 initializer_list와 혼동하지 않도록
    // 소괄호를 쓴다. 예를 들어 vector<int>{n, -1}은 n칸이 아니라 두 원소 n과 -1이 될 수 있다.
    //
    // [호출 계약: 일곱 std::vector 생성]
    // (1) 수신 객체는 아직 수명이 시작되지 않은 graph_(정확한 타입 Graph=vector<vector<int>>),
    //     parent_/order_/stack_/subtree_size_(vector<int>), distance_from_root_/answer_(vector<Distance>)다.
    //     vertex_count는 공식 범위 [1,200000]이고 모든 컨테이너에 기존 원소·관찰자가 없다.
    // (2) graph_는 explicit vector<Adjacency>::vector(size_type count), parent_와 subtree_size_는
    //     vector<int>::vector(size_type, const int&), 두 Distance 배열은
    //     vector<Distance>::vector(size_type, const Distance&)를 선택한다. order_와 stack_의 `{}`는
    //     vector<int>::vector() 기본 생성자를 선택하며 Allocator 템플릿 인자는 기본 할당자 타입이다.
    // (3) static_cast<std::size_t>(vertex_count)는 양의 size_type prvalue로 개수를 값 전달한다.
    //     -1, 1, Distance{0}은 각각 int/Distance prvalue로 const value_type&에 잠시 바인딩된 뒤 각 원소에
    //     복사된다. 외부 버퍼나 객체 소유권은 전달되지 않고 빈 vector 둘에는 명시 인자가 없다.
    // (4) 생성자는 반환값이 없다. 성공하면 graph_는 n개의 빈 Adjacency, parent_는 n개의 -1,
    //     subtree_size_는 n개의 1, 두 거리 배열은 n개의 0을 소유하며 order_/stack_은 비어 있다.
    // (5) 각 fill/count vector의 size는 n이 되고 독립적인 저장소를 가진다. 빈 vector 둘의 size는 0이며,
    //     실패 없이 모두 구성된 뒤에만 생성자 본문으로 진입한다.
    // (6) 전체 O(n) 시간·공간이며 여러 동적 할당이 가능하다. count가 max_size를 넘으면 length_error,
    //     메모리 부족이면 bad_alloc이 가능하고, 실패 시 이미 생성된 멤버는 역순으로 정리된다.
    //     새 객체라 무효화할 기존 참조는 없고 완성 전 외부 스레드에 공개하지 않는다.
    explicit TreeDistanceSums(const int vertex_count)
        : vertex_count_{vertex_count},
          graph_(static_cast<std::size_t>(vertex_count)),
          parent_(static_cast<std::size_t>(vertex_count), -1),
          order_{},
          stack_{},
          subtree_size_(static_cast<std::size_t>(vertex_count), 1),
          distance_from_root_(static_cast<std::size_t>(vertex_count), Distance{0}),
          answer_(static_cast<std::size_t>(vertex_count), Distance{0}) {
        // [호출 계약: std::vector<int>::reserve(size_type), 두 수신 객체]
        // (1) 수신 order_와 stack_은 정확한 타입 std::vector<int>의 비-const 빈 lvalue이고,
        //     size==capacity==0일 수 있다. vertex_count_는 [1,200000]의 초기화된 int다.
        // (2) 두 호출 모두 constexpr void vector<int>::reserve(size_type new_capacity)를 선택한다.
        // (3) static_cast로 만든 std::size_t prvalue n을 값 전달한다. 원소나 버퍼 소유권 입력은 없다.
        // (4) 반환형 void라 반환값은 없고, 이후 최대 n번 push할 때의 재할당을 피하려는 용도다.
        // (5) 성공하면 size는 0 그대로이고 capacity는 최소 n이다. 재할당했다면 과거 원소 포인터·참조·
        //     반복자는 모두 무효화되지만 지금은 원소와 관찰자가 없다. 두 vector는 서로 영향을 주지 않는다.
        // (6) 각 호출은 현재 size에 선형(여기서는 O(1) 원소 이동)이고 O(n) 저장소를 할당할 수 있다.
        //     max_size 초과 시 length_error, 할당 실패 시 bad_alloc이며 강한 예외 보장으로 수신자는 유지된다.
        //     저장소 수명은 각 vector와 같고 같은 vector를 다른 스레드가 동시에 접근하지 않는다.
        order_.reserve(static_cast<std::size_t>(vertex_count_));
        stack_.reserve(static_cast<std::size_t>(vertex_count_));
    }

    // 반환형 void는 결과 객체가 없다는 뜻이다. from/to는 호출자의 정점 번호를 값 복사해 받는다.
    // public 접근 지정자는 main이 이 함수를 부를 수 있게 하지만 private 데이터 자체를 직접 바꾸지는 못하게 한다.
    void add_edge(const int from, const int to) {
        // [호출 계약: Graph::operator[]와 std::vector<int>::push_back(const int&)]
        // (1) 수신 graph_는 정확한 타입 vector<vector<int>>의 비-const lvalue이며 size==n이다.
        //     graph_[from]과 graph_[to]는 유효한 vector<int> lvalue이고 지금까지 읽은 이웃들을 소유한다.
        // (2) 바깥 호출은 vector<Adjacency>::reference operator[](size_type position), 이어지는 호출은
        //     void vector<int>::push_back(const int& value) overload다. value_type 템플릿 인자는 int다.
        // (3) from/to를 변환한 std::size_t prvalue는 각각 [0,n)의 위치 값이다. 이름 있는 to/from은 int
        //     lvalue라 const int&에 바인딩되고 값만 새 원소로 복사되며 외부 소유권은 옮기지 않는다.
        // (4) operator[]는 해당 Adjacency의 lvalue 참조를 반환해 즉시 push_back의 수신자로 사용한다.
        //     push_back은 void를 반환하므로 결과를 저장하지 않는다.
        // (5) 성공한 각 인접 리스트의 size가 1 늘어 무방향 간선 양쪽 끝이 서로를 갖는다. 해당 안쪽 vector가
        //     재할당되면 그 vector의 기존 포인터·참조·반복자는 무효화되지만 코드는 그런 관찰자를 보존하지 않는다.
        // (6) operator[]는 O(1)·무할당이고 범위 검사를 하지 않아 위치가 잘못되면 UB다. 공식 입력과 0-based
        //     변환이 전제를 보장한다. push_back은 상각 O(1), 재할당 시 기존 size에 선형이며 bad_alloc 또는
        //     length_error가 가능하다. 한쪽 삽입 뒤 둘째가 실패할 수 있으나 예외로 프로그램이 풀리며 solver를
        //     계속 사용하지 않는다. 각 int 수명은 vector가 제거하거나 파괴할 때까지이고 동시 수정은 없다.
        graph_[static_cast<std::size_t>(from)].push_back(to);
        graph_[static_cast<std::size_t>(to)].push_back(from);
    }

    // 반환형의 `const std::vector<Distance>&`는 answer_를 복사하지 않고 읽기 전용 lvalue 참조로 빌려준다.
    // 호출자는 반환 참조를 통해 원소를 바꿀 수 없고, 이 객체보다 오래 보관해서는 안 된다.
    [[nodiscard]] const std::vector<Distance>& solve() {
        // 루트는 내부 0번(문제의 1번)이다. 자신의 부모를 자신으로 두면 실제 이웃과 비교할 sentinel이 된다.
        // [호출 계약: std::vector<int>::operator[](size_type) 쓰기]
        // (1) 수신 parent_는 정확한 타입 std::vector<int>의 비-const lvalue이며 size==n, 모든 값은 -1이다.
        // (2) constexpr reference vector<int>::operator[](size_type position) overload를 선택한다.
        // (3) std::size_t prvalue 0을 위치 값으로 전달하며 소유권 이동은 없다.
        // (4) 루트 int 원소의 lvalue 참조를 반환하고 대입의 왼쪽 피연산자로 사용한다.
        // (5) parent_[0]만 0으로 바뀌고 vector의 size/capacity와 모든 원소 주소는 유지된다.
        // (6) O(1), 무할당·비무효화다. 표준 선언에 일률적인 noexcept 명세가 있다고 가정하지 않는다.
        //     빈 vector나 범위 밖 위치면 UB지만 n>=1이 전제를 보장한다. 원소는 parent_가 소유하며
        //     solve 동안 같은 vector에 대한 동시 접근은 없다.
        parent_[std::size_t{0}] = 0;

        // [호출 계약: std::vector<int>::push_back(int&&)]
        // (1) 수신 stack_은 정확한 타입 std::vector<int>의 비-const 빈 lvalue이고, 앞선 reserve 성공으로
        //     capacity>=n>=1이다.
        // (2) void vector<int>::push_back(value_type&& value) overload를 선택하며 value_type은 int다.
        // (3) int prvalue 0이 임시로 구체화되어 int&&에 바인딩된다. 정점 값만 새 원소로 옮기며 외부 자원
        //     소유권은 없다. 0은 [0,n)의 유효한 루트 번호다.
        // (4) 반환형 void라 반환값은 없고, 새 마지막 원소가 이후 DFS 시작점이 된다.
        // (5) size가 0에서 1로 늘고 새 int의 수명이 시작된다. 확보한 capacity 안이라 재할당되지 않으며,
        //     기존 원소 관찰자는 없고 과거 past-the-end 반복자만 무효화된다.
        // (6) 상각 O(1)이고 이 호출은 용량이 충분해 할당하지 않는다. int 구성은 예외를 던지지 않는다.
        //     원소 수명은 pop/파괴 때까지이며 같은 vector를 다른 스레드가 동시에 접근하지 않는다.
        stack_.push_back(0);

        // 재귀 DFS 대신 힙 저장소를 쓰는 명시적 스택이다. 경로 모양 트리의 깊이 n에서도 호출 스택은 O(1)이다.
        // while은 조건이 참인 동안 본문을 반복한다. 매 회 정확히 한 정점을 pop하므로 반드시 끝난다.
        // [호출 계약: vector<int>::empty(), back(), pop_back()]
        // (1) 수신 stack_은 정확한 타입 std::vector<int>의 유효한 비-const lvalue다. 조건의 empty는 어떤
        //     size에도 가능하고, 본문에 들어왔을 때는 반드시 size>0이라 back/pop_back 전제가 성립한다.
        // (2) bool empty() const noexcept, reference back(), void pop_back() overload를 차례로 선택한다.
        // (3) 세 호출 모두 명시 인자가 없고 숨은 this로 같은 vector를 읽거나 수정한다. 소유권 인자는 없다.
        // (4) empty는 size==0 여부 bool을 분기에서 사용하고, back은 마지막 int lvalue 참조를 반환해 vertex에
        //     복사하며, pop_back은 void라 결과가 없다.
        // (5) empty/back은 상태를 바꾸지 않는다. pop_back은 마지막 int 수명을 끝내고 size를 1 줄이며 그
        //     원소와 과거 past-the-end 관찰자를 무효화한다. vertex는 먼저 복사했으므로 안전하다.
        // (6) 모두 O(1)·무할당이다. empty만 표준 선언상 noexcept이고 back/pop_back에 일률적인 noexcept를
        //     가정하지 않는다. 빈 상태의 back/pop_back은 UB지만 while 조건이 막는다. 이 int 인스턴스의
        //     소멸은 예외를 던지지 않으며 같은 stack_을 동시에 읽거나 쓰지 않아 데이터 경쟁이 없다.
        while (!stack_.empty()) {
            const int vertex{stack_.back()};
            stack_.pop_back();

            // 부모를 자식보다 먼저 넣는 preorder 불변식을 만든다. push_back 계약은 앞에서 설명했으며,
            // order_는 n을 reserve했으므로 주소 재배치 없이 size만 1 증가한다.
            order_.push_back(vertex);

            // [호출 계약: const Adjacency::size()와 Graph/Adjacency::operator[] 읽기]
            // (1) 수신 graph_는 n개 인접 리스트를 완성한 Graph 비-const lvalue이고 solve 중 구조 변경이 없다.
            //     graph_[vertex]는 정확한 타입 Adjacency=vector<int>의 유효한 lvalue다.
            // (2) 바깥/안쪽 인덱싱은 각각 vector<Adjacency>::reference 및 vector<int>::reference
            //     operator[](size_type), size는 size_type vector<int>::size() const noexcept를 선택한다.
            // (3) vertex와 adjacency_index를 변환한 std::size_t prvalue는 각각 [0,n), [0,degree(vertex))다.
            //     위치 값만 전달하고 그래프 소유권이나 원소 소유권은 이동하지 않는다.
            // (4) 바깥 []는 인접 vector lvalue, size는 이웃 수 prvalue, 안쪽 []는 이웃 int lvalue를 반환한다.
            //     size 결과는 반복 조건에, 이웃 값은 next의 복사 초기화에 사용한다.
            // (5) 모두 관찰 호출이라 graph_의 size/capacity/원소와 모든 포인터·참조 수명은 유지된다.
            // (6) 각 호출은 O(1), 무할당·비무효화이고 size는 noexcept다. []는 범위 밖이면 UB지만 루프 경계와
            //     트리 정점 범위가 전제를 보장한다. solve 동안 graph_ 수정과 다른 스레드 접근은 없다.
            for (int adjacency_index{0};
                 adjacency_index < static_cast<int>(
                     graph_[static_cast<std::size_t>(vertex)].size());
                 ++adjacency_index) {
                const int next{
                    graph_[static_cast<std::size_t>(vertex)]
                          [static_cast<std::size_t>(adjacency_index)]};

                // if는 비교 결과가 참일 때만 continue를 실행한다. 트리에는 사이클이 없으므로 바로 부모만
                // 제외하면 이미 방문한 다른 정점으로 돌아갈 수 없다. continue는 이번 이웃 처리만 건너뛴다.
                if (next == parent_[static_cast<std::size_t>(vertex)]) {
                    continue;
                }

                // 아래 parent_ []는 앞서 계약한 vector<int>의 O(1) 비검사 접근이다.
                parent_[static_cast<std::size_t>(next)] = vertex;

                // [호출 계약: std::vector<Distance>::operator[] 읽기와 쓰기]
                // (1) 수신 distance_from_root_는 정확한 타입 std::vector<Distance>의 비-const lvalue이고
                //     size==n이다. vertex의 거리는 이미 확정됐고 next의 초기값은 0 또는 아직 쓰지 않은 값이다.
                // (2) 두 호출 모두 reference vector<Distance>::operator[](size_type position) overload다.
                // (3) next/vertex를 변환한 std::size_t prvalue는 [0,n)의 위치 값이다. 값·저장소 소유권은
                //     옮기지 않고, 오른쪽 원소는 lvalue-to-rvalue 변환으로 읽는다.
                // (4) 각 호출은 Distance lvalue 참조를 반환한다. 부모 참조의 값에 1을 더한 Distance prvalue를
                //     자식 참조가 가리키는 원소에 대입한다.
                // (5) next 원소만 부모 거리+1로 바뀌고 vector의 size/capacity/주소 및 vertex 원소는 유지된다.
                // (6) O(1), 무할당·비무효화다. 표준 선언에 일률적인 noexcept를 가정하지 않으며 범위 밖이면
                //     UB지만 트리 입력과 순회 불변식이 막는다. 두 참조는 전체 표현식까지만 쓰고 동시 접근은 없다.
                distance_from_root_[static_cast<std::size_t>(next)] =
                    distance_from_root_[static_cast<std::size_t>(vertex)] + Distance{1};

                // 이름 있는 next는 int lvalue라 앞서 설명한 const int& push_back overload가 선택된다.
                // 부모·루트 거리를 저장한 뒤 자식을 스택에 넣는 것이 핵심 순회 불변식이다.
                stack_.push_back(next);
            }
        }

        // 역순에서는 모든 자손이 부모보다 먼저 나온다. 따라서 vertex의 subtree_size가 이미 완성되었고
        // 이를 부모에 한 번 더하면 된다. 같은 순회에서 모든 루트 거리도 합쳐 answer[root]를 만든다.
        Distance root_distance_sum{0};
        // for는 초기화, 조건 비교, 감소를 한곳에 쓴 제어문이다. index==0인 루트는 부모에 합치지 않는다.
        for (int order_index{vertex_count_ - 1}; order_index > 0; --order_index) {
            const int vertex{order_[static_cast<std::size_t>(order_index)]};
            const int parent{parent_[static_cast<std::size_t>(vertex)]};

            subtree_size_[static_cast<std::size_t>(parent)] +=
                subtree_size_[static_cast<std::size_t>(vertex)];
            root_distance_sum += distance_from_root_[static_cast<std::size_t>(vertex)];
        }
        answer_[std::size_t{0}] = root_distance_sum;

        // 재루팅 점화식의 불변식:
        // parent에서 자식 vertex로 기준점을 한 간선 옮기면 vertex 서브트리의 subtree_size[vertex]개 정점은
        // 거리가 1씩 줄고, 나머지 n-subtree_size[vertex]개 정점은 1씩 늘어난다.
        // 따라서 answer[vertex] = answer[parent] - size + (n-size) = answer[parent] + n - 2*size다.
        // 정순 order에서는 parent의 answer가 항상 먼저 완성되어 이 식의 오른쪽이 정의되어 있다.
        for (int order_index{1}; order_index < vertex_count_; ++order_index) {
            const int vertex{order_[static_cast<std::size_t>(order_index)]};
            const int parent{parent_[static_cast<std::size_t>(vertex)]};
            answer_[static_cast<std::size_t>(vertex)] =
                answer_[static_cast<std::size_t>(parent)]
                + static_cast<Distance>(vertex_count_)
                - Distance{2}
                      * static_cast<Distance>(
                          subtree_size_[static_cast<std::size_t>(vertex)]);
        }

        // n<=200000이면 거리 합은 최대 n(n-1)/2라 int64 범위에 넉넉히 들어간다.
        // 인접 리스트의 각 방향 간선을 한 번씩 읽고 배열을 세 번 선형 순회하므로 시간 O(n), 공간 O(n)이다.
        return answer_;
    }

private:
    // private 접근 지정자 아래의 멤버는 class 밖에서 직접 접근할 수 없다. 선언 순서는 실제 초기화 순서다.
    int vertex_count_;                      // 기본 타입 int는 정점 수·인덱스처럼 200000 이하인 값을 저장한다.
    Graph graph_;                           // 각 무방향 간선을 양방향 int 두 개로 소유한다.
    // [생성 계약: std::vector 멤버 선언과 생성자 초기화]
    // (1) 아래 여섯 수신 멤버는 선언 시점에는 아직 수명이 시작되지 않았고 기존 원소·관찰자가 없다.
    // (2) 실제 생성은 위 멤버 초기화 목록에서 parent_/subtree_size_와 두 Distance vector에는 fill 생성자,
    //     order_/stack_에는 인자 없는 기본 생성자를 선택한다. 이 선언 줄이 별도 두 번째 생성을 뜻하지 않는다.
    // (3) fill 생성자는 검증된 n의 size_type 값과 -1·1·0 값을 받으며 기본 생성자에는 명시 인자가 없다.
    // (4) 생성자는 반환값이 없고 성공하면 각 vector가 자신의 저장소와 원소 수명을 독점 관리한다.
    // (5) fill 결과 size는 n, 기본 생성 결과 size는 0이다. 객체 파괴 때 아래 선언의 역순으로 정리된다.
    // (6) 기본 생성은 빈 상태·상수 시간을 보장하지만 저장소 표현이나 allocator 호출 유무는 단정하지 않는다.
    //     fill 생성은 O(n)·할당 가능하며 length_error/bad_alloc이 가능하다. 구성 실패 시 이미 끝난 멤버는
    //     자동 파괴되고 외부 참조나 동시 접근은 아직 없다.
    std::vector<int> parent_;               // 임시 루트 0 기준 부모다.
    std::vector<int> order_;                // 부모가 자식보다 앞서는 반복 DFS 순서다.
    std::vector<int> stack_;                // 재귀 호출 스택을 대체하는 명시적 LIFO 저장소다.
    std::vector<int> subtree_size_;         // 임시 루트 기준 각 서브트리의 정점 수다.
    std::vector<Distance> distance_from_root_; // 임시 루트 0에서 각 정점까지의 거리다.
    std::vector<Distance> answer_;           // 모든 정점을 기준으로 한 최종 거리 합이다.
};

/*
기계 실행 관점
- 인접 리스트 순회는 대체로 vector 저장소에서 이웃 int를 load하고, 부모 int와 compare한 뒤 조건 branch한다.
- parent/distance/subtree/answer 갱신은 인덱스로 찾은 메모리의 load와 store이며, 점화식은 64비트 덧셈·뺄셈·곱셈으로 표현된다.
- 명시적 stack의 push/pop은 size 갱신과 연속 저장소 접근을 수반한다. reserve로 순회 중 재할당 분기를 줄인다.
- 이는 추상 기계 수준의 설명이다. 실제 명령 선택, 벡터화, 분기 예측, 메모리 배치는 CPU·ABI·컴파일러·
  표준 라이브러리 구현·최적화 옵션에 따라 달라지므로 특정 어셈블리 형태라고 단정할 수 없다.
*/

// main은 운영체제가 프로그램 시작점으로 부르는 함수이며 int 반환값 0은 정상 종료를 뜻한다.
// 매개변수가 없으므로 명령행 인자를 사용하지 않는다.
int main() {
    // [호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 별도 수신 객체가 없는 static 함수이며 표준 iostream 객체가 아직 입출력을 수행하지 않은 초기 상태다.
    // (2) static bool ios_base::sync_with_stdio(bool sync=true)를 bool 인자 하나로 호출한다.
    // (3) false는 bool prvalue이며 C stdio와 C++ 표준 스트림의 동기화를 끄라는 허용값이다. 소유권 입력은 없다.
    // (4) 이전 동기화 상태 bool을 반환하지만 온라인 저지에서는 복구하지 않아 반환값을 의도적으로 버린다.
    // (5) 이후 표준 C++ 스트림이 독립 버퍼링될 수 있어 빨라지며, C stdio와 섞은 출력 순서는 보장하지 않는다.
    // (6) 표준은 별도 복잡도 상한을 명시하지 않는다. 사용자 컨테이너 할당·참조 무효화는 없지만 입출력 뒤
    //     호출의 효과는 구현 정의이므로 반드시 첫 I/O 전에 부른다. 다른 스레드의 동시 I/O 없이 한 번 설정한다.
    std::ios::sync_with_stdio(false);

    // [호출 계약: std::basic_ios<char>::tie(nullptr)]
    // (1) 수신 std::cin의 정확한 타입은 입력 가능한 std::istream lvalue이며 초기에는 보통 std::cout에 묶여 있다.
    // (2) std::ostream* basic_ios<char>::tie(std::ostream* tiestr) setter를 선택한다.
    // (3) nullptr는 null pointer prvalue로 ostream 포인터에 변환되며 “묶을 출력 스트림 없음”을 뜻한다.
    //     객체 소유권을 넘기거나 std::cout을 파괴하지 않는 비소유 포인터 값이다.
    // (4) 이전 tie 대상 ostream*을 반환하지만 다시 복구하지 않으므로 반환값을 버린다.
    // (5) cin의 tie 포인터가 null이 되어 입력 전 자동 cout.flush가 사라지고 cin/cout 객체 수명은 유지된다.
    // (6) 표준은 별도 복잡도 상한을 명시하지 않는다. 사용자 할당·반복자 무효화가 없고, null이 아닌 인자는
    //     자신에서 tie 연결을 따라 도달 가능하지 않아야 한다. 설정 중 다른 스레드가 cin을 쓰지 않는다.
    std::cin.tie(nullptr);

    // int는 공식 n과 정점 번호를 충분히 담는다. `{0}` 직접 목록 초기화는 초기값을 확실히 0으로 만든다.
    int vertex_count{0};

    // [호출 계약: std::istream의 int 추출 operator>>]
    // (1) 왼쪽 수신 std::cin은 입력 가능한 std::istream lvalue이고 vertex_count는 살아 있는 int lvalue다.
    // (2) basic_istream<char>& basic_istream<char>::operator>>(int& value) 멤버 overload를 선택한다.
    // (3) vertex_count lvalue가 수정 가능한 int&에 바인딩된다. 입력 문자는 빌려 읽고 객체 소유권은 이동하지
    //     않으며 공식 입력의 첫 토큰은 [1,200000]의 올바른 십진 정수다.
    // (4) 같은 istream&를 반환하지만 연쇄나 상태 검사에 쓰지 않고 버린다.
    // (5) 성공하면 vertex_count에 n이 저장되고 입력 위치가 토큰 뒤로 이동한다. 실패하면 failbit/eofbit가
    //     설정될 수 있고 대상 값은 표준 추출 규칙을 따르지만 공식 입력은 성공을 보장한다.
    // (6) 읽은 문자 수에 선형이며 streambuf/locale 내부 작업을 할 수 있다. 예외 mask가 설정됐다면
    //     ios_base::failure가 가능하다. 참조는 호출 동안만 쓰고 다른 스레드가 cin을 동시에 읽지 않는다.
    std::cin >> vertex_count;

    // explicit 생성자는 중괄호 직접 초기화로 호출한다. solver는 main 끝까지 모든 그래프/DP 저장소를 소유한다.
    TreeDistanceSums solver{vertex_count};

    // 트리는 n-1개의 간선을 가진다. edge_index는 반복 횟수만 세며 간선 데이터의 일부가 아니다.
    for (int edge_index{0}; edge_index < vertex_count - 1; ++edge_index) {
        int from{0};
        int to{0};

        // 첫 추출과 같은 int& overload를 두 번 연쇄한다. 첫 호출이 반환한 istream&가 둘째 수신자가 되고,
        // 최종 참조는 버린다. 성공하면 두 변수와 입력 위치만 바뀌며 값·버퍼 소유권은 옮겨지지 않는다.
        std::cin >> from >> to;

        // 입력 경계에서만 1-based 번호를 0-based로 바꾼다. 두 뺄셈 결과는 int prvalue이고 값으로 복사된다.
        solver.add_edge(from - 1, to - 1);
    }

    // const 참조는 solve가 소유한 vector를 복사하지 않고 읽기만 한다. solver가 main 끝까지 더 오래 산다.
    const std::vector<Distance>& answer{solver.solve()};

    for (int output_index{0}; output_index < vertex_count; ++output_index) {
        // [호출 계약: const vector<Distance>::operator[]와 ostream 정수/문자 operator<<]
        // (1) 수신 answer는 정확한 타입 const std::vector<Distance> lvalue이고 size==n이다. std::cout은
        //     출력 가능한 std::ostream lvalue이며 현재까지 앞선 답과 구분자를 버퍼에 가질 수 있다.
        // (2) []는 constexpr const_reference vector<Distance>::operator[](size_type) const,
        //     정수 삽입은 basic_ostream<char>& ostream::operator<<(long long), 문자 삽입은
        //     basic_ostream<char>& operator<<(basic_ostream<char>&, char) overload를 선택한다.
        // (3) output_index를 바꾼 std::size_t prvalue는 [0,n)의 위치다. []가 준 const Distance&의 값을
        //     long long prvalue로 변환해 복사하고, 조건식이 만든 ' '/'\n' char prvalue를 전달한다.
        //     출력 스트림과 answer의 저장소 소유권은 옮기지 않는다.
        // (4) []는 읽기 전용 원소 lvalue 참조를 반환해 정수 변환에 사용한다. 각 <<는 같은 ostream&를
        //     반환해 다음 <<의 수신자로 사용하고 마지막 반환 참조는 버린다.
        // (5) answer는 변하지 않는다. 성공하면 숫자와, 마지막이면 줄바꿈 아니면 공백이 cout 버퍼에 추가된다.
        //     실패하면 badbit/failbit가 설정될 수 있으며 answer와 output_index는 그대로다.
        // (6) []는 O(1)·무할당·비무효화이고 범위 밖이면 UB지만 for 조건이 막는다. 출력은 문자 수에 선형이고
        //     streambuf 내부 할당/오류가 구현에 따라 가능하며 예외 mask 설정 시 ios_base::failure가 가능하다.
        //     반환 참조 수명은 cout의 정적 수명 안이고 단일 스레드에서만 출력한다.
        std::cout << static_cast<long long>(answer[static_cast<std::size_t>(output_index)])
                  << (output_index + 1 == vertex_count ? '\n' : ' ');
    }

    // main이 반환하면 answer 참조가 먼저 사라지고 solver가 파괴되며 모든 vector 저장소가 자동 해제된다.
    return 0;
}
