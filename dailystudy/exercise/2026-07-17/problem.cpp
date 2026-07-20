/*
[기초 문법부터 읽는 순서]
1. string_view는 문자열을 복사·소유하지 않고 읽습니다. 호출 중 원본이 살아 있어야 합니다.
2. optional<int>는 정상 정수 또는 '값 없음'을 표현하여 -1 같은 약속값을 피합니다.
3. int value{}는 정수를 0으로 초기화하고, const char*는 문자를 가리키는 읽기 전용 포인터입니다.
4. from_chars는 문자열 범위를 숫자로 바꾸며 구조적 바인딩 `[next, error]`로 두 결과를 받습니다.
5. `!=`, `||`, `<=`는 각각 다름, 또는, 작거나 같음을 검사합니다.
6. value_or(10)은 optional이 비었을 때 기본값 10을 사용합니다.
7. 함수 앞 [[nodiscard]]는 반환값을 실수로 버리지 않도록 경고를 요청합니다.
8. require는 조건이 거짓이면 cerr로 오류를 출력하고 exit로 프로그램을 종료하는 작은 테스트 함수입니다.
*/

#include <charconv>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string_view>

// A failed conversion is normal input, so std::optional expresses it without
// a magic number such as -1 and without throwing an exception.
[[nodiscard]] std::optional<int> parse_positive_count(std::string_view text) {
    int value{};
    const char* first = text.data();
    const char* last = first + text.size();
    const auto [next, error] = std::from_chars(first, last, value);

    if (error != std::errc{} || next != last || value <= 0) {
        return std::nullopt;
    }
    return value;
}

[[nodiscard]] int pages_to_read(std::optional<int> daily_pages, int days) {
    // value_or supplies a visible default when the optional is empty.
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
    require(*valid == 12, "operator* reads the contained value");
    require(!zero, "zero is rejected");
    require(!mixed, "the whole input must be numeric");
    require(pages_to_read(valid, 3) == 36, "present optionals are used");
    require(pages_to_read(std::nullopt, 3) == 30, "empty optionals use the default");

    std::cout << "valid daily pages: " << *valid << '\n';
    std::cout << "three-day fallback plan: " << pages_to_read(std::nullopt, 3) << '\n';
    std::cout << "[TESTS] optional syntax exercise passed\n";
}
