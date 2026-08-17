// <iostream>은 표준 출력 객체 std::cout와 출력 연산자 <<를 제공한다.
#include <iostream>
// <memory>는 단독 소유 스마트 포인터 std::unique_ptr와 std::make_unique를 제공한다.
#include <memory>
// <utility>는 소유권 이전 의도를 표현하는 std::move를 제공한다.
#include <utility>

// struct는 기본 접근이 public이며 단순 값 객체의 공개 데이터를 표현하기 좋다.
struct Money {
    int won{}; // int는 정수 기본 타입이고 {}는 0으로 값 초기화한다.
};

// class는 기본 접근이 private이다. public 순수 가상 함수만 노출해 정책 포트를 만든다.
class DiscountPolicy {
public:
    // 가상 소멸자는 기반 포인터로 파생 객체를 지울 때 올바른 소멸을 보장한다.
    virtual ~DiscountPolicy() = default;
    // 반환형 Money, const 참조 매개변수, 끝의 const는 각각 결과·무복사 입력·상태 불변 계약이다.
    [[nodiscard]] virtual Money apply(const Money& price) const = 0;
};

// final은 더 이상의 상속을 막아 이 어댑터의 동작 경계를 고정한다.
class FixedRatePolicy final : public DiscountPolicy {
public:
    // 생성자에는 반환형이 없다. explicit은 int가 정책으로 암시 변환되는 것을 막는다.
    explicit FixedRatePolicy(int percent)
        : percent_{percent} {} // 멤버 초기화 목록은 생성자 본문 전에 멤버를 직접 초기화한다.

    [[nodiscard]] Money apply(const Money& price) const override {
        // *와 /는 정수 산술 연산자다. Money{...} prvalue가 반환 목적지에 직접 생성될 수 있다.
        return Money{price.won * (100 - percent_) / 100};
    }

private:
    int percent_{}; // private 멤버는 구현 상태를 외부의 무제한 변경에서 숨긴다.
};

// using은 긴 템플릿 타입에 읽기 쉬운 별칭을 붙이며 새 타입을 만들지는 않는다.
using PolicyOwner = std::unique_ptr<DiscountPolicy>;

class CheckoutService {
public:
    // 값으로 받은 unique_ptr는 호출자가 소유권을 넘겨야 한다. nullptr 검사는 예제에서 생략한다.
    explicit CheckoutService(PolicyOwner policy)
        : policy_{std::move(policy)} {} // policy는 lvalue지만 std::move(policy)는 xvalue라 이동 생성된다.

    [[nodiscard]] Money checkout(const Money& price) const {
        // ->는 포인터가 가리키는 객체의 멤버를 호출한다. 가상 디스패치로 실제 정책이 선택된다.
        return policy_->apply(price);
    }

private:
    PolicyOwner policy_{}; // unique_ptr가 정책 객체의 수명을 단독 소유하며 서비스와 함께 파괴한다.
};

int main() { // 운영체제에 성공 여부를 int로 돌려주는 진입 함수다.
    // make_unique<FixedRatePolicy>(20)는 정수 20을 생성자에 전달해 객체를 만들고 unique_ptr<FixedRatePolicy>를 반환한다.
    // 반환 포인터가 유일한 소유자이며 할당 실패 시 bad_alloc이 가능하다.
    auto policy = std::make_unique<FixedRatePolicy>(20);
    // move(policy)는 unique_ptr&& xvalue를 반환하고 CheckoutService의 값 매개변수와 멤버로 소유권이 연쇄 이동된다.
    // 함수 자체가 객체를 옮기는 것은 아니며 이동 뒤 policy는 빈 유효 상태다.
    CheckoutService service{std::move(policy)};
    const Money result{service.checkout(Money{10000})}; // 임시 Money는 const 참조에 호출 동안 바인딩된다.
    std::cout << result.won << '\n'; // 멤버 접근 .과 출력 호출로 8000을 표시한다.
    // == 비교 결과에 따라 ?: 조건 연산자가 성공 0 또는 실패 1을 고른다.
    return result.won == 8000 ? 0 : 1;
}
