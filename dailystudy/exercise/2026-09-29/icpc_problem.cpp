/*
문제 요약 — CSES 1110 "Minimal Rotation"
출처: https://cses.fi/problemset/task/1110/

길이 n인 소문자 문자열의 회전은 앞 문자를 하나씩 뒤로 옮겨 만들 수 있다.
가능한 n개 회전 가운데 사전순으로 가장 작은 문자열 하나를 출력한다.

입력: 한 줄에 'a'부터 'z'까지로만 이루어진 길이 n의 문자열이 주어진다.
출력: 사전순 최소 회전을 한 줄에 출력한다.
제약: 1 <= n <= 1,000,000. 제한 시간이 짧으므로 모든 회전을 만들어 정렬하는 O(n^2 log n)은 불가능하다.
예제: acab -> abac.

풀이: 두 생존 후보와 공통 접두사 길이를 유지하는 Booth의 최소 표현 알고리즘을 쓴다.
구현 가까운 대표 문서: ../algorithm/booth-minimal-rotation.md
*/

#include <cstddef>  // std::size_t: 문자열 길이와 회전 시작 위치를 표현한다.
#include <iostream> // std::cin/std::cout: 문제 문자열을 읽고 정답을 출력한다.
#include <string>   // std::string: 입력, 두 배 문자열, 정답 문자를 소유한다.

// using은 긴 의미를 짧게 드러내는 별칭일 뿐 std::size_t와 구별되는 새 타입을 만들지 않는다.
using RotationIndex = std::size_t;

// doubled는 text+text를 한 번만 만든 소유 문자열이고 n은 원문 길이다. 반환값은 0-based 시작 위치다.
[[nodiscard]] RotationIndex minimal_rotation_index(
    const std::string& doubled,
    const std::size_t n) noexcept {
    std::size_t first{};
    std::size_t second{1U};
    std::size_t matched{};

    while (first < n && second < n && matched < n) {
        // [첫 호출 계약: const string::operator[]]
        // (1) 수신 doubled는 const로 관찰되는 2n자 std::string이고 루프 중 변경되지 않는다.
        // (2) const_reference operator[](size_type position) const가 두 번 선택된다.
        // (3) first+matched와 second+matched는 std::size_t prvalue다. 각 후보<n, matched<n이므로 둘 다 2n-2 이하이다.
        // (4) 각 호출은 const char&를 반환하고 즉시 char 값 left/right에 복사해 참조를 저장하지 않는다.
        // (5) doubled와 인덱스, 소유권은 변하지 않는다.
        // (6) 접근마다 O(1)·무할당·무예외다. 범위 밖은 전제조건 위반이고 동시 수정은 금지된다.
        const char left{doubled[first + matched]};
        const char right{doubled[second + matched]};

        if (left == right) {
            ++matched;
            continue;
        }

        // 불변식: 두 후보의 [0, matched) 접두사는 같다. 첫 불일치에서 더 큰 후보뿐 아니라
        // 그 후보부터 matched칸 뒤까지 모두 최소가 될 수 없으므로 matched+1만큼 한 번에 버린다.
        if (left > right) {
            first += matched + 1U;
            if (first == second) {
                ++first;
            }
        } else {
            second += matched + 1U;
            if (first == second) {
                ++second;
            }
        }
        matched = 0U;
    }

    // matched==n이면 주기 때문에 두 회전 문자열이 같고 어느 시작점을 골라도 출력은 동일하다.
    return first < second ? first : second;
}

[[nodiscard]] std::string minimal_rotation(const std::string& text) {
    // [첫 호출 계약: string::size]
    // (1) 수신 text는 호출자가 소유한 살아 있는 const std::string이며 호출 동안 수정되지 않는다.
    // (2) constexpr size_type size() const noexcept가 선택되고 데이터 인자는 없다.
    // (3) 매개변수와 소유권 이전은 없다.
    // (4) 문자 수를 string::size_type 값으로 반환해 n에 사용한다.
    // (5) text의 문자·크기·capacity·관찰자는 변하지 않는다.
    // (6) O(1), 무할당·무예외·무효화 없음이다. 동시 변경 없이 읽기만 해야 한다.
    const std::size_t n{text.size()};

    // [첫 생성 계약: std::string 복사 생성자]
    // (1) doubled는 생성 전이고 text는 유효한 const std::string lvalue다.
    // (2) std::string(const std::string&) 복사 생성자가 선택된다.
    // (3) 인자 text를 const 참조로 빌려 모든 문자를 읽고 소유권은 넘기지 않는다.
    // (4) 생성자는 반환값 없이 text와 같은 문자를 독립 소유하는 doubled를 만든다.
    // (5) 성공 뒤 doubled 변경은 text에 영향을 주지 않고 text 관찰자는 유지된다.
    // (6) O(n) 시간·공간이며 할당/길이 실패가 가능하다. 실패 시 text는 그대로이고 완성 doubled는 없다.
    std::string doubled{text};

    // [첫 호출 계약: string::operator+=]
    // (1) 수신 doubled는 text의 n자를 소유한 수정 가능 std::string이고 text는 별도 const 문자열이다.
    // (2) constexpr std::string& operator+=(const std::string& source)가 선택된다.
    // (3) source 식 text는 const std::string lvalue로 빌려 읽고 소유권을 넘기지 않는다.
    // (4) 갱신된 doubled를 가리키는 std::string&를 반환하지만 이 문장에서는 버린다.
    // (5) 성공 뒤 doubled는 text+text의 2n자를 소유하고 text는 변하지 않는다. 기존 doubled 관찰자는 무효일 수 있다.
    // (6) 표준은 별도 점근 복잡도 상한을 두지 않는다. n<=10^6이라 길이 한계 안이지만 재할당·bad_alloc은
    //     가능하고, 정상 예외에는 doubled에 다른 효과가 없다. 자체 동기화가 없어 같은 객체의 동시 접근은 금지한다.
    doubled += text;

    const std::size_t start{minimal_rotation_index(doubled, n)};

    // [첫 호출 계약: string::substr]
    // (1) 수신 doubled는 2n자를 소유한 const로 관찰되는 std::string이고 start<n, n>=1이다.
    // (2) C++23의 constexpr std::string substr(size_type position, size_type count) const &가 선택된다.
    // (3) start와 n은 std::size_t lvalue를 값으로 복사하며 현재 start<=size, n은 남은 범위 안이다.
    // (4) [start,start+n)의 문자를 독립 소유한 std::string prvalue를 반환해 함수 결과를 직접 초기화한다.
    // (5) doubled와 두 인자는 변하지 않고 새 문자열만 생긴다. 반환 문자열은 doubled 수명과 독립적이다.
    // (6) 결과 문자 수에 선형인 시간·공간과 bad_alloc 가능성이 있다. position>size는 전제 위반이 아니라
    //     정의된 out_of_range 경로지만 현재 불변식이 막는다. 수신 관찰자는 유지되고 공유 변경은 금지한다.
    return doubled.substr(start, n);
}

int main() {
    // [첫 호출 계약: std::ios::sync_with_stdio]
    // (1) 수신 객체가 없는 정적 설정 함수이며 첫 표준 I/O 전에 호출한다.
    // (2) 호출 식의 ios 별칭을 통해 상속 접근한 선언 주체 std::ios_base::sync_with_stdio의 static bool(bool)가 선택된다.
    // (3) false는 C와 C++ 표준 스트림 동기화를 끄는 bool prvalue이며 소유권 의미가 없다.
    // (4) 이전 동기화 상태 bool을 반환하지만 사용하지 않는다.
    // (5) 표준 C++ 스트림은 계속 유효하나 C stdio와 섞을 때 순서를 별도로 보장해야 한다.
    // (6) 표준이 복잡도를 명시하지 않으며 할당/관찰자 수명 변화는 없다. 첫 I/O 뒤 호출 효과는 구현 정의다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie]
    // (1) 수신 std::cin은 살아 있는 입력 스트림이고 기본적으로 출력 스트림에 tie될 수 있다.
    // (2) std::ostream* tie(std::ostream* tied_stream) overload가 선택된다.
    // (3) nullptr는 null pointer prvalue이며 새 소유권을 만들지 않는다.
    // (4) 이전 tied stream 포인터를 반환하지만 사용하지 않는다.
    // (5) cin은 자동 flush 대상 없이 유지되고 cout 수명·소유권은 변하지 않는다.
    // (6) 표준이 복잡도를 명시하지 않고 무효화·할당은 없다. 동시 설정 변경은 외부 동기화가 필요하다.
    std::cin.tie(nullptr);

    // [첫 생성 계약: std::string 기본 생성자]
    // (1) input은 생성 전 지역 객체다.
    // (2) std::string() 기본 생성자가 선택되고 명시 인자는 없다.
    // (3) 기본 allocator 정책 외 데이터 인자는 없다.
    // (4) 반환값 없이 빈 문자열을 만든다.
    // (5) 성공 뒤 input은 size()==0인 유효한 소유 객체다.
    // (6) 일반 컨테이너 요구상 O(1)이고 기본 allocator에서 noexcept다. 빈 표현/실제 할당은 구현 세부이며
    //     input은 지역 수명을 따르고 아직 공유되지 않아 thread 충돌이 없다.
    std::string input{};

    // [첫 호출 계약: std::cin >> std::string]
    // (1) 수신 std::cin은 유효한 std::istream이고 input은 수정 가능한 빈 std::string이다.
    // (2) operator>>(std::istream&, std::string&) 비멤버 overload가 선택된다.
    // (3) 첫 인자는 std::cin lvalue, 둘째는 input lvalue이며 input이 추출 문자를 소유하게 된다.
    // (4) 같은 std::istream& lvalue를 반환하고 이어지는 basic_ios::operator!의 수신자로 즉시 사용한다.
    // (5) 성공 시 다음 공백 전 문자열을 input이 소유하고 cin 위치/상태가 진행된다. 실패 시 상태 비트가 설정된다.
    // (6) 표준은 형식 추출에 단일 점근 복잡도 상한을 두지 않는다. 소비 문자·locale·장치 비용과 문자열 할당
    //     실패가 가능하고 기존 input 관찰자는 무효일 수 있다. 같은 stream의 동시 입력은 금지한다.
    // [첫 호출 계약: basic_ios::operator!]
    // (1) 수신은 추출이 반환한 std::istream lvalue이며 std::cin과 같은 살아 있는 객체이고 방금 상태가 갱신됐다.
    // (2) 상속된 bool basic_ios::operator!() const 멤버가 unary !의 overload resolution으로 직접 선택된다.
    // (3) 명시 데이터 인자는 없고 수신 stream의 소유권도 옮기지 않는다.
    // (4) fail()과 같은 bool 값을 반환하며 failbit 또는 badbit가 있으면 true이고 if 조건에 실제 사용한다.
    // (5) stream의 상태·버퍼·입력 위치와 input은 이 관찰 호출로 더 바뀌지 않고 참조도 무효화되지 않는다.
    // (6) 표준은 별도 점근 복잡도를 명시하지 않는다. 이 관찰은 할당하지 않으며 같은 stream의 동시 접근은
    //     외부에서 막는다. 예외 mask에 따른 오류는 앞선 추출에서 이미 발생할 수 있다.
    if (!(std::cin >> input)) {
        return 0;
    }

    const std::string answer{minimal_rotation(input)};

    // [첫 호출 계약: std::cout << 정답 << '\n']
    // (1) 수신 std::cout은 살아 있는 std::ostream이고 정상 상태라고 가정한다.
    // (2) std::string 비멤버 operator<< 뒤 operator<<(std::ostream&, char)가 선택된다.
    // (3) answer는 const std::string lvalue, '\n'은 char prvalue이며 둘 다 읽기 입력이고 소유권을 넘기지 않는다.
    // (4) 각 호출은 같은 std::ostream&를 반환해 연쇄하고 마지막 반환 참조는 버린다.
    // (5) answer는 유지되고 cout의 문자/상태만 갱신되며 줄바꿈은 강제 flush가 아니다.
    // (6) 표준은 형식 출력 전체에 단일 점근 복잡도 상한을 두지 않는다. 생성 문자·locale·장치 비용이 들고
    //     버퍼/장치 실패와 설정 시 예외가 가능하다. 레코드 단위 동시 원자성은 없다.
    std::cout << answer << '\n';
    return 0;
}
