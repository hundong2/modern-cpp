/*
문제: CSES 1686 — Coin Collector
공식 출처: https://cses.fi/problemset/task/1686/

문제 요약
- n개의 방과 m개의 단방향 터널이 있고, 각 방에는 양의 개수의 동전이 있다.
- 출발 방과 종료 방은 자유롭게 고를 수 있으며 터널 방향을 따라 이동한다.
- 한 번 방문한 방의 동전은 한 번만 얻는다고 볼 때 모을 수 있는 동전의 최댓값을 구한다.

입력
- 첫 줄: 방 수 n, 터널 수 m
- 둘째 줄: 각 방의 동전 k_i
- 다음 m줄: 단방향 터널 a -> b

출력
- 가능한 이동에서 모을 수 있는 동전 수의 최댓값 한 개

제약
- 1 <= n <= 100,000, 1 <= m <= 200,000
- 1 <= k_i <= 1,000,000,000, 1 <= a,b <= n
- 최댓값은 32-bit int를 넘을 수 있으므로 64-bit 정수로 누적한다.

예제
입력:
4 4
4 5 2 7
1 2
2 1
1 3
2 4
출력:
16

풀이 요약
1. 반복형 코사라주로 SCC를 구한다. 같은 SCC 안에서는 서로 왕복할 수 있으므로 그 SCC의 모든
   동전을 하나의 합으로 압축해도 된다.
2. 서로 다른 SCC 사이 터널만 압축 그래프에 남긴다. SCC 압축 그래프는 반드시 DAG다.
3. 진입 차수 0부터 Kahn 순서로 읽으며 best[next] = max(best[next], best[cur]+coin[next])를 갱신한다.
4. 재귀 대신 명시적 vector 스택을 써 n=100,000인 긴 사슬에서도 호출 스택에 의존하지 않는다.

대표 알고리즘 문서: ../algorithm/strongly-connected-components-kosaraju.md
시간 O(n+m), 추가 공간 O(n+m). 압축 간선의 중복을 제거하지 않아도 각 원래 간선을 한 번 처리한다.
*/

// <cstddef>는 크기와 색인을 나타내는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 동전 합에 쓰는 정확한 폭의 정수 별칭 std::int64_t를 선언한다.
#include <cstdint>
// <iostream>은 표준 입력/출력 객체와 스트림 연산자를 선언한다.
#include <iostream>
// <vector>는 인접 리스트, 스택, 순서, DP 배열을 소유하는 std::vector를 선언한다.
#include <vector>

// using은 새 타입을 만드는 것이 아니라 긴 중첩 컨테이너 타입에 의미 있는 별칭을 붙인다.
// 바깥/안쪽 template 인자는 각각 인접 리스트 원소 타입과 정점 번호 원소 타입이다.
using Graph = std::vector<std::vector<std::size_t>>;

// 반복 DFS 프레임은 정점과 다음에 검사할 간선 위치를 함께 보존한다.
// struct의 기본 접근은 public이라 두 필드를 aggregate 중괄호로 초기화한다. class의 기본 접근은
// private이며, 불변식을 숨길 객체라면 public 생성자와 private 멤버를 두는 편이 알맞다. 이 aggregate는
// 사용자 정의 생성자가 없어 explicit이나 멤버 초기화 목록을 쓰지 않는다.
struct DfsFrame {
    std::size_t vertex{};
    std::size_t next_edge{};
};

int main() {
    // int는 main의 반환형이며 매개변수 없는 형태다. 마지막 0은 정상 종료 상태다. 아래 const 지역
    // 변수는 초기화 뒤 재대입할 수 없지만 그 값이 가리키는 별도 owner의 수명을 늘리는 pointer는 아니다.
    // [첫 호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 정적 함수라 사용자 수신 객체는 없고 표준 스트림은 아직 입출력 전에 초기 상태다.
    // (2) static bool ios_base::sync_with_stdio(bool sync=true)가 선택된다.
    // (3) false는 bool prvalue이며 C 스트림과 C++ 스트림의 동기화를 끄라는 값이다.
    // (4) 이전 동기화 상태 bool을 반환하지만 성능 설정만 필요해 버린다.
    // (5) 이후 표준 C++ 스트림은 C stdio와 문자 단위 순서 보장을 공유하지 않는다.
    // (6) 프로그램 시작 뒤 첫 I/O 전에 호출한다. 이후 호출 효과는 구현 정의다. 표준 signature는
    //     noexcept가 아니며 이 설정 호출의 이식 가능한 점근 복잡도·내부 할당 보장은 없다. 별도
    //     컨테이너 무효화는 없지만 false 뒤에는 "synchronized standard stream"의 data-race 면제에
    //     기대지 않고 같은 stream의 동시 접근을 외부 동기화해야 한다. 복합 출력 원자성도 없다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie(nullptr)]
    // (1) 수신 std::cin은 유효한 std::istream lvalue이며 기본 tie를 가질 수 있다.
    // (2) basic_ostream<char>* basic_ios<char>::tie(basic_ostream<char>*) overload가 선택된다.
    // (3) nullptr prvalue는 새 tied output이 없음을 뜻하며 소유권을 이전하지 않는다.
    // (4) 이전 tied ostream pointer를 반환하지만 사용하지 않는다.
    // (5) 입력 전에 std::cout을 자동 flush하지 않으므로 대화형 문제가 아닌 오늘 입력에 적합하다.
    // (6) 포인터 소유권·컨테이너 무효화는 없지만 표준은 이 setter의 점근 복잡도·내부 할당을
    //     별도로 보장하지 않고 signature도 noexcept가 아니다. 같은 stream 설정의 동시 변경은 피한다.
    std::cin.tie(nullptr);

    std::size_t vertex_count{};
    std::size_t edge_count{};

    // [첫 호출 계약: std::cin operator>> 연쇄]
    // (1) 수신/왼쪽은 정상 상태의 std::istream lvalue std::cin이다.
    // (2) 오늘의 LLP64 w64devkit에서는 size_t가 unsigned long long이라 두 번 모두
    //     basic_istream<char>::operator>>(unsigned long long&)가 선택된다. 다른 ABI에서도 size_t가
    //     표준 unsigned 정수형의 alias라면 그 구체 overload가 선택된다. extended 정수형이라 직접
    //     writable-reference overload가 없다면 표준 정수 임시로 읽고 범위를 검사해 변환해야 한다.
    //     선택된 각 overload의 반환형은 basic_istream<char>&다.
    // (3) vertex_count와 edge_count는 쓰기 가능한 lvalue 참조 인자이며 소유권 이동은 없다.
    // (4) 첫 반환 참조를 다음 >> 수신으로 쓰고 마지막 반환 참조는 버린다.
    // (5) 성공하면 두 변수에 파싱한 값이 저장되고 입력 위치가 전진한다. 숫자 문자를 하나도 변환하지
    //     못하면 0과 failbit, 표현 범위를 넘으면 최댓값과 failbit가 남고 EOF를 만나면 eofbit도 선다.
    // (6) 표준은 이 형식 입력에 하나의 이식 가능한 점근 복잡도·내부 할당 보장을 정하지 않는다.
    //     locale/버퍼/장치 비용은 구현별이다. 유효한 공식 입력을 전제로 하며 exceptions mask가 설정된
    //     실패는 ios_base::failure를 던질 수 있다. 다른 thread에서 같은 stream을 동시에 읽지 않는다.
    std::cin >> vertex_count >> edge_count;

    // [첫 생성 계약: std::vector<std::int64_t>(count)]
    // (1) 목적 coins는 아직 없고 vertex_count는 공식 범위의 size_t 값이다.
    // (2) explicit vector(size_type count, const Allocator& = Allocator()) overload가
    //     T=int64_t로 선택된다.
    // (3) count를 값으로 복사하고 기본 allocator 인자는 생략한다. 외부 저장소를 빌리지 않는다.
    // (4) 생성자는 반환값 없이 count개 원소를 0으로 값 초기화한 소유 vector를 만든다.
    // (5) coins.size()==vertex_count이며 각 원소 수명은 vector가 관리한다.
    // (6) O(n) 시간/공간, length_error/bad_alloc 가능, 실패 시 완성 vector가 없다. 재할당이나
    //     파괴 전 원소 참조가 유효하고 구성 중 공유 접근은 없다.
    std::vector<std::int64_t> coins(vertex_count);

    // [첫 호출 계약: vector::operator[]]
    // (1) 수신 coins는 vertex_count개 int64_t를 소유한 non-const vector lvalue다.
    // (2) constexpr reference vector<int64_t>::operator[](size_type)가 선택되어 int64_t&를 반환한다.
    // (3) index는 [0,vertex_count)인 size_t lvalue 값이며 소유권을 넘기지 않는다.
    // (4) 반환 참조는 바로 뒤 operator>>가 동전 수를 저장할 위치로 사용한다.
    // (5) 색인 자체는 상태를 바꾸지 않고 입력 성공 시 해당 원소 값만 바뀐다.
    // (6) 유효 색인이 전제이며 위반은 UB다. 충족 시 O(1), 무할당/무효화 없음이다. 표준 signature는
    //     noexcept로 지정되지 않았으므로 generic 코드에서 noexcept라고 단정하지 않는다. 반환 참조는
    //     vector 파괴·재할당 전까지만 유효하고 같은 원소의 동시 쓰기는 없다.
    // [같은 식의 첫 호출 계약: basic_istream::operator>>(std::int64_t&)]
    // (1) 수신 std::cin은 유효한 std::istream lvalue이고 coins[index]는 살아 있는 int64_t lvalue다.
    // (2) 오늘 LLP64 환경에서는 int64_t가 long long이라
    //     basic_istream<char>::operator>>(long long&)가 선택되어 basic_istream<char>&를 반환한다.
    // (3) operator[]이 돌려준 쓰기 가능한 lvalue를 참조로 받고 소유권을 이전하지 않는다.
    // (4) 반환 stream 참조는 이 단일 추출 full-expression에서 사용하지 않고 버린다.
    // (5) 성공하면 해당 동전 값과 입력 위치가 갱신된다. 변환 실패는 0 또는 범위 경곗값과 failbit를,
    //     입력 끝을 만난 경우 eofbit도 남길 수 있다.
    // (6) 표준은 이 형식 입력의 단일 이식 가능 점근 복잡도·내부 할당을 정하지 않는다. 공식의
    //     유효한 양수 입력을 전제로 하며 실패 예외는 exceptions mask에 따르고 같은 stream은 공유하지 않는다.
    for (std::size_t index{}; index < vertex_count; ++index) {
        std::cin >> coins[index];
    }

    // [첫 생성 계약: Graph(count)와 내부 vector 기본 생성]
    // (1) graph/reversed는 아직 없고 vertex_count는 유효한 방 수다.
    // (2) 바깥 vector<vector<size_t>>(count) 생성자가 선택되고 각 원소는 빈 vector<size_t>로
    //     기본 생성된다. 두 문장은 서로 독립된 같은 overload 호출이다.
    // (3) count를 값으로 받고 외부 인접 리스트 저장소는 빌리지 않는다.
    // (4) 각 생성자는 반환값 없이 n개의 빈 인접 리스트를 소유하는 Graph를 만든다.
    // (5) 두 그래프 크기는 n, 모든 내부 size는 0이다.
    // (6) O(n) 기본 구성과 O(n) 메타데이터 공간, 할당 실패 가능이다. 성공한 두 owner는 독립이며
    //     이후 내부 push_back 재할당은 해당 내부 리스트 관찰자만 무효화한다.
    Graph graph(vertex_count);
    Graph reversed(vertex_count);

    for (std::size_t edge{}; edge < edge_count; ++edge) {
        std::size_t from{};
        std::size_t to{};
        std::cin >> from >> to;
        --from;
        --to;

        // [첫 호출 계약: std::vector<std::size_t>::push_back]
        // (1) 수신 graph[from]과 reversed[to]는 각각 유효한 내부 vector<size_t> lvalue다.
        // (2) constexpr void push_back(const value_type&)가 from/to lvalue에 선택된다.
        // (3) to와 from은 공식 범위 [0,n)의 size_t lvalue이며 값을 복사하고 소유권 의미는 없다.
        // (4) 반환형 void라 저장할 결과가 없다.
        // (5) 성공하면 각 리스트 끝에 간선 하나가 추가되고 size가 1 증가한다. 재할당이 없더라도
        //     삽입 전 past-the-end iterator는 무효가 된다.
        // (6) amortized O(1), 재할당하는 한 호출은 기존 size에 선형이다. signature는 noexcept가
        //     아니지만 비투척 size_t 복사인 오늘 호출은 길이 한계/할당 실패 등 예외 시 no effects다.
        //     재할당 시 그 내부 vector의 모든 관찰자가 무효화되고 다른 리스트는 영향 없다.
        //     구성은 한 thread에서 한다.
        graph[from].push_back(to);
        reversed[to].push_back(from);
    }

    // [첫 생성 계약: vector<unsigned char>(count,value) fill 생성자]
    // (1) 목적 visited는 아직 없고 vertex_count와 0 값은 유효하다.
    // (2) vector(size_type count, const value_type& value, const Allocator& = Allocator()) overload가
    //     T=unsigned char로 선택된다.
    // (3) count는 size_t 값, 두 번째 인자는 unsigned char prvalue가 const&에 결합되며 소유권은 없다.
    // (4) 생성자는 반환값 없이 count개 0 원소를 소유한 visited를 만든다.
    // (5) visited.size()==vertex_count이고 모든 원소는 미방문 값 0이다.
    // (6) O(n) 시간/공간, length_error/bad_alloc 가능이다. 실패 시 완성 객체가 없고 성공 뒤 저장소는
    //     vector 수명·재할당까지 유효하며 구성 중 다른 thread와 공유하지 않는다.
    std::vector<unsigned char> visited(vertex_count, static_cast<unsigned char>(0));

    // [첫 생성 계약: std::vector<std::size_t>() 기본 생성]
    // (1) 목적 order는 아직 없고 외부 저장소도 없다.
    // (2) vector() noexcept(noexcept(Allocator()))가 T=size_t로 선택된다.
    // (3) 명시 인자는 없고 기본 allocator를 내부에 보관한다.
    // (4) 생성자는 반환값 없이 size 0의 빈 소유 컨테이너를 만든다. capacity 값은 표준이 고정하지 않는다.
    // (5) order.empty()가 true이고 원소 수명은 아직 시작되지 않았다. capacity==0에 기대지 않는다.
    // (6) O(1)이고 기본 allocator 구성은 비투척이다. 원소 저장소 할당 여부를 이 코드가 가정하지
    //     않으며 기존 관찰자는 없고 다른 thread와 공유하지 않는다.
    std::vector<std::size_t> order{};

    // [첫 호출 계약: std::vector<std::size_t>::reserve]
    // (1) 수신 order는 size 0인 유효한 vector lvalue다.
    // (2) constexpr void reserve(size_type new_cap)가 선택된다.
    // (3) vertex_count는 필요한 최대 원소 수 size_t 값이며 소유권을 넘기지 않는다.
    // (4) 반환형 void라 값은 사용하지 않는다.
    // (5) 성공하면 capacity>=n, size는 0이라 이후 n번 push_back은 재할당하지 않는다.
    // (6) 표준의 원소 연산 복잡도는 현재 size에 선형이고 지금 size==0이라 옮길 원소는 없다.
    //     저장소 획득 비용·내부 할당 횟수는 allocator/구현에 따르며 bad_alloc/length_error 시 원본은
    //     유지된다. 실제 재할당이 있으면 기존 관찰자는 모두 무효이나 현재 없다. 동시 접근은 없다.
    order.reserve(vertex_count);

    // 알고리즘 대표 문서: ../algorithm/strongly-connected-components-kosaraju.md
    // 첫 단계 불변식: stack의 각 frame은 "이미 발견한 정점 + 다음 미검사 간선 위치"다.
    // 모든 나가는 간선을 처리한 순간에만 order에 넣으므로 정확한 DFS 종료 순서를 얻는다.
    for (std::size_t start{}; start < vertex_count; ++start) {
        if (visited[start] != static_cast<unsigned char>(0)) {
            continue;
        }
        visited[start] = static_cast<unsigned char>(1);

        // Frame vector의 기본 생성/소멸 계약은 위 빈 vector 계약과 같고 frame 값을 소유한다.
        std::vector<DfsFrame> stack{};

        // [첫 호출 계약: std::vector<DfsFrame>::push_back(value_type&&)]
        // (1) 수신 stack은 size 0인 유효한 vector<DfsFrame> lvalue이고 capacity는 특정하지 않는다.
        // (2) constexpr void push_back(value_type&&) overload가 선택되며 value_type은 DfsFrame이다.
        // (3) DfsFrame{start,0} prvalue가 임시 객체로 구체화되어 rvalue 참조에 결합된다. DfsFrame의
        //     암시적 이동 생성이 선택되지만 class 자원이 아닌 두 size_t 하위 객체 값은 복사되어
        //     초기화되므로 외부 자원 소유권 이전은 없다.
        // (4) 반환형 void라 사용할 값이 없다.
        // (5) 성공하면 size가 1이고 back()은 {start,0}이다. 임시는 full-expression 끝에 파괴된다.
        //     재할당이 없어도 삽입 전 past-the-end iterator는 무효가 된다.
        // (6) amortized O(1), 재할당 시 기존 size에 선형이다. signature는 noexcept가 아니지만
        //     비투척 DfsFrame 이동인 오늘 호출은 length_error/bad_alloc 등 예외 시 no effects다.
        //     재할당 시 모든 관찰자가 무효화되며 이 stack은 한 thread에서만 사용한다.
        stack.push_back(DfsFrame{start, 0});

        // [첫 호출 계약: std::vector<DfsFrame>::empty]
        // (1) 수신 stack은 현재 하나 이상의 frame을 소유하는 vector lvalue다.
        // (2) [[nodiscard]] bool empty() const noexcept가 선택된다.
        // (3) 명시 인자와 소유권 이동이 없다.
        // (4) size가 0인지 bool로 반환해 while 조건에 사용한다.
        // (5) 컨테이너와 모든 관찰자는 바뀌지 않는다.
        // (6) O(1), 무할당/비투척/무효화 없음이다. 같은 stack을 다른 thread와 공유하지 않는다.
        while (!stack.empty()) {
            // [첫 호출 계약: std::vector<DfsFrame>::back]
            // (1) 수신 stack은 위 while 조건으로 non-empty가 보장된 vector lvalue다.
            // (2) constexpr reference back()가 선택되어 DfsFrame&를 반환한다.
            // (3) 명시 인자나 소유권 이전은 없다.
            // (4) 반환 참조를 frame에 결합해 다음 간선 위치를 제자리 갱신한다.
            // (5) back 자체는 상태를 바꾸지 않지만 frame.next_edge 증가는 마지막 원소를 수정한다.
            // (6) non-empty가 전제이며 위반은 UB다. O(1), 무할당이며 표준 signature는 noexcept로
            //     지정되지 않았다. 뒤 push_back이 재할당하면 frame이 무효가 되므로 필요한 값은 push
            //     전에 읽고 이후 재사용하지 않는다.
            DfsFrame& frame{stack.back()};
            // frame은 vector 마지막 원소를 빌리는 non-const lvalue 참조다. 소유권이나 수명을 얻지 않고
            // stack 재할당 또는 해당 마지막 원소를 제거하는 pop_back 전까지만 유효하다. 다른 원소를
            // 제거하지 않는 오늘 stack 규칙도 함께 전제한다. pointer였다면 null 검사가 가능하지만 이
            // 참조는 non-empty 전제 아래 즉시 결합되어 null 상태가 없다.
            const std::size_t vertex{frame.vertex};

            // [첫 호출 계약: std::vector<std::size_t>::size]
            // (1) 수신 graph[vertex]는 살아 있는 내부 vector<size_t> lvalue다.
            // (2) size_type size() const noexcept가 선택된다.
            // (3) 명시 인자/소유권 이동이 없다.
            // (4) 현재 인접 정점 수 size_t prvalue를 경계 비교에 사용한다.
            // (5) 그래프와 반복자/참조는 바뀌지 않는다.
            // (6) O(1), 무할당/비투척/무효화 없음이며 그래프가 탐색 중 고정되어 수명이 안전하다.
            if (frame.next_edge < graph[vertex].size()) {
                const std::size_t next{graph[vertex][frame.next_edge]};
                ++frame.next_edge;
                if (visited[next] == static_cast<unsigned char>(0)) {
                    visited[next] = static_cast<unsigned char>(1);
                    stack.push_back(DfsFrame{next, 0});
                }
            } else {
                order.push_back(vertex);

                // [첫 호출 계약: std::vector<DfsFrame>::pop_back]
                // (1) 수신 stack은 non-empty이고 마지막 frame의 모든 간선을 처리한 상태다.
                // (2) constexpr void pop_back()가 선택된다.
                // (3) 명시 인자와 소유권 이전이 없다.
                // (4) 반환형 void라 결과는 없다.
                // (5) 마지막 frame 수명을 끝내고 size를 1 줄이며 capacity는 유지한다.
                // (6) non-empty가 전제다. O(1)이고 선언 자체에는 noexcept가 없지만 표준 컨테이너
                //     modifier 계약상 예외를 던지지 않는다. 오늘 frame 파괴도 비투척이다. 제거 원소
                //     관찰자와 이전 end iterator가 무효가 되며 현재 frame 참조도 즉시 무효라 쓰지 않는다.
                stack.pop_back();
            }
        }
    }

    // component[v]==vertex_count를 아직 SCC에 배정되지 않았다는 안전한 sentinel로 쓴다.
    std::vector<std::size_t> component(vertex_count, vertex_count);
    std::vector<std::int64_t> component_coins{};
    component_coins.reserve(vertex_count);

    // 종료 순서를 뒤에서 읽으면 역그래프 DFS 하나가 정확히 한 SCC만 모은다.
    for (std::size_t position{order.size()}; position > 0; --position) {
        const std::size_t start{order[position - 1]};
        if (component[start] != vertex_count) {
            continue;
        }

        const std::size_t component_id{component_coins.size()};
        component_coins.push_back(0);
        std::vector<std::size_t> stack{};
        stack.push_back(start);
        component[start] = component_id;

        while (!stack.empty()) {
            const std::size_t vertex{stack.back()};
            stack.pop_back();
            component_coins[component_id] += coins[vertex];

            for (std::size_t edge{}; edge < reversed[vertex].size(); ++edge) {
                const std::size_t next{reversed[vertex][edge]};
                if (component[next] == vertex_count) {
                    component[next] = component_id;
                    stack.push_back(next);
                }
            }
        }
    }

    const std::size_t component_count{component_coins.size()};
    Graph dag(component_count);
    std::vector<std::size_t> indegree(component_count, 0);

    // SCC가 다를 때만 압축 간선을 만든다. 중복 간선도 indegree와 처리 횟수에 똑같이 반영하면
    // Kahn 불변식이 유지되므로 별도 sort/unique 비용 없이 전체 O(n+m)를 지킨다.
    for (std::size_t vertex{}; vertex < vertex_count; ++vertex) {
        const std::size_t from_component{component[vertex]};
        for (std::size_t edge{}; edge < graph[vertex].size(); ++edge) {
            const std::size_t to_component{component[graph[vertex][edge]]};
            if (from_component != to_component) {
                dag[from_component].push_back(to_component);
                ++indegree[to_component];
            }
        }
    }

    std::vector<std::size_t> queue{};
    queue.reserve(component_count);
    for (std::size_t id{}; id < component_count; ++id) {
        if (indegree[id] == 0) {
            queue.push_back(id);
        }
    }

    // [첫 생성 계약: vector<int64_t> copy 생성]
    // (1) 목적 best는 아직 없고 component_coins는 SCC 합을 소유한 유효한 vector<int64_t> lvalue다.
    // (2) vector(const vector&) copy constructor overload가 선택되고 같은 원소/allocator 타입을 쓴다.
    // (3) copy 생성자의 인자 component_coins lvalue가 const vector&에 결합되며 원본 소유권은 유지된다.
    // (4) 생성자는 반환값 없이 독립 저장소를 소유하는 best를 완성한다.
    // (5) 성공 뒤 두 vector의 size와 값은 같지만 저장소·원소 수명은 독립이다.
    // (6) SCC 수 S에 O(S) 시간/공간, bad_alloc 가능이고 실패 시 원본은 유지된다. 원본 관찰자를
    //     무효화하지 않으며 각 vector를 별도 thread에서 다루려면 원소 접근 충돌도 없어야 한다.
    std::vector<std::int64_t> best{component_coins};
    // 위 식은 이름 있는 lvalue 원본을 사용하므로 copy다. 반면 임시 컨테이너 prvalue를 값으로 반환해
    // 같은 타입 결과를 직접 초기화하면 보장 복사 생략이 적용될 수 있고, 이름 있는 local 반환의 NRVO는
    // 허용되지만 의무는 아니다. 오늘 제출 코드는 그러한 factory 반환보다 한 번의 명시적 독립 복사를 택한다.
    std::int64_t answer{};

    // 불변식: queue[0..head)는 모든 들어오는 압축 간선을 이미 완화한 SCC들이다.
    // 각 SCC를 끝으로 하는 최댓값 best는 그 SCC를 pop할 때 확정된다.
    for (std::size_t head{}; head < queue.size(); ++head) {
        const std::size_t current{queue[head]};
        if (best[current] > answer) {
            answer = best[current];
        }

        for (std::size_t edge{}; edge < dag[current].size(); ++edge) {
            const std::size_t next{dag[current][edge]};
            const std::int64_t candidate{best[current] + component_coins[next]};
            if (candidate > best[next]) {
                best[next] = candidate;
            }
            --indegree[next];
            if (indegree[next] == 0) {
                queue.push_back(next);
            }
        }
    }

    // [첫 호출 계약: std::cout operator<<]
    // (1) 수신/왼쪽은 유효한 std::ostream lvalue std::cout이다.
    // (2) int64_t에는 대응하는 signed integral 멤버 overload, '\n'에는 char 비멤버 overload가
    //     선택되고 각각 std::ostream&를 반환한다.
    // (3) answer는 읽기만 하는 int64_t lvalue, newline은 char prvalue이며 소유권 이전이 없다.
    // (4) 첫 반환 참조를 다음 삽입에 쓰고 마지막 참조는 버린다.
    // (5) 성공하면 십진 최댓값과 줄바꿈을 기록하며 알고리즘 상태는 그대로다.
    // (6) 표준은 이 형식 출력 전체에 하나의 이식 가능한 점근 복잡도·내부 할당 보장을 정하지
    //     않는다. locale/버퍼/장치 비용은 구현별이고 실패는 상태 비트/설정된 예외로 보고된다.
    //     참조 무효화는 없고 여러 thread의 복합 레코드 원자성을 보장하지 않지만 이 프로그램은
    //     단일 thread다.
    std::cout << answer << '\n';

    // 기계 실행 관점에서 인접 리스트 탐색은 크기/색인 load, 비교와 조건 분기, DP 완화는 64-bit
    // load·덧셈·store를 유도할 수 있다. vector 재할당은 allocator 호출과 복사를 만들 수 있다.
    // 가상 함수가 없어 virtual 간접 호출은 요구되지 않는다. 실제 명령, cache miss, branch 예측,
    // inlining은 CPU·ABI·컴파일러·표준 라이브러리·최적화 옵션에 따라 달라 단정하지 않는다.
    return 0;
}
