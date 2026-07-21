#include <cassert>      // assert로 상태 전이와 설명 문자열을 검증한다.
#include <iostream>     // cout으로 최종 상태를 출력한다.
#include <string>       // 배송 번호와 설명 문자 버퍼를 소유한다.
#include <type_traits>  // 컴파일 시간 타입 비교 is_same_v와 decay_t를 사용한다.
#include <variant>      // 여러 상태 타입 중 하나만 저장하는 variant를 사용한다.

struct Pending {
    // struct 멤버는 기본 public이며 대기 상태에 필요한 데이터만 가진다.
    int item_count{}; // {}는 0 초기화. 이름 있는 멤버에 접근한 식은 일반적으로 lvalue다.
};

struct Paid {
    int item_count{};  // 결제된 상품 수를 0으로 값 초기화한다.
    int paid_won{};    // 결제 금액을 0으로 값 초기화한다.
};

struct Shipped {
    std::string tracking_number;  // 배송 번호 문자의 메모리와 수명을 소유한다.
};

using OrderState = std::variant<Pending, Paid, Shipped>; // 태그와 가장 큰 후보를 담을 저장 공간을 갖는 형태가 일반적이다.
// using은 새 클래스를 만들지 않고 세 상태를 담는 타입에 별칭을 붙인다.

std::string describe(const OrderState& state) {
    // 반환형 string은 호출자가 소유할 새 설명 문자열을 값으로 돌려준다는 뜻이다.
    // state는 const lvalue 참조라 variant와 내부 string을 복사하지 않고 읽는다.
    return std::visit(
        // 제네릭 람다의 auto 매개변수 타입은 현재 활성 상태에 맞춰 컴파일된다.
        [](const auto& current) -> std::string {
            // current는 참조 타입이다. decay_t로 const와 &를 제거해야 Pending 같은 원본 타입과 비교할 수 있다.
            using T = std::decay_t<decltype(current)>;

            if constexpr (std::is_same_v<T, Pending>) {
                // if constexpr는 거짓인 타입 분기의 코드를 컴파일 결과에서 제외한다.
                // + 연결 결과 string은 prvalue. 반환 객체를 직접 구성하거나 이동할 수 있다.
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
    // holds_alternative는 활성 타입이 Shipped인지 bool로 알려주며 값을 꺼내지 않는다.
    return std::holds_alternative<Shipped>(state);
}

int main() {
    // main은 프로그램 진입점이며 끝까지 실행되면 정상 종료 코드 0을 반환한다.
    OrderState state{Pending{2}}; // Pending{2}는 prvalue, state는 이후 이름으로 접근하므로 lvalue다.
    assert(std::get<Pending>(state).item_count == 2);
    assert(describe(state) == "결제 대기: 상품 2개");

    state = Paid{2, 25'000}; // 오른쪽 prvalue로 활성 후보를 Pending에서 Paid로 바꾸고 이전 객체 수명을 끝낸다.
    assert(std::get_if<Paid>(&state) != nullptr);
    // get_if는 상태 주소를 받아 타입이 맞으면 내부 객체 포인터, 아니면 nullptr를 돌려준다.
    assert(describe(state) == "결제 완료: 25000원");

    state = Shipped{"SEOUL-2026-0720"};
    // 대입은 기존 Paid 수명을 끝내고 새 Shipped 객체와 string을 활성화한다.
    assert(is_shipped(state));
    assert(describe(state) == "배송 중: SEOUL-2026-0720");

    std::cout << describe(state) << '\n';
    std::cout << "[검증 완료] variant 상태 모델 예제가 통과했습니다.\n";
}
