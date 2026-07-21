#include <cassert>      // assert로 성공 금액과 오류 종류를 검증한다.
#include <expected>     // 성공값 또는 오류값을 담는 expected를 사용한다.
#include <iostream>     // cout으로 영수증을 출력한다.
#include <string>       // 상품 이름 문자 버퍼를 소유한다.
#include <string_view>  // 오류 설명 문자열을 소유하지 않는 뷰로 반환한다.
#include <utility>      // 이동 관련 표준 유틸리티를 사용할 기반 헤더다.
#include <vector>       // 장바구니가 여러 LineItem을 소유한다.

// 강한 타입은 상품 개수와 돈처럼 표현은 int여도 의미가 다른 값을 섞지 못하게 한다.
struct Cents {
    // 강한 타입 구조체로 원시 int와 금액을 구분하며 멤버는 기본 public이다.
    int value{}; // 강한 타입: 기계 수준 표현은 int와 비슷해도 C++ 타입 검사는 서로 구분한다.
};

struct LineItem {
    std::string name;   // 상품명 문자의 메모리와 수명을 직접 소유한다.
    Cents unit_price;   // 원시 int 대신 강한 금액 타입을 사용한다.
    int quantity{};     // 상품 개수를 0으로 값 초기화한다.
};

enum class CheckoutError {
    // enum class는 오류 이름을 자체 범위에 두고 정수와 섞이지 않게 한다.
    empty_cart,
    blank_name,
    invalid_price,
    invalid_quantity
};

using Cart = std::vector<LineItem>;
// Cart는 새 클래스가 아니라 vector<LineItem>에 붙인 읽기 쉬운 타입 별칭이다.
using CheckoutResult = std::expected<Cents, CheckoutError>;
// 첫 템플릿 인자는 성공 금액, 둘째는 실패 이유이며 하나만 활성화된다.

[[nodiscard]] constexpr Cents operator+(Cents left, Cents right) {
    // operator+는 Cents끼리 + 문법을 쓸 때 호출되는 사용자 정의 연산자다.
    // constexpr는 가능한 입력이면 컴파일 시간에도 계산할 수 있음을 뜻한다.
    // 인수를 값으로 받아 독립 객체로 계산한다. 반환 Cents{...}는 prvalue이고 복사 생략 대상이다.
    return Cents{left.value + right.value};
}

[[nodiscard]] std::expected<LineItem, CheckoutError>
validate(LineItem item) {
    // 값 매개변수는 호출 인수로부터 복사/이동된다. 성공 시 이 지역 객체를 반환 객체로 이동할 수 있다.
    if (item.name.empty()) {
        return std::unexpected(CheckoutError::blank_name);
    }
    if (item.unit_price.value < 0) {
        return std::unexpected(CheckoutError::invalid_price);
    }
    if (item.quantity <= 0) {
        return std::unexpected(CheckoutError::invalid_quantity);
    }
    return item; // 지역 변수는 lvalue지만 반환 문맥에서는 암시적 이동 후보가 된다.
}

class CheckoutService {
public:
    // public 멤버는 외부 호출 계약이며 뒤의 const는 서비스 상태를 바꾸지 않음을 뜻한다.
    [[nodiscard]] CheckoutResult total(const Cart& cart) const {
        // const Cart&는 장바구니와 string 원소를 복사하지 않고 읽기만 한다.
        if (cart.empty()) {
            return std::unexpected(CheckoutError::empty_cart);
        }

        Cents result{};
        for (const LineItem& item : cart) { // const 참조로 string을 포함한 상품 복사를 피한다.
            auto checked = validate(item);
            if (!checked) {
                // expected 실패를 확인한 뒤 오류 열거값을 새 unexpected에 복사한다.
                return std::unexpected(checked.error());
            }
            // 오른쪽 덧셈 결과는 prvalue, 왼쪽 result는 대입 가능한 lvalue다.
            result = result + Cents{checked->unit_price.value * checked->quantity};
        }
        return result;
    }
};

[[nodiscard]] constexpr std::string_view message(CheckoutError error) {
    // switch는 enum 값과 일치하는 case로 분기하고 각 return이 함수를 끝낸다.
    switch (error) {
    case CheckoutError::empty_cart: return "the cart is empty";
    case CheckoutError::blank_name: return "an item name is blank";
    case CheckoutError::invalid_price: return "an item price is negative";
    case CheckoutError::invalid_quantity: return "an item quantity is not positive";
    }
    return "unknown error";
}

void print_receipt(const CheckoutResult& result) {
    // result는 비소유 const 참조여서 expected나 내부 값을 복사하지 않는다.
    if (result) {
        std::cout << "Checkout total: " << result->value << " cents\n";
    } else {
        std::cout << "Checkout failed: " << message(result.error()) << '\n';
    }
}

int main() {
    // CheckoutService는 상태가 없는 지역 객체이고 const라 변경 가능한 함수는 호출할 수 없다.
    const CheckoutService checkout;
    const Cart valid{{"Notebook", Cents{450}, 2}, {"Pen", Cents{120}, 3}};
    const Cart invalid{{"Mystery item", Cents{100}, 0}};

    const auto valid_result = checkout.total(valid);
    const auto invalid_result = checkout.total(invalid);

    print_receipt(valid_result);
    print_receipt(invalid_result);

    // 실행 가능한 증명: 규칙이 깨지면 assert가 프로그램을 중단한다.
    assert(valid_result && valid_result->value == 1260);
    assert(!invalid_result &&
           invalid_result.error() == CheckoutError::invalid_quantity);
}
