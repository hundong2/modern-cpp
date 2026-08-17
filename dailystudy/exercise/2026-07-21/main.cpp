#include <expected>  // std::expected와 std::unexpected를 선언하는 표준 헤더다.
#include <iostream>  // std::cout으로 학습 결과를 출력한다.
#include <string>    // 소유권을 가진 문자열 타입 std::string을 사용한다.
#include <string_view>  // 문자열을 복사하지 않고 읽기만 하는 뷰를 사용한다.
#include <utility>   // std::move로 이동 가능함을 명시한다.

// 입력 계층이 발견한 오류다. 구조체는 관련 데이터를 하나의 타입으로 묶는다.
struct ParseError {
    // struct의 멤버는 기본적으로 public이므로 외부에서 error.message로 읽을 수 있다.
    std::string message;  // 오류 객체가 문자의 수명을 직접 소유한다.
};

// 도메인 계층이 사용하는 유효한 주문 수량이다.
class Quantity {
// class의 멤버는 기본적으로 private이다. public: 아래만 외부에 공개된다.
public:
    // 생성자는 반환형을 쓰지 않고 객체가 만들어질 때 초기 상태를 정한다.
    // explicit은 int 하나가 Quantity로 암시적으로 변환되는 일을 막는다.
    // 따라서 Quantity quantity = 3;은 금지되고 Quantity quantity{3};처럼 의도를 밝혀야 한다.
    // int value는 호출자가 준 정수를 복사해 받는 매개변수다.
    // 콜론 뒤 멤버 초기화 목록은 본문 실행 전에 value_를 value로 직접 초기화한다.
    explicit Quantity(int value) : value_{value} {}

    // 반환형 int는 호출자에게 정수 값을 복사해 돌려준다.
    // const 멤버 함수이므로 *this가 가리키는 객체를 바꾸지 않는다.
    [[nodiscard]] int value() const { return value_; }

private:
    // private 멤버는 클래스 밖에서 직접 읽거나 바꿀 수 없다.
    // 이렇게 내부 표현을 감추면 이후 검증 규칙을 한곳에서 유지하기 쉽다.
    int value_{};  // 중괄호 초기화는 기본값 0을 명확히 한다.
};

// using은 새 클래스를 만드는 문법이 아니라 긴 타입에 읽기 좋은 별칭을 붙인다.
// expected의 첫 타입은 성공값, 둘째 타입은 오류값이다.
using ParseResult = std::expected<Quantity, ParseError>;

// text는 const 참조처럼 읽지만 소유하지 않는다. 호출 중 원본이 살아 있어야 한다.
[[nodiscard]] ParseResult parse_quantity(std::string_view text) {
    // [[nodiscard]]는 호출자가 반환한 성공/실패 결과를 무시하면 경고하도록 돕는다.
    // string_view::empty()는 인자가 없고 text를 수정하지 않으며 길이가 0인지 bool로 O(1)에 반환한다.
    if (text.empty()) {
        // unexpected 임시 객체는 prvalue이며 반환 결과의 오류 저장소를 초기화한다.
        return std::unexpected(ParseError{"입력이 비어 있습니다."});
    }

    int number{0};  // 지역 자동 객체이며 함수 호출이 끝나면 수명이 끝난다.
    for (const char ch : text) {  // 반복마다 문자를 값으로 복사해 검사한다.
        if (ch < '0' || ch > '9') {
            // ||는 왼쪽이 참이면 오른쪽 비교를 생략하는 단락 평가 논리합이다.
            return std::unexpected(ParseError{"숫자가 아닌 문자가 있습니다."});
        }
        // number는 이름 있는 객체이므로 lvalue다. 오른쪽 계산 결과는 prvalue다.
        number = number * 10 + (ch - '0');
    }

    if (number < 1 || number > 100) {
        // 유효 범위를 벗어나면 Quantity를 만들지 않고 오류 경로로 조기 반환한다.
        return std::unexpected(ParseError{"수량은 1~100이어야 합니다."});
    }

    // Quantity{number}는 prvalue다. 반환값 최적화/복사 생략으로 결과 안에 직접
    // 만들어질 수 있지만, 적용 여부와 기계 코드는 ABI·컴파일러·최적화에 따라 다르다.
    return Quantity{number};
}

// 응용 서비스는 파싱 정책을 호출하고, 성공 값으로 업무 메시지를 만든다.
[[nodiscard]] std::expected<std::string, ParseError>
make_order_message(std::string_view raw) {
    ParseResult parsed{parse_quantity(raw)};  // 반환 prvalue로 지역 결과를 초기화한다.

    if (!parsed) {  // 내부 성공 여부를 읽고 비교한 뒤 실패 경로로 분기할 수 있다.
        // error()는 lvalue 참조를 돌려준다. 여기서는 오류를 복사해 독립 수명을 준다.
        // error()는 인자 없이 실패 상태의 ParseError 참조를 반환한다. !parsed로 실패를 확인한 뒤 호출한다.
        // unexpected는 오류 값을 입력받아 새 expected 실패 상태를 만들며 여기서는 ParseError를 복사한다.
        return std::unexpected(parsed.error());
    }

    // operator* 결과는 Quantity&인 lvalue다. const 참조는 복사 없이 그 객체에 바인딩된다.
    const Quantity& quantity{*parsed};
    // to_string은 int를 소유 string prvalue로 바꾸고 +는 새 결과 문자열을 만든다.
    // quantity.value()는 사용자 정의 함수로 int를 반환하고, to_string(int)은 새 소유 string을 반환한다.
    return std::string{"주문 수량: "} + std::to_string(quantity.value());
}

int main() {
    // main은 프로그램이 시작되는 특별한 함수다. int 반환값은 종료 상태를 뜻한다.
    // 배열의 각 std::string은 문자열 버퍼의 소유권을 가진다.
    const std::string inputs[]{"12", "0", "1x"};

    for (const std::string& input : inputs) {  // const 참조로 각 원소의 복사를 피한다.
        auto result{make_order_message(input)};  // auto는 긴 expected 타입을 추론한다.
        if (result) {
            // << 연산자는 오른쪽 값을 출력 스트림으로 차례로 보낸다.
            std::cout << "[성공] " << *result << '\n';
        } else {
            // else는 바로 앞 if 조건이 거짓, 즉 오류 상태일 때 실행된다.
            std::cout << "[실패] " << result.error().message << '\n';
        }
    }

    auto movable{make_order_message("3")};
    // auto는 expected<string, ParseError> 타입을 추론한다. movable은 이름 있는 lvalue다.
    // expected::value()는 인자가 없고 성공 string의 && 참조를 반환한다. 오류 상태면 bad_expected_access를 던진다.
    // std::move는 movable을 xvalue로 만들어 && 오버로드를 고르고 새 문자열이 버퍼 소유권을 이동받는다.
    // movable expected는 여전히 성공 상태지만 내부 string 값은 이동 후 유효한 미지정 상태다.
    std::string owned_message{std::move(movable).value()};
    std::cout << "[이동] " << owned_message << '\n';

    // 실제 실행은 로드·저장·비교·조건 분기·함수 호출로 번역될 수 있다.
    // 정확한 명령과 인라이닝 여부는 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return 0;  // 운영체제에 성공 종료 코드 0을 반환한다.
}
