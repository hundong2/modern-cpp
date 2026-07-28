#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

std::optional<int> parse_small_digit(std::string_view text) {
    if (text.size() == 1 && text[0] >= '0' && text[0] <= '9') {
        return text[0] - '0';
    }
    return std::nullopt;
}

template<class T>
std::string category(const T&) {
    if constexpr (std::is_integral_v<T>) {
        return "integral";
    } else {
        return "non-integral";
    }
}

template<class... Ts>
auto product(Ts... values) {
    return (values * ... * 1);
}

int main() {
    std::map<std::string, int> scores{{"Ada", 10}, {"Bjarne", 20}};

    // 구조적 바인딩. const auto&이므로 map 요소를 복사하지 않는다.
    for (const auto& [name, score] : scores) {
        std::cout << name << ": " << score << '\n';
    }

    // if initializer: parsed의 scope는 if/else 안으로 제한된다.
    if (const auto parsed = parse_small_digit("7"); parsed) {
        std::cout << "parsed = " << *parsed << '\n';
    }

    std::variant<int, std::string> value = std::string{"hello"};
    std::visit([](const auto& current) {
        std::cout << "variant = " << current << '\n';
    }, value);

    std::cout << "42 is " << category(42) << '\n';
    std::cout << "3.14 is " << category(3.14) << '\n';
    std::cout << "product = " << product(2, 3, 4) << '\n';

    // string_view는 이 문자열 리터럴을 소유하지 않지만, 리터럴은 프로그램 끝까지 산다.
    constexpr std::string_view view{"C++17"};
    std::cout << view << '\n';
}
