/*
문제 ID/제목: CSES 1139 - Distinct Colors
출처: https://cses.fi/problemset/task/1139/

문제 요약:
1번 정점을 뿌리로 삼은 트리에서 각 정점에는 색 하나가 적혀 있다. 모든 정점 v에 대해
v와 그 자손들이 가진 색 가운데 서로 다른 값이 몇 개인지 계산한다. 저작권이 있는 원문 전체를
옮기지 않고 구현과 검증에 필요한 조건만 한국어로 다시 설명한다.

입력:
- 첫 줄에 정점 수 n이 주어진다.
- 다음 줄에 1번부터 n번 정점까지의 색 c_i가 순서대로 주어진다.
- 이어지는 n-1개 줄은 무방향 간선의 두 끝점 a, b를 준다. 입력 그래프는 연결된 트리다.
- 1 <= n <= 200,000, 1 <= a,b <= n, 1 <= c_i <= 1,000,000,000이다.

출력:
1번부터 n번 정점까지, 각 정점을 루트로 하는 서브트리의 서로 다른 색 개수를 한 줄에 출력한다.

예제:
입력
5
2 3 2 2 1
1 2
1 3
3 4
3 5
출력
3 1 2 1 1

풀이 요약:
재귀 DFS 대신 명시적 vector를 사용해 부모와 부모-선행 순서를 만든다. 그 순서를 거꾸로 읽으면
항상 자식을 부모보다 먼저 처리한다. 각 처리 성분은 색 집합과 그 집합이 대표하는 원래 정점 수를
함께 소유한다. 정점 수가 작은 성분의 unordered_set 노드를 큰 성분으로 merge하고, 중복 색으로
옮겨지지 않은 노드는 작은 성분을 파괴할 때 버린다. 자세한 불변식과 증명은 구현 가까이의
../algorithm/small-to-large-set-merging.md 링크를 참고한다.
*/

// <cstddef>는 컨테이너 크기와 부호 없는 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력·출력 객체와 정수 추출·삽입 연산을 선언한다.
#include <iostream>
// <memory>는 단독 소유권을 표현하는 std::unique_ptr와 std::make_unique를 선언한다.
#include <memory>
// <unordered_set>은 중복 없는 색을 평균 상수 시간 해시 연산으로 관리하는 컨테이너를 선언한다.
#include <unordered_set>
// <utility>는 이름 있는 unique_ptr을 이동시키기 위한 std::move를 선언한다.
#include <utility>
// <vector>는 색, 인접 리스트, 부모, 순서, 정답과 소유 포인터 배열의 연속 저장소를 선언한다.
#include <vector>

namespace {

// using은 긴 타입에 새 별칭만 붙이며 새 타입을 만들지 않는다. template 인자 int는 집합의 key_type이
// 기본 정수임을 뜻하고, hash/equality/allocator는 각각 표준 기본 인자를 사용한다.
using ColorSet = std::unordered_set<int>;

// ColorBag은 서로 겹치지 않는 원래 정점 성분 하나의 색 합집합과 정점 수를 함께 소유한다.
// struct와 class의 핵심 언어 기능은 같지만 struct의 기본 접근은 public, class의 기본 접근은 private다.
// 여기서는 대회 풀이의 단순한 상태 묶음임을 드러내려고 struct를 쓰고, public 접근 지정자도 명시한다.
struct ColorBag {
public:
    ColorSet colors;
    std::size_t represented_nodes;

    // [첫 생성 계약: ColorSet()]
    // (1) 수신 colors는 아직 수명이 시작되지 않은 ColorSet 멤버 저장소이고 원소가 없는 상태로 만들 예정이다.
    // (2) 선택 생성자는 unordered_set<int, hash<int>, equal_to<int>, allocator<int>>()이며 기본 hash,
    //     동등 비교자와 allocator를 값으로 소유한다.
    // (3) 명시 인자는 없다. bucket 수는 구현이 정하고 외부 메모리나 원소 소유권을 빌리지 않는다.
    // (4) 생성자는 반환값 없이 size()==0인 colors의 수명을 시작한다.
    // (5) represented_nodes는 이어지는 멤버 초기화 목록에서 1이 되고 생성자 인자 color는 변하지 않는다.
    // (6) 상수 시간이며 구현은 bucket 할당을 지연할 수 있다. 기본 정책 객체 생성은 이 타입들에서
    //     무예외지만 일반 서명은 조건부 noexcept다. 원소 관찰자·무효화 대상은 아직 없고, ColorBag 수명
    //     종료 때 colors가 원소와 bucket을 해제한다. 생성 중 객체는 다른 스레드와 공유하지 않는다.
    // explicit은 int 하나가 문맥상 ColorBag으로 몰래 변환되는 것을 막는다. `colors{}`와
    // `represented_nodes{1U}`는 생성자 본문 전에 실행되는 멤버 초기화 목록이며, 대입보다 먼저 각
    // 멤버의 수명을 올바른 값으로 시작한다.
    explicit ColorBag(const int color)
        : colors{}, represented_nodes{1U} {
        // [첫 호출 계약: ColorSet::insert(const int&)]
        // (1) 수신 colors는 방금 기본 생성되어 비어 있는 수정 가능한 ColorSet lvalue다.
        // (2) 선택 overload는 pair<iterator,bool> unordered_set<int>::insert(const value_type&)다.
        // (3) color는 [1,10^9]의 const int lvalue이며 const int&로 잠시 빌린다. 정수 값만 복사하고
        //     호출자 저장소나 소유권은 넘기지 않는다.
        // (4) 반환 pair는 삽입 위치와 성공 여부를 담지만, 빈 집합이라 성공이 보장되어 여기서는 버린다.
        // (5) colors는 정확히 color 하나를 소유하고 color 및 represented_nodes는 바뀌지 않는다.
        // (6) 평균 O(1), 충돌이 극단적이면 O(size)다. node/bucket 할당이나 rehash가 가능하고 rehash는
        //     모든 iterator를 무효화하지만 원소 pointer/reference는 유지한다. 할당·hash·동등 비교 예외가
        //     가능하며 실패 시 컨테이너는 유효하다. 생성 중 단일 스레드만 접근한다.
        colors.insert(color);
    }
};

} // namespace

// main은 운영체제에 int 종료 코드를 돌려주는 함수다. 매개변수가 없으므로 명령행 인자를 빌리지 않는다.
// 기본 타입 int는 정점 번호·색·정답 상한을 담고, std::size_t는 vector 크기와 인덱스를 담는다.
int main() {
    // [첫 호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 객체 수신자가 없는 정적 설정 함수이며 아직 어느 표준 stream에서도 입출력하지 않은 상태다.
    // (2) 선택 시그니처는 static bool ios_base::sync_with_stdio(bool sync = true)다.
    // (3) false는 bool prvalue이고 소유권 의미가 없으며 C stdio와 C++ iostream의 동기화 해제를 요청한다.
    // (4) 이전 동기화 설정 bool을 반환하지만 이 코드는 사용하지 않는다.
    // (5) 이후 C++ 표준 stream은 독립 버퍼링될 수 있고 cin/cout과 buffer의 소유권은 유지된다.
    // (6) 표준은 별도 점근 복잡도·noexcept를 보장하지 않는다. 첫 I/O 뒤 호출 효과는 구현 정의이므로
    //     지금 실행하며, 설정과 stream을 다른 스레드에서 동시에 건드리지 않는다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie(nullptr)]
    // (1) 수신 cin은 프로그램 수명 동안 유효한 istream lvalue이고 cout에 묶여 있을 수 있다.
    // (2) 선택 overload는 ostream* basic_ios::tie(ostream* tied)다.
    // (3) nullptr는 소유권 없는 null pointer prvalue이며 새 tie 대상을 두지 않겠다는 뜻이다.
    // (4) 이전 tie 대상 pointer를 반환하지만 사용하지 않는다.
    // (5) 입력 전 자동 cout flush 연결만 해제되고 stream과 buffer의 수명은 바뀌지 않는다.
    // (6) 표준은 별도 복잡도·noexcept를 규정하지 않는다. non-null tie 순환은 문제가 될 수 있지만
    //     nullptr는 순환을 만들지 않는다. 전역 stream 설정을 다른 스레드와 동시에 바꾸지 않는다.
    std::cin.tie(nullptr);

    // {}는 기본 타입을 0으로 값 초기화하여 미초기화 읽기를 막는다.
    int node_count{};

    // [첫 호출 계약: std::cin >> int와 basic_ios::operator!]
    // (1) 수신 cin은 유효한 istream lvalue이고 node_count는 수정 가능한 0인 int lvalue다.
    // (2) basic_istream<char>::operator>>(int&) overload가 선택되고, 연쇄 결과에는
    //     bool basic_ios<char>::operator!() const overload가 선택된다.
    // (3) node_count를 non-const int&로 빌리며 소유권은 넘기지 않는다. 공식 입력의 [1,200000] 값이다.
    // (4) 추출은 같은 istream&를 반환하고 operator!은 fail/bad 여부 bool을 반환해 if가 사용한다.
    // (5) 성공하면 node_count와 입력 위치가 바뀐다. 실패하면 상태 비트가 설정되고 프로그램을 끝낸다.
    // (6) formatted 입력은 문자 수와 locale 처리에 비례하고 공통 점근 상한은 없다. 예외 mask가 켜져
    //     있으면 ios_base::failure가 가능하며 부분 대입은 롤백되지 않는다. cin은 단일 스레드에서 읽는다.
    if (!(std::cin >> node_count)) {
        return 0;
    }

    // [첫 생성 계약: vector<int>(count, value)]
    // (1) colors는 아직 수명이 시작되지 않은 지역 std::vector<int> 객체다.
    // (2) vector(size_type count, const value_type& value, const Allocator& = Allocator()) overload와
    //     기본 allocator<int>가 선택된다.
    // (3) count는 n+1의 size_t prvalue, value는 0 int prvalue가 만든 임시를 const int&로 빌린다.
    //     둘 다 공식 범위이고 외부 저장소 소유권은 없다.
    // (4) 생성자는 반환값 없이 count개의 0을 소유한 colors를 완성한다.
    // (5) 0번 칸을 sentinel로 비워 둔 1-based 저장소가 생기고 node_count는 바뀌지 않는다.
    // (6) O(n) 시간·공간이고 length_error/bad_alloc 가능성이 있다. 실패 시 만든 원소와 저장소를
    //     정리한다. 구조 변경 전까지 원소 reference는 유효하며 scope 종료 때 모두 파괴한다.
    //     생성 중 다른 스레드와 공유하지 않는다. 같은 생성자 계약을 parent와 answer에도 재사용한다.
    std::vector<int> colors(
        static_cast<std::size_t>(node_count) + 1U, 0);

    for (int vertex{1}; vertex <= node_count; ++vertex) {
        // [첫 호출 계약: vector<int>::operator[]]
        // (1) 수신 colors는 크기 n+1인 수정 가능한 vector<int> lvalue이고 모든 칸이 초기화되어 있다.
        // (2) reference vector<int>::operator[](size_type position) noexcept overload가 선택된다.
        // (3) vertex는 [1,n] int lvalue이고 size_t prvalue로 명시 변환된다. 소유권 이동은 없다.
        // (4) 해당 int의 lvalue reference를 반환해 앞서 설명한 operator>>(int&)의 출력 인자로 사용한다.
        // (5) 성공한 추출은 그 칸의 값만 바꾸며 vector 크기/capacity와 다른 원소는 그대로다.
        // (6) O(1)·무할당·무효화 없음이다. 범위 검사를 하지 않아 position>=size면 UB지만 공식 범위와
        //     루프가 막는다. 반환 참조는 colors 수명/재할당 전까지 유효하며 단일 스레드에서 수정한다.
        std::cin >> colors[static_cast<std::size_t>(vertex)];
    }

    // [첫 생성 계약: vector<vector<int>>(count)]
    // (1) graph는 아직 수명이 시작되지 않은 지역 vector<vector<int>> 객체다.
    // (2) explicit vector(size_type count, const Allocator& = Allocator()) overload가 선택되어 count개의
    //     기본 생성된 빈 vector<int>를 소유한다.
    // (3) count는 n+1의 size_t prvalue이고 외부 저장소나 간선 소유권을 받지 않는다.
    // (4) 생성자는 반환값 없이 1-based 인접 리스트 graph를 완성한다.
    // (5) 모든 내부 인접 리스트가 비어 있고 colors/node_count는 바뀌지 않는다.
    // (6) 외부 vector 생성은 O(n) 시간·공간이며 length_error/bad_alloc 가능성이 있다. 각 내부 vector는
    //     아직 원소 할당이 없을 수 있다. scope 종료 시 중첩 소멸이 모든 간선 저장소를 정리한다.
    //     성공한 뒤 외부 graph 크기를 바꾸지 않으므로 내부 vector 객체의 주소는 안정적이다.
    std::vector<std::vector<int>> graph(
        static_cast<std::size_t>(node_count) + 1U);

    for (int edge_index{}; edge_index < node_count - 1; ++edge_index) {
        int from{};
        int to{};
        // 앞서 설명한 int 추출 overload를 두 번 연쇄한다. 공식 입력이 완전하다는 문제 계약을 사용한다.
        std::cin >> from >> to;

        // [첫 호출 계약: vector<vector<int>>::operator[]와 vector<int>::push_back(const int&)]
        // (1) graph는 크기 n+1인 수정 가능한 outer vector lvalue이고 graph[from]은 유효한 빈/기존
        //     vector<int>다. to는 입력된 int lvalue다.
        // (2) outer의 reference operator[](size_type)와 inner의 void push_back(const value_type&)가
        //     차례로 선택되며 inner value_type은 int다.
        // (3) from은 [1,n] int lvalue에서 size_t prvalue로 변환된다. to는 [1,n] int lvalue로서
        //     const int&로 잠시 빌리고 값만 복사하며 소유권을 넘기지 않는다.
        // (4) []는 vector<int>&를 push의 수신자로 반환하고 push_back은 void라 반환값이 없다.
        // (5) from의 인접 리스트 끝에 to 복사본이 하나 추가되고 outer 크기와 입력 변수는 그대로다.
        // (6) []는 O(1), push는 amortized O(1)이다. 범위 밖 []는 UB이나 제약이 막는다. inner 재할당은
        //     그 리스트의 기존 iterator/reference/pointer를 모두 무효화하고 bad_alloc이 가능하지만 int
        //     복사는 무예외라 실패 시 강한 보장을 제공한다. 같은 리스트를 병행 수정하지 않는다.
        graph[static_cast<std::size_t>(from)].push_back(to);
        graph[static_cast<std::size_t>(to)].push_back(from);
    }

    std::vector<int> parent(
        static_cast<std::size_t>(node_count) + 1U, 0);

    // [첫 생성 계약: vector<int>()]
    // (1) order는 아직 생성 전인 지역 vector<int>다.
    // (2) 기본 allocator<int>를 쓰는 vector() 기본 생성자가 선택된다.
    // (3) 명시 인자와 빌린 외부 저장소가 없다.
    // (4) 반환값 없이 size()==0인 유효한 빈 vector를 만든다.
    // (5) 다른 컨테이너는 바뀌지 않고 order는 아직 정점을 소유하지 않는다.
    // (6) O(1), 기본 allocator에서 noexcept이며 무효화할 관찰자가 없다. scope 종료 시 확보한 저장소를
    //     해제하고, 단일 스레드에서만 접근한다.
    std::vector<int> order{};

    // [첫 호출 계약: vector<int>::reserve(count)]
    // (1) 수신 order는 유효한 빈 vector<int> lvalue이고 capacity는 구현이 정한 값이다.
    // (2) void vector<int>::reserve(size_type new_capacity) overload가 선택된다.
    // (3) node_count는 [1,200000] int lvalue에서 size_t prvalue로 변환되며 소유권 의미가 없다.
    // (4) 반환형 void라 반환값은 없다.
    // (5) 성공하면 capacity>=n, size==0이어서 이후 정확히 n번 push하는 동안 재할당하지 않는다.
    // (6) O(n) 이하 저장 확보이며 length_error/bad_alloc이 가능하다. 실패 시 order는 변하지 않는다.
    //     실제 재할당 때 기존 iterator/reference/pointer가 모두 무효화되지만 현재 원소는 없다.
    //     저장소는 order가 소유하고 병행 접근하지 않는다.
    order.reserve(static_cast<std::size_t>(node_count));

    parent[1U] = 0;
    const int root{1};
    // vector<int>::push_back(const int&)는 graph에서 처음 설명했다. root 값만 order가 복사해 소유한다.
    order.push_back(root);

    // [첫 호출 계약: vector<int>::size()]
    // (1) 수신 order는 root부터 지금까지 발견한 정점을 소유한 유효한 vector<int> lvalue다.
    // (2) size_type vector<int>::size() const noexcept가 선택된다.
    // (3) 명시 인자가 없고 원소나 저장소를 빌리지 않는다.
    // (4) 현재 원소 수 size_t prvalue를 반환해 for의 비교가 사용한다.
    // (5) order의 원소·크기·capacity는 변하지 않는다.
    // (6) O(1)·무할당·무효화 없음·noexcept다. 같은 객체를 동시에 수정하지 않으므로 data race가 없다.
    //     조건을 매 반복 다시 읽으므로 아래 push로 늘어난 새 정점도 차례로 방문한다.
    for (std::size_t cursor{}; cursor < order.size(); ++cursor) {
        const int vertex{order[cursor]};
        for (std::size_t edge_index{};
             edge_index < graph[static_cast<std::size_t>(vertex)].size();
             ++edge_index) {
            const int next{
                graph[static_cast<std::size_t>(vertex)][edge_index]};

            // 트리에는 교차 간선이 없으므로 직전 부모만 건너뛰면 각 정점을 정확히 한 번 발견한다.
            if (next == parent[static_cast<std::size_t>(vertex)]) {
                continue;
            }

            parent[static_cast<std::size_t>(next)] = vertex;
            order.push_back(next);
        }
    }

    // [첫 생성 계약: vector<unique_ptr<ColorBag>>(count)]
    // (1) bags는 아직 수명이 시작되지 않은 지역 vector<unique_ptr<ColorBag>> 객체다.
    // (2) explicit vector(size_type count) 생성자가 count개의 default-constructed unique_ptr<ColorBag>를
    //     만들며 각 포인터의 deleter는 default_delete<ColorBag>다.
    // (3) count는 n+1 size_t prvalue이고 외부 pointer나 소유권을 받지 않는다.
    // (4) 생성자는 반환값 없이 count개의 null 단독 소유 핸들을 가진 bags를 완성한다.
    // (5) 아직 ColorBag 객체는 없고 다른 컨테이너도 바뀌지 않는다.
    // (6) O(n) 시간·공간이고 outer 저장소 할당 실패가 가능하다. null unique_ptr 소멸은 pointee를
    //     파괴하지 않는다. bags 크기를 바꾸지 않아 핸들 원소 참조는 안정적이며 단일 스레드 전용이다.
    std::vector<std::unique_ptr<ColorBag>> bags(
        static_cast<std::size_t>(node_count) + 1U);

    std::vector<int> answer(
        static_cast<std::size_t>(node_count) + 1U, 0);

    /*
    알고리즘 대표 문서: ../algorithm/small-to-large-set-merging.md

    대회 불변식과 정확성:
    - order를 뒤에서 읽을 때 현재 vertex의 모든 자식은 이미 완성된 bag을 정확히 하나 소유한다.
    - current.colors는 현재까지 합친, 서로 겹치지 않는 정점 성분들의 색 합집합이고
      represented_nodes는 그 성분들의 원래 정점 수 합이다.
    - swap은 ColorBag 자체를 복사하지 않고 어느 unique_ptr가 어느 bag을 소유하는지만 바꾼다.
      merge는 큰 집합에 없던 색 node만 옮기므로 결과는 두 색 집합의 합집합이다. 중복 node는 작은
      집합에 남았다가 reset 때 파괴된다. 모든 자식을 합친 뒤 current.colors는 정확히 vertex의
      서브트리 색 합집합이므로 그 size가 정답이다.

    우승 노하우와 복잡도:
    - 집합의 현재 서로 다른 색 수가 아니라 `대표하는 원래 정점 수`가 작은 성분을 큰 성분으로 옮긴다.
      따라서 옮겨져 살아남은 hash node가 다시 작은 쪽이 될 때마다 그 소유 성분 정점 수가 적어도
      두 배가 된다. 각 node는 최대 floor(log2 n)+1번 검사되고, 중복으로 남은 node는 즉시 파괴되어
      다시 검사되지 않는다.
    - hash/equality가 평균 O(1)이라는 통상 가정에서 전체 시간은 O(n log n), 살아 있는 hash node,
      graph, 순서와 배열을 합친 공간은 O(n)이다. 표준의 보장은 평균일 뿐이다. 의도적 충돌이나 나쁜
      hash 분포에서는 한 merge의 최악 상한이 source_size * destination_size에 비례할 수 있다.
      서로 다른 원래 정점 쌍에 그 곱을 충전하면 전체 보수적 최악 상한은 O(n^2)이다.
    - 재귀 DFS는 길이 200,000 사슬에서 호출 stack을 넘칠 수 있다. 명시적 order가 그 위험을 없앤다.

    값 범주·수명·소유권:
    - colors[index]와 이름 있는 current는 lvalue다. make_unique가 돌려주는 unique_ptr prvalue는
      current를 직접 초기화하고, std::move(current)는 이름 있는 lvalue를 xvalue로 바꿔 핸들만
      move-assignment하게 한다. ColorBag과 unordered_set 자체를 복사하지 않는다.
    - C++17 이후 prvalue 직접 초기화의 복사 생략 규칙 덕분에 반환 임시를 불필요하게 복사할 필요가
      없다. 이는 이름 있는 지역 객체의 NRVO와 구별된다. 이 풀이의 정확성은 선택적 NRVO에 의존하지
      않고, heap ColorBag의 주소와 수명은 unique_ptr 이동 뒤에도 그대로 유지된다.
    - const int 지역은 읽기 전용 의도를 고정한다. merge의 source는 수정 가능한 lvalue reference로
      잠시 빌리고, unique_ptr::operator->가 주는 raw pointer는 소유권을 주지 않는다.

    기계 실행 관점:
    vector 순회는 인덱스·부모·색의 load, parent/answer의 store, 크기 비교와 조건 분기를 만들 수 있다.
    unordered_set은 hash 계산 뒤 bucket과 node pointer를 간접 load하고 merge는 node 연결 pointer를
    다시 store할 수 있다. 이 소스에는 virtual 함수가 없어 virtual 간접 호출을 요구하지 않지만,
    실제 인라이닝·분기 제거·간접 호출·명령 배치·cache 효과는 CPU, ABI, 표준 라이브러리 구현,
    컴파일러와 최적화 옵션에 따라 달라진다. 소스만 보고 특정 어셈블리라고 단정하지 않는다.
    */
    for (std::size_t reverse_index{order.size()};
         reverse_index > 0U;
         --reverse_index) {
        const int vertex{order[reverse_index - 1U]};

        // [첫 호출 계약: std::make_unique<ColorBag>(int lvalue)]
        // (1) current는 아직 수명이 시작되지 않은 지역 unique_ptr<ColorBag>이고 대상 ColorBag도 없다.
        // (2) template 인자는 T=ColorBag, Args=int&로 추론된 make_unique<T>(Args&&...)가 선택되고,
        //     내부에서 explicit ColorBag(int) 생성자를 호출한다.
        // (3) colors[vertex]는 [1,10^9] int lvalue다. forwarding reference로 받은 뒤 int 값 매개변수에
        //     복사하며 vector 저장소나 소유권은 넘기지 않는다.
        // (4) 새 ColorBag을 단독 소유한 unique_ptr<ColorBag> prvalue를 반환해 current를 직접 초기화한다.
        // (5) current는 non-null이고 bag은 자기 색 하나와 represented_nodes==1을 갖는다. colors는 그대로다.
        // (6) ColorBag 및 hash node 동적 할당 비용이 들고 bad_alloc이나 생성자 내부 hash/비교 예외가
        //     가능하다. 실패하면 확보 자원을 정리한다. 성공한 pointee는 current/reset/이동된 최종
        //     owner가 파괴할 때까지 산다. 공유하지 않아 data race가 없다.
        auto current{std::make_unique<ColorBag>(
            colors[static_cast<std::size_t>(vertex)])};

        for (std::size_t edge_index{};
             edge_index < graph[static_cast<std::size_t>(vertex)].size();
             ++edge_index) {
            const int child{
                graph[static_cast<std::size_t>(vertex)][edge_index]};

            // parent[child]==vertex인 이웃만 이미 완성된 자식 bag이다. 부모 방향 이웃은 건너뛴다.
            if (parent[static_cast<std::size_t>(child)] != vertex) {
                continue;
            }

            // [첫 호출 계약: vector<unique_ptr<ColorBag>>::operator[], unique_ptr::operator->,
            //                  그리고 멤버 크기 비교]
            // (1) bags는 크기 n+1인 vector lvalue다. 역순 불변식으로 bags[child]는 non-null이며 current도
            //     non-null이다. 두 bag의 represented_nodes는 각자 대표하는 양의 정점 수다.
            // (2) vector의 reference operator[](size_type)와
            //     ColorBag* unique_ptr<ColorBag>::operator->() const noexcept가 선택된다.
            // (3) child는 [1,n] int lvalue에서 size_t prvalue로 변환된다. operator->에는 명시 인자가
            //     없고 포인터나 소유권을 전달하지 않는다.
            // (4) []는 unique_ptr<ColorBag>&, operator->는 소유하지 않는 ColorBag*를 반환하고 두
            //     size_t 값을 내장 `<`가 비교한 bool을 if가 사용한다.
            // (5) 비교만 하므로 두 owner, pointee, 집합, 정점 수는 모두 변하지 않는다.
            // (6) []/operator->/비교는 O(1)·무할당·무효화 없음이고 operator->는 noexcept다. 범위 밖 []나
            //     null 역참조는 UB지만 tree/후위 불변식이 막는다. raw pointer는 owner보다 오래 저장하지
            //     않으며 같은 bag을 다른 스레드가 접근하지 않는다.
            if (current->represented_nodes
                < bags[static_cast<std::size_t>(child)]->represented_nodes) {
                // [첫 호출 계약: unique_ptr<ColorBag>::swap]
                // (1) 수신 current와 인자 bags[child]는 서로 다른 non-null unique_ptr lvalue이고 각자
                //     유효한 ColorBag 하나를 단독 소유한다.
                // (2) void unique_ptr<ColorBag>::swap(unique_ptr& other) noexcept가 선택된다.
                // (3) bags[child]는 수정 가능한 unique_ptr lvalue reference로 빌리며 pointee 소유권을
                //     함수 밖으로 내보내지 않는다.
                // (4) 반환형 void라 반환값은 없다.
                // (5) 두 handle의 pointer/deleter가 교환되어 current가 더 큰 성분을 소유한다. ColorBag,
                //     hash node와 그 주소는 이동·복사·파괴되지 않는다.
                // (6) O(1)·무할당이고 default_delete 교환은 noexcept다. pointee 관찰자는 유효하지만
                //     어느 handle이 수명을 책임지는지는 바뀐다. 두 owner를 병행 접근하지 않는다.
                current.swap(bags[static_cast<std::size_t>(child)]);
            }

            // [첫 호출 계약: unordered_set<int>::merge(unordered_set&)]
            // (1) 수신 current->colors는 큰 성분의 유효한 ColorSet lvalue이고 source인
            //     bags[child]->colors는 작은 성분의 유효한 별도 ColorSet lvalue다.
            // (2) template merge<Hash2,Pred2>(unordered_set<Key,Hash2,Pred2,Allocator>& source)가
            //     Hash2=hash<int>, Pred2=equal_to<int>로 선택되며 두 allocator는 같은 기본 타입/값이다.
            // (3) source 식은 수정 가능한 lvalue reference로 빌린다. 컨테이너 자체 소유권은 유지하지만
            //     목적지에 없는 원소 node의 소유권은 수신 컨테이너로 이전해도 되는 상태다.
            // (4) 반환형 void라 반환값은 없다.
            // (5) 목적지에 없던 색 node만 복사/이동 생성 없이 이전된다. 중복 key node는 source에 남고,
            //     두 컨테이너는 유효하다. key int 값은 수정되지 않는다.
            // (6) source 원소 수 s에 대해 평균 O(s), 충돌 최악에는 목적지 크기와 곱해질 수 있다.
            //     node 자체는 재할당하지 않지만 목적지 rehash가 bucket을 할당할 수 있다. 표준 계약상
            //     목적지의 기존 iterator와 이전된 원소의 source iterator는 무효화되고, source에 남은
            //     원소 iterator는 유효하다. 이전 원소 pointer/reference는 유효하나 이제 목적지 수명에
            //     묶인다. allocator가 다르면 전제 위반이나 여기서는 같은 기본 allocator다.
            //     allocation/hash/equality 예외가 가능하므로 강한 전체 rollback에 의존하지 않으며,
            //     예외가 없다는 대회 환경에서 진행한다. 두 집합을 다른 스레드가 동시에 접근하지 않는다.
            current->colors.merge(
                bags[static_cast<std::size_t>(child)]->colors);

            current->represented_nodes +=
                bags[static_cast<std::size_t>(child)]->represented_nodes;

            // [첫 호출 계약: unique_ptr<ColorBag>::reset()]
            // (1) 수신 bags[child]는 merge source ColorBag을 여전히 단독 소유한 non-null unique_ptr lvalue다.
            //     source.colors에는 목적지와 중복되어 옮겨지지 않은 node만 남아 있을 수 있다.
            // (2) void unique_ptr<ColorBag>::reset(pointer ptr = pointer()) noexcept가 기본 null 인자로 선택된다.
            // (3) 생략된 인자는 null ColorBag* prvalue다. 새 소유권은 받지 않는다.
            // (4) 반환형 void라 반환값은 없다.
            // (5) source ColorBag과 남은 중복 node/bucket을 파괴·해제하고 bags[child]를 null로 만든다.
            //     이미 current로 이전된 node와 그 수명은 영향받지 않는다.
            // (6) 남은 원소 수에 비례해 파괴하며 별도 할당은 없다. 파괴된 source 원소의 모든 관찰자는
            //     무효다. reset은 noexcept이고 default_delete/멤버 소멸자는 던지지 않아야 한다.
            //     단일 스레드에서 소유권을 끝낸다.
            bags[static_cast<std::size_t>(child)].reset();
        }

        // [첫 호출 계약: unordered_set<int>::size()]
        // (1) 수신 current->colors는 vertex의 모든 자식과 자기 색을 합친 유효한 ColorSet lvalue다.
        // (2) size_type unordered_set<int>::size() const noexcept가 선택된다.
        // (3) 명시 인자와 소유권 이동이 없다.
        // (4) 서로 다른 색 수 size_t prvalue를 반환하며 n 이하라 int로 안전하게 변환해 answer에 저장한다.
        // (5) 집합의 node, bucket, 크기와 관찰자는 바뀌지 않는다.
        // (6) O(1)·무할당·무효화 없음·noexcept이며 동시 수정 없이 관찰한다.
        answer[static_cast<std::size_t>(vertex)] =
            static_cast<int>(current->colors.size());

        // [첫 호출 계약: std::move(unique_ptr&)와 unique_ptr move-assignment]
        // (1) 수신 bags[vertex]는 아직 null인 unique_ptr<ColorBag> lvalue이고 current는 완성된 bag을
        //     단독 소유하는 이름 있는 unique_ptr lvalue다.
        // (2) std::move는 remove_reference_t<T>&& move(T&&) noexcept에서 T=unique_ptr<ColorBag>&로
        //     추론되고, 이어 unique_ptr& operator=(unique_ptr&&) noexcept가 선택된다.
        // (3) current는 std::move에 lvalue로 전달되어 xvalue view가 된다. pointee를 복사하라는 뜻이
        //     아니라 소유 handle을 이전해도 된다는 표현이며 새 raw pointer 인자는 없다.
        // (4) move는 current를 가리키는 xvalue reference를, 대입은 수신 bags[vertex] lvalue reference를
        //     반환하지만 최종 반환값은 사용하지 않는다.
        // (5) bags[vertex]가 bag을 단독 소유하고 current는 null인 유효한 moved-from 상태가 된다.
        //     pointee, hash node, pointer/reference의 주소와 수명은 끊기지 않는다.
        // (6) O(1)·무할당·noexcept다. 기존 수신이 null이라 파괴할 pointee가 없다. unique_ptr 객체를
        //     가리키던 reference는 유효하지만 소유 상태가 바뀌며, 병행 접근하지 않는다.
        bags[static_cast<std::size_t>(vertex)] = std::move(current);
    }

    for (int vertex{1}; vertex <= node_count; ++vertex) {
        // [첫 호출 계약: std::cout << int]
        // (1) 수신 cout은 유효한 ostream lvalue이고 동기화는 해제됐지만 출력 buffer를 소유한다.
        // (2) basic_ostream<char>& basic_ostream<char>::operator<<(int value) overload가 선택된다.
        // (3) answer[vertex]는 [1,n] int lvalue이며 값으로 읽혀 소유권을 옮기지 않는다.
        // (4) 같은 ostream&를 반환하지만 이 문장에서는 버린다.
        // (5) locale 규칙에 따른 숫자 문자가 buffer에 추가되고 answer는 바뀌지 않는다. 실패하면 상태
        //     bit가 설정될 수 있다.
        // (6) 출력 문자 수와 locale/buffer에 비례하며 할당/원소 무효화는 없다. 예외 mask가 설정됐다면
        //     ios_base::failure가 가능하다. cout을 다른 스레드와 동시에 쓰지 않는다.
        std::cout << answer[static_cast<std::size_t>(vertex)];

        // [첫 호출 계약: std::cout << char]
        // (1) 수신 cout은 직전 정수 출력 뒤에도 유효한 ostream lvalue다.
        // (2) non-member basic_ostream<char>& operator<<(basic_ostream<char>&, char) overload가 선택된다.
        // (3) 조건 연산 결과는 마지막 원소면 '\n', 아니면 ' '인 char prvalue이며 소유권 의미가 없다.
        // (4) 같은 ostream&를 반환하지만 사용하지 않는다.
        // (5) 구분 문자 하나를 buffer에 추가하고 다른 객체는 바뀌지 않는다.
        // (6) 보통 O(1)이지만 buffer flush/device 비용은 구현·환경에 의존한다. 할당·관찰자 무효화는
        //     없고 상태 bit/설정된 예외로 오류를 알린다. 단일 스레드에서만 출력한다.
        std::cout << (vertex == node_count ? '\n' : ' ');
    }

    return 0;
}
