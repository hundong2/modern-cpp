#include <concepts>  // std::same_as concept로 반환형을 정확히 검사한다.
#include <iostream>  // std::cout으로 문제 풀이 결과를 출력한다.
#include <string>    // std::string으로 회원 등급 이름을 소유한다.
#include <utility>   // std::move로 문자열 자원 이동을 요청한다.
#include <vector>    // std::vector로 여러 가격을 소유하고 반복한다.

// struct는 기본 public이지만 접근 의도를 드러내려고 public을 쓴다.
struct Customer {
public:
    std::string grade;  // 각 객체의 회원 등급을 저장하는 멤버 변수다.
};

using Prices = std::vector<int>;  // vector<int>에서 int는 요소 타입 템플릿 인자다.

// Rule 타입이 apply(int)를 const 객체에서 호출하고 정확히 int를 반환하는지 검사한다.
template <typename Rule>
concept PriceRule = requires(const Rule& rule, int value) {
    { rule.apply(value) } -> std::same_as<int>;
};

class MemberRule {
public:
    // 생성자에는 반환형이 없고, 문자열 매개변수를 멤버 초기화 목록에서 이동해 소유한다.
    // explicit은 std::string이 MemberRule로 저절로 변환되는 것을 막는다.
    explicit MemberRule(std::string grade) : grade_{std::move(grade)} {}

    // 반환형은 int이고 value는 입력 가격이며 뒤의 const는 객체 상태를 바꾸지 않음을 뜻한다.
    [[nodiscard]] int apply(int value) const {
        // ==는 문자열 비교 연산자이며 조건이 참이면 10% 할인한 값을 반환한다.
        if (grade_ == "gold") {
            return value * 90 / 100;  // *, / 표준 산술 연산자로 정수 가격을 계산한다.
        }
        return value;
    }

private:
    std::string grade_;  // private 멤버라 외부에서 등급을 직접 덮어쓸 수 없다.
};

// Rule 템플릿 인자를 concept로 제한해 잘못된 규칙을 호출 코드보다 가까운 곳에서 진단한다.
template <PriceRule Rule>
[[nodiscard]] int sum_prices(const Prices& prices, const Rule& rule) {
    int sum{0};  // 기본 정수 타입을 중괄호로 0에 초기화한다.

    // const int&는 vector가 소유한 기존 int lvalue에 읽기 전용으로 바인딩된다.
    for (const int& price : prices) {
        sum += rule.apply(price);  // 점 연산자로 함수를 호출하고 +=로 결과를 누적 저장한다.
    }
    return sum;
}

int main() {
    // Prices가 요소를 소유하며 각 정수 리터럴은 int prvalue다.
    Prices prices{10'000, 20'000, 30'000};
    Customer customer{"gold"};  // 집계체를 중괄호 초기화한다.
    MemberRule rule{customer.grade};  // explicit 생성자를 직접 초기화 방식으로 호출한다.

    // TODO 연습: 아래 호출을 직접 다시 작성하고, 반환형·매개변수·템플릿 인자를 말로 설명한다.
    const int result{sum_prices<MemberRule>(prices, rule)};
    std::cout << "할인 합계: " << result << '\n';  // << 연산자로 표준 라이브러리 출력 객체에 값을 보낸다.

    // make_prices 같은 함수가 Prices{...} prvalue를 반환하면 목적 객체 직접 생성으로 복사 생략될 수 있다.
    // std::move(prices)는 이동이 아니라 lvalue를 xvalue로 바꾸며, 실제 이동은 받는 생성자/대입이 수행한다.
    // 참조와 포인터는 소유하지 않으므로 원본 객체 수명보다 오래 보관하면 안 된다.
    return result == 54'000 ? 0 : 1;  // 비교와 조건 연산자로 테스트 가능한 종료 코드를 반환한다.
}

