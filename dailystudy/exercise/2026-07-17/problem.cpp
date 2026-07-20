#include <charconv>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string_view>

// A failed conversion is normal input, so std::optional expresses it without
// a magic number such as -1 and without throwing an exception.
[[nodiscard]] std::optional<int> parse_positive_count(std::string_view text) {
    int value{}; // 이름 있는 객체이므로 lvalue이며 from_chars가 이 저장 위치에 결과를 쓴다.
    const char* first = text.data(); // 포인터 값은 복사되지만 가리킨 문자의 소유권은 얻지 않는다.
    const char* last = first + text.size();
    // 함수 호출 결과는 prvalue. 구조적 바인딩으로 결과 포인터와 오류 코드를 각각 이름 붙인다.
    const auto [next, error] = std::from_chars(first, last, value);

    if (error != std::errc{} || next != last || value <= 0) {
        return std::nullopt;
    }
    return value;
}

[[nodiscard]] int pages_to_read(std::optional<int> daily_pages, int days) {
    // value_or supplies a visible default when the optional is empty.
    // 곱셈 결과 int는 prvalue다. 작은 정수 연산은 보통 레지스터에서 수행되지만 컴파일러가 결정한다.
    return daily_pages.value_or(10) * days;
}

void require(bool condition, std::string_view explanation) {
    if (!condition) {
        std::cerr << "[FAILED] " << explanation << '\n';
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    const auto valid = parse_positive_count("12");
    const auto zero = parse_positive_count("0");
    const auto mixed = parse_positive_count("12pages");

    require(valid.has_value(), "12 is a positive integer");
    require(*valid == 12, "operator* reads the contained value"); // *valid는 optional 내부 int의 lvalue 참조다.
    require(!zero, "zero is rejected");
    require(!mixed, "the whole input must be numeric");
    require(pages_to_read(valid, 3) == 36, "present optionals are used");
    require(pages_to_read(std::nullopt, 3) == 30, "empty optionals use the default");

    std::cout << "valid daily pages: " << *valid << '\n';
    std::cout << "three-day fallback plan: " << pages_to_read(std::nullopt, 3) << '\n';
    std::cout << "[TESTS] optional syntax exercise passed\n";
}
