/*
CSES 2220 — Counting Numbers
출처: https://cses.fi/problemset/task/2220/

문제 요약:
닫힌 구간 [a, b]에 속한 정수 중에서, 보통의 십진 표기에 서로 이웃한 두 자릿값이
같지 않은 수의 개수를 구한다. 자릿수를 고정 길이로 맞출 때 덧붙이는 선행 0은 실제
표기의 자릿수가 아니므로 인접 중복 판정에 포함하면 안 된다. 정수 0은 유효한 수 하나다.

입력: 한 줄에 두 정수 a, b가 주어진다.
출력: 조건을 만족하는 정수의 개수 하나를 출력한다.
제약: 0 <= a <= b <= 10^18, 시간 1초, 메모리 512MB.

공식 예제 입력:
123 321
공식 예제 출력:
171
*/

#include <cstddef>  // std::size_t: string 첨자의 부호 없는 크기 타입을 정확히 표현한다.
#include <iostream> // std::cin/cout과 정수 스트림 연산, 입출력 설정을 사용한다.
#include <string>   // std::string과 std::to_string으로 상한의 십진 자릿수를 소유한다.

namespace {
// long long은 이 환경에서 적어도 64비트이며 10^18+1 이하의 개수를 담는다.
// sentinel 10은 실제 자릿수 0..9와 겹치지 않는 "아직 시작 전" 이전 값이다.
constexpr int no_previous_digit{10};
constexpr int decimal_base{10};
constexpr int maximum_digits{19};

// DigitCounter는 특정 limit의 문자 자릿수와 그 limit 전용 memo를 함께 소유한다.
// class는 기본 private라 외부가 memo 불변식을 직접 깨뜨릴 수 없다.
class DigitCounter {
public:
    // explicit은 long long이 DigitCounter로 뜻밖에 암시 변환되는 것을 막는다.
    // [호출 계약: std::to_string과 std::string 결과 생성]
    // (1) 수신 객체 없는 자유 함수이며 digits_는 아직 생성 전이다. limit는 [0,10^18] long long 값이다.
    // (2) std::string std::to_string(long long value) 오버로드를 선택한다.
    // (3) limit lvalue의 정수 값을 복사해 전달하며 외부 메모리나 소유권을 빌리지 않는다.
    // (4) NUL 문자를 제외한 십진 표현을 소유하는 string prvalue를 반환해 digits_를 직접 초기화한다.
    // (5) digits_가 문자를 독립 소유하고 limit는 변하지 않는다. C++17부터 같은 타입 prvalue가
    //     digits_를 직접 구성하므로 별도 중간 string 복사·이동은 일어나지 않는다.
    // (6) 표준은 to_string의 별도 복잡도 상한을 명시하지 않는다. 문자열 할당 실패 예외가
    //     전파될 수 있다. 음수도 '-'를 만들 수 있지만 count_up_to가 비음수만 넘기는 것이 전제다.
    //     공유 수신 객체는 없고, 생성된 digits_는 이 단일 스레드 DigitCounter만 접근한다.
    explicit DigitCounter(long long limit) : digits_{std::to_string(limit)} {}

    // 반환형 long long은 [0, limit]의 유효한 정수 개수다.
    [[nodiscard]] long long count() {
        return count_suffix(0, no_previous_digit, false, true);
    }

private:
    // 상태 불변식:
    // - position 앞의 접두사는 이미 조건을 만족한다.
    // - started=false이면 previous는 sentinel이고 지금까지 고른 0은 선행 0뿐이다.
    // - tight=true이면 접두사가 limit 접두사와 같아서 현재 자릿수 상한을 따라야 한다.
    [[nodiscard]] long long count_suffix(
        int position,
        int previous,
        bool started,
        bool tight) {
        // [호출 계약: string::size]
        // (1) 수신 digits_는 이 DigitCounter가 소유한 유효한 std::string lvalue다.
        // (2) size_type basic_string::size() const noexcept를 인자 없이 선택한다.
        // (3) 명시 인자와 소유권 이동이 없다.
        // (4) 문자 수 size_type 값을 반환해 int로 변환한 뒤 종료 비교에 사용한다.
        // (5) digits_와 모든 문자·관찰자는 변하지 않는다.
        // (6) 상수 시간, 무할당·무예외·무효화 없음. 최대 19라 int 변환도 안전하다. 같은 string을
        //     다른 스레드가 동시에 변경하지 않아야 하며 이 객체는 한 스레드에 갇혀 있다.
        if (position == static_cast<int>(digits_.size())) {
            // 모든 자리를 선행 0으로 고른 단 하나의 경로가 정수 0을 정확히 한 번 센다.
            return 1;
        }

        // tight=false 상태의 이후 선택은 limit 값과 무관하므로 재사용한다.
        // tight=true 상태는 각 위치에서 한 경로뿐이라 캐시하지 않아 캐시 키에서 상한을 빼도 안전하다.
        const int started_index{started ? 1 : 0};
        if (!tight && cached_[position][previous][started_index]) {
            return memo_[position][previous][started_index];
        }

        int upper_digit{decimal_base - 1};
        if (tight) {
            // [호출 계약: string::operator[]]
            // (1) 수신 digits_는 비어 있지 않은 유효한 std::string lvalue이고
            //     0 <= position < digits_.size()가 위 종료 분기로 보장된다.
            // (2) reference basic_string::operator[](size_type pos) 비-const 오버로드를 선택한다.
            // (3) static_cast<size_t>(position)은 유효 첨자 prvalue이고 문자 소유권을 옮기지 않는다.
            // (4) 저장 문자에 대한 char&를 반환하지만 곧 값으로 읽어 '0'을 빼며 참조를 보관하지 않는다.
            // (5) 문자열과 문자는 변하지 않고 참조·포인터 무효화도 없다.
            // (6) 상수 시간·무할당·무예외다. 범위 밖 첨자는 미정의 동작 위험이 있어 선행 검사가 필수다.
            //     같은 string의 동시 변경은 허용하지 않으며 이 객체는 한 스레드에서만 사용한다.
            upper_digit = digits_[static_cast<std::size_t>(position)] - '0';
        }

        long long ways{};
        // 현재 실제 상한까지 모든 자릿값을 시도한다. 최대 10개라 상태당 상수 규모다.
        for (int digit{0}; digit <= upper_digit; ++digit) {
            const bool next_started{started || digit != 0};

            // 이미 실제 표기가 시작되었다면 바로 앞 실제 자릿수와 같은 선택만 금지한다.
            if (started && digit == previous) {
                continue;
            }

            const int next_previous{next_started ? digit : no_previous_digit};
            const bool next_tight{tight && digit == upper_digit};
            ways += count_suffix(position + 1, next_previous, next_started, next_tight);
        }

        if (!tight) {
            cached_[position][previous][started_index] = true;
            memo_[position][previous][started_index] = ways;
        }
        return ways;
    }

    std::string digits_; // limit의 자릿수 문자를 소유해 재귀 중 주소·수명이 안정적이다.
    // 첫 차원 20은 position 0..19의 안전한 저장 공간, 둘째 11은 0..9와 sentinel,
    // 셋째 2는 started=false/true다. {}로 모든 숫자는 0, bool은 false로 초기화한다.
    long long memo_[maximum_digits + 1][decimal_base + 1][2]{};
    bool cached_[maximum_digits + 1][decimal_base + 1][2]{};
};

// 대표 알고리즘 문서: ../algorithm/digit-dp-no-equal-adjacent.md
// F(limit) = [0, limit] 유효 수 개수다. 음수 prefix는 빈 구간이므로 0을 돌려준다.
[[nodiscard]] long long count_up_to(long long limit) {
    if (limit < 0) {
        return 0;
    }
    // DigitCounter{limit}는 prvalue이고 전체 식 동안 살아 있다. count 결과는 독립 정수 값이다.
    return DigitCounter{limit}.count();
}
} // namespace

/*
우승 노하우와 정확성 요약:
1. 각 재귀 경로는 limit와 같은 길이의 0 포함 자릿수열 하나다. tight가 상한을 넘지 않게 하므로
   경로와 [0, limit] 정수가 일대일 대응한다.
2. started 전의 0은 실제 표기에서 제외하고, started 뒤에는 digit == previous만 배제한다.
   따라서 알고리즘은 정확히 인접 자릿값이 다른 수만 센다.
3. 기저에서 started=false도 1을 반환해야 0을 놓치지 않는다. a=0이면 a-1=-1을 먼저 거른다.
4. D<=19, previous 11개, started/tight 각 2개, 전이 10개이므로 시간은 O(D*10^2),
   공간은 O(D*10)와 재귀 깊이 O(D)다. 단순 [a,b] 반복은 최대 10^18이라 불가능하다.

값 범주·수명·소유권: digits_는 이름 있는 lvalue이자 소유 문자열이고 operator[] 결과는 짧게
빌리는 참조다. std::to_string 결과 prvalue는 C++17 규칙으로 digits_를 중간 string 복사·이동 없이
직접 구성한다. DigitCounter{limit} prvalue는 멤버 count 호출을 위해 임시 객체로 materialize되고
전체 식 끝에 파괴된다. 명시적 xvalue 소유권 이전은 없으며 scalar 결과는 독립 값이다.

기계 실행 관점: memo 접근은 주소 계산 뒤 load/store, 캐시 여부·tight·인접 중복 검사는 비교와
조건 분기, 재귀는 호출/복귀로 구현될 수 있다. 작은 고정 상태는 인라인되거나 분기가 재배치될 수
있으며 실제 명령·스택·캐시·벡터화는 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.
*/

int main() {
    // [첫 호출 계약: std::ios_base::sync_with_stdio]
    // (1) 수신 객체 없는 정적 함수이며 프로그램 시작 시 C/C++ 표준 스트림 동기화는 켜져 있다.
    // (2) static bool std::ios_base::sync_with_stdio(bool sync = true)에 false를 명시한다.
    // (3) false bool prvalue는 설정값이고 버퍼 소유권을 전달하지 않는다.
    // (4) 이전 동기화 상태 bool을 반환하지만 여기서는 의도적으로 버린다.
    // (5) 뒤의 C++ 표준 스트림은 C stdio와 독립적으로 동작할 수 있어 두 계열을 섞은 출력 순서를
    //     별도 조치 없이 보장할 수 없다.
    // (6) 첫 I/O 전에 한 번 수행한다. 표준이 일반 복잡도 상한을 명시하지 않으며 이 한 번의 설정
    //     비용은 구현에 맡겨진다.
    //     스트림 수명·관찰자는 유지되고 직접 예외를 약속하지 않는다. 이후 C stdio와 섞을 때의
    //     순서 오류를 별도 관리하며, 다른 I/O 스레드와 동시에 설정하지 않는다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie]
    // (1) 수신 std::cin은 유효한 std::istream lvalue이며 기본 연결 출력 스트림을 가질 수 있다.
    // (2) std::basic_ostream<char>* std::basic_ios<char>::tie(std::basic_ostream<char>*)를 고른다.
    // (3) nullptr prvalue는 연결 해제를 뜻하고 어떤 stream도 소유하지 않는다.
    // (4) 이전 연결 ostream 포인터를 반환하지만 사용하지 않는다.
    // (5) 이후 입력 전 자동 flush 연결이 해제되며 cin/cout 수명은 유지된다.
    // (6) 표준은 일반 복잡도·할당 상한을 명시하지 않는다. 포인터 수명은 각 표준 stream에 묶이고,
    //     잘못된 tie 순환은 미정의 동작 위험이 있다. 대화형 문제는 수동 flush가 필요할 수 있다.
    //     같은 stream의 tie를 다른 스레드에서 동시에 바꾸지 않는다.
    std::cin.tie(nullptr);

    long long a{};
    long long b{};
    // [첫 호출 계약: istream 정수 추출 operator>>]
    // (1) 수신 std::cin은 입력 가능한 std::istream lvalue, a와 b는 0인 long long lvalue다.
    // (2) basic_istream<char>::operator>>(long long&)를 두 번 연쇄한다.
    // (3) 두 쓰기 가능 참조에 공식 범위 정수를 기록하며 소유권 이동은 없다.
    // (4) 각 호출이 같은 istream&를 반환해 다음 추출에 쓰고 마지막 참조는 버린다.
    // (5) 성공하면 a,b와 입력 위치가 바뀌고 실패하면 상태 비트가 설정된다.
    // (6) 표준은 일반 복잡도·할당 상한을 명시하지 않는다. 구현은 입력 문자를 해석하며 사용자 변수의
    //     참조·포인터를 무효화하지 않는다. 형식/범위/EOF 오류는 failbit로 드러나고, 같은 stream을
    //     다른 스레드에서 동시에 읽지 않는다.
    std::cin >> a >> b;

    const long long answer{count_up_to(b) - count_up_to(a - 1)};

    // [첫 호출 계약: ostream 정수·문자 삽입 operator<<]
    // (1) 수신 std::cout은 출력 가능한 std::ostream lvalue이고 answer는 살아 있는 const long long이다.
    // (2) ostream::operator<<(long long) 뒤 비멤버 operator<<(std::ostream&, char)를 선택한다.
    // (3) answer lvalue의 값을 읽고 '\n' char prvalue를 출력하며 DP 저장소를 빌리지 않는다.
    // (4) 첫 호출의 ostream&는 다음 호출 수신자로 쓰고 마지막 반환 참조는 버린다.
    // (5) 출력 버퍼·상태 비트가 바뀌며 answer는 그대로다.
    // (6) 표준은 일반 복잡도·할당 상한을 명시하지 않는다. 실패는 상태 비트 또는 exception mask의
    //     예외로 나타난다. 같은 출력 스트림을 여러 스레드에서 복합 식으로 갱신하려면 동기화가 필요하다.
    std::cout << answer << '\n';
}
