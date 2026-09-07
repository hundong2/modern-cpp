/*
문제 요약 — CSES 1690 Hamiltonian Flights
출처: https://cses.fi/problemset/task/1690/

단방향 항공편이 있는 n개 도시에서 1번 도시를 출발해 n번 도시에 도착하려 한다.
모든 도시를 정확히 한 번씩 방문하는 서로 다른 경로 수를 1,000,000,007로 나눈 값을 구한다.
공식 설명은 중복 연결의 식별 방식을 따로 명시하지 않는다. 이 구현은 입력의 각 간선 occurrence를
별개 항공편 선택으로 보며, 같은 방향 연결이 여러 번 나오면 점화식에서 각각 한 번 더한다.

입력
- 첫 줄: 도시 수 n, 항공편 수 m
- 다음 m줄: 단방향 항공편 a -> b

출력
- 조건을 만족하는 경로 수를 1,000,000,007로 나눈 정수 하나

공식 제약
- 2 <= n <= 20
- 1 <= m <= n^2
- 1 <= a,b <= n
- 시간 제한 1초, 메모리 제한 512MB

예제
입력:
4 6
1 2
1 3
2 3
3 2
2 4
3 4

출력:
2

핵심은 DP(mask, v)를 “1번에서 시작해 mask의 도시를 정확히 한 번씩 방문하고 v에서 끝나는
경로 수”로 잡는 부분집합 DP다. 마지막 항공편 u -> v를 떼면 이전 상태가 유일하게 정해진다.
*/

// <cstddef>는 컨테이너 크기와 평탄화 인덱스에 쓰는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력 std::cin, 출력 std::cout과 스트림 연산자를 선언한다.
#include <iostream>
// <vector>는 동적 길이 인접 목록과 DP 저장소를 소유하는 std::vector를 선언한다.
#include <vector>

// using은 새 타입을 만들지 않고 기존 타입에 문제 의미를 붙인다. unsigned long long은 표준상 적어도
// 64 value bits를 제공하므로 공식 n<=20의 shift count와 2^20 상태를 구현의 size_t 폭과 무관하게 담는다.
using Mask = unsigned long long;

constexpr int kMod{1'000'000'007};

// 이 제출 코드는 상태를 숨길 객체가 필요 없어 자유 함수와 지역 변수만 쓴다. 일반적으로 struct는 기본
// public, class는 기본 private이며 접근 지정자는 이름 접근을 통제한다. 생성자는 반환형이 없고 멤버 초기화
// 목록은 본문 전 멤버를 직접 구성한다. 한 인자 생성자의 explicit은 의도치 않은 암시 변환을 막는다.
// std::vector<int>의 int는 원소 타입 템플릿 인자이고 vector<vector<int>>는 각 도시의 vector를 소유한다.

// 2차원 논리 좌표 (mask, city)를 연속 1차원 vector 위치로 바꾼다.
// 모든 매개변수는 기본 타입 값 복사이고 반환형 size_t도 독립된 prvalue다. const 지역값은 이후 바뀌지 않는다.
[[nodiscard]] std::size_t flat_index(Mask mask, int city, int city_count) noexcept {
    return static_cast<std::size_t>(mask) * static_cast<std::size_t>(city_count)
           + static_cast<std::size_t>(city);
}

int main() {
    // [호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 정적 함수라 수신 객체가 없고 아직 표준 스트림 I/O를 시작하지 않은 상태다.
    // (2) static bool sync_with_stdio(bool sync = true)에서 bool 인자 하나인 overload를 선택한다.
    // (3) 인자는 bool prvalue false이고 소유권 의미가 없으며 허용되는 두 값 중 하나다.
    // (4) 이전 동기화 설정 bool을 반환하지만 사용하지 않는다.
    // (5) C stdio와 C++ 스트림 동기화를 끄되 스트림 객체 수명과 소유권은 유지한다.
    // (6) 첫 I/O 뒤 효과는 구현별이라 지금 호출한다. 복잡도·할당·예외는 별도 상한이 없고 C/C++ I/O
    //     혼용 순서나 애플리케이션 레코드 단위 스레드 안전을 보장하지 않는다.
    // 대표 문서: ../standard-library/io-parsing-and-utilities.md
    std::ios::sync_with_stdio(false);

    // [호출 계약: std::cin.tie(nullptr)]
    // (1) 수신자는 정상 상태의 std::cin이며 정확한 타입은 std::istream lvalue다.
    // (2) std::ostream* tie(std::ostream*) setter overload가 선택된다.
    // (3) 인자는 null pointer prvalue이고 비소유이며 null은 자동 flush 연결 해제를 뜻하는 허용값이다.
    // (4) 이전 tied ostream의 비소유 포인터를 반환하지만 사용하지 않는다.
    // (5) 입력 전 cout 자동 flush 연결만 해제하고 양쪽 스트림 상태·수명·소유권은 유지한다.
    // (6) O(1)·무할당·비무효화다. 오류 보고나 외부 동기화를 추가하지 않으며 대화형 출력이 없는 입력이다.
    std::cin.tie(nullptr);

    int city_count{};   // int{}는 0이며 공식 최댓값 20을 안전하게 담는다.
    int flight_count{}; // 공식 최댓값 400을 담는다.

    // [호출 계약: std::istream int 추출 연쇄]
    // (1) 수신자는 입력 가능한 std::cin/std::istream lvalue이고 두 int 목적 객체가 살아 있다.
    // (2) std::istream& operator>>(int&) 멤버 overload를 두 번 선택한다.
    // (3) 인자는 수정 가능한 city_count와 flight_count lvalue 참조이고 공식 범위 정수를 받아 소유하지 않는다.
    // (4) 첫 반환 std::istream&는 다음 추출 수신자로 쓰고 마지막 반환은 버린다.
    // (5) 성공하면 두 값과 스트림 읽기 위치가 갱신되고 다른 수명·소유권은 변하지 않는다.
    // (6) 비용은 소비 문자·locale·버퍼에 의존한다. EOF/형식/범위 실패는 상태 비트를 세우며 설정에 따라
    //     ios_base::failure가 가능하다. 별도 참조 무효화나 논리적 다중 스레드 보장은 없다.
    std::cin >> city_count >> flight_count;

    // [호출 계약: std::vector<std::vector<int>> count 생성자]
    // (1) 아직 존재하지 않는 incoming 목적 객체를 만들며 정확한 타입은 vector<vector<int>>다.
    // (2) explicit vector(size_type count, const Allocator& = Allocator())가 선택된다.
    // (3) 인자는 city_count를 바꾼 size_t prvalue [2,20]이고 각 원소는 빈 vector<int>로 값 초기화된다.
    // (4) 생성자에는 반환값이 없다.
    // (5) 바깥 vector가 n개의 안쪽 vector와 저장소를 소유하고 각 안쪽 size는 0이다.
    // (6) O(n) 원소 구성과 바깥 저장소 할당이 필요하며 length_error/bad_alloc 가능성이 있다. 실패하면
    //     완성 객체가 없고 기존 관찰자 무효화도 없다. 원소 수명은 바깥 vector 수명에 묶이고 단일 스레드다.
    // 대표 문서: ../standard-library/containers-and-views.md
    std::vector<std::vector<int>> incoming(static_cast<std::size_t>(city_count));

    for (int edge{}; edge < flight_count; ++edge) {
        int from{};
        int to{};
        // 앞의 int 추출 계약과 같다. 같은 stream&를 연쇄 사용하며 성공 시 두 1-based 도시 번호를 갱신한다.
        std::cin >> from >> to;
        --from;
        --to;

        // [호출 계약: vector::operator[]와 vector<int>::push_back(const int&)]
        // (1) 바깥 수신 incoming은 size=n이고 해당 안쪽 vector는 유효하다. to는 입력 검증 뒤 [0,n)다.
        // (2) vector<vector<int>>::operator[](size_type)는 vector<int>&, push_back(const int&)는 void를 반환한다.
        // (3) [] 인자는 to의 size_t prvalue이고 push 인자는 from int lvalue다. 값을 복사하며 입력 소유권은 없다.
        // (4) [] 참조는 즉시 push 수신자로 쓰고 push의 void에는 사용할 반환값이 없다.
        // (5) to 도시의 predecessor 목록 size가 1 늘고 from 사본을 소유한다. 바깥 size는 그대로다.
        // (6) []는 O(1)·무할당이고 범위 밖은 UB이나 제약이 배제한다. push는 분할 상환 O(1), 할당 예외가
        //     가능하고 안쪽 재할당 시 그 안쪽 원소 관찰자만 모두 무효다. 아직 저장한 관찰자가 없고 단일 스레드다.
        incoming[static_cast<std::size_t>(to)].push_back(from);
    }

    // city_count<=20이라 shift count는 unsigned long long의 최소 64 value-bit 폭보다 작고,
    // 모든 부분집합 수를 정확히 만든다.
    const Mask state_count{Mask{1} << static_cast<unsigned int>(city_count)};
    const Mask full_mask{state_count - Mask{1}};
    const Mask start_bit{Mask{1}};
    const Mask destination_bit{Mask{1} << static_cast<unsigned int>(city_count - 1)};
    const std::size_t city_count_size{static_cast<std::size_t>(city_count)};
    const std::size_t cell_count{state_count * city_count_size};

    // [호출 계약: std::vector<int> fill 생성자]
    // (1) 아직 없는 dp 목적 객체를 만들며 템플릿 인자 int, 기본 allocator<int>다.
    // (2) vector(size_type count, const int& value, const Allocator& = Allocator()) overload가 선택된다.
    // (3) 첫 인자 cell_count는 최대 20*2^20인 size_t prvalue, 둘째 0은 const int&에 붙는 prvalue다.
    // (4) 생성자는 반환값이 없다.
    // (5) dp가 정확히 cell_count개의 0을 연속 저장소에 소유한다.
    // (6) O(n*2^n) 값 구성·공간과 한 저장소 할당이 필요하다. length_error/bad_alloc 가능, 실패 시 완성
    //     객체가 없다. 새 컨테이너라 무효화할 기존 관찰자는 없고 수명은 main 블록 끝까지다.
    std::vector<int> dp(cell_count, 0);

    // 공용 알고리즘 문서: ../algorithm/bitmask-dp-hamiltonian-path.md
    // [호출 계약: non-const std::vector<int>::operator[] 기저 쓰기]
    // (1) 수신자는 size=cell_count인 유효한 non-const dp lvalue다.
    // (2) vector<int>::operator[](size_type)의 non-const overload가 선택된다.
    // (3) 유일한 인자는 flat_index가 만든 size_t prvalue이고 start_bit=1, city=0이라 [0,cell_count)다.
    // (4) 해당 원소의 int&를 반환하며 대입의 수정 가능한 왼쪽 피연산자로 사용한다.
    // (5) 기저 칸만 1이 되고 dp의 size/capacity/소유권/수명은 그대로라 기존 관찰자를 무효화하지 않는다.
    // (6) O(1)·무할당이며 유효 인덱스에서는 지정된 예외가 없다. 표준 선언 자체는 noexcept가 아니고
    //     C++20에서 범위 밖 접근은 UB다. 현재 스레드만 dp에 접근해 데이터 경쟁은 없다.
    // 기저: 도시 1만 방문했고 도시 1에서 끝나는 빈 경로는 한 가지다.
    dp[flat_index(start_bit, 0, city_count)] = 1;

    // 불변식: 처리 대상 상태에서는 시작점 비트가 있고, 도착점 비트는 full 상태에서만 허용한다.
    // dp[mask][v]는 mask의 각 도시를 정확히 한 번 방문하고 v에서 끝나는 경로 수의 나머지다.
    for (Mask mask{start_bit}; mask < state_count; ++mask) {
        if ((mask & start_bit) == Mask{0}) {
            continue;
        }
        // 도착 도시를 일찍 방문한 경로는 이후 도착 도시를 다시 쓸 수 없으므로 정답에 기여하지 않는다.
        if ((mask & destination_bit) != Mask{0} && mask != full_mask) {
            continue;
        }

        for (int city{1}; city < city_count; ++city) {
            const Mask city_bit{Mask{1} << static_cast<unsigned int>(city)};
            if ((mask & city_bit) == Mask{0}) {
                continue;
            }
            if (mask == full_mask && city != city_count - 1) {
                continue;
            }

            const Mask previous_mask{mask ^ city_bit};
            int ways{};

            // [호출 계약: non-const 바깥 vector::operator[]와 const vector::size]
            // (1) incoming은 size=n인 유효한 vector<vector<int>> lvalue이고 city는 [1,n)다. 반환 안쪽 vector도
            //     수명 유효하며 입력에서 저장한 predecessor들을 소유한다.
            // (2) const가 아닌 바깥 operator[](size_type)은 vector<int>&를, 안쪽 size() const noexcept는
            //     size_type을 반환한다. 여기서는 참조를 const vector<int>&에 바인딩해 읽기만 한다.
            // (3) [] 인자는 city의 size_t prvalue이고 size에는 데이터 인자가 없다. 소유권 이전은 없다.
            // (4) [] 반환 참조는 predecessors에 바인딩되고 size 반환값은 predecessor_count에 저장해 사용한다.
            // (5) 어떤 vector의 size/capacity/값/수명도 바뀌지 않으며 참조는 incoming 수명 동안 유효하다.
            // (6) 모두 O(1)·무할당·비무효화, size는 noexcept다. [] 범위 밖은 UB이나 루프가 배제하고,
            //     반복 중 수정·외부 동시 접근이 없어 재할당과 데이터 경쟁도 없다.
            const std::vector<int>& predecessors{incoming[static_cast<std::size_t>(city)]};
            const std::size_t predecessor_count{predecessors.size()};

            for (std::size_t index{}; index < predecessor_count; ++index) {
                // [호출 계약: const std::vector<int>::operator[] predecessor 읽기]
                // (1) 수신자는 입력 뒤 변하지 않는 const predecessors lvalue이고 size=predecessor_count다.
                // (2) const overload operator[](size_type) const가 const int&를 반환한다.
                // (3) 유일한 index 인자는 size_t lvalue를 값으로 사용하며 루프 조건으로 [0,size)다.
                // (4) 반환 const int&의 값을 predecessor int에 복사한다. 소유권 이전은 없다.
                // (5) 컨테이너와 원소는 변하지 않고 참조는 즉시 사용하므로 수명·관찰자 변화가 없다.
                // (6) O(1)·무할당이고 유효 인덱스에서 지정 예외가 없다. 선언은 noexcept가 아니며 범위 밖은
                //     C++20에서 UB지만 조건이 배제한다. 수정과 외부 동시 접근이 없어 데이터 경쟁도 없다.
                const int predecessor{predecessors[index]};
                const Mask predecessor_bit{Mask{1} << static_cast<unsigned int>(predecessor)};
                if ((previous_mask & predecessor_bit) == Mask{0}) {
                    continue;
                }

                // [호출 계약: non-const std::vector<int>::operator[] DP 이전 상태 읽기]
                // (1) 수신자는 size=cell_count인 non-const dp lvalue다.
                // (2) non-const operator[](size_type)이 int&를 반환하고 여기서는 그 값을 읽는다.
                // (3) 유일한 인자는 previous_mask<n상태 수, predecessor<n으로 만든 유효 size_t prvalue다.
                // (4) 반환 int&의 값을 ways의 복합 대입 오른쪽 피연산자로 사용하며 소유권 이동은 없다.
                // (5) 이 호출은 dp를 바꾸지 않고 size/capacity/관찰자도 그대로다.
                // (6) 기저 쓰기와 같이 O(1)·무할당, 유효 범위에서 지정 예외 없음, 선언은 non-noexcept다.
                //     범위 밖 C++20 UB는 계산으로 배제하고 단일 스레드라 데이터 경쟁이 없다.
                // ways와 addend가 각각 [0,kMod)이므로 덧셈 최댓값 2*kMod-2는 int 범위 안이다.
                ways += dp[flat_index(previous_mask, predecessor, city_count)];
                if (ways >= kMod) {
                    ways -= kMod;
                }
            }

            // [호출 계약: non-const std::vector<int>::operator[] DP 현재 상태 쓰기]
            // (1)~(2) 수신자와 overload는 위 dp 접근과 같다.
            // (3) mask<state_count, city<n으로 만든 유일한 size_t 인자는 [0,cell_count)다.
            // (4) 반환 int&를 대입 왼쪽 피연산자로 사용한다.
            // (5) 현재 칸이 [0,kMod) ways로 바뀌고 구조·소유권·수명·관찰자는 변하지 않는다.
            // (6) O(1)·무할당, 유효 범위에서 지정 예외 없음, 선언은 non-noexcept다. 범위 밖 C++20 UB와
            //     데이터 경쟁은 위 범위 증명과 단일 스레드 실행으로 배제한다.
            dp[flat_index(mask, city, city_count)] = ways;
        }
    }

    // [호출 계약: vector::operator[]와 std::ostream 삽입 연쇄]
    // (1) dp는 완성된 size=cell_count vector이고 cout은 출력 가능한 std::ostream lvalue다.
    // (2) const가 아닌 vector<int>::operator[](size_type), ostream의 int 삽입과
    //     operator<<(std::ostream&, char)가 선택된다.
    // (3) [] 인자는 full/n-1의 유효한 size_t 위치, 다음 인자는 그 int lvalue와 개행 char prvalue다.
    //     모두 읽기 용도이고 소유권 이전은 없다.
    // (4) []는 int&, 각 삽입은 같은 ostream&를 반환한다. 첫 ostream&는 연쇄하고 마지막은 버린다.
    // (5) 정답과 개행이 버퍼에 추가되고 dp와 수명·관찰자는 변하지 않는다.
    // (6) []는 O(1)·무할당이고 범위 밖 UB는 계산으로 배제한다. 출력 비용은 자릿수·locale·버퍼/장치에
    //     의존하고 실패는 상태 비트/설정 예외로 나타난다. 컨테이너 무효화·레코드 원자성 보장은 없다.
    std::cout << dp[flat_index(full_mask, city_count - 1, city_count)] << '\n';

    // 대회 최적화 관점: 시작 없는 mask와 도착을 일찍 포함한 mask를 건너뛰고, 들어오는 간선만 순회해
    // O(2^n(n+m)) 시간과 O(n*2^n) 공간을 쓴다. 20*2^20 int는 약 80MiB라 512MB 안이다.
    // 기계 실행은 연속 dp load/store, bit AND/shift, 정수 비교·조건 분기가 중심이다. 실제 캐시 적중,
    // 벡터화, 분기 배치와 명령 수는 CPU·ABI·컴파일러·최적화 옵션에 따라 달라 특정 어셈블리로 단정하지 않는다.

    // 이름 있는 incoming/dp/predecessors 식은 lvalue, 정수 리터럴과 flat_index 반환은 prvalue다. 이 풀이는
    // 소유 vector를 이동하는 식이 없어 xvalue나 RVO가 핵심 경로에 없다. 반환형 void가 아닌 main의 int 0은
    // 운영체제에 정상 종료를 알리는 값이며 지역 vector는 역순으로 파괴되어 저장소를 자동 해제한다.
    return 0;
}
