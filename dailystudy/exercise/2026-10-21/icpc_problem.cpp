/*
문제 ID/제목: CSES 2129 - Task Assignment
출처: https://cses.fi/problemset/task/2129/

문제 요약:
직원 n명과 작업 n개가 있고, 직원 i가 작업 j를 맡을 때 비용 c[i][j]가 든다.
각 직원에게 서로 다른 작업을 정확히 하나씩 배정하여 모든 작업도 정확히 한 번 처리되게 할 때,
총비용의 최솟값과 그 배정 하나를 구한다. 최적 배정이 여러 개면 아무 것이나 출력해도 된다.

입력:
첫 줄에 직원 수이자 작업 수인 n이 주어진다. 이어지는 n개 줄의 각 n개 정수는
직원 i가 작업 j를 수행할 때 드는 비용이다.

출력:
첫 줄에 최소 총비용을 출력한다. 다음 n개 줄에는 "직원 번호 작업 번호"를 출력한다.

제약:
1 <= n <= 200, 1 <= c[i][j] <= 1000. 제한 시간은 1초, 메모리는 512MB다.

예제:
입력                         가능한 출력
4                            33
17 8 16 9                   1 4
7 15 12 19                  2 1
6 9 10 11                   3 3
14 7 13 10                  4 2

공용 알고리즘 문서:
../algorithm/hungarian-assignment.md

대회 핵심:
완전 이분 그래프의 최소 비용 완전 매칭이다. Hungarian 알고리즘은 직원 잠재값 u와 작업 잠재값 v를
유지하여 감소 비용 c[i][j]-u[i]-v[j]를 음수가 아니게 하고, 현재 매칭 간선은 감소 비용 0인
tight 간선으로 유지한다. 한 직원을 추가할 때 alternating tree의 바깥 작업까지의 최소 slack을
관리하고, 가장 작은 slack delta만큼 잠재값을 바꾸면 적어도 새 tight 간선 하나가 생긴다.
빈 작업을 찾으면 predecessor를 거꾸로 따라 증대 경로를 뒤집는다. 직원마다 최대 n회의 확장,
확장마다 n개 작업을 훑으므로 O(n^3) 시간이며 비용 행렬 O(n^2), 작업 배열 O(n) 공간이다.
*/

// <iostream>은 표준 입력 std::cin, 표준 출력 std::cout과 스트림 상태/연산을 선언한다.
#include <iostream>
// <vector>는 실행 중 결정되는 n에 맞춰 원소를 연속 저장하고 수명을 소유하는 동적 배열을 선언한다.
#include <vector>

// using 별칭은 새 타입을 만들지 않는다. long long에 "비용"이라는 문제 의미를 붙여 누적 의도를 드러낸다.
using Cost = long long;
// 바깥 vector의 각 원소가 한 직원의 비용 행을 소유한다. 0번 행/열은 알고리즘의 더미 자리다.
using CostMatrix = std::vector<std::vector<Cost>>;

// 구현 근거 문서: ../algorithm/hungarian-assignment.md
// 반환형 Cost는 최소 총비용이다. costs는 소유하지 않는 const lvalue 참조라 복사 없이 읽기만 하고,
// task_of_employee는 호출자가 소유한 vector에 결과를 기록하는 non-const lvalue 참조다.
// 두 참조는 함수가 끝날 때까지 실제 객체가 살아 있어야 하며 main의 지역 객체 수명이 이를 만족한다.
[[nodiscard]] Cost solve_assignment(
    const CostMatrix& costs,
    const int employee_count,
    std::vector<int>& task_of_employee) {
    // 1-based 인덱스와 더미 인덱스 0을 함께 담기 위해 실제 개수보다 하나 큰 크기를 쓴다.
    const int dimension{employee_count + 1};
    // sentinel은 공식 최대 답 200*1000보다 훨씬 크다. 2^60은 이후 제한 내 덧셈/뺄셈에도
    // long long 범위를 넘지 않아, "무한대" 흉내 값 때문에 signed overflow UB가 생기지 않는다.
    constexpr Cost infinity{Cost{1} << 60};

    // [첫 생성 계약: std::vector<T>(size_type, const T&)]
    // (1) 아래 여섯 수신 객체는 아직 수명이 시작되지 않았고 dimension은 제약상 [2,201]이다.
    // (2) vector(size_type count, const value_type& value) 채움 생성자가 T=Cost, int,
    //     unsigned char에 맞게 선택된다. allocator는 각 vector의 기본 std::allocator<T>다.
    // (3) dimension은 int lvalue에서 각 vector의 size_type 값으로 변환된다. Cost{0}, 0, infinity,
    //     0U는 각각 값 초기화된 prvalue/lvalue이며 value의 const 참조에는 호출 동안만 바인딩된다.
    //     생성자는 인자의 소유권을 가져가지 않고 그 값을 count번 복사해 독립 원소를 만든다.
    // (4) 생성자는 반환값이 없으며 성공한 각 객체가 dimension개 원소의 소유자가 된다.
    // (5) 성공 뒤 size()==dimension이다. 잠재값·매칭·선행자는 0, slack은 infinity로 시작하며
    //     사용 표시는 0이다. 인자는 변하지 않고 vector 수명은 이 함수 블록 끝까지다.
    // (6) 각 생성은 O(dimension) 시간/공간이고 동적 할당할 수 있다. length_error 또는 bad_alloc이
    //     가능하며 실패하면 이미 만든 원소를 파괴한다. 아직 iterator/reference가 없어 무효화 대상은 없다.
    //     객체를 다른 스레드와 공유하지 않으므로 data race가 없지만 같은 vector의 동시 변경 보장은 없다.
    std::vector<Cost> employee_potential(dimension, Cost{0});
    std::vector<Cost> task_potential(dimension, Cost{0});
    std::vector<int> matched_employee(dimension, 0);
    std::vector<int> previous_task(dimension, 0);

    // for의 employee는 기본 타입 int이며 중괄호 초기화로 1에서 시작한다. ++는 한 명씩 전진한다.
    // 바깥 루프가 끝날 때마다 정확히 employee명의 직원이 서로 다른 실제 작업에 매칭된다는 불변식을 쓴다.
    for (int employee{1}; employee <= employee_count; ++employee) {
        // min_slack[j]는 현재 alternating tree 안 직원들에서 미방문 작업 j로 가는 감소 비용의 최솟값이다.
        std::vector<Cost> minimum_slack(dimension, infinity);
        // unsigned char는 0/1 방문 표시에 충분하며 vector<bool>의 proxy reference 특수화를 피한다.
        std::vector<unsigned char> used_task(dimension, 0U);

        // [첫 호출 계약: std::vector<int>::operator[] 비-const]
        // (1) 수신 matched_employee는 size=dimension인 유효한 non-const vector<int> lvalue이며
        //     인덱스 0은 실제 작업이 아닌 더미 작업 자리다.
        // (2) reference vector<int>::operator[](size_type pos) noexcept 오버로드가 선택된다.
        // (3) 피연산자 0은 허용 범위의 int prvalue가 size_type 0으로 변환된다. employee는 [1,n]인
        //     int lvalue이고 대입이 값을 복사할 뿐 소유권이나 객체 수명을 옮기지 않는다.
        // (4) 호출은 int&를 반환하며 대입의 왼쪽 피연산자로 사용된다. 별도 반환값은 보관하지 않는다.
        // (5) 호출과 대입 뒤 0번 원소가 새 직원 번호가 되고 vector의 크기·용량은 그대로다.
        // (6) O(1), 무할당·무효화·무예외다. pos>=size면 UB지만 0<dimension으로 배제한다.
        //     반환 참조는 vector와 해당 원소 수명 안에서만 유효하며 같은 원소의 동시 쓰기는 안전하지 않다.
        matched_employee[0] = employee;

        // current_task는 alternating tree에서 마지막으로 들어온 작업 열이다. 0은 새 직원으로 향하는 더미다.
        int current_task{0};

        // do-while은 먼저 더미 열을 처리해야 하므로 적합하다. 빈 실제 작업을 만날 때까지 확장한다.
        do {
            // [첫 호출 계약: std::vector<unsigned char>::operator[] 비-const]
            // (1) 수신 used_task는 size=dimension인 유효한 non-const vector<unsigned char> lvalue이고
            //     current_task는 불변식상 [0,n]이다.
            // (2) reference operator[](size_type pos) noexcept에서 value_type=unsigned char가 선택된다.
            // (3) current_task의 현재 int 값이 size_type으로 변환된다. 1U는 소유 자원이 없는 unsigned
            //     int prvalue이며 unsigned char 1로 값 변환되어 대입된다.
            // (4) unsigned char&를 반환해 방문 표식 대입에 사용하고 반환 참조는 저장하지 않는다.
            // (5) 해당 원소만 1이 되며 size/capacity와 다른 원소는 그대로다.
            // (6) O(1), 무할당·무효화·무예외다. 범위 밖 UB는 current_task 불변식으로 배제한다.
            //     참조 수명은 원소/컨테이너 수명에 묶이고 같은 vector의 동시 변경은 보장되지 않는다.
            used_task[current_task] = 1U;

            // 방문 작업에 현재 매칭된 직원이다. 첫 회에는 p[0]=employee라 새 직원이 선택된다.
            const int current_employee{matched_employee[current_task]};
            Cost delta{infinity};
            int next_task{0};

            // 아직 alternating tree 바깥인 모든 실제 작업으로 나가는 간선을 완화한다.
            for (int task{1}; task <= employee_count; ++task) {
                // unsigned char 0은 false, 1은 true로 문맥 변환된다.
                if (used_task[task] != 0U) {
                    continue;
                }

                // [첫 호출 계약: const CostMatrix/vector<Cost> 및 작업 vector의 operator[]]
                // (1) costs는 dimension개의 행, 각 행도 dimension개 Cost인 유효한 const CostMatrix
                //     lvalue다. current_employee/task는 [1,n]이다. 잠재값·slack·선행자 vector도
                //     size=dimension인 유효한 non-const lvalue다.
                // (2) costs의 바깥 const operator[](size_type)은 const vector<Cost>&를, 그 행의 const
                //     operator[]은 const Cost&를 반환한다. employee_potential/task_potential/minimum_slack의
                //     non-const operator[]은 Cost&, previous_task의 것은 int&를 반환한다.
                // (3) 각 int 인덱스 식은 size_type으로 값 변환되며 모두 범위 안이다. 읽기는 비소유
                //     참조에서 값만 가져오고, 아래 대입은 Cost/int 값을 복사해 소유권 이전이 없다.
                // (4) 반환 참조들은 감소 비용 계산, 비교, 대입의 피연산자로 즉시 사용하고 저장하지 않는다.
                // (5) 읽기 호출은 아무 객체도 바꾸지 않는다. 조건이 참이면 해당 slack과 predecessor만
                //     갱신되며 모든 vector의 크기·용량은 유지된다.
                // (6) 각 인덱싱은 O(1), 무할당·무효화·무예외다. 범위 밖은 UB지만 루프/매칭 불변식으로
                //     배제한다. 참조는 재할당 또는 vector 파괴 전까지만 유효하며 동시 쓰기는 하지 않는다.
                const Cost reduced_cost{
                    costs[current_employee][task]
                    - employee_potential[current_employee]
                    - task_potential[task]};

                // 더 작은 slack을 만든 직원 쪽 경로를 기억한다. 엄격한 <는 동률에서 기존 경로를 유지한다.
                if (reduced_cost < minimum_slack[task]) {
                    minimum_slack[task] = reduced_cost;
                    previous_task[task] = current_task;
                }

                // 가장 작은 slack과 그 작업을 고른다. 작업 번호 오름차순이라 동률은 작은 번호가 남는다.
                if (minimum_slack[task] < delta) {
                    delta = minimum_slack[task];
                    next_task = task;
                }
            }

            // 완전 이분 그래프이므로 미방문 실제 작업이 적어도 하나 있고 그 slack은 유한하다.
            // 잠재값 갱신 불변식:
            // - 방문 작업 j와 그 매칭 직원 p[j]에는 +delta/-delta가 함께 적용되어 tight 간선이 유지된다.
            // - tree 안 직원 -> 바깥 작업의 slack은 delta만큼 줄되 delta가 최솟값이라 음수가 되지 않는다.
            // - 다른 간선은 그대로이거나 dual 제약이 더 느슨해져 u[i]+v[j] <= c[i][j]가 보존된다.
            for (int task{0}; task <= employee_count; ++task) {
                if (used_task[task] != 0U) {
                    employee_potential[matched_employee[task]] += delta;
                    task_potential[task] -= delta;
                } else if (task != 0) {
                    minimum_slack[task] -= delta;
                }
            }

            // delta를 최소 slack으로 골랐으므로 next_task의 새 slack은 정확히 0이다.
            // 비교·저장·조건 분기는 추상 기계 관점에서 일어나지만 실제 load/store/분기 배치는 CPU,
            // 컴파일러, 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령을 단정하지 않는다.
            current_task = next_task;
        } while (matched_employee[current_task] != 0);

        // current_task는 아직 어떤 직원도 맡지 않은 작업이다. way를 거슬러 더미 0까지 가면서
        // alternating path의 매칭/비매칭 간선을 뒤집으면 기존 직원의 유일성은 보존되고 새 직원만 추가된다.
        do {
            const int predecessor{previous_task[current_task]};
            matched_employee[current_task] = matched_employee[predecessor];
            current_task = predecessor;
        } while (current_task != 0);
    }

    // p[task]=employee 방향을 출력에 편한 assignment[employee]=task 방향으로 뒤집는다.
    for (int task{1}; task <= employee_count; ++task) {
        task_of_employee[matched_employee[task]] = task;
    }

    Cost minimum_total_cost{0};
    for (int employee{1}; employee <= employee_count; ++employee) {
        minimum_total_cost += costs[employee][task_of_employee[employee]];
    }

    // Cost는 기본 스칼라이므로 이 식은 독립 long long prvalue를 호출자에게 준다. vector를 반환할 때의
    // NRVO/이동과 달리 소유 자원이 없고 참조 수명 문제도 없다. 복사 생략 여부에 답의 의미가 의존하지 않는다.
    return minimum_total_cost;
}

int main() {
    // [첫 호출 계약: std::ios::sync_with_stdio와 std::cin.tie]
    // (1) 첫 호출은 static 멤버라 수신 객체가 없다. 두 번째 호출의 수신 객체는 초기화된 정확한 타입
    //     std::istream의 lvalue std::cin이고, 이 프로그램은 아직 어떤 입출력도 수행하지 않았다.
    // (2) static bool ios_base::sync_with_stdio(bool)와
    //     std::ostream* basic_ios<char>::tie(std::ostream*)가 선택된다.
    // (3) false는 C와 C++ 표준 스트림 동기화를 끄라는 bool prvalue, nullptr는 묶을 출력 스트림이
    //     없다는 null pointer prvalue다. 어떤 버퍼나 스트림 소유권도 이전하지 않는다.
    // (4) 각각 이전 동기화 상태 bool과 이전 tie 대상 ostream*을 반환하지만 의도적으로 버린다.
    // (5) 이후 cin이 매 입력 전 cout을 자동 flush하지 않고 C stdio와 순서를 섞을 보장이 사라진다.
    // (6) 입력 크기에 비례하는 호출은 아니다. 표준은 두 호출을 noexcept로 선언하지 않으므로 무예외를
    //     단정하지 않는다. 스트림/버퍼 수명과 컨테이너 관찰자에는 영향이 없다. 첫 I/O 전에 한 스레드에서
    //     설정하며, 이후 여러 스레드의 한 레코드 단위 입출력 순서까지 보장하는 설정은 아니다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // 빈 중괄호 초기화는 기본 타입 int를 0으로 값 초기화하여 초기화되지 않은 값 사용을 막는다.
    int employee_count{};

    // [첫 호출 계약: istream 정수 추출과 basic_ios::operator!]
    // (1) 수신 std::cin은 살아 있는 std::istream lvalue이고 employee_count는 쓰기 가능한 int lvalue다.
    // (2) basic_istream<char>& operator>>(int&) 뒤 bool basic_ios<char>::operator!() const가 선택된다.
    // (3) 인자는 employee_count의 비소유 lvalue 참조이며 공식 입력의 [1,200] 정수를 기대한다.
    // (4) 추출은 std::cin 참조를 반환하고 operator!은 fail 상태면 true를 반환해 if 조건에 사용한다.
    // (5) 성공 시 값을 저장하고 입력 위치가 전진한다. 실패 시 fail/eof 상태가 설정되며 즉시 종료한다.
    // (6) 소비 문자 수에 비례하며 locale/버퍼 작업이 가능하다. 예외 mask에 따라 ios_base::failure를
    //     던질 수 있고 기본 설정은 상태 비트로 보고한다. 반환 참조는 정적 수명 cin의 비소유 별칭이다.
    //     같은 stream/변수를 여러 스레드가 동시에 추출·수정하는 순서는 보장되지 않는다.
    if (!(std::cin >> employee_count)) {
        return 0;
    }

    const int dimension{employee_count + 1};

    // [첫 생성 계약: CostMatrix 외부 fill 생성과 std::vector<Cost> 행 fill 생성]
    // (1) 수신 costs와 이름 없는 행 객체는 아직 수명 전이고 dimension은 [2,201]이다.
    // (2) 안쪽 vector<Cost>(size_type, const Cost&)와 바깥
    //     vector<vector<Cost>>(size_type, const vector<Cost>&) 채움 생성자가 안쪽부터 선택된다.
    // (3) 두 count에는 dimension의 int 값이 size_type으로 변환된다. Cost{0} prvalue는 안쪽 value의
    //     const Cost&에, 완성된 행 prvalue는 바깥 value의 const vector<Cost>&에 전체 식 동안 바인딩된다.
    //     바깥 생성은 행을 복사하며 원래 행의 소유권을 빼앗지 않는다.
    // (4) 생성자는 반환값이 없고 성공하면 costs가 모든 행과 셀을 독점 소유한다.
    // (5) costs.size()==dimension이고 각 행 size도 dimension이며 모든 셀이 0이다. 행 prvalue는 전체 식
    //     끝에서 파괴되지만 복사된 행들은 독립 저장소를 가져 영향을 받지 않고 main 끝까지 산다.
    // (6) 전체 O(n^2) 시간/공간이며 여러 동적 할당에서 length_error/bad_alloc 또는 Cost가 아닌 일반
    //     원소라면 복사 예외도 가능하다. 실패 시 이미 만든 행을 정리한다. 관찰자가 없어 무효화 대상은
    //     없고, 공개 전 단일 스레드 생성이라 data race가 없다.
    CostMatrix costs(dimension, std::vector<Cost>(dimension, Cost{0}));

    for (int employee{1}; employee <= employee_count; ++employee) {
        for (int task{1}; task <= employee_count; ++task) {
            // [첫 호출 계약: CostMatrix/vector<Cost>::operator[] 비-const와 연쇄 operator>>]
            // (1) costs는 dimension개 독립 행을 가진 유효한 non-const CostMatrix lvalue이고 두 인덱스는
            //     [1,n]이다. std::cin도 오류 전의 유효한 std::istream lvalue다.
            // (2) 바깥 vector<vector<Cost>>::operator[](size_type)은 vector<Cost>&, 안쪽
            //     vector<Cost>::operator[](size_type)은 Cost&를 반환하고 operator>>(long long&)가 선택된다.
            // (3) 두 int 인덱스는 size_type으로 변환된다. 마지막 Cost&는 입력 목적지인 비소유 참조이며
            //     공식 값 [1,1000]을 받는다. 저장소 소유권 이동은 없다.
            // (4) 인덱싱 참조는 다음 호출의 수신/인자로 즉시 쓰고, 추출은 cin&를 반환하나 버린다.
            // (5) 성공하면 해당 셀만 입력 비용으로 바뀌고 입력 위치가 전진한다. 행 크기/용량은 그대로다.
            // (6) 인덱싱 O(1), 추출은 소비 문자 수에 비례한다. 범위 밖 인덱싱은 UB지만 루프가 배제한다.
            //     추출 실패는 상태 비트/설정된 예외로 나타나며 vector 할당·무효화는 없다. 반환 참조 수명은
            //     각 행과 cin 수명에 묶이고 이 단일 스레드 프로그램에는 동시 접근 문제가 없다.
            std::cin >> costs[employee][task];
        }
    }

    // 결과 vector도 같은 채움 생성자로 dimension개 0을 소유한다. solve_assignment는 이름 있는 costs와
    // task_of_employee라는 lvalue에 참조 바인딩하므로 행렬 복사도 소유권 이전도 없다.
    std::vector<int> task_of_employee(dimension, 0);
    const Cost minimum_cost{solve_assignment(costs, employee_count, task_of_employee)};

    // [첫 호출 계약: std::ostream 정수·문자 삽입 operator<<]
    // (1) 수신 std::cout은 유효한 std::ostream lvalue이고 minimum_cost는 [1,200000]의 const Cost,
    //     employee/task 값은 [1,n]의 int이며 '\n'은 char 값이다.
    // (2) 첫 식은 basic_ostream<char>& operator<<(long long), 이어지는 식들은 멤버
    //     operator<<(int)와 비멤버 operator<<(ostream&, char)의 해당 오버로드를 연쇄 선택한다.
    // (3) 각 산술 피연산자는 값으로 전달되어 소유권을 옮기지 않는다. task_of_employee 인덱싱 반환
    //     int&는 값을 읽는 데만 쓰며 범위 안 employee를 사용한다.
    // (4) 각 삽입은 같은 cout 참조를 반환해 다음 <<의 수신이 되고, 줄 끝의 최종 참조는 버린다.
    // (5) 출력 버퍼에 최소 비용, 이어서 각 배정과 줄바꿈이 순서대로 추가되고 입력 값은 변하지 않는다.
    // (6) 숫자는 생성 문자 수, char는 한 문자 처리에 비례하며 실제 버퍼 I/O가 생길 수 있다. 실패는
    //     상태 비트 또는 예외 mask의 ios_base::failure로 보고된다. '\n'은 강제 flush하지 않는다.
    //     인덱싱은 O(1)·무효화 없음이고 출력 객체/피연산자 수명은 바뀌지 않는다. 여러 스레드 레코드
    //     원자성은 보장하지 않지만 여기서는 한 스레드만 쓴다.
    std::cout << minimum_cost << '\n';
    for (int employee{1}; employee <= employee_count; ++employee) {
        std::cout << employee << ' ' << task_of_employee[employee] << '\n';
    }

    // 정확성 요약: 매 단계는 dual feasibility와 기존 매칭의 tightness를 보존하고 새 tight 간선을
    // 하나 이상 만든다. 빈 작업까지의 tight 증대 경로로 매칭 크기가 1 증가하므로 n회 뒤 완전 매칭이다.
    // 완성 매칭의 모든 간선이 tight여서 primal 비용과 dual 값이 같으므로 약한 쌍대성에 의해 최적이다.
    // 흔한 실수: p의 방향을 반대로 출력, 매 직원마다 slack/used 미초기화, task 0 갱신 누락,
    // potential 부호 뒤집기, int로 일반 문제의 큰 비용을 누적, INF에 무조건 산술 적용하기다.
    return 0;
}
