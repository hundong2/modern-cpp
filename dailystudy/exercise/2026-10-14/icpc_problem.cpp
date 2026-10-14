/*
문제 ID/제목: CSES 2105 — Distinct Substrings
출처: https://cses.fi/problemset/task/2105/

문제 요약:
소문자 a~z로만 이루어진 문자열에서 서로 다른 연속 부분 문자열의 개수를 센다. 같은 문자 내용이
여러 위치에 나타나도 한 번만 센다. 저작권이 있는 원문 전체 대신 구현에 필요한 조건만 요약한다.

입력: 길이 n인 문자열 한 줄. 1 <= n <= 100,000, 각 문자는 a~z다.
출력: 서로 다른 부분 문자열의 개수 하나.
예제: 입력 abaa, 출력 8.

아래 구현은 suffix automaton을 온라인으로 만들고 각 상태가 새로 대표하는 길이 구간을 합한다.
긴 강의와 정확성 증명은 공용 문서에 두고, 구현부에는 대회 중 지켜야 할 불변식을 가까이 적는다.
*/

// <array>는 알파벳 26개 전이를 고정 크기 연속 저장하는 std::array를 선언한다.
#include <array>
// <cstddef>는 문자열 크기와 인덱스에 쓰는 부호 없는 std::size_t를 선언한다.
#include <cstddef>
// <cstdint>는 최대 약 5e9인 답을 담는 std::int64_t를 선언한다.
#include <cstdint>
// <iostream>은 표준 입력·출력 객체 std::cin, std::cout과 stream 연산을 선언한다.
#include <iostream>
// <string>은 입력 문자를 직접 소유하는 std::string을 선언한다.
#include <string>
// <vector>는 최대 2n 상태를 연속 저장하는 std::vector를 선언한다.
#include <vector>

namespace {

constexpr std::size_t alphabet_size{26U};
constexpr int absent_state{0};
constexpr int root_state{1};

// 상태 v는 원점에서 v에 도달하는 문자열들의 endpos 동치류를 대표한다.
struct State {
    // longest는 이 상태가 대표하는 가장 긴 문자열 길이, suffix_link는 가장 긴 proper suffix 상태다.
    int longest{};
    int suffix_link{};

    // [첫 생성 계약: std::array<int, 26> 값 초기화]
    // (1) 수신 next는 State 안에서 아직 생성되지 않은 정확한 array<int,26> 멤버다.
    // (2) array는 aggregate이며 {}가 26개 int 원소를 값 초기화하는 내장 aggregate 초기화를 선택한다.
    // (3) 명시 데이터 인자는 없고 template 인자는 원소 int와 크기 26이다. 외부 저장소를 빌리지 않는다.
    // (4) 함수 반환값은 없으며 aggregate 초기화 결과 모든 원소가 0인 완성된 array 객체가 된다.
    // (5) 0은 전이 부재 sentinel이며 다른 State나 입력은 변하지 않는다.
    // (6) 정확히 26개 int 초기화라 O(26)=O(1), 무할당·무예외다. iterator/reference는 아직 없고
    //     수명은 State와 같으며 같은 원소의 동시 쓰기는 별도 동기화가 필요하다.
    std::array<int, alphabet_size> next{};
};

class SuffixAutomaton {
public:
    // explicit은 문자열 길이 정수 하나가 automaton으로 암시 변환되는 것을 막는다.
    // [첫 생성 계약: std::vector<State>(count)]
    // (1) 수신 states_는 아직 생성 전인 vector<State> 멤버다.
    // (2) 선택 overload는 explicit vector(size_type count)이고 allocator는 기본 template 인자다.
    // (3) count는 max_length*2+1의 size_type prvalue다. n<=100000이라 곱셈이 size_t 범위 안이고,
    //     외부 원소 저장소나 소유권을 받지 않는다.
    // (4) 생성자는 반환값 없이 count개의 값 초기화된 State를 소유하는 vector를 완성한다.
    // (5) root=1과 앞으로 쓸 모든 슬롯이 0으로 초기화되고 max_length 인자는 변하지 않는다.
    // (6) O(count*26) 시간·O(count*26) 공간이며 할당/길이 오류가 가능하다. 실패 시 부분 원소를
    //     정리한다. 생성 직후 외부 관찰자는 없고 객체를 다른 스레드와 공유하지 않는다.
    explicit SuffixAutomaton(const std::size_t max_length)
        : states_(max_length * 2U + 1U) {}

    // 문자를 하나 붙인 모든 접미사를 반영한다. 입력 계약상 character는 'a'..'z'다.
    void extend(const char character) noexcept {
        const std::size_t symbol{static_cast<std::size_t>(character - 'a')};
        const int current{++state_count_};

        // [첫 호출 계약: vector::operator[]와 array::operator[]]
        // (1) states_는 생성 때 충분한 슬롯을 가진 vector<State>이고 각 next는 array<int,26>이다.
        // (2) vector의 reference operator[](size_type)와 array의 reference operator[](size_type)가 선택된다.
        // (3) 상태 인덱스는 [1,state_count_]이고 symbol은 입력 제약으로 [0,25]다. 값 인자이며 소유권은 없다.
        // (4) 두 호출은 각각 State&와 int&를 반환해 읽기·대입에 사용한다.
        // (5) 대입이 있는 식만 대상 상태를 바꾸며 컨테이너 크기/capacity와 다른 원소는 그대로다.
        // (6) 둘 다 O(1)·무할당이나 표준 시그니처는 noexcept를 명시하지 않고 범위도 검사하지 않는다.
        //     vector 인덱스가 실제 size보다 작고 array 인덱스가 26보다 작도록, 생성한 2n+1개 원소와
        //     소문자 입력 불변식으로 보장한다. 재할당이 없어 참조는 유지되며 동시 수정은 지원하지 않는다.
        states_[current].longest = states_[last_state_].longest + 1;

        int cursor{last_state_};
        // 불변식: 아직 character 전이가 없는 suffix-link 조상에만 current 전이를 추가한다.
        while (cursor != absent_state && states_[cursor].next[symbol] == absent_state) {
            states_[cursor].next[symbol] = current;
            cursor = states_[cursor].suffix_link;
        }

        if (cursor == absent_state) {
            // 모든 접미사가 새 문자에서 끊기면 current의 가장 긴 proper suffix는 빈 문자열 root다.
            states_[current].suffix_link = root_state;
        } else {
            const int target{states_[cursor].next[symbol]};
            if (states_[cursor].longest + 1 == states_[target].longest) {
                // 길이가 정확히 한 칸 이어지면 기존 target을 suffix link로 바로 쓸 수 있다.
                states_[current].suffix_link = target;
            } else {
                const int clone{++state_count_};

                // [첫 호출 계약: State 복사 대입과 std::array 복사 대입]
                // (1) 왼쪽 states_[clone]은 유효한 기본 상태이고 오른쪽 states_[target]은 완성 상태다.
                // (2) 암시적으로 정의된 State::operator=(const State&)가 선택되고 그 안에서
                //     array<int,26>::operator=(const array&)가 26개 전이를 복사한다.
                // (3) 오른쪽 states_[target]은 수정 가능한 State lvalue지만 const State& 매개변수에
                //     바인딩되어 읽기만 하고, 왼쪽은 수정 가능한 lvalue다. 소유권 공유나 이동은 없다.
                // (4) State 대입은 State&를 반환하지만 문장 결과를 버린다. array 대입 반환도 내부에서만 쓰인다.
                // (5) clone은 target의 전이와 link를 복사한 뒤 longest만 cursor.longest+1로 줄인다.
                // (6) O(26)=O(1), 무할당이며 int/array 복사는 noexcept다. self-assignment가 아니고 참조
                //     무효화도 없다. 동일 상태를 다른 스레드가 수정하지 않는 단일 스레드 풀이를 전제한다.
                states_[clone] = states_[target];
                states_[clone].longest = states_[cursor].longest + 1;

                // target으로 가던 exactly-character 전이만 clone으로 돌린다. 다른 전이는 보존한다.
                while (cursor != absent_state && states_[cursor].next[symbol] == target) {
                    states_[cursor].next[symbol] = clone;
                    cursor = states_[cursor].suffix_link;
                }

                // clone은 endpos 집합을 나누는 구조 상태라 새 terminal 문자를 직접 추가한 상태가 아니다.
                states_[target].suffix_link = clone;
                states_[current].suffix_link = clone;
            }
        }

        last_state_ = current;
    }

    [[nodiscard]] std::int64_t distinct_substring_count() const noexcept {
        std::int64_t answer{};
        // [첫 호출 계약: const vector::operator[](state/link)]
        // (1) 수신 states_는 완성된 const vector<State> lvalue이고 모든 사용 상태와 suffix link가 유효하다.
        // (2) 두 식 모두 const_reference operator[](size_type pos) const overload를 선택한다.
        // (3) state는 [2,state_count_], link는 [1,state_count_]의 int lvalue이며 size_type 값으로 변환된다.
        //     둘 다 vector가 소유한 원소를 비소유 관찰할 뿐 소유권을 옮기지 않는다.
        // (4) 각 호출은 const State&를 반환해 longest 값을 읽는 데 사용하고 참조를 저장하지 않는다.
        // (5) vector의 크기·capacity·원소와 두 인자는 호출 뒤 그대로다.
        // (6) O(1)·무할당이며 표준 시그니처에 noexcept와 범위 검사는 없다. 두 인덱스가 size보다 작다는
        //     automaton 불변식이 깨지면 UB다. 구조 변경이 없어 참조는 식 끝까지 유효하고 동시 쓰기는 없다.
        // 상태 v가 새로 기여하는 길이는 (longest[link[v]], longest[v]]의 정수 개수다.
        for (int state{root_state + 1}; state <= state_count_; ++state) {
            const int link{states_[state].suffix_link};
            answer += static_cast<std::int64_t>(
                states_[state].longest - states_[link].longest);
        }
        return answer;
    }

private:
    // 1-based 상태 인덱스로 0을 “없음” sentinel에 양보한다. 모든 n>=1에서 상태 수는 최대 2n이고,
    // n>=2일 때 더 타이트한 상한은 2n-1이다. 생성 때 확보한 2n+1 슬롯이면 마지막 인덱스도 안전하다.
    std::vector<State> states_;
    int state_count_{root_state};
    int last_state_{root_state};
};

} // namespace

int main() {
    // [첫 호출 계약: std::ios::sync_with_stdio(false)]
    // (1) 인스턴스 수신자가 없는 정적 stream 설정이며 표준 stream I/O 전 상태다.
    // (2) bool sync_with_stdio(bool sync=true)가 false 인자 하나로 선택된다.
    // (3) false는 C stdio와 C++ iostream 동기화를 끄는 bool prvalue이며 소유권 의미가 없다.
    // (4) 이전 동기화 설정 bool을 반환하지만 오늘은 사용하지 않는다.
    // (5) 이후 표준 C++ stream이 독립 버퍼링될 수 있고 입력 문자열 등은 아직 없다.
    // (6) 표준은 별도 복잡도 상한/noexcept를 명시하지 않는다. I/O 뒤 호출 효과는 구현 정의이므로
    //     반드시 첫 I/O 전에 호출한다. 이후 같은 stream의 무동기 병행 접근을 하지 않는다.
    std::ios::sync_with_stdio(false);

    // [첫 호출 계약: std::cin.tie(nullptr)]
    // (1) 수신 cin은 프로그램 수명의 유효한 istream lvalue이고 현재 cout에 tie되어 있을 수 있다.
    // (2) ostream* basic_ios::tie(ostream* tied)가 선택된다.
    // (3) nullptr는 어느 stream도 소유하지 않는 null pointer 값이며 tie 순환을 만들지 않는다.
    // (4) 이전 tie 포인터를 반환하지만 사용하지 않는다.
    // (5) cin 입력 전 cout 자동 flush 연결이 해제되고 두 stream과 buffer 수명은 그대로다.
    // (6) 표준은 별도 복잡도/noexcept 상한을 두지 않는다. non-null 순환 tie는 UB지만 nullptr는 안전하다.
    //     전역 stream 설정을 다른 스레드가 동시에 바꾸지 않는다.
    std::cin.tie(nullptr);

    // [첫 생성 계약: std::string 기본 생성자]
    // (1) 수신 text는 아직 생성되지 않은 지역 string 객체다.
    // (2) basic_string() 기본 생성자가 선택되고 문자·traits·allocator 기본 template 인자를 쓴다.
    // (3) 명시 인자가 없으며 외부 문자 저장소를 빌리지 않는다.
    // (4) 생성자는 반환값 없이 유효한 빈 소유 문자열을 완성한다.
    // (5) text.size()==0이고 stream 상태는 변하지 않는다.
    // (6) 기본 allocator에서 O(1)·noexcept이며 보통 무할당이다. 관찰자는 아직 없고 지역 수명을 가진다.
    std::string text{};

    // [첫 호출 계약: std::cin >> text와 basic_ios::operator!]
    // (1) 수신 cin은 유효한 istream lvalue이고 text는 수정 가능한 빈 string lvalue다.
    // (2) string용 operator>>(istream&, string&) 뒤 실패 검사의 operator!() const가 선택된다.
    // (3) text는 non-const lvalue 참조로 전달되어 공백 전 문자를 소유한다. 소유권 이전은 없다.
    // (4) 추출은 같은 istream&를 반환하고 operator!은 fail 상태 bool을 반환해 if가 사용한다.
    // (5) 성공하면 text가 입력 토큰을 소유한다. 실패하면 상태 비트가 설정되고 조건에서 즉시 종료한다.
    // (6) formatted 입력 전체 복잡도에는 표준 공통 점근 상한이 없다. 문자열 성장의 할당 실패나 예외 mask에
    //     따른 stream 예외가 가능하고, 성장 중 text의 기존 관찰자는 무효지만 아직 없다. 동기화를 끈
    //     cin을 다른 스레드와 공유하지 않는다.
    if (!(std::cin >> text)) {
        return 0;
    }

    // [첫 호출 계약: string::size()]
    // (1) 수신 text는 길이 1..100000의 유효한 수정 가능 string lvalue지만 const-qualified 관찰만 한다.
    // (2) size_type size() const noexcept가 선택된다.
    // (3) 데이터 인자는 없고 문자 소유권을 이동하거나 빌리지 않는다.
    // (4) 문자 수 size_type prvalue를 반환해 SuffixAutomaton 생성 인자로 사용한다.
    // (5) text의 문자·capacity·관찰자는 그대로다.
    // (6) O(1), 무할당, noexcept, 무효화 없음이다. 동시 수정 중 읽기는 데이터 경쟁이지만 단일 스레드다.
    SuffixAutomaton automaton{text.size()};

    // [첫 호출 계약: string::operator[](index)]
    // (1) 수신 text는 입력을 소유하는 수정 가능한 string lvalue지만 이 식에서는 읽기만 한다.
    // (2) reference operator[](size_type pos)가 선택된다.
    // (3) index는 loop 불변식으로 [0,text.size())의 size_t 값이며 소유권 의미가 없다.
    // (4) char&를 반환하고 extend의 char 값 매개변수로 복사해 사용한다.
    // (5) text와 문자는 바뀌지 않고 automaton만 갱신된다.
    // (6) O(1)·무할당이나 표준 시그니처에는 noexcept가 명시되지 않는다. pos>size()는 UB이고
    //     pos==size()는 null 문자지만 오늘은 둘 다 loop 조건으로 제외한다. text를 구조 변경하지 않아
    //     참조 수명도 유지되고 같은 문자열에 대한 동시 쓰기는 없다.
    for (std::size_t index{}; index < text.size(); ++index) {
        automaton.extend(text[index]);
    }

    // 구현 가까운 대표 문서: ../algorithm/suffix-automaton-distinct-substrings.md
    // [첫 호출 계약: std::cout << int64_t << char]
    // (1) 최초 수신 cout은 프로그램 수명의 유효한 ostream lvalue다.
    // (2) 이 w64devkit ABI에서 int64_t가 long long이므로 basic_ostream::operator<<(long long)과
    //     char 비멤버 삽입 overload가 차례로 선택된다.
    // (3) 첫 오른쪽 피연산자는 답 int64_t prvalue, 둘째는 '\n' char prvalue이며 소유권은 없다.
    // (4) 각 호출은 같은 ostream&를 반환해 연쇄하고 마지막 참조는 버린다.
    // (5) 십진 답과 개행이 기록되며 실패 시 stream 상태 비트가 바뀔 수 있다. automaton은 그대로다.
    // (6) 복잡도는 형식화/출력 문자와 장치에 좌우되고 공통 상한은 없다. 예외 mask에 따라 I/O 예외가
    //     가능하고 부분 출력은 롤백되지 않는다. 동기화를 끈 cout을 다른 스레드와 공유하지 않는다.
    std::cout << automaton.distinct_substring_count() << '\n';

    // 상태 접근은 연속 메모리의 load/store, 문자 비교, suffix-link 조건 분기로 실행될 수 있다.
    // cache·분기 예측·인라이닝과 실제 명령은 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.
    return 0;
}
