#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

struct Pending {
    int item_count{};
};

struct Paid {
    int item_count{};
    int paid_won{};
};

struct Shipped {
    std::string tracking_number;
};

using OrderState = std::variant<Pending, Paid, Shipped>;

std::string describe(const OrderState& state) {
    return std::visit(
        [](const auto& current) -> std::string {
            using T = std::decay_t<decltype(current)>;

            if constexpr (std::is_same_v<T, Pending>) {
                return "결제 대기: 상품 " + std::to_string(current.item_count) + "개";
            } else if constexpr (std::is_same_v<T, Paid>) {
                return "결제 완료: " + std::to_string(current.paid_won) + "원";
            } else {
                return "배송 중: " + current.tracking_number;
            }
        },
        state);
}

bool is_shipped(const OrderState& state) {
    return std::holds_alternative<Shipped>(state);
}

int main() {
    OrderState state{Pending{2}};
    assert(std::get<Pending>(state).item_count == 2);
    assert(describe(state) == "결제 대기: 상품 2개");

    state = Paid{2, 25'000};
    assert(std::get_if<Paid>(&state) != nullptr);
    assert(describe(state) == "결제 완료: 25000원");

    state = Shipped{"SEOUL-2026-0720"};
    assert(is_shipped(state));
    assert(describe(state) == "배송 중: SEOUL-2026-0720");

    std::cout << describe(state) << '\n';
    std::cout << "[검증 완료] variant 상태 모델 예제가 통과했습니다.\n";
}
