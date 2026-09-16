#include <charconv>     // std::from_chars: 임시 문자열이나 예외 없이 숫자 문자를 읽는다.
#include <expected>     // std::expected/std::unexpected: 성공값과 실패값을 타입으로 구별한다.
#include <iostream>     // std::cout: 실행 결과를 표준 출력에 기록한다.
#include <string>       // std::string: 설정의 이름을 자신의 저장소로 소유한다.
#include <string_view>  // std::string_view: 입력 문자를 복사하지 않고 함수 호출 동안 빌린다.
#include <system_error> // std::errc: from_chars의 숫자 변환 오류를 비교한다.
#include <utility>      // std::in_place/std::move: 기대값의 직접 생성과 명시적 이동을 표현한다.

// 실패를 문자열 메시지가 아닌 닫힌 열거형으로 표현하면 호출자가 모든 경우를 분기할 수 있다.
enum class ParseError { Invalid, OutOfRange };

// struct는 기본 public, class는 기본 private다. 멤버는 숨기되 포트 유효성은 parse_port 경계에서 검증한다.
class Config {
public:
    // using은 새 타입이 아닌 별칭이다. unsigned는 음수가 없는 기본 타입이며 포트 상한은 별도로 검증한다.
    using Port = unsigned;

    // explicit은 단일 인자 변환 생성자에 특히 중요하다. 이 다중 인자 생성자는 copy-list 초기화를 막는다.
    // name을 값으로 받아 호출자의 lvalue는 복사하고 prvalue는 직접 구성/이동할 수 있게 한다.
    // [호출 계약: std::move와 std::string 이동 생성]
    // (1) 수신 대상 name_은 아직 생성 전의 std::string 멤버, 인자 name은 유효한 std::string lvalue다.
    // (2) move<string&>(string&) noexcept -> string&&; basic_string(string&&) 생성자가 선택된다.
    // (3) name은 어떤 문자열도 허용하는 lvalue이고 move 결과 xvalue는 소유 문자 버퍼의 이전을 허용한다.
    // (4) move는 string&&를 다음 생성자에 전달하고 생성자는 별도 반환값이 없다.
    // (5) name_은 원래 내용을 소유하며 name은 유효하지만 값은 미지정; port_는 입력 값을 복사해 보관한다.
    // (6) 기본 allocator의 문자열 이동은 상수 시간·무할당으로 구현 가능한 표준 계약을 따른다.
    //     참조/포인터의 이전 대상 수명은 name_에 묶이고, 이 생성 중 동시 접근은 없어야 한다.
    explicit Config(Port port, std::string name)
        : port_{port}, name_{std::move(name)} {} // 본문 전에 선언 순서대로 멤버를 초기화한다.

    // const 멤버 함수는 객체를 변경하지 않는다. 반환 기본 타입은 값 복사, 문자열 참조는 비소유 관찰자다.
    [[nodiscard]] Port port() const { return port_; }
    [[nodiscard]] const std::string& name() const { return name_; }

private:
    Port port_{};       // 중괄호 초기화는 기본 타입의 미초기화 값을 피한다.
    std::string name_; // 이 멤버가 문자를 소유하며 Config와 수명이 같다.
};

// 반환형 expected<Config,ParseError>는 예외 대신 성공 Config 또는 명시적 오류 하나를 값으로 소유한다.
// text는 읽기 전용 비소유 뷰이므로 호출 동안 원본 문자가 살아 있어야 한다.
[[nodiscard]] std::expected<Config, ParseError> parse_port(std::string_view text) {
    // [호출 계약: std::string_view::empty]
    // (1) 수신 text는 유효한 std::string_view lvalue이고 원본 문자는 호출 동안 살아 있다.
    // (2) constexpr bool empty() const noexcept; 템플릿 인자와 데이터 인자는 없다.
    // (3) 외부 인자는 없고 문자 소유권도 이전하지 않는다.
    // (4) bool 반환값을 if 조건에 사용한다: 길이가 0이면 true다.
    // (5) 뷰와 원본은 변하지 않는다.
    // (6) O(1), 무할당·무예외·무효화 없음. 뷰의 대상 수명은 호출자가 보장하고 동시 쓰기는 안 된다.
    if (text.empty()) {
        // [생성 계약: std::unexpected<ParseError> 및 expected 실패 변환]
        // (1) 아직 생성되지 않은 unexpected<ParseError>와 반환 expected<Config,ParseError>가 대상이다.
        // (2) unexpected(E&&)에서 E=ParseError, 이어 expected(unexpected<G>&&)에서 G=ParseError가 선택된다.
        // (3) Invalid 열거형 prvalue는 오류값으로 복사/이동되며 외부 자원 소유권은 없다.
        // (4) 생성자는 반환값이 없고 return 식은 오류 상태 expected 값을 만든다.
        // (5) 성공 Config는 생성되지 않으며 입력 뷰는 그대로다.
        // (6) 상수 시간·무할당, 이 열거형 구성은 예외가 없다. 반환값은 자기 오류를 소유하고 뷰를 빌리지 않는다.
        return std::unexpected{ParseError::Invalid};
    }

    // [호출 계약: std::string_view::data/size]
    // (1) 수신 text는 비어 있지 않은 유효한 std::string_view; 원본은 살아 있다.
    // (2) const char* data() const noexcept, size_type size() const noexcept가 선택된다.
    // (3) 두 호출 모두 데이터 인자가 없고 문자 저장소를 빌리기만 한다.
    // (4) data의 const char*는 첫 문자, size의 size_type은 문자 개수; 둘 다 from_chars 범위에 사용한다.
    // (5) text와 원본은 변하지 않고 포인터는 원본 수명을 연장하지 않는다.
    // (6) 각각 O(1), 무할당·무예외·무효화 없음. 비어 있지 않아 first+length는 같은 배열 끝까지 유효하다.
    const char* const first{text.data()};
    const char* const last{first + text.size()};
    Config::Port parsed{};

    // [호출 계약: std::from_chars 정수 오버로드]
    // (1) 수신 객체는 없는 비멤버 함수다. [first,last)는 살아 있는 연속 문자 범위, parsed는 초기값 0인 unsigned lvalue다.
    // (2) template<class Integer> from_chars_result from_chars(const char*,const char*,Integer&,int=10), Integer=unsigned.
    // (3) first/last는 빌린 포인터 prvalue/lvalue, parsed는 수정 가능한 lvalue 참조, base=10 기본값이다.
    //     first<=last이며 두 포인터는 같은 배열 범위다. 공백·선행 '+'는 정수 문법에서 허용하지 않는다.
    // (4) 반환 from_chars_result의 ptr과 ec를 모두 검사한다. 성공하면 ptr은 읽은 끝, ec는 errc{}다.
    // (5) 성공 시 parsed가 숫자가 되고 원본 문자는 그대로다. 실패 시 parsed는 기존 값으로 남는다.
    // (6) 범위 길이에 선형, 무할당·무예외; 범위/parsed 수명은 호출 동안 유지, 동시 쓰기 금지.
    const auto result{std::from_chars(first, last, parsed)};
    if (result.ec == std::errc::result_out_of_range || parsed > 65535U) {
        return std::unexpected{ParseError::OutOfRange};
    }
    // ptr이 끝이 아니면 접미 문자가 남았다. errc{}는 기본 성공 오류 코드 객체다.
    if (result.ec != std::errc{} || result.ptr != last || parsed == 0U) {
        return std::unexpected{ParseError::Invalid};
    }

    // [생성 계약: std::string, std::in_place, std::expected 직접 값 생성]
    // (1) 대상은 아직 생성되지 않은 string과 expected<Config,ParseError>; parsed는 검증된 값이다.
    // (2) string(const char*)은 리터럴 "service"의 const char[8] -> const char* 변환을 받는다.
    //     expected(in_place_t, Args&&...)는 Args={Port&, string}로 Config(Port,string)을 내부에서 직접 만든다.
    // (3) 리터럴은 정적 수명 lvalue 배열, string prvalue는 소유 객체, parsed는 값으로 복사된다.
    // (4) 생성자 자체 반환은 없고 expected prvalue가 반환되어 C++17 보장 복사 생략으로 결과 객체가 된다.
    // (5) 결과는 자신의 Config/string을 소유한다. 입력 뷰나 호출자 버퍼를 붙잡지 않는다.
    // (6) string 생성은 문자 수에 선형·할당/예외 가능; expected의 추가 구성은 Config 생성 비용이다.
    //     실패하면 구성된 부분 객체는 정리되며 뷰는 변하지 않는다. 완성 객체만 스레드 사이에 공유한다.
    return std::expected<Config, ParseError>{std::in_place, parsed, std::string{"service"}};
}

int main() {
    Config active{Config::Port{80}, std::string{"fallback"}};
    // 리터럴의 배열 수명은 정적이고 string_view prvalue는 문자를 소유하지 않는다.
    // [생성 계약: std::string_view 문자열 리터럴 생성]
    // (1) 아직 생성되지 않은 string_view가 수신 대상; 리터럴 "8080"은 정적 수명 const char[5]다.
    // (2) basic_string_view(const char*) 생성자가 선택되어 널 종료 문자 전까지 읽는다.
    // (3) 리터럴 배열 lvalue가 포인터로 변환되어 전달되며 소유권 이전은 없다.
    // (4) 생성자 반환은 없고 뷰 prvalue가 parse_port 매개변수로 복사된다.
    // (5) 뷰만 문자 범위를 가리키고 리터럴은 변하지 않는다.
    // (6) 문자 수 선형 탐색, 무할당, 유효한 널 종료 배열이어야 한다; 리터럴은 프로그램 끝까지 살아 있다.
    auto candidate{parse_port("8080")};

    // [호출 계약: expected::operator bool/operator*와 std::move]
    // (1) candidate는 유효한 expected<Config,ParseError> lvalue이고 값 보유 상태; active는 유효 Config다.
    // (2) explicit operator bool() const noexcept -> bool; T& operator*() & noexcept -> Config&;
    //     move<Config&>(Config&) noexcept -> Config&&. 이어 Config의 기본 이동 대입이 사용된다.
    // (3) bool은 인자 없음, 역참조 전 값 보유를 검사한다. 참조 lvalue를 xvalue로 표시해 active에 자원 이전을 허용한다.
    // (4) bool은 분기에, Config&는 move에, Config&&는 대입에 사용된다. 대입은 Config&를 반환하나 버린다.
    // (5) 성공 뒤 active는 8080/"service"; candidate의 contained Config는 살아 있으나 이동된 문자열 값은 미지정이다.
    // (6) 관찰 O(1), move 자체 무할당·무예외; string 이동 대입은 allocator 특성에 따른다.
    //     값 없는 expected 역참조는 UB이므로 분기한다. active와 candidate는 별개 객체이며 동시 변경은 없다.
    if (candidate) {
        active = std::move(*candidate); // 검증이 끝난 뒤에만 현재 설정을 교체한다.
    }

    // [호출 계약: ostream 삽입 연산자와 Config 접근자의 반환 관찰]
    // (1) 수신 std::cout은 살아 있는 std::ostream lvalue, active는 살아 있는 const로 관찰 가능한 Config다.
    // (2) 정수에는 basic_ostream::operator<<(unsigned), "active="에는 비멤버 const char* 삽입,
    //     '\n'에는 비멤버 char 삽입 오버로드가 선택된다. 이 식은 string 삽입을 사용하지 않는다.
    // (3) active.port()의 unsigned prvalue를 값으로 받고 리터럴 배열을 const char*로 빌린다;
    //     개행 char prvalue도 값 전달하며 어떤 피연산자도 소유권을 넘기지 않는다.
    // (4) 각 삽입은 ostream&를 반환해 연결에 사용하며 마지막 반환은 버린다.
    // (5) 스트림의 출력 위치/상태가 변하고 active는 그대로다.
    // (6) 출력 문자 수에 비례, 스트림 버퍼 오류 시 badbit/예외 설정에 따름. 동시 쓰기 출력의 원자성은 없다.
    std::cout << "active=" << active.port() << '\n';

    auto rejected{parse_port("oops")};
    if (!rejected) { // expected의 bool 관찰은 앞에서 설명한 같은 오버로드/상태 계약을 재사용한다.
        // [호출 계약: std::expected::error]
        // (1) rejected는 오류를 보유한 expected<Config,ParseError> lvalue다.
        // (2) E& error() & noexcept -> ParseError&; 데이터 인자/템플릿 함수 인자는 없다.
        // (3) 소유권 이전 없이 현재 오류를 읽는다. 오류 상태라는 전제조건을 앞의 분기로 검증했다.
        // (4) 반환 참조를 Invalid와 비교해 출력 분기를 결정한다.
        // (5) rejected와 active는 변하지 않는다. 참조는 rejected의 상태 전환/파괴 뒤 사용하면 안 된다.
        // (6) O(1), 무할당·무예외, 값 상태에서 호출하면 UB. 같은 객체의 동시 변경은 안 된다.
        std::cout << "error=" << (rejected.error() == ParseError::Invalid ? "invalid" : "range") << '\n';
    }
    std::cout << "after-invalid=" << active.port() << '\n';

    // 경계값도 실행 파일 자체에서 검증한다. &&는 왼쪽부터 단락 평가하므로 오류 상태에서만 error(),
    // 성공 상태에서만 operator*를 평가한다. 위의 동일 오버로드 계약과 포인터/수명 조건을 재사용한다.
    const auto zero{parse_port("0")};
    const auto suffix{parse_port("42x")};
    const auto upper{parse_port("65535")};
    const auto high{parse_port("65536")};
    const auto huge{parse_port("99999999999999999999999")};
    const auto plus{parse_port("+42")};
    const auto space{parse_port(" 42")};
    std::cout << "zero=" << (!zero && zero.error() == ParseError::Invalid ? "ok" : "bad") << '\n';
    std::cout << "suffix=" << (!suffix && suffix.error() == ParseError::Invalid ? "ok" : "bad") << '\n';
    std::cout << "upper=" << (upper && (*upper).port() == 65535U ? "ok" : "bad") << '\n';
    std::cout << "high=" << (!high && high.error() == ParseError::OutOfRange ? "ok" : "bad") << '\n';
    std::cout << "huge=" << (!huge && huge.error() == ParseError::OutOfRange ? "ok" : "bad") << '\n';
    std::cout << "plus=" << (!plus && plus.error() == ParseError::Invalid ? "ok" : "bad") << '\n';
    std::cout << "space=" << (!space && space.error() == ParseError::Invalid ? "ok" : "bad") << '\n';
}
