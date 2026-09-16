/*
문제 ID·제목: CSES 1751 - Planets Cycles
공식 출처: https://cses.fi/problemset/task/1751/

문제 요약
- 행성마다 정확히 하나의 다음 행성으로 가는 순간이동 장치가 있다. 자기 자신으로 갈 수도 있다.
- 각 행성에서 출발해 이미 한 번 방문한 행성에 도착할 때까지의 순간이동 횟수를 각각 구한다.
  따라서 사이클 안에서는 사이클 길이, 사이클 밖에서는 사이클까지의 거리와 길이의 합이 답이다.

입력: 첫 줄에 행성 수 n, 다음 줄에 1번부터 n번 행성의 도착지 t_i가 순서대로 주어진다.
출력: 1번부터 n번까지의 답 n개를 공백으로 구분해 한 줄에 쓴다.
공식 제약: 1 <= n <= 200,000, 1 <= t_i <= n. 시간 1초, 메모리 512MB.

공식 예제
입력:
5
2 4 3 1 4
출력:
3 3 1 3 4
*/

// <cstddef>는 vector의 크기·첨자 타입 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 정수 입력·출력 객체와 스트림 추출/삽입 연산을 선언한다.
#include <iostream>
// <vector>는 크기를 입력 후 결정하는 연속 저장 컨테이너 std::vector를 선언한다.
#include <vector>

/*
값 범주·수명·소유권: successor 같은 이름 있는 vector는 lvalue이며 자기 정수 저장소를 소유한다.
successor[index]는 원소를 빌린 int lvalue이고, n + 1과 answer[planet] + 1은 int prvalue다.
이 풀이에는 이동 변환으로 만든 xvalue나 큰 객체의 값 반환이 없어 이동·RVO가 이득을 주지 않는다.
원소 참조를 보존하지 않고 즉시 값으로 읽으므로 peeled의 재할당도 다른 배열의 원소 수명을 건드리지 않는다.
각 vector는 main 종료 때 저장소를 자동 해제한다. 함수 간 참조를 넘긴다면 소유 vector보다 오래
살아서는 안 되지만, 여기서는 모든 데이터의 수명이 main 안에서 닫힌다.
*/

// 구현 가까운 공용 알고리즘 문서: ../algorithm/functional-graph-cycle-peeling.md
// main의 int 반환형은 운영체제에 종료 상태를 전달한다. 매개변수가 없으므로 입력은 표준 스트림에서 읽는다.
int main() {
    // [호출 계약: std::ios::sync_with_stdio(bool)]
    // (1) 수신 객체 없는 정적 함수이며, 표준 C++ 스트림과 C stdio의 동기화 상태는 시작 기본값 true다.
    // (2) static bool ios_base::sync_with_stdio(bool sync = true)의 bool 인자 오버로드다.
    // (3) false는 bool prvalue로 값 전달하며 소유권이나 외부 버퍼를 넘기지 않는다.
    // (4) bool 이전 동기화 상태를 반환하지만 여기서는 쓰지 않는다.
    // (5) 이후 C++ 표준 스트림은 C stdio와 문자 단위 동기화를 하지 않는다.
    // (6) 기존 I/O 전에 실행한다. O(1), 무할당·참조 무효화 없음; C stdio와 섞어 읽고 쓰는 순서는
    //     별도로 관리해야 한다. 초기 설정은 단일 스레드에서 끝내고 이 풀이에는 동시 I/O가 없다.
    std::ios::sync_with_stdio(false);

    // [호출 계약: std::cin.tie(nullptr)]
    // (1) 수신 std::cin은 살아 있는 std::istream lvalue이고 기본적으로 std::cout에 묶여 있다.
    // (2) basic_ostream<char>* basic_ios<char>::tie(basic_ostream<char>* tiestr) setter다.
    // (3) nullptr는 null 포인터 prvalue로 변환되어 값 전달된다. 스트림 소유권은 전달되지 않는다.
    // (4) 이전에 묶인 ostream*을 반환하나 버린다.
    // (5) 입력 전에 자동 출력 flush를 요청하던 묶음이 해제된다. std::cout 객체 수명은 그대로다.
    // (6) O(1), 무할당·비무효화다. 대화형 입출력이라면 수동 flush가 필요할 수 있다. 이 배치
    //     문제에는 해당하지 않고 단일 스레드에서만 스트림 설정을 변경한다.
    std::cin.tie(nullptr);

    // int는 공식 최대 200,000, 답 최대 200,000을 안전하게 담는다. 중괄호는 n을 0으로 초기화한다.
    int n{};
    // [호출 계약: istream의 operator>>(int&)]
    // (1) 수신 std::cin은 정상 입력을 기다리는 std::istream lvalue이고 n은 초기값 0의 int lvalue다.
    // (2) basic_istream<char>& operator>>(int& val) 정수 추출 오버로드다.
    // (3) n은 쓰기 가능한 int&로 빌려 주며 소유권은 그대로다. 공식 입력의 [1,200000]을 기대한다.
    // (4) std::istream&를 반환하지만 사용하지 않는다.
    // (5) 성공 시 n은 읽은 수가 되고 입력 위치가 숫자 뒤로 이동한다. 실패 시 failbit가 설정된다.
    // (6) 숫자 자릿수에 선형, 보통 무할당·참조 무효화 없음. 형식 오류·EOF는 실패 상태로 나타나며
    //     온라인 저지의 유효한 입력을 전제로 한다. 공유 std::cin에 대한 동시 읽기는 하지 않는다.
    std::cin >> n;

    // [호출 계약: 세 vector<int> fill 생성과 한 기본 생성]
    // (1) successor, indegree, answer, peeled는 아직 생성 전이며 기존 원소·참조가 없다.
    // (2) 앞의 셋은 vector<int>::vector(size_type count, const int& value), peeled는
    //     vector<int>::vector()를 선택한다. 템플릿 원소 타입은 int, 할당자는 기본 할당자다.
    // (3) n+1은 [2,200001]의 int prvalue가 size_type으로 변환되고, 0은 int prvalue가
    //     const int&에 잠시 묶여 각 원소로 복사된다. peeled에는 인자가 없다. 외부 소유권은 없다.
    // (4) 생성자는 별도 반환값이 없다. 세 배열은 n+1개의 0을 갖고 peeled는 빈 상태가 된다.
    // (5) 각 vector가 서로 독립적인 저장소를 소유한다. 생성 실패 시 완성된 선행 객체는 정리된다.
    // (6) 총 O(n) 시간·공간, 동적 할당 가능. 길이 초과는 length_error, 메모리 부족은 bad_alloc이
    //     가능하다. 아직 관찰자가 없어 무효화 문제는 없고 외부 스레드에 공개하지 않는다.
    std::vector<int> successor(static_cast<std::size_t>(n + 1), 0);
    std::vector<int> indegree(static_cast<std::size_t>(n + 1), 0);
    std::vector<int> answer(static_cast<std::size_t>(n + 1), 0);
    std::vector<int> peeled;

    // [호출 계약: peeled.reserve(size_type)]
    // (1) 수신 peeled는 비어 있는 std::vector<int> lvalue이며 다른 곳이 원소를 참조하지 않는다.
    // (2) void std::vector<int>::reserve(size_type new_capacity)다.
    // (3) n을 변환한 std::size_t prvalue [1,200000]을 값 전달한다. 원소 소유권은 옮기지 않는다.
    // (4) void라 반환값은 없으며 이후 최대 n번 삽입의 재할당을 피한다.
    // (5) 크기 0은 유지되고 capacity는 적어도 n이 된다. 재할당되면 기존 참조·반복자는 무효지만 없다.
    // (6) 이 빈 vector에서는 할당 O(n) 공간, 원소 이동 O(1). length_error/bad_alloc 가능, 실패해도
    //     원래 vector는 유지된다. 저장소 수명은 peeled 수명까지이며 동시 접근은 없다.
    peeled.reserve(static_cast<std::size_t>(n));

    // for는 planet을 1부터 n까지 한 번씩 늘린다. destination은 각 입력 값을 담는 지역 int다.
    for (int planet{1}; planet <= n; ++planet) {
        int destination{};
        // 위와 동일한 int& 추출 오버로드: destination lvalue에 공식 범위 [1,n] 값을 기록한다.
        std::cin >> destination;
        // [호출 계약: std::vector<int>::operator[](size_type)]
        // (1) 수신 successor/indegree는 정확히 vector<int>의 비-const lvalue, size=n+1이다.
        // (2) reference vector<int>::operator[](size_type position) 비-const 오버로드다.
        // (3) planet/destination은 유효한 [1,n] int lvalue에서 size_type 값으로 바뀐다. 소유권 이동 없음.
        // (4) int& 원소를 반환해 대입 왼쪽 또는 ++의 피연산자로 즉시 사용한다.
        // (5) successor[planet]은 destination, indegree[destination]은 종전 값+1이 된다.
        //     컨테이너 크기·저장소·다른 원소는 그대로이고 반환 참조는 두 vector 수명에 묶인다.
        // (6) 각 O(1), 무할당·비무효화. 범위 밖은 UB라 공식 입력 범위가 필수다. 같은 원소에 대한
        //     동시 수정은 없으며 int 진입 차수는 최대 n이라 overflow하지 않는다.
        successor[static_cast<std::size_t>(planet)] = destination;
        ++indegree[static_cast<std::size_t>(destination)];
    }

    // 진입 차수 0 정점은 어떠한 유향 사이클에도 속할 수 없다. 최초 제거 대기열에 넣는다.
    for (int planet{1}; planet <= n; ++planet) {
        if (indegree[static_cast<std::size_t>(planet)] == 0) {
            // [호출 계약: peeled.push_back(const int&)]
            // (1) 수신 peeled는 capacity>=n인 std::vector<int> lvalue, 현재 size<n이다.
            // (2) void vector<int>::push_back(const int& value)를 선택한다.
            // (3) planet은 유효한 정점 번호 int lvalue이고 const 참조로 빌려 값만 복사한다.
            // (4) void로 반환값이 없으며 정점을 대기열 맨 뒤에 기록한다.
            // (5) size가 1 늘고 새 원소 수명이 시작된다. reserve 덕에 저장소 재할당은 없다.
            // (6) O(1), 이 호출에는 할당 없음. 이전 end 반복자만 무효화되나 보관하지 않는다.
            //     int 복사는 예외를 던지지 않고 동시 읽기/쓰기는 없다.
            peeled.push_back(planet);
        }
    }

    // peeled를 삭제 순서 배열 겸 FIFO로 쓴다. 각 정점은 마지막 들어오는 간선이 사라질 때 한 번만 삽입된다.
    // [호출 계약: peeled.size()와 peeled[head]]
        // (1) 수신 peeled는 std::vector<int> lvalue이고 size<=n, head는 현재 [0,size)다.
        // (2) size_type vector<int>::size() const noexcept와 reference operator[](size_type)다.
        // (3) size에는 명시 인자가 없고, []의 head는 std::size_t lvalue를 값으로 전달한다.
        // (4) size는 현재 원소 수를 반복 조건에 제공하고 []는 정점 int&를 반환해 값 복사에 쓴다.
        // (5) 둘 다 읽기 전용이어서 상태 변화가 없다. 뒤의 push_back은 size만 늘려 이후 순회에 포함한다.
        // (6) 각각 O(1), 무할당·비무효화. [] 범위 전제는 조건이 보장한다. 원소 주소는 reserve로
        //     고정되고 참조를 반복 경계 밖으로 보존하지 않는다. 동시 수정 스레드는 없다.
    for (std::size_t head{}; head < peeled.size(); ++head) {
        const int planet{peeled[head]};
        const int destination{successor[static_cast<std::size_t>(planet)]};

        // 삭제한 planet의 유일한 나가는 간선 효과를 지운다. destination이 0이 되는 순간만 큐에 넣는다.
        if (--indegree[static_cast<std::size_t>(destination)] == 0) {
            peeled.push_back(destination);
        }
    }

    // 제거되지 않은 정점은 각자 사이클 위다. answer==0을 아직 길이를 기록하지 않은 표시로 쓴다.
    for (int planet{1}; planet <= n; ++planet) {
        if (indegree[static_cast<std::size_t>(planet)] == 0 ||
            answer[static_cast<std::size_t>(planet)] != 0) {
            continue;
        }

        // do-while은 시작 정점도 세고 나서 다시 도착했는지 검사한다. 자기 루프면 길이 1이다.
        int cycle_length{};
        int cursor{planet};
        do {
            ++cycle_length;
            cursor = successor[static_cast<std::size_t>(cursor)];
        } while (cursor != planet);

        // 같은 사이클의 모든 정점은 최초 재방문까지 정확히 cycle_length번 이동한다.
        cursor = planet;
        do {
            answer[static_cast<std::size_t>(cursor)] = cycle_length;
            cursor = successor[static_cast<std::size_t>(cursor)];
        } while (cursor != planet);
    }

    // 제거 순서를 뒤집으면 각 정점의 유일한 다음 행성 answer가 이미 확정되어 있다.
    // 점화식 answer[v] = answer[successor[v]] + 1, 즉 꼬리 거리 + 도달한 사이클 길이다.
    for (std::size_t remaining{peeled.size()}; remaining > 0; --remaining) {
        const int planet{peeled[remaining - 1]};
        const int destination{successor[static_cast<std::size_t>(planet)]};
        answer[static_cast<std::size_t>(planet)] =
            answer[static_cast<std::size_t>(destination)] + 1;
    }

    // [호출 계약: ostream의 operator<<(int)와 비멤버 operator<<(ostream&, char)]
    // (1) 수신 std::cout은 출력 가능한 std::ostream lvalue이고 answer는 완성된 vector<int>다.
    // (2) 정수는 basic_ostream<char>& basic_ostream<char>::operator<<(int), 문자는
    //     basic_ostream<char>& operator<<(basic_ostream<char>&, char) 오버로드를 선택한다.
    // (3) answer[planet]의 int lvalue는 값으로 읽혀 [1,n]을 출력한다. 구분자 char prvalue는
    //     마지막만 '\n', 나머지는 ' '이다. 포인터·버퍼 소유권을 넘기지 않는다.
    // (4) 각 연산은 ostream&를 반환해 연쇄 호출에 사용하지만 최종 반환 참조는 버린다.
    // (5) 출력 버퍼에 문자들이 추가되고 실패 시 상태 비트가 바뀔 수 있다. answer는 변하지 않는다.
    // (6) 출력 문자 수에 선형, 버퍼 할당 가능하나 vector 원소/참조는 무효화하지 않는다. I/O 오류는
    //     기본적으로 badbit/failbit로 기록된다. 출력 스트림 동시 공유 사용은 하지 않는다.
    for (int planet{1}; planet <= n; ++planet) {
        std::cout << answer[static_cast<std::size_t>(planet)]
                  << (planet == n ? '\n' : ' ');
    }
}
