#include <algorithm>
#include <concepts>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>

template <typename T>
concept PrintableNumber = std::integral<T> || std::floating_point<T>;

constexpr auto square(PrintableNumber auto value)
{
    if consteval {
        return value * value;
    }

    return value * value;
}

static_assert(square(3) == 9);

int main()
{
    constexpr std::string_view standard{"C++23"};
    std::vector numbers{5, 1, 4, 2, 3};

    std::ranges::sort(numbers);

    auto even_squares = numbers
        | std::views::filter([](int value) { return value % 2 == 0; })
        | std::views::transform([](int value) { return square(value); });

    std::cout << standard << " build environment is ready: ";
    for (const int value : even_squares) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}
