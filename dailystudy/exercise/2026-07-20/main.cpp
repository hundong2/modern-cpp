/*
[기초 문법부터 읽는 순서]
1. #include는 필요한 표준 기능의 선언을 가져옵니다. string은 문자열,
   variant는 여러 후보 중 한 값을 저장할 때 사용합니다.
2. struct는 상태별 데이터를 묶고, `int item_count{}`는 정수를 0으로 초기화합니다.
3. using OrderState는 긴 variant 타입에 별명을 붙입니다. Pending, Paid, Shipped 중
   한 시점에는 정확히 하나만 저장됩니다.
4. 함수 매개변수의 const OrderState&는 state를 복사하거나 수정하지 않고 읽습니다.
5. 람다 `[](const auto& current)`의 []는 외부 변수를 쓰지 않음, auto는 현재 상태
   타입을 컴파일러가 추론함, &는 복사하지 않음을 뜻합니다.
6. decltype은 식의 타입을 구하고 decay_t는 const와 참조를 제거합니다.
7. if constexpr는 컴파일 시 타입에 맞는 분기만 선택하고, std::visit는 현재 상태를
   그 람다에 전달합니다.
8. get은 맞는 타입이라고 확신할 때, get_if는 틀리면 nullptr가 필요할 때,
   holds_alternative는 타입 여부만 묻고 싶을 때 사용합니다.
9. assert는 괄호 안 조건이 참인지 확인하여 상태 변화와 출력 결과를 검증합니다.
*/

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
