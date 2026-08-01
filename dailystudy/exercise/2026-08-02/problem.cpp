// <iostream>은 연습 결과 출력에 필요한 std::cout을 제공한다.
#include <iostream>
// <optional>은 성공 값 또는 빈 상태를 함께 표현한다.
#include <optional>
// <string>은 소유 문자열 타입을 제공한다.
#include <string>
// <string_view>는 문자열을 소유하지 않고 읽는 뷰를 제공한다.
#include <string_view>

// using은 새 타입이 아니라 기존 긴 타입 이름에 읽기 쉬운 별칭을 붙인다.
using MaybeNumber = std::optional<int>;

// 반환형 MaybeNumber와 읽기 전용 뷰 매개변수가 함수의 입력·출력 계약이다.
[[nodiscard]] MaybeNumber parse_digit(std::string_view text) {
    if (text.size() != 1) { // size() 반환값을 1과 비교하고 !=가 참이면 실패한다.
        return std::nullopt;
    }
    const char ch{text.front()}; // char 기본 타입에 첫 문자를 중괄호 직접 초기화한다.
    if (ch < '0' || ch > '9') { // ||는 왼쪽이 참이면 오른쪽을 평가하지 않는 논리 OR이다.
        return std::nullopt;
    }
    // 문자 숫자는 연속된 코드 값을 가지므로 '0'과의 차이가 실제 정수 값이다.
    return ch - '0';
}

int main() {
    const std::string input{"7"}; // string이 문자 버퍼를 소유해 string_view가 함수 호출 동안 안전하다.
    const MaybeNumber number{parse_digit(input)}; // 반환 prvalue로 optional 객체를 직접 초기화한다.
    if (!number) { // optional의 bool 문맥 변환으로 값 존재를 검사하고 !로 반전한다.
        std::cout << "invalid\n";
        return 1;
    }
    const int& value{*number}; // *가 optional 내부 int lvalue를 만들고 참조는 number 수명 안에서만 유효하다.
    std::cout << value << '\n';
    // 개념적으로 문자 로드·비교·뺄셈·분기가 있지만 실제 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다.
    return value == 7 ? 0 : 1;
}
