/*
CSES 1140 — Projects
출처: https://cses.fi/problemset/task/1140/

문제 요약:
n개의 프로젝트마다 시작일, 종료일, 보상이 주어진다. 시작일과 종료일을 모두 포함하는 기간 동안
그 프로젝트에 참여하며, 같은 날에는 두 프로젝트에 참여할 수 없다. 서로 겹치지 않는 프로젝트들을
골라 받을 수 있는 보상 합의 최댓값을 구한다.

입력:
첫 줄에 프로젝트 수 n이 주어진다. 다음 n줄마다 시작일 a_i, 종료일 b_i, 보상 p_i가 주어진다.

출력:
조건을 만족하도록 프로젝트를 골랐을 때 얻는 최대 보상 하나를 출력한다.

제약:
1 <= n <= 200000, 1 <= a_i <= b_i <= 10^9, 1 <= p_i <= 10^9.
시간 제한은 1초, 메모리 제한은 512MB이다.

예제 입력:
4
2 4 4
3 6 6
6 8 2
5 7 3

예제 출력:
7
*/

#include <algorithm> // std::ranges::sort: 프로젝트를 종료일 순으로 정렬한다.
#include <cstddef>   // std::size_t: 컨테이너 크기와 0부터 시작하는 첨자를 표현한다.
#include <iostream>  // std::cin/std::cout과 >>/<< 스트림 연산을 제공한다.
#include <vector>    // std::vector: 프로젝트, 종료일, DP 표를 연속 동적 배열로 소유한다.

// using은 새 정수 타입을 만드는 문법이 아니라 기존 long long에 문맥 있는 별칭을 붙인다.
// 공식 최댓값 n * p_i = 2 * 10^14이므로 32비트 int 대신 64비트 이상인 long long을 쓴다.
using Day = long long;
using Money = long long;

// struct의 멤버는 기본적으로 public이고 class의 멤버는 기본적으로 private이다.
// 세 필드의 {} 멤버 초기화는 기본 생성될 때 0으로 만들며 축소 변환도 막는다.
// 이 단순 값 묶음에는 생성자가 필요 없다. 단일 인자 class 생성자를 추가한다면 explicit으로
// 뜻밖의 암시 변환을 막고, 멤버 초기화 목록으로 본문보다 먼저 멤버를 구성하는 편이 좋다.
struct Project {
    Day start{};
    Day finish{};
    Money reward{};
};

/*
초보자 문법 지도:
- 함수 반환형 std::size_t는 조건에 맞는 "앞쪽 프로젝트 개수"를 돌려준다. 매개변수의
  const std::vector<Day>&는 vector를 복사하거나 소유권을 옮기지 않고 읽기만 하는 lvalue 참조다.
- 정상적인 참조는 null일 수 없고 다른 객체로 다시 바인딩되지 않는다. 포인터는 주소를 보관하며
  null일 수 있는데, 아래 nullptr는 cin과 연결할 출력 스트림이 없다는 포인터 값이다.
- const 지역 변수는 초기화 뒤 재대입할 수 없다. if는 조건 분기, while/for는 조건·횟수 반복이며
  중괄호는 지역 객체의 수명과 이름의 유효 범위를 정한다.
- std::vector<Day>에서 Day는 템플릿 타입 인자다. vector 객체가 원소 저장소를 소유한다.
*/

// 정렬된 앞부분 [0, prefix_size)에서 finish < start_day인 원소 수를 구한다.
// 종료일과 시작일이 같은 날이면 두 프로젝트가 그날 겹치므로 <=가 아니라 엄격한 <가 핵심이다.
[[nodiscard]] std::size_t compatible_prefix_size(
    const std::vector<Day>& sorted_finish_days,
    const std::size_t prefix_size,
    const Day start_day) {
    std::size_t low{};
    std::size_t high{prefix_size};

    // 이분 탐색 불변식:
    // [0, low)는 finish < start_day임이 확정되었고, [high, prefix_size)는 finish >= start_day임이
    // 확정되었다. 아직 모르는 [low, high)를 절반씩 줄이면 low가 첫 finish >= start_day 위치,
    // 곧 호환 가능한 프로젝트의 개수가 된다.
    while (low < high) {
        const std::size_t middle{low + (high - low) / 2U};
        if (
            // [첫 호출 계약: const std::vector<Day>::operator[]]
            // (1) 수신 sorted_finish_days의 정확한 타입은 const std::vector<long long> lvalue이고,
            //     적어도 prefix_size개의 정렬된 원소를 소유한다. middle은 [low,high) 안이다.
            // (2) const_reference std::vector<Day>::operator[](size_type pos) const 오버로드를 선택한다.
            // (3) middle은 size_t lvalue에서 복사된 유효 첨자 값이며 소유권 이전이나 수명 연장은 없다.
            // (4) const Day&를 반환해 start_day와 비교하고, 그 참조를 호출 뒤 저장하지 않는다.
            // (5) vector, 원소, 인자는 변하지 않으며 탐색 경계만 이후 분기에서 바뀐다.
            // (6) O(1), 무할당·무효화다. 표준 선언은 noexcept를 보장하지 않지만 유효 첨자에서 지정된
            //     예외 경로는 없다. 범위 밖 첨자는 미정의 동작이므로 prefix_size <= vector 크기가
            //     전제이며, 동시 구조 변경 없이 단일 스레드에서 읽는다.
            sorted_finish_days[middle] < start_day) {
            low = middle + 1U;
        } else {
            high = middle;
        }
    }
    return low;
}

// 대표 알고리즘 문서: ../algorithm/weighted-interval-scheduling.md
int main() {
    // [첫 호출 계약: std::ios_base::sync_with_stdio]
    // (1) 수신 객체가 없는 정적 함수이며, 첫 입출력 전 C/C++ 표준 스트림 동기화 상태를 설정한다.
    // (2) static bool std::ios_base::sync_with_stdio(bool sync = true)의 bool 인자 호출을 선택한다.
    // (3) false는 bool prvalue이고 버퍼 소유권 없이 C stdio와의 동기화 해제를 요청한다.
    // (4) 이전 동기화 상태 bool을 반환하지만 이 코드는 의도적으로 사용하지 않는다.
    // (5) 이후 C++ 표준 스트림은 독립 버퍼링될 수 있고 스트림 객체 수명은 그대로 유지된다.
    // (6) 표준은 일반 복잡도·할당·예외 상한을 정하지 않는다. 첫 I/O 전에 호출하고 C/C++ I/O를
    //     섞지 않는 것이 전제다. 설정 중 다른 스레드가 표준 스트림을 쓰지 않는 제출 프로그램이다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::basic_ios<char>::tie]
    // (1) 수신 std::cin은 프로그램 수명 동안 유효한 std::istream lvalue이고 현재 출력 스트림과
    //     연결되어 있을 수 있다.
    // (2) std::basic_ostream<char>* tie(std::basic_ostream<char>* tiestr) 비-const 오버로드를 선택한다.
    // (3) nullptr는 새 연결 대상이 없다는 포인터 prvalue이며 어떤 스트림 소유권도 이전하지 않는다.
    // (4) 이전 연결 스트림의 비소유 포인터를 반환하지만 여기서는 버린다.
    // (5) std::cin의 자동 flush 연결만 해제되고 cin/cout과 각 버퍼의 수명은 유지된다.
    // (6) 표준은 일반 복잡도·할당 상한을 정하지 않는다. 지정된 오류값·iostate 보고는 없고 선언 자체는
    //     noexcept가 아니다. tie 순환은 미정의 동작 위험이 있지만 nullptr는 순환을 만들지 않으며,
    //     다른 실행 흐름이 같은 스트림 연결을 쓰는 동안 동기화 없이 재설정하지 않는다.
    std::cin.tie(nullptr);

    int project_count_input{};
    // [첫 호출 계약: std::istream의 int 추출 operator>>]
    // (1) 수신 std::cin은 읽기 가능한 std::istream lvalue이고 project_count_input은 살아 있는 int다.
    // (2) std::basic_istream<char>::operator>>(int&) 정수 오버로드를 선택한다.
    // (3) 쓰기 가능한 int lvalue를 참조로 빌리며 버퍼나 변수 소유권은 이전하지 않는다.
    // (4) 같은 std::istream&를 반환하지만 이 단독 호출의 반환 참조는 사용하지 않는다.
    // (5) 성공하면 입력 위치와 project_count_input 값이 바뀌고, 실패하면 상태 비트가 설정된다.
    // (6) 표준은 일반 복잡도·할당 상한을 정하지 않는다. 공식 입력의 형식과 int 범위를 전제로 하며
    //     실패는 failbit/eofbit 또는 exception mask의 예외로 보고된다. 동시 읽기는 하지 않는다.
    std::cin >> project_count_input;

    // 공식 n은 양수이고 200000 이하라 size_t 변환에서 값이 보존된다. cast 결과는 prvalue다.
    const std::size_t project_count{static_cast<std::size_t>(project_count_input)};

    // [첫 호출 계약: std::vector<Project>(count)]
    // (1) 생성될 수신 객체 projects와 원소 저장소는 아직 없고 정확한 타입은 std::vector<Project>다.
    // (2) explicit vector(size_type count, const Allocator& = Allocator()) 생성자를 선택한다.
    // (3) project_count는 [1,200000]의 size_t lvalue 값을 복사해 전달하며 외부 저장소를 넘기지 않는다.
    // (4) 생성자는 반환값 없이 projects 객체를 완성한다.
    // (5) count개의 Project가 기본 삽입되고 각 멤버 {} 초기화로 0이 되며 vector가 저장소를 소유한다.
    // (6) 시간·공간 O(n), length_error/bad_alloc 가능, 실패 시 완성된 원소를 정리한다. 아직 외부
    //     관찰자가 없어 무효화 대상이 없고 원소 수명은 projects 수명에 묶이며 동시 접근도 없다.
    std::vector<Project> projects(project_count);

    for (std::size_t index{}; index < project_count; ++index) {
        // [첫 호출 계약: std::vector<Project>::operator[] 비-const]
        // (1) projects는 project_count개의 Project를 가진 유효한 vector lvalue이고 index는 범위 안이다.
        // (2) reference std::vector<Project>::operator[](size_type pos) 비-const 오버로드를 선택한다.
        // (3) index 값은 size_t로 복사되며 저장소 소유권은 이동하지 않는다.
        // (4) 해당 Project의 lvalue 참조를 반환해 project에 묶고 반복 끝까지만 사용한다.
        // (5) 호출 자체는 상태를 바꾸지 않지만 반환 참조를 통해 세 멤버를 곧 수정한다.
        // (6) O(1), 무할당이며 유효 첨자가 전제다. 표준 선언은 noexcept를 보장하지 않지만 유효
        //     첨자에서 지정된 예외 경로는 없다. 위반은 미정의 동작이다. 구조 변경이 없어 참조
        //     무효화는 없고 같은 원소를 다른 스레드가 동시에 접근하지 않는다.
        Project& project{projects[index]};

        // [첫 호출 계약: std::istream의 long long 추출 operator>>]
        // (1) std::cin은 읽기 가능하고 project의 세 정확한 타입 long long 멤버가 살아 있다.
        // (2) std::basic_istream<char>::operator>>(long long&)를 세 번 연쇄해 선택한다.
        // (3) start/finish/reward는 쓰기 가능한 long long lvalue 참조로 빌리며 소유권 이전이 없다.
        // (4) 앞 두 호출의 std::istream& 반환은 다음 호출의 수신자로 쓰고 마지막 반환은 버린다.
        // (5) 성공하면 세 값과 입력 위치가 바뀌며 projects의 크기·용량은 변하지 않는다.
        // (6) 일반 복잡도·할당 상한은 표준이 정하지 않는다. 공식 범위/형식을 전제로 하고 실패는
        //     상태 비트 또는 설정된 예외로 나타난다. 참조는 유지되며 같은 스트림을 동시 읽지 않는다.
        std::cin >> project.start >> project.finish >> project.reward;
    }

    // [첫 호출 계약: std::ranges::sort]
    // (1) 호출 객체 std::ranges::sort는 표준 CPO이고, 수신 범위 projects는 수정 가능한
    //     std::vector<Project> lvalue이며 모든 입력 Project를 소유한다.
    // (2) range 오버로드에서 R=vector<Project>&, Comp=Comparator가 되고 기본 identity projection을 쓴다.
    //     Comparator는 아래 캡처 없는 lambda의 고유 타입이다.
    // (3) projects는 forwarding reference에 lvalue로 빌려 주며 소유권을 넘기지 않는다. lambda prvalue는
    //     값으로 전달되고 두 const Project&를 받아 finish, start 순의 엄격 약순서를 만든다.
    // (4) std::vector<Project>::iterator, 즉 정렬된 범위의 끝을 반환하지만 여기서는 사용하지 않는다.
    // (5) 원소들이 이동·교환되어 종료일 순으로 재배치되며 크기·용량·저장소 소유권은 유지된다.
    // (6) O(n log n) 비교·projection이며 표준은 추가 메모리·할당 상한을 따로 보장하지 않는다.
    //     엄격 약순서가 전제다. iterator/참조의 주소 유효성은 유지되어도 그 위치의 값은 바뀔 수 있다.
    //     이 비교와 Project 이동은 noexcept지만 구현 내부 할당이 있다면 bad_alloc 가능성을 배제할 수
    //     없고, 예외가 나면 원소 순서는 미지정일 수 있다. 같은 vector의 동시 접근은 허용하지 않는다.
    std::ranges::sort(
        projects,
        [](const Project& left, const Project& right) noexcept {
            if (left.finish != right.finish) {
                return left.finish < right.finish;
            }
            return left.start < right.start;
        });

    // [첫 호출 계약: std::vector<Day>(count, value)]
    // (1) 생성될 수신 객체 finish_days와 best 저장소는 아직 없고 정확한 타입은 각각 std::vector<long long>이다.
    // (2) vector(size_type count, const value_type& value, const Allocator& = Allocator())를 선택한다.
    // (3) 첫 호출은 project_count와 long long으로 변환되는 0 prvalue, 둘째 호출은 count+1 prvalue와
    //     0을 전달한다. 값은 복사되고 외부 소유권은 없다.
    // (4) 생성자는 반환값 없이 각 vector 객체를 완성한다.
    // (5) 지정 개수의 독립적인 0 원소가 생기고 각 vector가 자체 저장소를 독점 소유한다.
    // (6) 각 호출은 시간·공간 O(count), length_error/bad_alloc 가능하며 실패 시 부분 구성을 정리한다.
    //     관찰자가 아직 없어 무효화 대상은 없고 원소 수명은 각 vector 수명에 묶이며 동시 접근은 없다.
    std::vector<Day> finish_days(project_count, 0);
    std::vector<Money> best(project_count + 1U, 0);

    // DP 불변식: 반복 시작 시 best[i]는 종료일 순 첫 i개 프로젝트만 사용한 최적 보상이다.
    // 또한 finish_days[0..i)는 바로 그 i개 프로젝트의 비감소 종료일을 담는다.
    for (std::size_t index{}; index < project_count; ++index) {
        // projects의 크기를 바꾸지 않았으므로 앞서 설명한 operator[] 계약과 같은 호출이다.
        const Project& current{projects[index]};

        // [첫 호출 계약: std::vector<Day>::operator[] 비-const]
        // (1) finish_days는 project_count개의 long long을 소유하고 index는 유효하다. best도 같은
        //     원소 타입의 vector이며 이후 사용하는 첨자는 [0,project_count] 안이다.
        // (2) reference std::vector<Day>::operator[](size_type pos) 비-const 오버로드를 선택한다.
        // (3) index 및 이후 compatible/index+1은 size_t 값으로 전달되고 소유권 이동은 없다.
        // (4) 해당 long long의 lvalue 참조를 반환해 대입하거나 값을 읽으며 참조를 보관하지 않는다.
        // (5) 첫 대입은 finish_days[index]만 바꾸고 vector 크기·용량·주소는 그대로다.
        // (6) O(1), 무할당이며 범위 안 첨자가 전제다. 표준 선언은 noexcept를 보장하지 않지만 유효
        //     첨자에서 지정된 예외 경로는 없다. 위반은 미정의 동작이다. 구조 변경이 없어 무효화가
        //     없고 동일 원소에 대한 동시 접근은 하지 않는다.
        finish_days[index] = current.finish;

        const std::size_t compatible{
            compatible_prefix_size(finish_days, index, current.start)};
        const Money take{best[compatible] + current.reward};
        const Money skip{best[index]};

        // 두 선택은 서로 배타적이고 모든 최적해는 current를 선택하거나 선택하지 않는다.
        best[index + 1U] = (take > skip) ? take : skip;
    }

    // [첫 호출 계약: ostream long long 및 char 삽입 operator<<]
    // (1) 수신 std::cout은 출력 가능한 std::ostream lvalue이고 best[project_count]는 유효한
    //     long long lvalue이며 '\n'은 char prvalue다.
    // (2) 멤버 basic_ostream<char>::operator<<(long long) 뒤 비멤버
    //     operator<<(basic_ostream<char>&, char)를 선택한다.
    // (3) 최대 보상 값과 개행 문자를 값으로 전달하며 vector/버퍼 소유권은 이전하지 않는다.
    // (4) 첫 ostream& 반환은 다음 삽입의 수신자로 쓰고 마지막 ostream&는 버린다.
    // (5) 출력 버퍼·위치·상태만 바뀌고 best 및 그 원소는 변하지 않는다.
    // (6) 일반 복잡도·할당 상한은 표준이 정하지 않는다. 실패는 상태 비트 또는 exception mask의
    //     예외로 나타난다. 참조 무효화는 없고 같은 스트림의 복합 출력을 동시 수행하지 않는다.
    std::cout << best[project_count] << '\n';

    /*
    정확성·우승 노하우:
    1. 종료일 정렬 뒤 current를 고르는 해와 함께 올 수 있는 이전 프로젝트는 정확히
       finish < current.start인 접두사에 있다. 날짜 양끝이 포함되므로 finish == start는 겹친다.
    2. 0-based P[i]까지 처리해 best[i+1]을 만들 때 최적해는 P[i]를 제외해 best[i]인 경우,
       또는 포함해 reward + best[compatible]인 경우 중 하나다. 두 경우가 모든 가능성을 분할한다.
    3. 위 DP 불변식과 i에 대한 귀납으로 best[n]은 전체 프로젝트의 최대 보상이다.
    4. 같은 종료일의 정렬 순서는 정답에 영향을 주지 않지만 tie-break를 두면 재현 가능한 순서가 된다.
       이분 탐색은 반드시 현재 index 이전 접두사만 보아 자기 자신을 선택하는 순환을 막는다.
    5. 정렬 O(n log n), n번의 이분 탐색 O(n log n), DP O(n)이므로 전체 시간 O(n log n),
       projects/finish_days/best가 차지하는 추가 공간은 O(n)이다.

    값 범주·복사·이동·수명·소유권:
    projects[index]와 vector::operator[] 결과는 lvalue이고 current는 그 lvalue에 묶인 const lvalue
    참조다. static_cast 결과, 산술 결과, lambda 식은 prvalue다. sort 내부에서는 Project 원소가
    이동·교환될 수 있지만 세 멤버는 값 타입이라 외부 자원을 공유하지 않는다. 소스에 명시적
    std::move가 없어 우리가 직접 만드는 xvalue 식은 없다. 세 vector가 저장소를 독점 소유하며
    지역 객체와 참조는 각 블록 끝에서 수명이 끝난다. 이 코드는 객체를 값으로 반환하지 않아 RVO가
    관찰되는 식이 없지만, 풀이 함수를 만들어 지역 vector를 값으로 반환하면 RVO/NRVO가 복사를
    생략할 수 있다.

    기계 실행 관점:
    이분 탐색은 연속 배열의 원소 load, 비교, 조건 분기를 반복하고 DP는 best 원소 load/store를 한다.
    ranges sort는 비교와 원소 이동을 반복한다. 사용자 정의 Project에는 virtual 함수가 없어 그 객체에
    대한 가상 간접 호출은 필요하지 않지만, iostream 내부의 streambuf 경로에는 구현에 따라 가상 호출이
    나타날 수 있다. 실제 명령, 분기 예측, 인라이닝, 메모리 접근 순서와 할당은 CPU·ABI·표준 라이브러리
    구현·컴파일러·최적화 옵션에 따라 달라지므로 특정 어셈블리 형태로 단정하지 않는다.
    */

    return 0;
}
