// <compare>는 std::strong_ordering과 C++20 삼방향 비교 기능을 제공한다.
#include <compare>
// <iostream>은 표준 출력 객체 std::cout과 출력 연산자 <<를 제공한다.
#include <iostream>
// <string>은 문자를 소유하고 관리하는 std::string을 제공한다.
#include <string>
// <tuple>은 여러 값을 묶어 사전식으로 비교하는 std::tuple을 제공한다.
#include <tuple>
// <utility>는 객체를 xvalue로 바꾸는 std::move를 제공한다.
#include <utility>
// <vector>는 같은 타입의 원소를 연속 메모리에 소유하는 가변 배열을 제공한다.
#include <vector>

// using은 긴 타입 이름에 별칭을 붙인다. 새 타입을 만드는 것이 아니라 기존 int와 같은 타입이다.
using Won = int;

// struct는 class와 달리 기본 접근이 public이다. 간단한 데이터 묶음에 자주 쓴다.
struct RawOrder {
    // std::string 멤버 변수는 주문 이름 문자열의 메모리를 직접 소유한다.
    std::string name;
    // 기본 타입 int 멤버는 등급 숫자를 값으로 저장한다.
    int grade;
    // std::size_t는 컨테이너 크기와 인덱스에 알맞은 부호 없는 정수 타입이다.
    std::size_t arrival;
};

// class의 기본 접근은 private이다. 불변식을 지키는 값 객체에 적합하다.
class Money {
public:
    // 생성자에는 반환형이 없다. explicit은 int 하나가 Money로 몰래 바뀌는 암시적 변환을 막는다.
    explicit Money(Won amount)
        // 멤버 초기화 목록은 생성자 본문보다 먼저 amount_를 직접 초기화한다.
        : amount_{amount < 0 ? 0 : amount} {}

    // const 멤버 함수는 amount_를 바꾸지 않으며, 반환형 Won(int)을 값으로 돌려준다.
    [[nodiscard]] Won amount() const { return amount_; }

    // = default는 int 멤버의 완전한 순서를 이용해 비교 함수를 컴파일러가 만들게 한다.
    auto operator<=>(const Money&) const = default;
    // ==도 기본 생성해야 두 Money의 동등 비교 연산자 ==를 사용할 수 있다.
    bool operator==(const Money&) const = default;

private:
    // private 멤버 변수는 외부 코드가 음수 값을 직접 넣지 못하게 막는다.
    Won amount_;
};

class OrderPriority {
public:
    // 매개변수 grade와 arrival은 호출자가 전달한 값을 받는다. 생성자에는 반환형이 없다.
    explicit OrderPriority(int grade, std::size_t arrival)
        : grade_{grade}, arrival_{arrival} {}

    // 반환형 std::strong_ordering은 모든 두 객체가 작음·같음·큼 중 하나임을 뜻한다.
    [[nodiscard]] std::strong_ordering operator<=>(const OrderPriority& other) const {
        // std::tuple의 클래스 템플릿 인자 추론이 두 표현식의 타입을 알아내 값 튜플을 만든다.
        // tuple{a,b} 생성자는 두 값을 입력받아 소유 튜플 prvalue를 만든다. <=>는 첫 원소부터 비교해 strong_ordering을 반환한다.
        // 높은 grade를 먼저 두려고 단항 - 연산자를 쓰고, arrival는 작을수록 먼저 둔다. 원본 멤버는 바뀌지 않는다.
        return std::tuple{-grade_, arrival_} <=> std::tuple{-other.grade_, other.arrival_};
    }

    bool operator==(const OrderPriority&) const = default;

private:
    int grade_;
    std::size_t arrival_;
};

// 템플릿 인자 T는 호출 시 실제 타입으로 치환된다. 여기서는 Money가 T가 된다.
template <typename T>
[[nodiscard]] const T& smaller(const T& left, const T& right) {
    // const T&는 복사하지 않는 비소유 참조다. 인자가 함수 호출 동안 살아 있어야 한다.
    // < 연산자는 Money의 <=>에서 재작성된다. 비교 뒤 참인 쪽을 조건 연산자 ?:로 선택한다.
    return left < right ? left : right;
}

// 추상 class는 출력이라는 아키텍처 포트를 정의하며 기본 접근은 private이다.
class IOrderSink {
public:
    // virtual 소멸자는 기반 클래스 포인터로 파생 객체를 지울 때 올바른 소멸을 보장한다.
    virtual ~IOrderSink() = default;
    // = 0은 순수 가상 함수다. 반환형 void는 값을 돌려주지 않고, 매개변수는 const 비소유 참조다.
    virtual void publish(const RawOrder& order, const Money& price) const = 0;
};

class ConsoleOrderSink final : public IOrderSink {
public:
    // override는 기반 포트의 함수 시그니처를 정확히 구현했는지 컴파일러가 검사하게 한다.
    void publish(const RawOrder& order, const Money& price) const override {
        // << 연산자는 값을 std::cout 출력 스트림에 차례로 전달한다.
        std::cout << order.name << ": " << price.amount() << "원\n";
    }
};

// 반환형 std::vector<RawOrder>는 동적 배열을 값으로 반환한다.
[[nodiscard]] std::vector<RawOrder> make_orders() {
    // 중괄호 초기화는 축소 변환을 막고 각 struct 멤버를 선언 순서대로 초기화한다.
    std::vector<RawOrder> orders{{"문서 생성", 2, 1}, {"긴급 장애", 5, 2}, {"코드 정리", 2, 0}};
    // 이름 있는 orders는 lvalue다. 값 반환에서 NRVO로 복사를 생략할 수 있다.
    return orders;
}

// 반환형 void: 결과 값 없이 포트를 호출한다. sink는 호출자 소유 객체를 가리키는 비소유 참조다.
void publish_all(const std::vector<RawOrder>& orders, const IOrderSink& sink) {
    // 범위 for는 각 원소를 const 참조로 바인딩해 문자열을 포함한 RawOrder 복사를 피한다.
    for (const RawOrder& order : orders) {
        // 함수 호출은 인자를 전달하고 제어를 옮긴다. virtual 호출은 런타임 실제 타입의 함수를 간접 선택할 수 있다.
        sink.publish(order, Money{1000 + order.grade * 500});
    }
}

// main의 반환형 int는 운영체제에 종료 상태를 전달한다. 매개변수 없는 함수 호출로 시작된다.
int main() {
    // direct-list-initialization은 explicit 생성자를 올바르게 호출한다. Money price = 1200;은 허용되지 않는다.
    const Money cheap{1200};
    const Money expensive{5000};
    // smaller 호출의 템플릿 인자 Money는 추론된다. 반환 참조는 살아 있는 두 지역 객체 중 하나를 가리킨다.
    const Money& selected{smaller(cheap, expensive)};
    std::cout << "더 작은 금액: " << selected.amount() << "원\n";

    const OrderPriority normal{2, 10};
    const OrderPriority urgent{5, 20};
    // 비교 과정은 값을 로드·비교하고 조건 분기를 만들 수 있으나 실제 명령은 CPU·ABI·컴파일러·옵션에 따라 다르다.
    if (urgent < normal) {
        std::cout << "긴급 주문이 정렬상 먼저입니다.\n";
    }

    // 함수가 반환한 prvalue로 orders를 직접 초기화한다. 보장/선택적 복사 생략으로 중간 복사가 없어질 수 있다.
    std::vector<RawOrder> orders{make_orders()};
    // ConsoleOrderSink 객체는 스택 수명을 가지며 publish_all 호출이 끝날 때까지 살아 있다.
    const ConsoleOrderSink sink{};
    publish_all(orders, sink);

    // 포인터는 주소를 저장하며 nullptr일 수 있다. &sink는 살아 있는 객체의 주소를 얻는다.
    const IOrderSink* sink_pointer{&sink};
    // nullptr과 비교해 주소 유효 조건을 확인한 뒤 -> 연산자로 가상 함수를 호출한다.
    if (sink_pointer != nullptr) {
        // vector::front()는 인자 없이 첫 RawOrder&를 반환한다. orders가 비어 있지 않아야 하며 vector 상태는 바꾸지 않는다.
        sink_pointer->publish(orders.front(), Money{900});
    }

    // std::move는 orders를 xvalue로 만들 뿐이다. vector 이동 생성자가 저장소 소유권을 moved로 넘긴다.
    // move(orders)는 vector&& xvalue를 반환하고 moved의 이동 생성자가 버퍼 소유권을 넘겨받는다. orders는 유효한 미지정 상태다.
    std::vector<RawOrder> moved{std::move(orders)};
    // size()는 인자 없이 현재 원소 수 size_type을 O(1)에 반환하고 moved를 바꾸지 않는다.
    std::cout << "이동된 주문 수: " << moved.size() << '\n';
    return 0;
}
