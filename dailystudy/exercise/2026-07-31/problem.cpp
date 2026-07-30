// <iostream>은 연습 결과를 출력하는 std::cout과 << 연산자를 제공한다.
#include <iostream>
// <numbers>는 C++20 원주율 상수 pi_v를 제공한다.
#include <numbers>

// using 별칭으로 계산에 쓰는 기본 실수 타입을 한곳에서 정한다.
using Number = double;

// struct의 기본 접근은 public이므로 입력 데이터 멤버를 간단히 공개한다.
struct CircleInput {
    Number radius{}; // 중괄호 초기화로 값이 없을 때 0.0이 되게 한다.
};

// class의 기본 접근은 private이며 public/private 지정자가 경계를 명시한다.
class CircleCalculator {
public:
    // 생성자에는 반환형이 없다. explicit은 Number 하나가 객체로 암시적 변환되는 것을 막는다.
    explicit CircleCalculator(Number scale)
        // 생성자 매개변수 scale로 private 멤버를 본문 전에 직접 초기화한다.
        : scale_{scale} {}

    // 반환형 Number, 매개변수는 복사를 피하는 const 참조, 뒤 const는 멤버 불변 약속이다.
    [[nodiscard]] Number area(const CircleInput& input) const {
        // TODO 1: 반지름 0도 허용하려면 <=를 <로 바꾸고 결과를 관찰한다.
        if (input.radius <= 0.0) {
            return 0.0; // prvalue 0.0이 호출자 결과 객체에 저장된다.
        }
        // pi_v<Number>에서 Number는 표준 상수의 타입을 고르는 템플릿 인자다.
        return std::numbers::pi_v<Number> * input.radius * input.radius * scale_;
    }

private:
    Number scale_{}; // private 멤버 변수는 클래스의 함수만 직접 읽고 쓸 수 있다.
};

int main() { // 함수 호출은 main에서 시작하며 int 반환값은 프로그램 성공 여부다.
    // explicit 생성자는 CircleCalculator calculator = 1.0;을 막고 이 직접 초기화는 허용한다.
    const CircleCalculator calculator{1.0};
    const CircleInput input{2.0}; // 이름 있는 input은 lvalue이고 CircleInput{2.0}은 prvalue로 시작한다.

    // &input은 lvalue 객체 주소를 얻는다. 포인터는 소유하지 않으며 input보다 오래 쓰면 안 된다.
    const CircleInput* const pointer{&input};
    // 참조는 기존 객체의 별명이다. input이 더 오래 살아 있으므로 이 바인딩의 수명은 안전하다.
    const CircleInput& reference{input};

    // nullptr 비교 뒤 && 단축 평가로 null 포인터 역참조를 피한다.
    if (pointer != nullptr && calculator.area(reference) > 12.0) {
        // 표준 라이브러리 함수 area 호출 결과와 -> 멤버 접근 결과를 출력한다.
        std::cout << "radius=" << pointer->radius
                  << ", area=" << calculator.area(*pointer) << '\n';
    }

    // TODO 2: 반복 횟수를 2에서 3으로 바꾸고 출력 줄 수를 먼저 예측한다.
    for (int i{0}; i < 2; ++i) { // ++ 연산자는 i에 1을 더해 다시 저장한다.
        std::cout << "practice " << i << '\n';
    }

    // AreaResult 같은 작은 값 반환은 복사 생략 대상이 될 수 있고, 여기 Number는 값으로 반환된다.
    // 실제 로드·비교·분기·호출 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.
    return calculator.area(input) > 0.0 ? 0 : 1;
}
