/*
문제 ID·제목: CSES 2417 - Counting Coprime Pairs
공식 출처: https://cses.fi/problemset/task/2417/

문제 요약: 양의 정수 n개에서 서로 다른 두 위치 i<j를 골랐을 때 두 수의 최대공약수가
1인 쌍의 개수를 구한다. 같은 값이 여러 번 나와도 위치가 다르면 서로 다른 쌍이다.
입력: 첫 줄에 n, 다음 줄에 n개의 양의 정수 x_i가 주어진다.
출력: 서로소인 위치 쌍의 개수 하나를 출력한다.
제약: 1 <= n <= 100,000, 1 <= x_i <= 1,000,000. 시간 1초, 메모리 512MB.

공식 예제 입력:
8
5 4 20 1 16 17 5 15
공식 예제 출력:
19
*/

// <iostream>은 표준 입력·출력 객체, 정수 추출·삽입 연산, 입출력 동기화 설정을 선언한다.
#include <iostream>

namespace {
// int는 값·빈도·최소 소인수를 담는다. 최대 빈도 100,000은 int에 안전하다.
// constexpr는 컴파일 시 확정되는 정수 상수이고 배열 크기와 공식 입력 상한을 묶는다.
constexpr int value_limit{1'000'000};

// 정적 저장 기간 배열은 main 이전에 0으로 초기화되고 프로그램 종료까지 살아 있다.
// 동적 할당 실패와 스택 초과를 피하며, 입력의 실제 최댓값까지만 순회한다.
int frequency[value_limit + 1]{};
int smallest_prime[value_limit + 1]{};
int primes[value_limit + 1]{};
// Möbius 값은 -1, 0, 1뿐이라 signed char 하나로 충분하다. 곱셈 때는 정수로 승격된다.
signed char mobius[value_limit + 1]{};
}  // namespace

/*
값 범주·수명·소유권: frequency[x]는 정적 배열 원소를 가리키는 int lvalue이고,
count * (count - 1) / 2는 임시 long long prvalue다. 이동 대상 xvalue는 없다.
동적 소유 객체를 반환하지 않아 복사·이동·RVO는 이 풀이의 병목과 무관하다.
배열 전체는 프로그램 종료까지 살아 있으며 원소 주소가 무효화되지 않는다.
실행 관점: 첨자 읽기는 메모리 로드, 빈도 증가는 읽기/쓰기, 소수·배수 검사는
비교와 조건 분기로 구현될 수 있다. 실제 명령·벡터화·캐시는 CPU, 컴파일러,
최적화 옵션에 따라 달라져 특정 어셈블리 형태를 보장하지 않는다.
*/

// 구현 가까운 대표 알고리즘 문서: ../algorithm/mobius-inversion-coprime-pairs.md
// main은 운영체제에 종료 상태를 돌려주는 int 반환 함수다. 매개변수 없이 표준 입력을 사용한다.
int main() {
    // [첫 호출 계약: std::ios_base::sync_with_stdio]
    // (1) 수신 객체 없는 정적 함수이며 표준 C++ 스트림과 C stdio 동기화는 시작 시 켜져 있다.
    // (2) static bool std::ios_base::sync_with_stdio(bool sync = true)의 bool 인자 호출이다.
    // (3) false는 bool prvalue를 값 전달하며 버퍼 소유권을 이전하지 않는다.
    // (4) 이전 동기화 상태 bool을 반환하나 여기서는 사용하지 않는다.
    // (5) 뒤따르는 C++ 표준 스트림 I/O는 C stdio와 문자 단위 동기화를 하지 않는다.
    // (6) 첫 I/O 전에 수행한다. 설정 자체 O(1), 배열 참조 무효화 없음; 구현 내부 자원
    //     처리는 구현별일 수 있다. C stdio와 섞을 때 순서 보장은 따로 관리해야 한다.
    //     스레드 안전을 위해 설정은 다른 I/O 스레드가 생기기 전 단일 스레드에서 한다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie]
    // (1) 수신 std::cin은 살아 있는 std::istream lvalue이며 기본적으로 cout에 묶여 있다.
    // (2) std::basic_ostream<char>* std::basic_ios<char>::tie(std::basic_ostream<char>*)
    //     setter 오버로드를 택한다.
    // (3) nullptr는 null 포인터 prvalue로 값 전달되며 출력 스트림 소유권을 넘기지 않는다.
    // (4) 이전에 묶인 ostream 포인터를 반환하지만 사용하지 않는다.
    // (5) 이후 입력 전에 cout을 자동 flush하는 묶음이 해제되고 두 스트림 수명은 유지된다.
    // (6) O(1), 배열·반복자 무효화 없음, 일반적으로 할당 없음. 대화형 문제라면 수동
    //     flush가 필요하지만 여기는 배치 입력이다. 공유 스트림 설정을 동시에 바꾸지 않는다.
    std::cin.tie(nullptr);

    // 중괄호 초기화는 읽기 실패 시에도 n을 0으로 둔다. 공식 입력은 항상 유효하다고 가정한다.
    int n{};
    int max_value{};
    // [첫 호출 계약: istream 정수 추출 operator>>(int&)]
    // (1) 수신 std::cin은 입력 가능한 std::istream lvalue, n은 초기값 0의 int lvalue다.
    // (2) basic_istream<char>& basic_istream<char>::operator>>(int&) 오버로드다.
    // (3) n을 쓰기 가능한 int&로 빌려 주며 소유권은 그대로다. 허용 입력은 [1,100000]이다.
    // (4) 스트림 자신에 대한 std::istream&를 반환하나 이 식에서는 버린다.
    // (5) 성공 시 n에 값을 기록하고 입력 위치를 이동한다. 실패 시 스트림 상태 비트가 바뀐다.
    // (6) 자릿수에 선형, 보통 사용자 배열 할당·무효화 없음. EOF/형식 오류는 failbit로
    //     드러나며 유효한 저지 입력을 전제로 한다. 같은 스트림의 동시 읽기는 하지 않는다.
    std::cin >> n;

    // for는 정확히 n개의 값을 한 번씩 읽는다. x는 매 회 새로 초기화되는 지역 int다.
    for (int index{0}; index < n; ++index) {
        int x{};
        // 같은 int& 추출 오버로드: x lvalue에 공식 범위 [1,1000000]의 수를 기록한다.
        std::cin >> x;
        // 동일한 수가 여러 위치에 있으면 그 횟수를 보존해야 C(cnt,2)가 올바르다.
        ++frequency[x];
        if (x > max_value) {
            max_value = x;
        }
    }

    // μ(1)=1. 선형 체의 불변식: p=smallest_prime[x]까지의 소수 p로만 x*p를
    // 만들면 각 합성수의 최소 소인수를 처음이자 단 한 번 확정한다.
    mobius[1] = 1;
    int prime_count{};
    for (int x{2}; x <= max_value; ++x) {
        if (smallest_prime[x] == 0) {
            smallest_prime[x] = x;
            primes[prime_count] = x;
            ++prime_count;
            mobius[x] = -1;
        }

        // x*p <= max_value를 나눗셈으로 확인해 곱셈의 범위 초과 가능성을 제거한다.
        // p가 x의 최소 소인수이면 p² | x*p라 μ=0; 그렇지 않으면 부호가 뒤집힌다.
        for (int j{0}; j < prime_count; ++j) {
            const int p{primes[j]};
            if (p > smallest_prime[x] || x > max_value / p) {
                break;
            }
            const int composite{x * p};
            smallest_prime[composite] = p;
            if (p == smallest_prime[x]) {
                mobius[composite] = 0;
                break;
            }
            mobius[composite] = static_cast<signed char>(-mobius[x]);
        }
    }

    // count[d]는 d의 배수인 입력 원소 수다. gcd가 1인 쌍의 지시 함수는
    // Σ_{d|gcd(a,b)} μ(d)이므로 Σ_d μ(d)*C(count[d],2)가 답이다.
    // d=1은 모든 원소를 세며, μ(d)=0인 제곱인수 포함 d는 곧바로 건너뛴다.
    long long answer{};
    for (int divisor{1}; divisor <= max_value; ++divisor) {
        if (mobius[divisor] == 0) {
            continue;
        }
        long long divisible_count{};
        for (int multiple{divisor}; multiple <= max_value; multiple += divisor) {
            divisible_count += frequency[multiple];
        }
        // 가장 큰 C(100000,2)=4,999,950,000은 int 범위를 넘는다.
        // 첫 곱셈부터 long long으로 계산하여 항과 누적합을 안전하게 유지한다.
        answer += static_cast<long long>(mobius[divisor]) *
                  (divisible_count * (divisible_count - 1) / 2);
    }

    // [첫 호출 계약: ostream 정수·문자 삽입 operator<<]
    // (1) 수신 std::cout은 출력 가능한 std::ostream lvalue, answer는 계산된 long long이다.
    // (2) 정수는 basic_ostream<char>::operator<<(long long), '\n'은
    //     비멤버 operator<<(std::basic_ostream<char>&, char) 오버로드를 선택한다.
    // (3) answer는 long long lvalue에서 값으로 읽혀 [0,C(n,2)]을 출력한다. '\n'은
    //     char prvalue 값이다. 배열·버퍼 소유권을 넘기지 않는다.
    // (4) 첫 연산은 ostream&를 반환해 다음 삽입의 수신자로 쓰고, 마지막 참조는 버린다.
    // (5) 출력 버퍼에 정수 문자와 줄바꿈이 추가된다. I/O 실패 시 상태 비트가 바뀐다.
    // (6) 출력 자릿수에 선형, 내부 버퍼 처리는 구현별이며 입력 배열 참조는 무효화하지
    //     않는다. 기본 예외 설정에서는 오류가 failbit/badbit로 남는다. 동시 출력은 없다.
    std::cout << answer << '\n';
}
