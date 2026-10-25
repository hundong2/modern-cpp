// 초보자 실습: optional 값이 비어 있을 때만 기본 별칭을 만드는 or_else를 직접 확인한다.

// <iostream>은 std::cout과 출력 삽입 연산을 선언한다.
#include <iostream>
// <optional>은 std::optional과 std::nullopt를 선언한다.
#include <optional>
// <string>은 문자를 소유하는 std::string을 선언한다.
#include <string>
// <utility>는 std::move를 선언한다.
#include <utility>

// 값 매개변수 configured는 호출자가 넘긴 optional을 복사 또는 이동해 독립적으로 소유한다.
[[nodiscard]] std::optional<std::string>
with_default(std::optional<std::string> configured) {
    // [첫 호출 계약: std::move(configured)와 optional<string>::or_else(F&&) &&]
    // (1) configured는 유효한 optional<string> lvalue다. engaged이면 string을 소유하고, 아니면 비어 있다.
    // (2) move<T>에서 T=optional<string>&이고, 그 xvalue에 `or_else(F&&) &&`가 선택된다.
    //     F는 캡처 없는 lambda 타입이다. Constraints는 invocable<F>와 move_constructible<string>,
    //     Mandates는 `is_same_v<remove_cvref_t<invoke_result_t<F>>, optional<string>>`가 true인 것이다.
    // (3) move는 수신자를 xvalue로 표시할 뿐 상태를 바꾸지 않는다. lambda prvalue는 빈 경로에서만
    //     1회 호출되어 string 리터럴을 복사 소유한 optional을 반환한다.
    // (4) move는 optional<string>&&를 반환해 즉시 or_else의 수신자로 사용하고, or_else가 반환한
    //     optional<string> prvalue는 with_default의 함수 반환 목적 객체를 직접 초기화한다.
    // (5) engaged 경로는 contained string을 새 결과로 이동하고 lambda를 건너뛴다. configured는
    //     계속 engaged이고 안의 string만 유효하지만 값이 미지정인 이동 후 상태다. 빈 경로에서는
    //     configured가 계속 disengaged이고 lambda 결과가 "guest"를 소유한다. 수신 상태는 자동 전환되지 않는다.
    // (6) 상태 분기 O(1), wrapper 자체 무할당이다. string 이동은 기본 allocator에서 상수 시간이고
    //     fallback string 구성은 길이에 선형·할당/bad_alloc 가능이다. 동시 접근은 없다.
    return std::move(configured).or_else([] {
        // [첫 생성 계약 A: std::string(const char*)]
        // (1) string 목적 객체는 생성 전이고 "guest"는 정적 수명의 null 종료 const char 배열이다.
        // (2) `basic_string(const char*, const Allocator& = Allocator())`가 기본 allocator로 선택된다.
        // (3) 배열 lvalue는 const char* prvalue로 변환되며 읽기 입력일 뿐 소유권을 넘기지 않는다.
        // (4) 생성자는 별도 반환값 없이 local 소유 string fallback의 수명을 시작한다.
        // (5) fallback이 다섯 문자를 독립 소유하고 리터럴은 변하지 않는다.
        // (6) 문자 수에 선형이고 SSO가 아니면 할당하며 length_error/bad_alloc 가능이다. null pointer나
        //     읽을 수 없는 null 종료 범위는 전제 위반이다. 리터럴 관찰은 유지되고 데이터 경쟁이 없다.
        std::string fallback{"guest"};

        // [첫 생성 계약 B: optional<string> converting value constructor]
        // (1) optional 목적 저장소는 생성 전이고 fallback은 완성된 유효 string lvalue다.
        // (2) `template<class U=T> constexpr optional(U&&)`에서 T=string, U=string이 추론되고
        //     contained value에는 기본 allocator `basic_string(basic_string&&) noexcept`가 선택된다.
        // (3) `move<T>(T&&)`에서 T=string&인 호출이 fallback lvalue를 string&& xvalue로 표시하고,
        //     그 결과가 U&&에 바인딩되어 contained storage로 문자 소유권을 옮긴다. move cast 자체는
        //     fallback을 바꾸지 않고 뒤 string 이동 생성자가 실제 이전을 수행한다.
        // (4) 생성자는 별도 반환값 없이 engaged optional<string> prvalue를 만들어 lambda가 반환한다.
        // (5) 결과 optional이 "guest"를 소유하고 fallback은 유효하지만 값이 미지정인 이동 후 상태로
        //     lambda block 끝에서 파괴된다.
        // (6) 바깥 wrapper와 기본 allocator string 이동은 O(1)·별도 할당 없음이고 contained string
        //     이동은 noexcept다. 그러나 표준의 optional(U&&) 선언 자체에는 noexcept 명세가 없으므로
        //     portable `noexcept(이 생성식)`이 true라고 요구할 수 없고, 구현은 더 강하게 보장할 수 있다.
        //     표준상 선택된 T 생성자 예외만 전파한다. 관찰자 무효화 대상은 아직 없고, 결과를 thread
        //     사이에서 변경 공유하면 동기화가 필요하다.
        return std::optional<std::string>{std::move(fallback)};
    });
}

int main() {
    // 중괄호 초기화는 narrowing을 막는다. 첫 optional은 engaged, 둘째는 명시적 빈 상태다.
    const std::optional<std::string> configured{
        with_default(std::optional<std::string>{std::string{"admin"}})};
    // [첫 생성 계약: optional<string>(nullopt_t)]
    // (1) missing에 넘길 임시 optional<string> 목적 객체는 생성 전이며 contained string이 없다.
    // (2) `constexpr optional(nullopt_t) noexcept` overload가 선택된다.
    // (3) 인자 식 std::nullopt는 정적 수명의 const nullopt_t lvalue이고, by-value nullopt_t 매개변수로
    //     복사된다. 문자나 다른 자원의 소유권을 전달하지 않고 "비어 있음"만 지정한다.
    // (4) 생성자는 별도 반환값 없이 with_default의 by-value 매개변수 configured 객체를 disengaged
    //     상태로 직접 초기화한다. 같은 타입 prvalue와 C++17 보장 복사 생략 덕분에 별도 optional
    //     임시를 만든 뒤 이동하는 경로가 아니다.
    // (5) configured 안에서 string 수명은 시작되지 않고 함수 진입 시 바로 disengaged다. 태그 객체는
    //     변하지 않으며 이후 or_else가 만든 결과가 missing을 직접 초기화한다.
    // (6) O(1)·무할당·noexcept이며 iterator/reference 무효화가 없다. 빈 값을 operator*로 읽으면
    //     UB이고 value()는 bad_optional_access지만 여기서는 or_else가 먼저 engaged 결과로 바꾼다.
    //     독립 local 객체를 single thread에서 전달하므로 데이터 경쟁이 없다.
    const std::optional<std::string> missing{
        with_default(std::optional<std::string>{std::nullopt})};

    // [첫 호출 계약: optional::value() const &와 std::cout operator<<]
    // (1) 두 const optional<string> lvalue는 with_default의 후조건상 engaged이고 cout은 정상 상태다.
    // (2) `constexpr const T& value() const &`에서 T=string인 overload 두 번과, 선택된 특수화 기준
    //     `std::ostream& operator<<(std::ostream&, const std::string&)` 두 번 및
    //     `std::ostream& operator<<(std::ostream&, char)` 두 번이 선택된다. 문자 template 인자는
    //     CharT=char, Traits=char_traits<char>이고 string은 기본 allocator를 쓴다.
    // (3) value에는 데이터 인자가 없다. 각 free operator<<의 첫 인자는 앞 호출이 반환한 ostream
    //     lvalue, 둘째 인자는 두 const string lvalue 또는 ' '/'\n' char prvalue다. 모두 호출 동안
    //     빌릴 뿐 optional/string 소유권을 넘기지 않는다.
    // (4) value의 const string&를 각 출력에 사용하고 각 ostream& 반환은 연쇄 뒤 버린다.
    // (5) optional은 그대로이고 출력 buffer만 바뀐다.
    // (6) value는 O(1)·무할당이나 빈 상태면 bad_optional_access를 던진다. 출력의 표준 점근 상한은
    //     별도 지정되지 않고 문자 수·locale·streambuf에 따르며 내부 buffering, 실패 bit와 설정된
    //     예외가 가능하다. 반환 참조는 optional 파괴 뒤 무효다. 기본 동기화 상태의 표준 stream은
    //     문자 단위 데이터 경쟁을 막지만 여러 삽입으로 만든 레코드 원자성은 보장하지 않으며,
    //     같은 optional을 다른 thread가 동시에 변경하면 외부 동기화가 필요하다.
    std::cout << configured.value() << ' ' << missing.value() << '\n';
    return 0;
}
