// <iostream>은 연습 결과를 출력하는 std::cout을 제공한다.
#include <iostream>
// <string>은 결과 문자열을 소유한다.
#include <string>
// <type_traits>는 방문한 대안의 타입을 컴파일 시간에 판별한다.
#include <type_traits>
// <variant>는 닫힌 명령 집합과 타입 안전 방문을 제공한다.
#include <variant>

struct Add { int left{}; int right{}; }; // struct의 공개 멤버 두 개를 중괄호로 0 초기화한다.
struct Reset {}; // 데이터가 없는 명령도 서로 다른 타입으로 의미를 표현한다.
using Operation = std::variant<Add, Reset>;

// 반환형 int, 매개변수는 복사를 피하는 const 참조이며 함수 호출자는 활성 대안을 몰라도 된다.
[[nodiscard]] int evaluate(const Operation& operation) {
    return std::visit([](const auto& value) -> int {
        using T = std::remove_cvref_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Add>) {
            return value.left + value.right; // + 연산자가 두 int를 더한다.
        } else {
            return 0; // Reset 불변식: 결과는 언제나 0이다.
        }
    }, operation);
}

int main() {
    const Operation addition{Add{20, 22}}; // prvalue Add가 variant 안에 들어가며 addition은 const lvalue가 된다.
    const Operation reset{Reset{}};
    const int sum{evaluate(addition)}; // 함수 반환 int prvalue로 sum을 직접 초기화한다.
    std::cout << sum << ' ' << evaluate(reset) << '\n';
    // 직접 해보기: Multiply 타입을 추가하면 방문자가 모든 대안을 처리하도록 분기를 추가해야 한다.
    return sum == 42 && evaluate(reset) == 0 ? 0 : 1;
}
