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
    std::span<const int> readings, // 원소 복사 없이 시작 주소와 길이만 값으로 전달한다.
    std::size_t window_size,
    int threshold) {

    if (window_size == 0 || readings.size() < window_size) {
        return std::nullopt; // 값이 없다는 상태를 optional 내부 태그로 표현한다.
    }

    // window_sum은 이름 있는 지역 객체이므로 lvalue다. 대입 시 이 객체의 저장 위치가 갱신된다.
    int window_sum = std::accumulate(
        readings.begin(),
        readings.begin() + static_cast<std::ptrdiff_t>(window_size),
        0);

    if (window_sum >= threshold) {
        return 0;
    }

    for (std::size_t right = window_size; right < readings.size(); ++right) {
        window_sum += readings[right]; // 배열 원소 식도 lvalue이며, 값을 레지스터로 load해 합산하는 형태가 일반적이다.
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
        // 함수 반환 optional은 prvalue다. auto 지역 객체 index를 직접 초기화해 불필요한 복사를 피할 수 있다.
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
