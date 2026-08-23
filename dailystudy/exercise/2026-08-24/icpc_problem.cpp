/*
문제 ID·제목: Kattis oceancurrents - Ocean Currents (UVa 11573)
대회·출처: Waterloo Programming Contest 2009-02-08
출처 URL: https://open.kattis.com/problems/oceancurrents (Kattis Online Judge)

문제 요약:
직사각형 호수의 각 칸에는 0부터 7까지 한 자리 숫자로 표시된 해류 방향이 있다. 배는 현재 칸의
해류와 같은 방향으로 인접한 칸에 가면 에너지를 쓰지 않고, 나머지 일곱 방향 중 하나로 가면 에너지
1을 쓴다. 상하좌우와 네 대각선, 총 여덟 방향으로만 움직이며 호수 밖으로 나갈 수 없다. 여러 항해
질문마다 시작 칸에서 목적 칸까지 필요한 최소 에너지를 구한다. 이 설명은 원문을 복제하지 않고 요지를
한글로 다시 쓴 것이다.

입력:
첫 줄에 행 수 R과 열 수 C가 주어진다. 다음 R줄에는 길이 C인 숫자 문자열이 주어진다. 방향은 0=북,
1=북동, 2=동, 3=남동, 4=남, 5=남서, 6=서, 7=북서로 시계 방향이다. 다음 줄의 Q는 항해 질문 수다.
이후 Q줄에는 시작 행 rs, 시작 열 cs, 목적 행 rd, 목적 열 cd가 1부터 시작하는 좌표로 주어진다.

출력:
각 질문마다 최소 에너지 소비량을 한 줄에 하나씩 출력한다.

제약:
R과 C는 각각 1 이상 1,000 이하이고 Q는 최대 50이다. 모든 이동 간선 비용은 정확히 0 또는 1이다.
한 질문의 암시적 그래프는 정점 RC개, 간선 최대 8RC개이므로 0-1 BFS 한 번은 O(RC) 시간과 O(RC)
추가 공간에 동작한다. 전체는 O(QRC) 시간이다.

예제:
입력
5 5
04125
03355
64734
72377
02062
3
4 2 4 2
4 5 1 4
5 3 3 4

출력
0
2
1
*/

// <array>는 크기 8이 컴파일 시간에 고정된 방향 변화표 std::array를 선언한다.
#include <array>
// <cstddef>는 컨테이너 크기와 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <deque>는 비용 0 정점을 앞, 비용 1 정점을 뒤에 넣는 std::deque를 선언한다.
#include <deque>
// <iostream>은 표준 입력·출력과 고속 입출력 설정을 선언한다.
#include <iostream>
// <limits>는 도달하지 못한 거리를 표시할 int 한계를 질의하는 std::numeric_limits를 선언한다.
#include <limits>
// <string>은 각 격자 행의 숫자 문자를 소유하는 std::string을 선언한다.
#include <string>
// <vector>는 격자와 거리 배열을 동적으로 소유하는 std::vector를 선언한다.
#include <vector>

// 공용 알고리즘 문서: ../algorithm/zero-one-bfs.md
// 방향 번호 0..7은 북쪽에서 시작해 시계 방향이다. array 집합체는 동적 할당 없이 8개 int를 값으로 소유한다.
constexpr std::array<int, 8> row_delta{-1, -1, 0, 1, 1, 1, 0, -1};
constexpr std::array<int, 8> column_delta{0, 1, 1, 1, 0, -1, -1, -1};

// grid는 호출자가 소유한 격자를 const 참조로 빌린다. 네 좌표는 0부터 시작하는 int 값 복사다.
[[nodiscard]] int minimum_energy(const std::vector<std::string>& grid,
                                 int start_row,
                                 int start_column,
                                 int destination_row,
                                 int destination_column) {
    // vector::size() const noexcept는 인자 없이 행 수 size_type을 반환하고 grid를 바꾸지 않는 O(1) 관찰이다.
    // 결과를 int로 명시 변환한다. 문제 제약 1,000 이하여서 표현 가능하며 반환값은 rows에 저장한다.
    const int rows{static_cast<int>(grid.size())};
    // string::size() const noexcept는 첫 행 문자 수 size_type을 O(1)에 반환하고 문자열·버퍼 수명을 유지한다.
    // 입력 계약상 grid는 비어 있지 않으므로 grid[0] 접근이 유효하며 결과를 columns에 저장한다.
    const int columns{static_cast<int>(grid[0].size())};
    // numeric_limits<int>::max()는 상태와 인자 없이 int 최댓값을 반환한다. 4로 나눠 +1 완화가 넘치지 않게 한다.
    constexpr int infinity{std::numeric_limits<int>::max() / 4};

    // 위치를 행 우선 1차원 인덱스로 바꾼다. 모든 입력은 범위 안이라는 문제 계약을 따른다.
    const auto index_of = [columns](int row, int column) {
        return row * columns + column;
    };

    const int vertex_count{rows * columns};
    const int start{index_of(start_row, start_column)}; // 람다 호출은 두 int를 값 복사하고 시작 인덱스를 반환한다.
    const int destination{index_of(destination_row, destination_column)};
    // vector<int>(count,value)는 vertex_count개의 infinity 복사 원소를 연속 저장소에 소유한다.
    // O(RC) 시간·공간과 한 번의 할당이 가능하고 bad_alloc을 던질 수 있다. 이후 크기를 바꾸지 않아 참조가 안정적이다.
    std::vector<int> distance(static_cast<std::size_t>(vertex_count), infinity);
    // deque<int> 기본 생성자는 빈 양끝 큐를 만든다. 블록 할당 세부와 반복자 안정성은 구현·연산 계약을 따른다.
    std::deque<int> frontier{};

    // vector::operator[](size_type)은 범위 검사 없이 int&를 O(1)에 반환한다. start 범위 불변식이 안전을 보장한다.
    distance[static_cast<std::size_t>(start)] = 0;
    // deque::push_front(const int&)는 start lvalue 값을 복사해 앞에 넣고 void를 반환한다.
    // 호출 전 frontier는 비어 있고 성공 뒤 size가 1이다. O(1), 할당 실패가 가능하며 기존 반복자는 무효화될 수 있다.
    frontier.push_front(start);

    // 불변식: deque 앞에는 현재 발견된 최소 거리 후보가 있고, 0 간선 완화는 앞, 1 간선 완화는 뒤로 간다.
    // deque::empty() const noexcept는 인자·상태 변화 없이 bool을 O(1)에 반환해 반복 조건에 즉시 사용한다.
    while (!frontier.empty()) {
        // deque::front()은 비어 있지 않은 수신 객체의 첫 int&를 O(1)에 반환한다. 그 값을 current에 복사한다.
        // 반환 참조는 이어지는 pop_front에서 무효화되므로 참조 자체를 보관하지 않는다.
        const int current{frontier.front()};
        // deque::pop_front()는 첫 int를 파괴해 size를 1 줄이고 void를 반환한다. 비어 있지 않다는 전제를 만족한다.
        // O(1)이며 제거 원소의 참조는 무효가 되고 값을 반환하지 않으므로 앞에서 먼저 복사했다.
        frontier.pop_front();

        const int row{current / columns};    // 정수 /는 1차원 인덱스의 행 몫을 계산한다.
        const int column{current % columns}; // %는 같은 인덱스의 열 나머지를 계산한다.
        // 현재 칸 문자는 '0'..'7'이다. 문자 코드 차로 해류 방향 int를 얻는다.
        const int current_direction{grid[static_cast<std::size_t>(row)]
                                        [static_cast<std::size_t>(column)] -
                                    '0'};

        // 여덟 이웃을 모두 살펴 암시적 간선을 그때 계산하므로 별도 인접 리스트를 만들지 않는다.
        for (int direction{}; direction < 8; ++direction) {
            // array::operator[](size_type)은 검사 없이 const int&를 반환한다. direction 0..7 불변식이 유효성을 보장한다.
            const int next_row{row + row_delta[static_cast<std::size_t>(direction)]};
            const int next_column{
                column + column_delta[static_cast<std::size_t>(direction)]};
            // ||는 왼쪽부터 단락 평가한다. 격자 밖 정점은 만들지 않고 다음 방향으로 건너뛴다.
            if (next_row < 0 || next_row >= rows || next_column < 0 ||
                next_column >= columns) {
                continue;
            }

            const int next{index_of(next_row, next_column)};
            // 해류와 같은 방향이면 비용 0, 아니면 비용 1이다. == 결과 bool이 조건 연산자의 분기를 고른다.
            const int edge_cost{direction == current_direction ? 0 : 1};
            const int candidate{distance[static_cast<std::size_t>(current)] + edge_cost};
            // 더 싼 경로가 아니면 거리와 deque를 모두 유지한다.
            if (candidate >= distance[static_cast<std::size_t>(next)]) {
                continue;
            }

            // 완화가 성공하면 distance[next]는 시작점에서 알려진 더 작은 상한 candidate로 바뀐다.
            distance[static_cast<std::size_t>(next)] = candidate;
            if (edge_cost == 0) {
                // push_front(const int&)는 0비용 도착 정점을 앞에 복사해 같은 거리 층을 먼저 처리한다.
                // void 반환은 버리고 O(1), 할당 실패 가능성이 있으며 저장된 int 소유권은 deque에 있다.
                frontier.push_front(next);
            } else {
                // push_back(const int&)은 1비용 도착 정점을 뒤에 복사해 현재 거리 층 뒤로 미룬다.
                // void 반환은 버리고 O(1)이며 성공 뒤 size가 1 늘고 next 지역 변수는 그대로다.
                frontier.push_back(next);
            }
        }
    }

    // 모든 칸은 여덟 방향 이동으로 연결되므로 유한한 최소 에너지가 저장된다. int 값을 복사 반환한다.
    return distance[static_cast<std::size_t>(destination)];
}

int main() {
    // ios::sync_with_stdio(false)는 bool false를 받아 C/C++ 스트림 동기화를 끄고 이전 bool은 버린다.
    // 호출 뒤 전역 스트림 설정이 바뀌며 C stdio와 임의 순서로 섞지 않는다. 스레드 동기화는 제공하지 않는다.
    std::ios::sync_with_stdio(false);
    // cin.tie(nullptr)는 널 ostream*을 받아 입력 전 자동 flush 연결을 해제하고 이전 포인터를 반환하지만 버린다.
    // cin/cout 객체의 소유권과 수명은 유지되고 대화형 문제가 아니므로 수동 프롬프트 flush가 필요 없다.
    std::cin.tie(nullptr);

    int rows{};
    int columns{};
    // operator>>(int&) 두 호출은 각 lvalue를 갱신하고 같은 istream&를 연쇄 반환한다.
    // 첫 입력 실패면 상태 비트를 남기며 bool 문맥에서 false가 되어 계산 없이 종료한다.
    if (!(std::cin >> rows >> columns)) {
        return 0;
    }

    // vector<string>(count)는 rows개의 빈 string을 값 초기화한다. O(R), 문자열 문자 버퍼는 이어지는 입력이 소유한다.
    std::vector<std::string> grid(static_cast<std::size_t>(rows));
    for (std::string& line : grid) {
        // operator>>(string&)은 공백 전까지 문자를 line에 저장하고 입력 위치·상태를 갱신한다.
        // 반환 istream&는 버린다. 문자열 확장 시 할당과 기존 관찰자 무효화가 가능하며 입력 계약상 길이는 columns다.
        std::cin >> line;
    }

    int trip_count{};
    std::cin >> trip_count; // 추출은 trip_count lvalue를 갱신하고 최종 istream&를 버린다.
    for (int trip{}; trip < trip_count; ++trip) {
        int start_row{};
        int start_column{};
        int destination_row{};
        int destination_column{};
        // 네 추출 호출은 각 int lvalue에 1기반 좌표를 저장하고 같은 스트림 참조를 다음 호출로 넘긴다.
        std::cin >> start_row >> start_column >> destination_row >> destination_column;
        // --는 입력 좌표를 제자리에서 0기반으로 바꾼다. 문제 계약상 원래 1 이상이므로 음수가 되지 않는다.
        --start_row;
        --start_column;
        --destination_row;
        --destination_column;

        // minimum_energy 호출은 grid를 const 참조로 빌리고 네 int를 값 복사한다. grid 상태·소유권은 유지된다.
        // 반환 int를 즉시 operator<<의 값 인자로 복사해 출력하고 '\n'을 이어 쓴다. ostream& 최종 반환은 버린다.
        std::cout << minimum_energy(
                         grid, start_row, start_column, destination_row, destination_column)
                  << '\n';
    }

    // 질문마다 각 정점·최대 8간선을 상수 번 보므로 전체 O(QRC), 거리와 deque는 O(RC) 공간이다.
    return 0;
}
