#include <expected> // expected/unexpected는 검증 실패를 호출자의 분기 가능한 값으로 반환한다.
#include <iostream> // cout은 연습 결과를 표준 출력에 기록한다.
#include <string>   // string은 결과 객체의 레이블 문자를 소유한다.
#include <utility>  // move는 이름이 있는 값 매개변수를 이동 가능한 xvalue로 표시한다.

// 초보자 연습: 잘못된 요청에는 Quota를 아예 만들지 않는 두 단계 검증 파이프라인을 읽고 바꿔 보자.
enum class ValidationError { Negative, TooLarge };

// struct는 기본 public, class는 기본 private다. 내부 단위를 감춘 class로 유효한 객체 생성 경계를 보여준다.
class Quota {
public:
    using Units = unsigned; // using은 타입 별칭이지 별도의 새 정수 타입은 아니다.

    // explicit은 암시적 사용자 타입 변환을 막는 습관이다. 값 매개변수 label은 이 함수 안에서 소유한다.
    // [호출 계약: std::move 및 std::string 이동 생성]
    // (1) 수신 대상 label_은 아직 구성 전 string 멤버; label은 살아 있는 string lvalue 매개변수다.
    // (2) move<string&>(string&) noexcept -> string&&, basic_string(string&&) 생성자를 선택한다.
    // (3) 임의의 유효 문자열 label을 xvalue로 표시하고 소유 버퍼 이전을 허용한다.
    // (4) move 반환 참조는 생성에 쓰이며 생성자 자체 반환값은 없다.
    // (5) label_은 문자를 소유하고 label은 유효하지만 값이 미지정인 상태로 함수 끝에서 파괴된다.
    // (6) 기본 allocator 문자열 이동은 상수 시간·무할당; 관찰자는 객체 파괴 전에만 유효하고 동시 쓰기는 금지한다.
    explicit Quota(Units units, std::string label)
        : units_{units}, label_{std::move(label)} {}

    [[nodiscard]] Units units() const { return units_; } // const 함수는 객체를 바꾸지 않고 기본 타입 값을 복사한다.
    [[nodiscard]] const std::string& label() const { return label_; } // 참조는 비소유이며 Quota보다 오래 쓰면 안 된다.

private:
    Units units_{};     // 중괄호 초기화는 기본 타입의 불확정 값을 피한다.
    std::string label_; // 선언 순서가 멤버 초기화 순서를 정한다.
};

// int는 음수도 표현하므로 입력 검증에 쓰고, 성공할 때만 unsigned로 바꾼다.
[[nodiscard]] std::expected<Quota::Units, ValidationError> validate(int requested) {
    if (requested < 0) { // 비교 후 조건 분기로 음수 입력을 제거한다.
        // [생성 계약: std::unexpected와 expected 오류 변환]
        // (1) 아직 생성되지 않은 unexpected<ValidationError> 및 expected<Units,ValidationError>가 대상이다.
        // (2) unexpected(E&&)의 E=ValidationError, expected(unexpected<G>&&)의 G=ValidationError가 선택된다.
        // (3) enum prvalue는 값으로 전달되고 외부 자원은 빌리거나 이전하지 않는다.
        // (4) 생성자 반환은 없고 return 식은 오류를 소유한 expected 결과가 된다.
        // (5) 성공 숫자 객체 대신 Negative 오류가 보관되고 requested는 변하지 않는다.
        // (6) 상수 시간·무할당·무예외, 반환 객체 수명은 호출자가 관리하며 공유 중 변경은 동기화한다.
        return std::unexpected{ValidationError::Negative};
    }
    if (requested > 50) {
        return std::unexpected{ValidationError::TooLarge};
    }
    // [생성 계약: std::expected<Units,ValidationError> 값 변환 생성]
    // (1) 반환 목적 expected는 아직 없고 requested는 검증된 [0,50] int lvalue다.
    // (2) expected(U&&)에서 U=unsigned prvalue가 선택되어 contained Units를 직접 구성한다.
    // (3) static_cast 결과 [0,100] unsigned prvalue를 값으로 받아 외부 소유권을 빌리지 않는다.
    // (4) 생성자는 별도 반환값이 없고 return 식은 성공 상태 expected 값으로 사용된다.
    // (5) 새 결과가 두 배 Units를 소유하며 requested는 변하지 않는다.
    // (6) O(1), 무할당·무예외; 검증 분기가 범위를 보장하고 독립 결과의 수명은 호출자가 관리한다.
    // int -> unsigned 값 변환은 0..50 범위임을 분기로 증명했으므로 좁힘/랩어라운드가 없다.
    return static_cast<Quota::Units>(requested * 2);
}

// 요청의 원본 값을 소유한 expected에서 검증을 합성하고 성공일 때만 결과 Quota를 구성한다.
[[nodiscard]] std::expected<Quota, ValidationError> make_quota(int requested) {
    // [호출 계약: expected 값 생성, and_then, transform, string 생성]
    // (1) 임시 expected<int,ValidationError>는 requested 성공값을 가진 유효한 prvalue 수신 객체다.
    //     and_then 결과 expected<Units,ValidationError>는 성공 시 두 배 단위, 실패 시 오류를 보유한 prvalue다.
    // (2) expected(U&&)에서 U=int&, and_then(F&&) &&에서 F는 validate 함수 lvalue 참조로 추론되고,
    //     transform(F&&) &&에서 F는 람다 타입, string(const char*)는 "worker" 리터럴을 받는다.
    // (3) requested는 int lvalue에서 값 복사; validate는 int 값을 받고 expected 오류를 변경 없이 전파한다.
    //     transform 람다는 성공 Units 값을 값으로 받으며, 리터럴은 정적 수명 배열을 빌려 문자열에 복사한다.
    // (4) and_then은 expected<Units,Error>, transform은 expected<Quota,Error> prvalue를 반환해 그대로 쓴다.
    // (5) 성공이면 Quota가 레이블을 소유하고, 실패면 람다는 실행되지 않아 Quota가 생성되지 않는다.
    //     원본 requested는 변하지 않으며 임시 expected는 전체 식 뒤 파괴된다.
    // (6) 각 monadic 분기는 O(1)+실제 callable 비용, 문자열 구성은 길이에 선형·할당/예외 가능.
    //     오류 경로의 and_then/transform은 저장 오류 타입을 생성할 수 있어 그 생성 예외가 전파된다.
    //     유효한 callable 반환 타입과 E 복사/이동 가능성이 전제다. 임시 수명은 전체 식 끝까지,
    //     반환 Quota는 자기 문자열을 소유한다. 이 로컬 체인에는 공유 데이터 경쟁이 없다.
    return std::expected<int, ValidationError>{requested}
        .and_then(validate)
        .transform([](Quota::Units units) { return Quota{units, std::string{"worker"}}; });
}

int main() {
    auto good{make_quota(6)};
    auto bad{make_quota(-1)};

    // [호출 계약: expected::operator bool/operator->]
    // (1) good은 Quota 값을 보유한 expected<Quota,ValidationError> lvalue다.
    // (2) explicit operator bool() const noexcept -> bool, T* operator->() noexcept -> Quota*.
    // (3) 데이터 인자 없이 보유 여부를 검사하고, 그 참인 분기에서만 포인터로 내부 객체를 빌린다.
    // (4) bool은 if에, 포인터는 units()/label()의 수신 객체 선택에 사용한다.
    // (5) good과 Quota는 변하지 않는다. 포인터는 good의 상태 전환/파괴 뒤 무효다.
    // (6) 각각 O(1), 무할당·무예외; 값이 없을 때 ->는 UB. 같은 객체의 동시 변경은 없다.
    if (good) {
        // [호출 계약: std::ostream 삽입]
        // (1) std::cout은 살아 있는 std::ostream lvalue, good의 Quota/string도 살아 있다.
        // (2) const char*/string/char에는 비멤버 operator<<(ostream&,...) 오버로드,
        //     unsigned에는 basic_ostream::operator<<(unsigned) 멤버가 차례로 선택된다.
        // (3) 입력 인자 리터럴과 label()의 const string&를 빌리고 units()의 unsigned 및 '\n' char는
        //     값으로 받는다. 어떤 피연산자도 소유권을 넘기지 않는다.
        // (4) 각 ostream& 반환 결과로 연결하고 최종 참조는 버린다.
        // (5) 출력 버퍼/상태만 변경되고 good은 유지된다.
        // (6) 기록 문자 수에 비례하며 I/O 오류는 상태 비트/예외 설정에 따름; 동시 출력 원자성은 보장하지 않는다.
        std::cout << "quota=" << good->units() << ",label=" << good->label() << '\n';
    }

    if (!bad) {
        // [호출 계약: expected::error]
        // (1) bad는 오류를 보유한 expected<Quota,ValidationError> lvalue다.
        // (2) E& error() & noexcept -> ValidationError&; 인자는 없다.
        // (3) 소유권 이전 없이 현재 오류를 읽으며, 오류 상태임을 if로 확인했다.
        // (4) 반환 참조의 enum 값을 비교해 출력할 문자열을 정한다.
        // (5) bad는 변하지 않고 참조는 객체의 상태 전환/파괴 뒤 무효다.
        // (6) O(1), 무할당·무예외; 성공값 상태의 error()는 UB. 동시 변경 금지.
        std::cout << "rejected=" << (bad.error() == ValidationError::Negative ? "negative" : "large") << '\n';
    }
}
