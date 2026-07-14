/*
Daily Modern C++ Syntax Drill - 2026-07-15

Problem:
  Given server load samples, find the first sample at or above a threshold and
  compute the average load.

Why this is useful:
  The code repeats std::span, std::optional, vector initialization, range-based
  for loops, and simple asserts.
*/

#include <cassert>
#include <iomanip>
#include <iostream>
#include <optional>
#include <span>
#include <vector>

std::optional<std::size_t> first_index_at_least(
    std::span<const int> values,
    int threshold) {

    for (std::size_t index = 0; index < values.size(); ++index) {
        if (values[index] >= threshold) {
            return index;
        }
    }

    return std::nullopt;
}

std::optional<double> average(std::span<const int> values) {
    if (values.empty()) {
        return std::nullopt;
    }

    int total = 0;
    for (int value : values) {
        total += value;
    }

    return static_cast<double>(total) / static_cast<double>(values.size());
}

void run_tests() {
    {
        const std::vector<int> loads = {22, 48, 65, 81, 44};
        const auto index = first_index_at_least(loads, 80);
        assert(index.has_value());
        assert(*index == 3);
    }

    {
        const std::vector<int> loads = {10, 20, 30};
        const auto index = first_index_at_least(loads, 90);
        assert(!index.has_value());
    }

    {
        const std::vector<int> loads = {10, 20, 30};
        const auto result = average(loads);
        assert(result.has_value());
        assert(*result == 20.0);
    }

    {
        const std::vector<int> loads;
        const auto result = average(loads);
        assert(!result.has_value());
    }
}

int main() {
    run_tests();

    const std::vector<int> loads = {22, 48, 65, 81, 44};
    const int warning_threshold = 80;

    const auto first_warning = first_index_at_least(loads, warning_threshold);
    if (first_warning) {
        std::cout << "first warning index: " << *first_warning << '\n';
    } else {
        std::cout << "no warning sample\n";
    }

    const auto mean = average(loads);
    if (mean) {
        std::cout << "average load: " << std::fixed << std::setprecision(1) << *mean << '\n';
    }

    std::cout << "[TESTS] problem exercise passed\n";
    return 0;
}
