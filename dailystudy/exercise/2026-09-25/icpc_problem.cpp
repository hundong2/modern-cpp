/*
문제 ID·제목: CSES 1628 - Meet in the Middle
출처: https://cses.fi/problemset/task/1628/

문제 요약:
양의 정수 n개가 주어진다. 각 위치의 수를 고르거나 고르지 않아 만든 부분집합 중 원소 합이 x인 경우의 수를 센다.
값이 같아도 위치가 다르면 서로 다른 선택이다. 빈 부분집합도 부분집합이지만 공식 제약에서는 x>=1이라 답에 포함되지 않는다.

입력:
첫 줄에 배열 크기 n과 목표 합 x가 주어진다. 둘째 줄에 n개의 양의 정수 t_i가 주어진다.

출력:
합이 정확히 x인 부분집합의 개수를 출력한다.

제약:
1 <= n <= 40, 1 <= x <= 10^9, 1 <= t_i <= 10^9.
부분 합은 최대 40*10^9이고 답은 최대 2^40이므로 합·목표·정답을 64비트 long long에 저장해야 한다.

예제:
입력  4 5 / 1 2 3 2
출력  3
세 선택은 위치 기준으로 {1,2(첫째),2(둘째)}, {2(첫째),3}, {3,2(둘째)}다.

핵심:
2^40 전수 조사는 너무 크다. 배열을 최대 20개씩 둘로 나누면 각 절반의 부분집합 합은 최대 2^20개다.
오른쪽 합을 정렬하고 왼쪽 합 a마다 x-a와 같은 오른쪽 합의 구간을 이진 탐색해 중복 개수까지 더한다.
구현 가까운 공용 문서: ../algorithm/meet-in-the-middle-subset-sum.md
*/

// <algorithm>은 범위 정렬 std::ranges::sort와 같은 값 구간 탐색 std::ranges::equal_range를 선언한다.
#include <algorithm>
// <cstddef>는 컨테이너 크기와 인덱스에 쓰는 std::size_t를 선언한다.
#include <cstddef>
// <iostream>은 표준 입력/출력 객체와 스트림 연산을 선언한다.
#include <iostream>
// <ranges>는 equal_range가 반환하는 subrange와 range 개념을 선언한다.
#include <ranges>
// <vector>는 절반별 부분집합 합을 소유하는 연속 동적 배열을 선언한다.
#include <vector>

// 최대 절반 상태 수 2^20을 std::size_t가 표현하는지 직접 검사한다. 좁은 size_t가 shift 전에 int로 승격되는
// 경우에도 단순 shift 검사는 잘못 통과할 수 있으므로, 64비트 상수를 변환했다가 원값과 비교한다.
static_assert(
    static_cast<std::size_t>(1'048'576ULL) == 1'048'576ULL,
    "std::size_t must represent every half-subset count");

// 반환형 vector<long long>은 모든 부분합을 소유한다. values는 const&로 빌리고 [first,last)만 읽는다.
[[nodiscard]] std::vector<long long> enumerate_subset_sums(
    const std::vector<long long>& values,
    const std::size_t first,
    const std::size_t last) {
    // [첫 생성 계약: vector<long long> 기본 생성]
    // (1) 수신 sums는 아직 생성 전이다. (2) vector() 기본 생성자와 기본 allocator가 선택된다.
    // (3) 인자는 없다. (4) 생성자는 반환값 없이 빈 owner를 만든다. (5) 성공 뒤 size=0이다.
    // (6) 이 기본 allocator specialization은 상수 시간·noexcept다. 빈 상태는 원소 저장소를 요구하지 않지만
    //     구현 내부 할당 여부를 표준 계약 이상으로 단정하지 않는다. 아직 관찰자·공유 원소는 없다.
    std::vector<long long> sums{};

    const std::size_t width{last - first};
    // width<=20과 위 표현 범위 guard 덕분에 integral promotion 뒤에도 2^width가 표현 가능하다. 마지막 cast는
    // 좁은 unsigned 타입이 int로 승격될 수 있는 구현에서도 의도한 std::size_t 결과 타입을 명시한다.
    const std::size_t subset_count{static_cast<std::size_t>(std::size_t{1} << width)};

    // [첫 호출 계약: vector::reserve]
    // (1) 수신 sums는 빈 std::vector<long long>이다.
    // (2) void reserve(size_type new_capacity)를 선택한다.
    // (3) subset_count는 1..2^20의 std::size_t lvalue이고 값만 읽으며 소유권 의미가 없다.
    // (4) 반환형 void라 사용하지 않는다.
    // (5) 성공하면 size=0, capacity>=subset_count이고 인자는 그대로다.
    // (6) 재할당은 현재 size에 선형이며 bad_alloc/length_error가 가능하다. 성공 시 기존 관찰자를 모두
    //     무효화하지만 현재 원소는 없다. 동일 vector 동시 접근을 동기화하지 않는다.
    sums.reserve(subset_count);

    // [첫 호출 계약: vector::push_back(long long&&)]
    // (1) 수신 sums는 size=0이며 필요한 전체 capacity를 예약했다.
    // (2) void push_back(value_type&&)가 long long prvalue 0에 선택된다.
    // (3) 0LL은 소유 자원이 없는 long long prvalue이며 모든 표현값이 허용된다.
    // (4) 반환형 void라 버린다.
    // (5) 성공 뒤 size=1이고 빈 부분집합 합 0을 저장한다.
    // (6) 예약 범위에서 상수 시간이고 재할당하지 않는다. 일반적으로 할당/이동 예외가 가능하지만 long long
    //     이동은 던지지 않는다. 기존 원소 관찰자는 유지되지만 과거 end 반복자는 무효화되며 동시 접근을
    //     동기화하지 않는다.
    sums.push_back(0LL);

    for (std::size_t position{first}; position < last; ++position) {
        // [첫 호출 계약: const vector::operator[]]
        // (1) 수신 values는 유효하고 position<last<=values.size()인 읽기 전용 vector다.
        // (2) const_reference operator[](size_type) const가 선택된다. 표준 선언은 noexcept를 요구하지 않는다.
        // (3) position은 범위 안 std::size_t lvalue이고 소유권을 넘기지 않는다.
        // (4) const long long&를 반환해 value에 복사한다.
        // (5) vector와 position은 바뀌지 않는다.
        // (6) 상수 시간·무할당이고 이 기본 정수 접근은 정상 범위에서 오류값을 내지 않는다. 다만 표준
        //     시그니처가 noexcept인 것은 아니며, 범위 검사를 하지 않아 전제조건을 깨면 미정의 동작이다.
        //     읽기 중 다른 스레드가 같은 vector를 쓰면 데이터 경쟁이다.
        const long long value{values[position]};

        // [첫 호출 계약: vector::size]
        // (1) 수신 sums는 앞선 원소들로 만들 수 있는 부분합을 정확히 한 번씩 저장한다.
        // (2) size_type size() const noexcept가 선택되고 인자는 없다.
        // (3) 인자·소유권 이전은 없다. (4) 현재 원소 수를 값으로 반환해 previous_size에 저장한다.
        // (5) 수신 상태는 변하지 않는다. (6) 상수 시간·무할당·noexcept, 관찰자 무효화 없음이다.
        const std::size_t previous_size{sums.size()};

        // 불변식: 루프 시작의 [0,previous_size)는 현재 value를 고르지 않은 모든 합이다.
        // 그 고정 접두사만 읽어 value를 더한 합을 뒤에 붙인다. 새로 붙인 원소까지 다시 읽으면 같은 값을
        // 여러 번 고르는 버그가 생긴다. reserve 덕분에 이 루프에서는 재할당도 없다.
        for (std::size_t index{}; index < previous_size; ++index) {
            // 앞서 설명한 operator[]와 push_back 계약을 재사용한다. index<previous_size라 읽기가 유효하고,
            // 공식 제약의 절반 합은 2*10^10 이하라 signed overflow가 없다.
            sums.push_back(sums[index] + value);
        }
    }

    // 이름 있는 sums는 lvalue다. NRVO가 적용되면 호출자의 결과 vector에 직접 생성되고, 적용되지 않아도
    // vector 이동이 소유 저장소를 이전한다. 반환 뒤 values는 그대로다.
    return sums;
}

int main() {
    // [첫 호출 계약: 표준 스트림 동기화 설정과 tie 해제]
    // (1) sync_with_stdio는 사용자 수신 객체 없는 static 함수로 전역 동기화 상태를 다룬다. tie의 수신자는
    //     프로그램 시작 뒤 아직 I/O 전인 정확한 타입 std::istream의 전역 객체 std::cin이다.
    // (2) static bool sync_with_stdio(bool)와 std::ostream* tie(std::ostream*)가 선택된다.
    // (3) false는 C 스트림과의 동기화 해제, nullptr는 std::ostream* null 포인터로 변환되어 자동 flush
    //     대상 없음이라는 제어값이 된다. 두 인자 모두 소유권 의미가 없다.
    // (4) 첫 호출은 이전 설정 bool을 반환해 버리고, tie는 이전 ostream* 비소유 포인터를 반환해 버린다.
    // (5) 이후 C/C++ 혼합 I/O 순서와 입력 전 자동 flush 정책이 바뀌며 인자는 그대로다.
    // (6) 구현 정의 비용의 전역 설정이다. 별도 오류값은 없고 stream 수명은 프로그램 종료까지다. I/O 뒤
    //     호출 의미는 구현에 따라 달라질 수 있으며, 해제 뒤 C I/O와 섞으면 순서를 별도로 관리해야 한다.
    //     전역 설정을 여러 스레드에서 경쟁 호출하지 않는다.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int number_count{};
    long long target_sum{};

    // [첫 호출 계약: std::cin operator>>]
    // (1) 수신 std::cin은 유효한 std::istream이고 number_count/target_sum은 0으로 초기화됐다.
    // (2) int&와 long long&에 대한 형식 추출 overload를 왼쪽부터 선택한다.
    // (3) 두 인자는 수정 가능한 lvalue 참조이며 입력 소유권을 얻지 않고 파싱 결과만 저장한다.
    // (4) 각 호출은 같은 istream&를 반환해 연쇄에 사용하고 마지막 참조는 버린다.
    // (5) 성공하면 두 값과 입력 위치가 갱신된다. sentry 실패로 추출을 시작하지 않으면 대상은 유지된다. 숫자
    //     변환 문자가 없으면 대상에 0을 쓰고 failbit, 표현 범위 밖이면 해당 타입의 min/max로 포화해 쓰고
    //     failbit를 세운다. 연쇄 첫 실패 뒤 둘째 추출의 sentry도 실패해 둘째 대상은 그대로일 수 있다.
    // (6) 형식 변환·locale·버퍼 처리 비용이 들며 표준은 이 추출에 단순한 "문자 수 선형" 복잡도를 별도로
    //     보장하지 않는다. eofbit/failbit/badbit와 설정된 stream 예외가 가능하고, 동일 stream의 동시 추출
    //     순서나 한 레코드 원자성을 보장하지 않는다. 공식 입력은 성공 형식을 만족한다.
    std::cin >> number_count >> target_sum;

    // [첫 생성 계약: vector count 생성자]
    // (1) 수신 values는 아직 생성 전이다.
    // (2) explicit vector(size_type count) 생성자가 원소 타입 long long과 기본 allocator로 선택된다.
    // (3) number_count를 std::size_t로 변환한 값은 공식 제약상 1..40이고 소유권 의미가 없다.
    // (4) 생성자는 반환값 없이 count개의 값 초기화된 0 원소를 소유한다.
    // (5) 성공 뒤 size=count다. (6) O(count)·한 번의 할당 가능, bad_alloc/length_error 가능이며
    //     생성 실패 시 완성 객체가 없고 외부 참조도 없다. 자체 동기화는 없다.
    std::vector<long long> values(static_cast<std::size_t>(number_count));

    // [첫 호출 계약: mutable vector range-for와 반복되는 operator>>]
    // (1) 수신 values는 number_count개의 long long을 소유하고 std::cin은 첫 줄 뒤 위치다.
    // (2) iterator begin() noexcept, end() noexcept, reference operator*() const, iterator& operator++(),
    //     bool 비교(operator!= 또는 ==에서 합성), 그리고 long long& 추출 overload가 선택된다.
    // (3) begin/end/*/++에는 명시 데이터 인자가 없다. 숨은 비교는 현재/끝 iterator lvalue 두 개를 비소유
    //     피연산자로 쓰고, value는 각 원소의 mutable lvalue 참조이며 추출이 그 원소를 갱신한다.
    // (4) 숨은 반복 반환과 istream&는 루프가 소비한다.
    // (5) 성공하면 모든 원소가 입력값이 되고 vector 구조는 유지된다.
    // (6) begin/end와 각 iterator 연산은 O(1)·무할당이고 begin/end는 noexcept다. vector가 살아 있고 구조
    //     변경이 없어야 하며 past-the-end 역참조·증가나 무효 iterator 사용은 미정의 동작이다. 전체 O(n),
    //     추가 할당·무효화 없음이다. stream 오류는 상태 비트/예외로 나타나며 동시 쓰기는 데이터 경쟁이다.
    for (long long& value : values) {
        std::cin >> value;
    }

    const std::size_t middle{values.size() / 2U};
    // [첫 생성 계약: 반환 vector prvalue로 left_sums/right_sums 직접 초기화]
    // (1) 두 수신 std::vector<long long>은 아직 생성 전이고 values는 입력 전체를 소유한 유효한 lvalue다.
    // (2) enumerate_subset_sums 반환과 vector의 같은 타입 prvalue 직접 초기화가 선택되며, 함수 안 NRVO가
    //     적용되지 않는 경로에서는 vector(vector&&)가 저장소를 이전할 수 있다.
    // (3) values는 const&로 빌리고 0/middle/values.size() 경계는 0<=middle<=size를 만족한다.
    // (4) 각 함수는 절반 부분합을 소유한 vector 값을 반환해 이름 있는 결과 객체 초기화에 모두 사용한다.
    // (5) values는 그대로이고 두 결과가 서로 독립된 저장소를 소유한다.
    // (6) 각 절반 O(2^m) 시간·공간이며 reserve 할당 실패가 전파될 수 있다. 실패 시 완성 결과는 없고
    //     values 관찰자는 유지된다. 결과 vector들은 별도 동기화 없이 한 스레드에서 사용한다.
    std::vector<long long> left_sums{enumerate_subset_sums(values, 0U, middle)};
    std::vector<long long> right_sums{enumerate_subset_sums(values, middle, values.size())};

    // 공용 알고리즘 설명: ../algorithm/meet-in-the-middle-subset-sum.md
    // [첫 호출 계약: std::ranges::sort]
    // (1) 사용자 수신 객체는 없고 right_sums는 모든 오른쪽 부분합을 소유한 유효한 vector다.
    // (2) ranges::sort(R&& range, Comp=less, Proj=identity) 범위 overload가 선택된다.
    // (3) right_sums는 non-const lvalue라 빌려 전달되고 기본 엄격 오름차순 비교/항등 projection을 쓴다.
    // (4) 반환 iterator는 정렬 뒤 end 위치이며 이 호출은 사용하지 않는다.
    // (5) 원소가 제자리 오름차순으로 재배치되고 크기·용량·소유권은 유지된다.
    // (6) O(R log R) 비교, 추가 동적 할당 보장은 없고 반복자 위치의 값은 바뀐다. 비교/이동 예외는 전파될 수
    //     있으나 long long 연산은 던지지 않는다. 같은 vector 동시 접근은 데이터 경쟁이다.
    std::ranges::sort(right_sums);

    long long answer{};
    for (const long long left_sum : left_sums) {
        const long long needed{target_sum - left_sum};

        // [첫 호출 계약: std::ranges::equal_range와 반환 subrange::size]
        // (1) 사용자 수신 객체는 없고 right_sums는 오름차순 정렬된 vector다. 반환 matches는 곧 생성된다.
        // (2) ranges::equal_range(R&&, const T&, Comp=less, Proj=identity)에서 R=vector<long long>&,
        //     T=long long이며 반환은 borrowed_subrange_t<R>다. 이어 size() const를 선택한다.
        // (3) right_sums는 빌리는 lvalue, needed는 읽기 전용 long long lvalue다. 기본 비교는 엄격 약순서다.
        // (4) 첫 호출은 needed와 동등한 [lower,upper) 비소유 subrange를 반환해 matches에 저장하고,
        //     size는 같은 합의 개수를 부호 없는 값으로 반환해 long long으로 안전 변환 후 answer에 사용한다.
        // (5) vector·needed·원소는 바뀌지 않고 answer만 그 개수만큼 증가한다.
        // (6) random-access vector에서 O(log R) 비교와 O(1) size, 무할당·무효화 없음이다. partition·엄격
        //     약순서·owner 수명·iterator 유효성 전제조건을 깨고 호출하면 미정의 동작이다. 비교 예외는
        //     전파될 수 있고 동시 쓰기는 데이터 경쟁이다.
        const auto matches{std::ranges::equal_range(right_sums, needed)};
        answer += static_cast<long long>(matches.size());
    }

    // [첫 호출 계약: std::cout operator<<]
    // (1) 수신 std::cout은 출력 가능한 std::ostream이다. (2) long long과 char 삽입 overload가 선택된다.
    // (3) answer는 읽는 lvalue, '\n'은 char prvalue이며 소유권 이전은 없다.
    // (4) 각 호출은 ostream&를 반환해 연쇄하고 최종 참조는 버린다.
    // (5) 출력 버퍼/상태만 바뀌고 answer는 유지된다.
    // (6) 형식 변환·locale·버퍼 비용이 들며 표준은 단순한 "출력 문자 수 선형" 복잡도를 별도로 보장하지
    //     않는다. 실패는 상태 비트 또는 설정 예외로 나타나고 참조 무효화는 없으며, 여러 스레드 출력의
    //     레코드 원자성은 보장하지 않는다.
    std::cout << answer << '\n';
}
