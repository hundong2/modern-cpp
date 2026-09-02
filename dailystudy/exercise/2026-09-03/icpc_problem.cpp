/*
문제 ID·제목: CSES 1684 - Giant Pizza
출처: CSES Problem Set / Graph Algorithms
출처 URL: https://cses.fi/problemset/task/1684

문제 요약:
n명의 사람과 m종류의 토핑이 있다. 각 사람은 “토핑 x를 넣어 달라(+)” 또는 “토핑 x를 빼 달라(-)”라는
두 소망을 말한다. 모든 사람에 대해 두 소망 중 적어도 하나를 만족하도록 각 토핑의 포함 여부를 정한다.
아래 내용은 저작권이 있는 원문 전체를 옮긴 것이 아니라 입력과 논리 조건을 한글로 재서술한 것이다.

입력:
첫 줄에 사람 수 n과 토핑 수 m이 주어진다. 이어지는 n줄에는 `부호 번호 부호 번호` 형태로 두 소망이
주어진다. `+ x`는 x번 토핑을 포함한다는 리터럴, `- x`는 포함하지 않는다는 리터럴이다.

출력:
가능하면 1번부터 m번 토핑까지 포함은 `+`, 제외는 `-`로 출력한다. 가능한 답이 여러 개면 아무 답이나 된다.
어떤 배정도 모든 사람을 만족시키지 못하면 `IMPOSSIBLE`을 출력한다.

제약:
1 <= n,m <= 100,000, 1 <= x <= m. 시간 제한 1초, 메모리 제한 512MB다.

예제:
입력
3 5
+ 1 + 2
- 1 + 3
+ 4 - 2

가능한 출력
- + + + -
*/

// <cstddef>는 컨테이너 크기와 인덱스에 쓰는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 std::cin/std::cout, 스트림 설정과 추출·삽입 연산자를 선언한다.
#include <iostream>
// <vector>는 인접 목록, 상태 배열, 명시적 DFS 스택을 소유하는 std::vector를 선언한다.
#include <vector>

// using은 긴 중첩 템플릿 타입에 새 이름만 붙인다. Graph도 결국 vector<vector<int>> 값을 소유한다.
using Graph = std::vector<std::vector<int>>;

// 재귀 함수 한 프레임이 기억할 “현재 정점”과 “다음 간선 위치”를 이름 있는 필드로 표현한다.
struct DfsFrame {
    int vertex{}; // int{}는 0으로 값 초기화하며 최대 노드 번호 199,999를 안전하게 담는다.
    std::size_t next_edge{}; // size_t{}는 0이며 vector의 size_type과 비교할 부호 없는 인덱스다.
};

[[nodiscard]] int literal_node(int one_based_topping, char sign) noexcept {
    const int zero_based{one_based_topping - 1};
    // 각 변수에 연속한 두 노드를 배정한다. 짝수는 positive, 홀수는 negative라서 ^1로 부정을 얻는다.
    return zero_based * 2 + (sign == '+' ? 0 : 1);
}

[[nodiscard]] int negated(int node) noexcept {
    return node ^ 1; // 이진수 최하위 비트를 뒤집어 x와 not x 사이를 O(1)에 오간다.
}

int main() {
    // static bool ios_base::sync_with_stdio(bool sync=true)는 인스턴스 수신자 없이 bool prvalue false를 받아
    // C stdio와의 동기화를 끄고 이전 설정 bool을 반환하지만 버린다. 첫 입출력 전에 호출하며 이후 C/C++ I/O를
    // 섞지 않는다. 표준 스트림 수명·소유권은 유지되고 별도 복잡도 상한은 없으며 버퍼 동작은 구현에 달린다.
    // 오류를 직접 보고하지 않고 같은 스트림에 대한 동시 비동기 접근의 안전성도 이 설정만으로 보장하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::ios::sync_with_stdio(false);
    // 수신자는 정확히 std::istream 타입의 std::cin lvalue다. setter overload ostream* tie(ostream* tied)는
    // 소유권 없는 null ostream* prvalue를 받아 자동 flush 연결을 해제하고 이전 포인터를 반환하지만 버린다.
    // cin/cout과 버퍼 수명은 유지되며 할당·무효화는 없다. 표준의 별도 복잡도·예외 보장은 없고 대화형 출력이 없다.
    std::cin.tie(nullptr);

    int clause_count{}; // 기본 타입 int를 0으로 값 초기화한다. 입력 최댓값 100,000을 담을 수 있다.
    int topping_count{};
    // 첫 멤버 operator>>(int&)의 수신자는 cin lvalue, 데이터 인자는 clause_count int lvalue이며 두 번째 호출은
    // 첫 반환 std::istream&를 수신자로 topping_count lvalue를 받는다. 각 성공 시 해당 int와 입력 위치가 바뀌고
    // 마지막 std::istream&는 버린다. 소유권·할당·참조 수명 변화는 없다. 비용은 소비 문자·locale·버퍼에
    // 의존하며 입력 실패·범위 오류는 기본적으로 예외 대신 상태 비트에 기록된다. 이 문제 입력은 유효하다고 보장된다.
    std::cin >> clause_count >> topping_count;

    const int node_count{topping_count * 2}; // 각 토핑의 positive/negative 리터럴을 각각 정점으로 만든다.
    // Graph는 vector<vector<int>> 별칭이다. count 생성자 vector(size_type count)는 수신 객체·반환값 없이
    // size_t prvalue node_count 하나와 기본 allocator를 받아 빈 vector<int>를 count개 값 초기화한다. count는
    // max_size 이하이고 음수 변환이 아니어야 하며 입력 제약이 보장한다. 성공 뒤 graph가 모든 안쪽 컨테이너를
    // 소유한다. O(node_count) 초기화와 저장소 할당이 들고 length_error/bad_alloc 가능성이 있다. 새 객체라 기존
    // 관찰자 무효화는 없으며 이 함수 단일 스레드 밖의 동시 접근도 없다.
    // 대표 문서: ../standard-library/containers-and-views.md
    Graph graph(static_cast<std::size_t>(node_count));
    Graph reversed(static_cast<std::size_t>(node_count)); // 같은 count 생성 계약으로 역그래프를 독립 소유한다.

    for (int clause{}; clause < clause_count; ++clause) {
        char left_sign{}; // char{}는 null 문자로 초기화되고 입력이 '+' 또는 '-'로 덮어쓴다.
        int left_topping{};
        char right_sign{};
        int right_topping{};
        // 첫 operator>>(char&)는 cin/std::istream lvalue와 left_sign char lvalue를 받아 기본 skipws 규칙으로
        // 비공백 문자 하나를 저장하고 같은 istream&를 반환한다. 이어 int&, char&, int& 선택 오버로드가 각각
        // lvalue 출력 인자를 갱신하며 마지막 반환은 버린다. 소유권·할당은 없고 입력 위치·상태만 바뀐다.
        // 비용은 소비 문자·locale·버퍼 구현에 의존한다. 실패는 상태 비트이며 CSES는 부호·범위를 보장한다.
        std::cin >> left_sign >> left_topping >> right_sign >> right_topping;

        const int left{literal_node(left_topping, left_sign)};
        const int right{literal_node(right_topping, right_sign)};
        const int not_left{negated(left)};
        const int not_right{negated(right)};

        // (left OR right)는 (not left -> right)와 (not right -> left) 두 implication과 동치다.
        // 바깥 vector::operator[](size_type)의 수신자는 graph lvalue, 인자는 범위 [0,node_count)의 size_t prvalue다.
        // 비const 오버로드는 해당 vector<int>&를 O(1)에 반환하며 graph 자체는 유지된다. 범위 검사가 없어 위반은
        // UB지만 literal mapping과 입력 제약이 범위를 증명한다. 바깥 vector는 이후 재할당하지 않아 반환 참조의
        // 수명은 graph까지다. 이어 push_back(const int&)의 수신자는 선택한 안쪽 vector<int>, 인자는 right int
        // lvalue가 const int&에 비소유 바인딩된 값이다. 값을 복사하고 size를 1 늘려 void를 반환한다. 분할 상환
        // O(1), 필요 시 할당/bad_alloc이 있고 재할당하면 그 안쪽 vector의 기존 포인터·참조·반복자가 무효다.
        // 아직 그런 관찰자를 저장하지 않으며 단일 스레드에서 구성한다.
        graph[static_cast<std::size_t>(not_left)].push_back(right);
        graph[static_cast<std::size_t>(not_right)].push_back(left);
        reversed[static_cast<std::size_t>(right)].push_back(not_left);
        reversed[static_cast<std::size_t>(left)].push_back(not_right);
    }

    // 2-SAT reduction, SCC 판정, 배정 순서의 자세한 근거는 공용 문서
    // ../algorithm/two-sat-implication-graph.md 를 따른다.

    // vector<char>(size_type count, const char& value) fill 생성자는 size_t prvalue count와 char prvalue 0에
    // 바인딩되는 const char&를 받아 count개 복사본을 소유한다. 반환값 없는 생성 뒤 size=count이고 모든 노드는
    // 미방문이다. O(count) 시간·공간, length_error/bad_alloc 가능성이 있으며 새 객체라 무효화 대상은 없다.
    std::vector<char> visited(static_cast<std::size_t>(node_count), char{0});
    // vector<int>() 기본 생성자는 수신 객체·인자·반환값 없이 size 0인 소유 컨테이너를 O(1)에 만든다.
    // 원소 수명은 아직 시작하지 않고 물리적 capacity/할당 여부는 단정하지 않는다. 생성은 예외 없이 끝난다.
    std::vector<int> finish_order;
    // reserve(size_type new_cap)의 수신자는 빈 finish_order lvalue, 인자는 node_count size_t prvalue다. count가
    // max_size 이하여야 한다. 성공하면 size 0을 유지하고 capacity를 최소 count로 만들어 이후 int push에서
    // 재할당하지 않는다. 현재 size에 선형이나 지금은 0, 반환은 void다. length_error/bad_alloc 시 원래 vector는
    // 유지되며 아직 무효화할 관찰자가 없다. 별도 스레드 동기화는 제공하지 않는다.
    finish_order.reserve(static_cast<std::size_t>(node_count));
    std::vector<DfsFrame> frames; // 재귀 호출 대신 종료 시점을 기억하는 명시적 프레임 스택이다.
    frames.reserve(static_cast<std::size_t>(node_count)); // 같은 reserve 계약으로 최악 깊이를 한 번 확보한다.

    // 첫 번째 DFS 불변식: visited 노드는 발견 즉시 1이고, finish_order에는 모든 outgoing edge가 끝난 뒤 한 번 들어간다.
    for (int start{}; start < node_count; ++start) {
        // vector<char>::operator[]의 수신자는 visited lvalue, 인자는 범위가 증명된 size_t start다. char&를 O(1)에
        // 반환하고 vector 상태는 유지한다. 검사 없는 범위 위반은 UB이며 반환 참조는 재할당·파괴 전까지 유효하다.
        if (visited[static_cast<std::size_t>(start)] != 0) {
            continue;
        }

        visited[static_cast<std::size_t>(start)] = 1;
        // push_back(DfsFrame&&)의 수신자는 capacity가 node_count 이상인 frames, 유일한 인자는 aggregate prvalue
        // DfsFrame{start,0}이 바인딩되는 rvalue 참조다. 원소를 끝에 이동 구성해 size를 1 늘리고 void를 반환한다.
        // 분할 상환 O(1)이고 reserve 덕분에 이 문제에서는 재할당하지 않는다. 일반적으로 할당 실패나 재할당 시
        // 관찰자 무효화가 가능하지만 여기에는 저장된 관찰자가 없으며 DfsFrame 이동은 예외를 던지지 않는다.
        frames.push_back(DfsFrame{start, std::size_t{0}});

        // empty() 수신자는 frames의 const 관찰 상태이고 인자는 없다. size==0 여부 bool을 O(1)·무할당·
        // noexcept로 반환해 바로 조건에 쓴다. 객체·원소·관찰자 수명은 바뀌지 않고 단일 스레드에서 호출한다.
        while (!frames.empty()) {
            // back() 수신자는 비어 있지 않은 frames lvalue이고 인자는 없다. 마지막 DfsFrame&를 O(1)에 반환하며
            // 상태·할당은 유지된다. 빈 vector 호출은 UB지만 while이 배제한다. 참조는 pop/reallocation 전까지만
            // 유효하다. 아래 push 뒤에는 사용하지 않아 잠재 무효화를 피하고 실제로는 reserve로 재할당도 막았다.
            DfsFrame& frame{frames.back()};
            const auto& next_vertices{graph[static_cast<std::size_t>(frame.vertex)]};

            // size() 수신자는 const vector<int> lvalue next_vertices이고 인자는 없다. 현재 원소 수 size_type을
            // O(1)·무할당·noexcept로 반환해 범위 증명에 쓴다. 컨테이너·소유권·참조 수명은 유지된다. true이면
            // 이어지는 operator[](frame.next_edge)가 const int&를 O(1)에 반환하며 범위 검사 실패는 UB다.
            if (frame.next_edge < next_vertices.size()) {
                const int next{next_vertices[frame.next_edge]};
                ++frame.next_edge; // 같은 간선을 두 번 처리하지 않는 프레임 불변식을 갱신한다.
                if (visited[static_cast<std::size_t>(next)] == 0) {
                    visited[static_cast<std::size_t>(next)] = 1; // 발견 시 표시해 중복 프레임을 막는다.
                    frames.push_back(DfsFrame{next, std::size_t{0}});
                }
                continue;
            }

            // vector<int>::push_back(const int&)이 frame.vertex int lvalue를 복사해 finish_order 끝에 추가한다.
            // 반환 void, 분할 상환 O(1), reserve로 재할당 없음이며 모든 자식 완료 뒤라 진짜 종료 순서가 된다.
            finish_order.push_back(frame.vertex);
            // pop_back() 수신자는 비어 있지 않은 frames이고 인자·반환값이 없다. 마지막 DfsFrame 수명을 끝내고
            // size를 1 줄이며 capacity는 유지한다. O(1), DfsFrame 소멸은 예외가 없다. 제거 원소 참조와 이전
            // past-the-end 반복자는 무효다. 바로 위 frame 참조는 이 호출 뒤 절대 사용하지 않는다.
            frames.pop_back();
        }
    }

    // component[v] == -1은 두 번째 DFS에서 아직 SCC를 배정하지 않았다는 뜻이다. vector<int> fill 생성자는
    // size_t count와 const int&에 바인딩되는 -1 prvalue를 받아 O(node_count)에 원소를 복사 소유한다.
    // 성공 후 size=count, 반환값 없음, 할당/length_error/bad_alloc 가능, 새 객체라 기존 관찰자 무효화 없음이다.
    std::vector<int> component(static_cast<std::size_t>(node_count), -1);
    std::vector<int> stack; // 기본 생성 계약으로 size 0인 두 번째 DFS 스택을 만든다.
    stack.reserve(static_cast<std::size_t>(node_count)); // 최악 SCC 크기만큼 확보하며 size는 0 그대로다.

    int component_id{};
    // finish_order 역순에서 역그래프 DFS를 시작하면 SCC 압축 DAG의 source 순서로 번호가 증가한다.
    for (std::size_t reverse_index{finish_order.size()}; reverse_index > 0; --reverse_index) {
        const int start{finish_order[reverse_index - 1]};
        if (component[static_cast<std::size_t>(start)] != -1) {
            continue;
        }

        component[static_cast<std::size_t>(start)] = component_id;
        stack.push_back(start); // int const& 복사 overload이며 reserve 덕분에 재할당하지 않는다.
        while (!stack.empty()) {
            const int vertex{stack.back()}; // 값을 복사한 뒤 제거하므로 제거 원소 참조가 남지 않는다.
            stack.pop_back();

            const auto& incoming{reversed[static_cast<std::size_t>(vertex)]};
            for (std::size_t edge_index{}; edge_index < incoming.size(); ++edge_index) {
                const int next{incoming[edge_index]};
                if (component[static_cast<std::size_t>(next)] == -1) {
                    component[static_cast<std::size_t>(next)] = component_id;
                    stack.push_back(next);
                }
            }
        }
        ++component_id;
    }

    // 우승 포인트 1: x와 not x가 같은 SCC면 서로를 강제하므로 어떤 진릿값도 모순을 피할 수 없다.
    for (int topping{}; topping < topping_count; ++topping) {
        const int positive{topping * 2};
        const int negative{negated(positive)};
        if (component[static_cast<std::size_t>(positive)] == component[static_cast<std::size_t>(negative)]) {
            // 첫 삽입은 수신자 std::cout(정확한 타입 std::ostream) lvalue와 non-null 문자열 리터럴 포인터를
            // 받는 비멤버 overload다. 문자를 버퍼에 기록하고 같은 std::ostream&를 반환하지만 버린다. 입력·소유권은
            // 유지되고 cout 위치·상태만 바뀐다. 비용은 문자 수·locale·버퍼/장치에 의존하며 별도 상한은 없다.
            // 출력 실패는 기본적으로 예외 대신 상태 비트에 남고 동시 출력 레코드 원자성은 보장하지 않는다.
            std::cout << "IMPOSSIBLE\n";
            return 0;
        }
    }

    // 우승 포인트 2: SCC 번호가 source->sink 위상 순서를 따르므로 더 큰 번호 쪽 리터럴을 참으로 두면 implication을
    // 거스르지 않는다. 해를 따로 탐색하지 않고 SCC 한 번으로 판정과 구성까지 O(n+m)에 끝낸다.
    for (int topping{}; topping < topping_count; ++topping) {
        const int positive{topping * 2};
        const int negative{negated(positive)};
        const char chosen{component[static_cast<std::size_t>(positive)] >
                                  component[static_cast<std::size_t>(negative)]
                              ? '+'
                              : '-'};
        if (topping != 0) {
            // operator<<(std::ostream&, char)의 첫 인자는 cout lvalue, 둘째는 공백 char prvalue다. 같은 ostream&를
            // 반환하지만 버린다. 인자와 소유권은 유지되고 출력 위치·상태만 바뀐다. 입력 문자 수는 하나지만 표준은
            // 별도 복잡도 상한을 두지 않고 버퍼/장치 비용은 구현에 따른다. 실패는 상태 비트이며 할당·참조 무효화·
            // 별도 스레드 동기화는 없다.
            std::cout << ' ';
        }
        std::cout << chosen; // 같은 char overload로 선택 부호를 기록하고 반환 ostream&는 버린다.
    }
    std::cout << '\n'; // 같은 char overload로 줄바꿈을 기록한다.

    return 0;
}
