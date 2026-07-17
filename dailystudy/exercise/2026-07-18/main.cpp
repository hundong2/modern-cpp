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
