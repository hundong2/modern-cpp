#include <charconv>     // 예외 없이 문자를 정수로 바꾸는 from_chars를 사용한다.
#include <cstdlib>      // std::exit와 EXIT_FAILURE로 검증 실패를 종료한다.
#include <iostream>     // cout과 cerr로 결과·오류를 출력한다.
#include <optional>     // 유효한 양수 또는 값 없음 상태를 표현한다.
#include <string_view>  // 입력 문자를 복사하거나 소유하지 않고 읽는다.

// 변환 실패는 정상적인 입력 결과다. -1 같은 마법 값이나 예외 대신 optional로 표현한다.
[[nodiscard]] std::optional<int> parse_positive_count(std::string_view text) {
    // [[nodiscard]]는 호출자가 성공/실패 결과를 버릴 때 경고하도록 돕는다.
    int value{}; // 이름 있는 객체이므로 lvalue이며 from_chars가 이 저장 위치에 결과를 쓴다.
    const char* first = text.data(); // 포인터 값은 복사되지만 가리킨 문자의 소유권은 얻지 않는다.
    const char* last = first + text.size();
    // 함수 호출 결과는 prvalue. 구조적 바인딩으로 결과 포인터와 오류 코드를 각각 이름 붙인다.
    const auto [next, error] = std::from_chars(first, last, value);

    if (error != std::errc{} || next != last || value <= 0) {
        // 세 조건 중 하나라도 참이면 전체 입력이 유효한 양수가 아니다.
        return std::nullopt;
    }
    return value;
}

[[nodiscard]] int pages_to_read(std::optional<int> daily_pages, int days) {
    // 값 매개변수는 호출자의 optional과 독립된 작은 복사본을 사용한다.
    // optional이 비었을 때 value_or가 눈에 보이는 기본값 10을 제공한다.
    // 곱셈 결과 int는 prvalue다. 작은 정수 연산은 보통 레지스터에서 수행되지만 컴파일러가 결정한다.
    return daily_pages.value_or(10) * days;
}

void require(bool condition, std::string_view explanation) {
    // 실패 설명 view는 호출 중 문자열 리터럴을 빌리며 별도 할당이 없다.
    if (!condition) {
        std::cerr << "[FAILED] " << explanation << '\n';
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    // auto는 함수 반환식으로부터 optional<int> 타입을 추론한다.
    const auto valid = parse_positive_count("12");
    const auto zero = parse_positive_count("0");
    const auto mixed = parse_positive_count("12pages");

    require(valid.has_value(), "12 is a positive integer");
    require(*valid == 12, "operator* reads the contained value"); // *valid는 optional 내부 int의 lvalue 참조다.
    require(!zero, "zero is rejected");
    require(!mixed, "the whole input must be numeric");
    require(pages_to_read(valid, 3) == 36, "present optionals are used");
    require(pages_to_read(std::nullopt, 3) == 30, "empty optionals use the default");
    // nullopt는 optional의 '값 없음' 상태를 명시적으로 만드는 태그 객체다.

    std::cout << "valid daily pages: " << *valid << '\n';
    std::cout << "three-day fallback plan: " << pages_to_read(std::nullopt, 3) << '\n';
    std::cout << "[TESTS] optional syntax exercise passed\n";
}
