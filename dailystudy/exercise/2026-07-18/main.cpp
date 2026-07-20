#include <cassert>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// A strong type prevents accidentally mixing an item count with money.
struct Cents {
    int value{}; // 강한 타입: 기계 수준 표현은 int와 비슷해도 C++ 타입 검사는 서로 구분한다.
};

struct LineItem {
    std::string name;
    Cents unit_price;
    int quantity{};
};

enum class CheckoutError {
    empty_cart,
    blank_name,
    invalid_price,
    invalid_quantity
};

using Cart = std::vector<LineItem>;
using CheckoutResult = std::expected<Cents, CheckoutError>;

[[nodiscard]] constexpr Cents operator+(Cents left, Cents right) {
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
    [[nodiscard]] CheckoutResult total(const Cart& cart) const {
        if (cart.empty()) {
            return std::unexpected(CheckoutError::empty_cart);
        }

        Cents result{};
        for (const LineItem& item : cart) { // const 참조로 string을 포함한 상품 복사를 피한다.
            auto checked = validate(item);
            if (!checked) {
                return std::unexpected(checked.error());
            }
            // 오른쪽 덧셈 결과는 prvalue, 왼쪽 result는 대입 가능한 lvalue다.
            result = result + Cents{checked->unit_price.value * checked->quantity};
        }
        return result;
    }
};

[[nodiscard]] constexpr std::string_view message(CheckoutError error) {
    switch (error) {
    case CheckoutError::empty_cart: return "the cart is empty";
    case CheckoutError::blank_name: return "an item name is blank";
    case CheckoutError::invalid_price: return "an item price is negative";
    case CheckoutError::invalid_quantity: return "an item quantity is not positive";
    }
    return "unknown error";
}

void print_receipt(const CheckoutResult& result) {
    if (result) {
        std::cout << "Checkout total: " << result->value << " cents\n";
    } else {
        std::cout << "Checkout failed: " << message(result.error()) << '\n';
    }
}

int main() {
    const CheckoutService checkout;
    const Cart valid{{"Notebook", Cents{450}, 2}, {"Pen", Cents{120}, 3}};
    const Cart invalid{{"Mystery item", Cents{100}, 0}};

    const auto valid_result = checkout.total(valid);
    const auto invalid_result = checkout.total(invalid);

    print_receipt(valid_result);
    print_receipt(invalid_result);

    // Executable proof: the program stops if its rules are broken.
    assert(valid_result && valid_result->value == 1260);
    assert(!invalid_result &&
           invalid_result.error() == CheckoutError::invalid_quantity);
}
