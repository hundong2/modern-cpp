#include <expected>  // std::expected와 std::unexpected를 선언하는 표준 헤더다.
#include <iostream>  // std::cout으로 학습 결과를 출력한다.
#include <string>    // 소유권을 가진 문자열 타입 std::string을 사용한다.
#include <string_view>  // 문자열을 복사하지 않고 읽기만 하는 뷰를 사용한다.
#include <utility>   // std::move로 이동 가능함을 명시한다.

// 입력 계층이 발견한 오류다. 구조체는 관련 데이터를 하나의 타입으로 묶는다.
struct ParseError {
    std::string message;  // 오류 객체가 문자의 수명을 직접 소유한다.
};

// 도메인 계층이 사용하는 유효한 주문 수량이다.
class Quantity {
public:
    // explicit은 int가 Quantity로 암시적으로 바뀌는 실수를 막는다.
    explicit Quantity(int value) : value_{value} {}

    // 반환형 int는 호출자에게 정수 값을 복사해 돌려준다.
    // const 멤버 함수이므로 *this가 가리키는 객체를 바꾸지 않는다.
    [[nodiscard]] int value() const { return value_; }

private:
    int value_{};  // 중괄호 초기화는 기본값 0을 명확히 한다.
};

// 성공하면 Quantity, 실패하면 ParseError를 갖는 합 타입이다.
using ParseResult = std::expected<Quantity, ParseError>;

// text는 const 참조처럼 읽지만 소유하지 않는다. 호출 중 원본이 살아 있어야 한다.
[[nodiscard]] ParseResult parse_quantity(std::string_view text) {
    if (text.empty()) {  // 크기를 읽고 비교한 뒤 조건에 따라 분기하는 코드가 될 수 있다.
        // unexpected 임시 객체는 prvalue이며 반환 결과의 오류 저장소를 초기화한다.
        return std::unexpected(ParseError{"입력이 비어 있습니다."});
    }

    int number{0};  // 지역 자동 객체이며 함수 호출이 끝나면 수명이 끝난다.
    for (const char ch : text) {  // 반복마다 문자를 값으로 복사해 검사한다.
        if (ch < '0' || ch > '9') {
            return std::unexpected(ParseError{"숫자가 아닌 문자가 있습니다."});
        }
        // number는 이름 있는 객체이므로 lvalue다. 오른쪽 계산 결과는 prvalue다.
        number = number * 10 + (ch - '0');
    }

    if (number < 1 || number > 100) {
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
        return std::unexpected(parsed.error());
    }

    // operator* 결과는 Quantity&인 lvalue다. const 참조는 복사 없이 그 객체에 바인딩된다.
    const Quantity& quantity{*parsed};
    return std::string{"주문 수량: "} + std::to_string(quantity.value());
}

int main() {
    // 배열의 각 std::string은 문자열 버퍼의 소유권을 가진다.
    const std::string inputs[]{"12", "0", "1x"};

    for (const std::string& input : inputs) {  // const 참조로 각 원소의 복사를 피한다.
        auto result{make_order_message(input)};  // auto는 긴 expected 타입을 추론한다.
        if (result) {
            std::cout << "[성공] " << *result << '\n';
        } else {
            std::cout << "[실패] " << result.error().message << '\n';
        }
    }

    auto movable{make_order_message("3")};
    // std::move(movable).value()는 저장된 string을 가리키는 xvalue를 만든다.
    // 새 문자열이 버퍼 소유권을 이동받는다. movable은 유효하지만 값은 미지정 상태다.
    std::string owned_message{std::move(movable).value()};
    std::cout << "[이동] " << owned_message << '\n';

    // 실제 실행은 로드·저장·비교·조건 분기·함수 호출로 번역될 수 있다.
    // 정확한 명령과 인라이닝 여부는 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return 0;  // 운영체제에 성공 종료 코드 0을 반환한다.
}
