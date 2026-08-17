#include <concepts>  // std::convertible_to로 표현식 결과 타입의 조건을 설명한다.
#include <iostream>  // std::cout으로 학습 결과를 콘솔에 출력한다.
#include <string>    // std::string으로 상품 이름 문자열을 소유한다.
#include <utility>   // std::move로 lvalue를 xvalue로 바꾸는 예를 만든다.
#include <vector>    // std::vector로 여러 Item 객체를 연속 컨테이너에 소유한다.

// struct의 기본 접근은 public이지만, 초보자가 경계를 눈으로 확인하도록 public을 적는다.
struct Item {
public:
    std::string name;  // 멤버 변수는 각 Item 객체가 가진 상품 이름 상태다.
    int price;         // int는 기본 정수 타입이며 여기서는 원 단위 가격을 저장한다.
};

// 긴 컨테이너 타입에 읽기 쉬운 별칭을 붙인다. Item은 vector의 템플릿 인자다.
using ItemList = std::vector<Item>;

// 함수 앞의 ItemList는 반환형이고, 이 함수는 매개변수 없이 샘플 목록을 만든다.
ItemList make_items() {
    // 각 Item{...}은 prvalue이고, 반환 결과로 목적 ItemList를 직접 만들면 복사 생략이 적용될 수 있다.
    return ItemList{Item{"책", 20'000}, Item{"키보드", 50'000}, Item{"펜", 1'000}};
}

// P는 검사할 타입 템플릿 인자다. requires 괄호의 p와 price는 검사 전용 가상 매개변수다.
template <typename P>
concept DiscountPolicy = requires(const P& p, int price) {
    // 함수 호출 결과가 int로 변환 가능해야 한다는 문법 계약이며, 실제 할인값의 타당성까지 보장하지는 않는다.
    { p.discount_for(price) } -> std::convertible_to<int>;
};

// class의 기본 접근은 private다. 여기서는 공개 생성자와 비공개 상태를 명시한다.
class RatePolicy {
public:
    // 생성자는 반환형이 없고, int 매개변수 rate를 받아 멤버 초기화 목록으로 rate_를 만든다.
    // explicit은 RatePolicy policy = 10; 같은 암시적 변환을 막고 RatePolicy policy{10}; 직접 초기화만 허용한다.
    explicit RatePolicy(int rate) : rate_{rate} {}

    // 반환형 int, 읽기 전용 매개변수 price, 뒤의 const는 이 호출이 rate_를 바꾸지 않음을 뜻한다.
    [[nodiscard]] int discount_for(int price) const {
        // *와 /는 곱셈·나눗셈 연산자다. 정수 나눗셈은 소수 부분을 버린다.
        return price * rate_ / 100;
    }

private:
    int rate_;  // private 멤버 변수는 클래스 밖에서 직접 변경할 수 없는 할인율 상태다.
};

// 실행 중 출력 목적지를 교체하기 위한 추상 인터페이스다.
class IReceiptSink {
public:
    // 기본 생성자와 달리 가상 소멸자는 기반 클래스 포인터로 파생 객체를 지울 때 올바른 파괴를 보장한다.
    virtual ~IReceiptSink() = default;

    // = 0인 순수 가상 함수는 파생 클래스가 구현해야 하는 계약이다.
    virtual void write(int total, int discount, int payment) const = 0;
};

class ConsoleReceiptSink final : public IReceiptSink {
public:
    // override는 기반 클래스 가상 함수와 서명이 맞는지 컴파일러가 검사하게 한다.
    void write(int total, int discount, int payment) const override {
        // <<는 스트림 삽입 연산자이고 std::cout은 표준 출력 객체다.
        std::cout << "합계: " << total << ", 할인: " << discount << ", 결제: " << payment << '\n';
    }
};

// concept로 Policy 템플릿 인자가 DiscountPolicy 계약을 만족하도록 제한한다.
template <DiscountPolicy Policy>
class CheckoutService {
public:
    // Policy를 값으로 받아 소유하고, IReceiptSink*는 소유하지 않고 관찰한다.
    CheckoutService(Policy policy, const IReceiptSink* sink)
        // move(policy)는 인자 하나를 받아 Policy&&로 캐스팅한 xvalue를 반환한다. 직접 이동하지 않고 policy_ 생성자가 이동을 수행한다.
        // policy 값은 이동 후 유효하지만 내용은 타입 계약에 따른 미지정 상태이고 sink 포인터는 그대로 복사된다.
        : policy_{std::move(policy)}, sink_{sink} {}

    // const ItemList&는 목록을 복사하지 않고 읽으며, 함수 뒤 const는 서비스 상태를 바꾸지 않는다는 뜻이다.
    [[nodiscard]] int checkout(const ItemList& items) const {
        int total{0};  // 기본 타입 int 변수를 중괄호로 0에 초기화한다.

        // 범위 기반 for가 각 요소를 순회하고 const 참조 item은 기존 Item lvalue에 바인딩된다.
        for (const Item& item : items) {
            total += item.price;  // += 복합 대입은 현재 합계를 읽고 가격을 더한 뒤 다시 저장한다.
        }

        // 점 연산자로 정책 멤버 함수를 호출한다. 템플릿 정책이라 정적 호출/인라인 후보가 된다.
        const int raw_discount{policy_.discount_for(total)};
        int discount{raw_discount};

        // ||는 논리 OR이고 비교 결과에 따라 조건 분기가 일어날 수 있다.
        if (discount < 0 || discount > total) {
            discount = 0;  // 잘못된 정책 결과를 안전한 값으로 저장한다.
        }

        const int payment{total - discount};  // - 연산자로 결제 금액을 계산하고 const로 재대입을 막는다.

        // 포인터는 nullptr일 수 있으므로 역참조 전 비교한다.
        if (sink_ != nullptr) {
            // ->는 포인터가 가리키는 객체의 멤버에 접근하며, write는 가상 간접 호출이 될 수 있다.
            sink_->write(total, discount, payment);
        }
        return payment;
    }

private:
    Policy policy_;               // 값 멤버로 정책 객체를 소유한다.
    const IReceiptSink* sink_;    // 원시 포인터는 여기서 비소유 관찰 관계를 나타낸다.
};

// int는 운영체제에 종료 상태를 돌려주는 반환형이고 main은 프로그램 시작 함수다.
int main() {
    ItemList items{make_items()};  // make_items()의 prvalue 결과로 items를 직접 초기화해 복사 생략을 활용한다.
    ConsoleReceiptSink sink{};     // 빈 중괄호로 기본 생성한다.
    RatePolicy policy{10};         // explicit 생성자의 올바른 직접 초기화 예다.

    // <RatePolicy>는 템플릿 인자이며 컴파일러가 DiscountPolicy 만족 여부를 검사한다.
    // 같은 move 호출은 RatePolicy&&를 반환하며 service 생성자의 값 매개변수를 이동 초기화한다. 반환 xvalue는 별도 객체가 아니다.
    CheckoutService<RatePolicy> service{std::move(policy), &sink};
    const int paid{service.checkout(items)};  // &는 sink의 주소를 얻고, 함수 호출 결과를 paid에 저장한다.

    // rvalue 참조가 prvalue 임시 객체에 바인딩되어 temporary의 수명까지 객체 수명이 연장된다.
    Item&& temporary{Item{"수명 연장 예제", 0}};
    std::cout << temporary.name << '\n';

    // == 비교 연산자의 bool 결과로 성공 여부를 결정하고 삼항 조건 연산자가 반환값을 고른다.
    return paid == 63'900 ? 0 : 1;
}

