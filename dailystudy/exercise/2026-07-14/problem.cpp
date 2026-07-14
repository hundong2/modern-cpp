/*
Daily Modern C++ Syntax Drill - 2026-07-14

Problem:
  Given a list of temperature readings, find the first fixed-size window whose
  sum is greater than or equal to a threshold.

Why this is useful:
  This is a small algorithm exercise that repeats std::span, std::optional,
  vector initialization, asserts, and a fixed-size sliding window.
*/

#include <cassert>
#include <iostream>
#include <numeric>
#include <optional>
#include <span>
#include <vector>

std::optional<std::size_t> first_window_at_least(
    std::span<const int> readings,
    std::size_t window_size,
    int threshold) {

    if (window_size == 0 || readings.size() < window_size) {
        return std::nullopt;
    }

    int window_sum = std::accumulate(
        readings.begin(),
        readings.begin() + static_cast<std::ptrdiff_t>(window_size),
        0);

    if (window_sum >= threshold) {
        return 0;
    }

    for (std::size_t right = window_size; right < readings.size(); ++right) {
        window_sum += readings[right];
        window_sum -= readings[right - window_size];

        if (window_sum >= threshold) {
            return right + 1 - window_size;
        }
    }

    return std::nullopt;
}

void run_tests() {
    {
        const std::vector<int> readings = {4, 1, 7, 3, 6, 2};
        const auto index = first_window_at_least(readings, 3, 16);
        assert(index.has_value());
        assert(*index == 2); // 7 + 3 + 6
    }

    {
        const std::vector<int> readings = {1, 1, 1, 1};
        const auto index = first_window_at_least(readings, 2, 5);
        assert(!index.has_value());
    }

    {
        const std::vector<int> readings = {10, 1, 1};
        const auto index = first_window_at_least(readings, 1, 10);
        assert(index.has_value());
        assert(*index == 0);
    }

    {
        const std::vector<int> readings = {3, 4};
        const auto index = first_window_at_least(readings, 3, 7);
        assert(!index.has_value());
    }
}

int main() {
    run_tests();

    const std::vector<int> readings = {4, 1, 7, 3, 6, 2};
    const auto index = first_window_at_least(readings, 3, 16);

    if (index) {
        std::cout << "first matching window starts at index " << *index << '\n';
    } else {
        std::cout << "no matching window\n";
    }

    std::cout << "[TESTS] window exercise passed\n";
    return 0;
}
