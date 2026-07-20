/*
[기초 문법부터 읽는 순서]
1. Cents 구조체는 돈을 일반 int와 구별하는 강한 타입입니다. 같은 정수라도 의미가
   다른 값을 실수로 섞지 않게 합니다.
2. vector<LineItem>에 Cart라는 별명을 붙여 장바구니가 여러 상품을 소유함을 표현합니다.
3. expected<Cents, CheckoutError>는 합계 또는 오류 중 하나만 저장합니다.
4. operator+를 정의하면 Cents끼리 `left + right` 문법을 사용할 수 있습니다.
5. constexpr는 가능한 경우 컴파일 시점에도 계산할 수 있음을 뜻합니다.
6. const Cart&는 장바구니를 복사하거나 수정하지 않고 읽고, 범위 for로 상품을 하나씩 검사합니다.
7. if (!checked)는 expected의 실패를 확인하고 error()로 오류 값을 꺼냅니다.
8. enum class와 switch는 가능한 오류를 이름으로 제한하고 각 오류의 문구를 선택합니다.
9. CheckoutService는 검증·합계 규칙을 담당하고 print_receipt는 출력만 담당하여 역할을 분리합니다.
*/

#include <cassert>
#include <expected>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// A strong type prevents accidentally mixing an item count with money.
struct Cents {
    int value{};
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
    return Cents{left.value + right.value};
}

[[nodiscard]] std::expected<LineItem, CheckoutError>
validate(LineItem item) {
    if (item.name.empty()) {
        return std::unexpected(CheckoutError::blank_name);
    }
    if (item.unit_price.value < 0) {
        return std::unexpected(CheckoutError::invalid_price);
    }
    if (item.quantity <= 0) {
        return std::unexpected(CheckoutError::invalid_quantity);
    }
    return item;
}

class CheckoutService {
public:
    [[nodiscard]] CheckoutResult total(const Cart& cart) const {
        if (cart.empty()) {
            return std::unexpected(CheckoutError::empty_cart);
        }

        Cents result{};
        for (const LineItem& item : cart) {
            auto checked = validate(item);
            if (!checked) {
                return std::unexpected(checked.error());
            }
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
