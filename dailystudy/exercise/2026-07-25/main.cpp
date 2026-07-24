// <iostream>은 표준 출력 객체 std::cout과 스트림 삽입 연산자 <<를 제공한다.
#include <iostream>
// <ranges>는 C++20 범위 알고리즘과 filter/transform 같은 지연 평가 뷰를 제공한다.
#include <ranges>
// <utility>는 객체를 xvalue로 바꾸는 std::move를 제공한다.
#include <utility>
// <vector>는 원소를 연속 공간에 소유하는 std::vector 컨테이너를 제공한다.
#include <vector>

// class의 기본 접근은 private이며, 돈의 잘못된 변경을 막기 위해 내부 값을 감춘다.
class Money {
public:
    // 생성자는 반환형이 없다. explicit은 int가 Money로 몰래 변환되는 것을 막는다.
    explicit Money(int won) : won_{won} { // 매개변수 won으로 private 멤버를 먼저 초기화한다.
    }

    // 반환형 int, 매개변수 없음. const는 이 호출이 멤버 상태를 바꾸지 않음을 뜻한다.
    [[nodiscard]] int won() const { return won_; }

private:
    int won_{}; // 기본 정수 타입 int 멤버를 중괄호로 0 초기화한다.
};

// struct의 기본 접근은 public이지만 학습을 위해 public을 명시한다.
struct Order {
public:
    int id{};       // 주문 식별자를 저장하는 기본 정수 타입 멤버다.
    Money price{0}; // 직접 중괄호 초기화는 explicit 생성자를 올바르게 호출한다.
};

// using은 긴 컨테이너 타입에 별칭을 붙이고, <Order>는 vector의 템플릿 인자다.
using OrderList = std::vector<Order>;

class OrderRepository {
public:
    // 생성자는 반환형이 없고 OrderList 값을 매개변수로 받는다.
    explicit OrderRepository(OrderList orders)
        : orders_{std::move(orders)} { // std::move(orders)는 lvalue 매개변수를 xvalue로 바꿔 멤버로 이동시킨다.
    }

    // const OrderList&는 소유 컨테이너를 복사하지 않는 읽기 전용 lvalue 참조다.
    [[nodiscard]] const OrderList& orders() const { return orders_; }

private:
    OrderList orders_{}; // 저장소가 주문 벡터의 수명과 소유권을 책임진다.
};

// struct 인터페이스의 기본 public 접근 아래에 가상 출력 계약을 둔다.
struct IOrderSink {
    virtual ~IOrderSink() = default; // 기반 포인터로 파괴해도 올바른 소멸자가 호출된다.
    virtual void write(int id, int won) const = 0; // void 반환형, 두 int 매개변수인 순수 가상 함수다.
};

class ConsoleOrderSink final : public IOrderSink {
public:
    void write(int id, int won) const override {
        // << 연산자는 값을 출력 스트림에 차례로 삽입한다.
        std::cout << "order #" << id << ": " << won << " won\n";
    }
};

class ExpensiveOrderQuery {
public:
    // 참조 멤버는 nullptr일 수 없고, repository가 이 조회 객체보다 오래 살아야 한다.
    explicit ExpensiveOrderQuery(const OrderRepository& repository)
        : repository_{repository} {
    }

    // auto 반환형은 filter_view의 긴 정확한 타입을 컴파일러가 추론하게 한다.
    [[nodiscard]] auto with_minimum(Money minimum) const {
        // [minimum]은 조건 함수 객체 안에 기준 금액을 값으로 복사해 수명을 안전하게 만든다.
        return repository_.orders() | std::views::filter([minimum](const Order& order) {
            // >= 비교 연산자는 두 정수를 비교해 bool을 만들고 filter가 조건 분기한다.
            return order.price.won() >= minimum.won();
        });
    }

private:
    const OrderRepository& repository_; // 비소유 const 참조: 저장은 저장소, 정책은 조회 객체가 맡는다.
};

// 반환형 OrderList, 매개변수 없음인 일반 함수다.
OrderList make_sample_orders() {
    // 반환 prvalue가 호출자의 목적 객체를 직접 만들 수 있어 불필요한 복사가 생략된다.
    return OrderList{
        Order{1, Money{500}},
        Order{2, Money{1500}},
        Order{3, Money{2500}},
    };
}

// int 반환형 main은 운영체제에 프로그램 종료 상태를 돌려주는 진입 함수다.
int main() {
    // make_sample_orders()의 prvalue로 값 매개변수를 만들고 벡터 소유권을 저장소로 이동한다.
    OrderRepository repository{make_sample_orders()};
    const ExpensiveOrderQuery query{repository}; // const 객체는 생성 뒤 관찰 가능한 상태를 바꿀 수 없다.
    const Money minimum{1000};                    // explicit 생성자는 직접 초기화로 호출한다.
    const ConsoleOrderSink sink{};
    const IOrderSink* sink_ptr{&sink}; // 포인터는 주소를 저장하며 nullptr 가능성이 있는 표현 방식이다.

    // view는 이름이 있으므로 lvalue이며 repository의 원소를 소유하지 않고 관찰한다.
    auto view{query.with_minimum(minimum)};
    int count{0}; // 기본 타입 int 변수를 중괄호로 초기화한다.

    // const Order&는 각 기존 원소 lvalue에 읽기 전용으로 바인딩되고 복사를 만들지 않는다.
    for (const Order& order : view) {
        sink_ptr->write(order.id, order.price.won()); // ->와 ()로 가상 함수를 간접 호출한다.
        ++count; // 전위 증가 연산자가 count를 1 늘려 다시 저장한다.
    }

    // &&는 왼쪽이 참일 때만 오른쪽을 평가하는 논리 AND이고 ==는 값을 비교한다.
    if (sink_ptr != nullptr && count == 2) {
        return 0;
    }
    return 1;
}
