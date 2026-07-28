#include <algorithm>
#include <bit>
#include <compare>
#include <concepts>
#include <iostream>
#include <numbers>
#include <ranges>
#include <source_location>
#include <span>
#include <string_view>
#include <vector>

template<std::integral T>
T twice(T value) {
    return value + value;
}

struct Point {
    int x;
    int y;
    auto operator<=>(const Point&) const = default;
};

void print_all(std::span<const int> values) {
    // span은 값을 소유하지 않는다. 호출 중 원본이 살아 있어야 한다.
    for (int value : values) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}

void log(std::string_view message,
         const std::source_location location =
             std::source_location::current()) {
    std::cout << location.line() << ": " << message << '\n';
}

int main() {
    Point point{.x = 1, .y = 2}; // aggregate designated initialization
    const Point other{.x = 1, .y = 3};
    std::cout << std::boolalpha << "point < other? " << (point < other) << '\n';

    std::vector<int> values{1, 2, 3, 4, 5, 6};
    print_all(values);

    auto even_squares =
        values
        | std::views::filter([](int value) { return value % 2 == 0; })
        | std::views::transform([](int value) { return value * value; });

    for (int value : even_squares) {
        std::cout << value << ' ';
    }
    std::cout << '\n';

    std::cout << "twice = " << twice(21) << '\n';
    std::cout << "pi = " << std::numbers::pi << '\n';
    std::cout << "popcount(0b1011) = "
              << std::popcount(0b1011u) << '\n';
    log("source_location captures the call site");

    // TRY_COMPILE_ERROR: concept 제약 때문에 double은 integral이 아니다.
    // twice(1.5);
}
