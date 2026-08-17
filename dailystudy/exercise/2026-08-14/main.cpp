// <iostream>은 std::cout, <memory>는 unique_ptr, <utility>는 std::move를 제공한다.
#include <iostream>
#include <memory>
#include <utility>

// class는 기본 private이다. 공개 계약만 public에 둔다.
class PricePolicy {
public:
    // 생성자·소멸자에는 반환형이 없다. 가상 소멸자는 기반 포인터 삭제를 안전하게 한다.
    virtual ~PricePolicy() = default;
    [[nodiscard]] virtual int apply(int price) const = 0; // const 순수 가상 함수는 읽기 전용 계약이다.
};

// public 상속은 PricePolicy 계약을 구현하고, final은 추가 파생을 막아 정책 구현 경계를 고정한다.
class PercentDiscount final : public PricePolicy {
public:
    // explicit은 int의 암시 변환을 막는다. 멤버 초기화 목록은 본문 전에 멤버를 초기화한다.
    explicit PercentDiscount(int percent) : percent_{percent} {}
    // int 반환형과 int 매개변수를 가지며, override로 기반 가상 함수를 재정의한다. *, -, /는 할인액을 정수 연산한다.
    [[nodiscard]] int apply(int price) const override { return price * (100 - percent_) / 100; }
private:
    int percent_{}; // 기본 타입 멤버를 중괄호로 0 초기화한다.
};

// Checkout은 계산을 직접 구현하지 않고 PricePolicy 인터페이스에 위임하는 Strategy 사용 객체다.
class Checkout final {
public:
    explicit Checkout(std::unique_ptr<PricePolicy> policy) : policy_{std::move(policy)} {} // 소유권을 이동한다.
    [[nodiscard]] int total(int price) const { return policy_->apply(price); } // ->와 가상 호출로 정책을 사용한다.
private:
    std::unique_ptr<PricePolicy> policy_{}; // 단일 소유권 멤버가 객체 수명을 관리한다.
};

int main() {
    // make_unique<PercentDiscount>(20)는 정수 인자를 생성자에 전달하고 unique_ptr<PercentDiscount> prvalue를 반환한다.
    // 반환 포인터가 객체를 단독 소유하며 할당 실패 시 bad_alloc이 가능하다.
    auto policy{std::make_unique<PercentDiscount>(20)};
    // move(policy)는 unique_ptr&& xvalue를 반환하고 Checkout 생성자가 소유권을 이동한다. policy는 빈 유효 상태다.
    Checkout checkout{std::move(policy)};
    const int result{checkout.total(1000)}; // const int를 직접 초기화한다.
    std::cout << result << '\n'; // << 연산자가 값을 스트림에 삽입한다.
    // 가상 호출은 간접 호출일 수 있으나 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return result == 800 && !policy ? 0 : 1; // ==, &&, !로 결과와 이동 후 빈 소유권을 검사하고 ?:로 종료 코드를 고른다.
}
