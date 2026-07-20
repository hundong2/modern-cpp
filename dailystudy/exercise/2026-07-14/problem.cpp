/*
[기초 문법부터 읽는 순서]
1. vector<int>는 int 여러 개를 연속으로 저장하고, span<const int>는 그 배열을
   복사하지 않고 읽기 전용으로 바라봅니다.
2. optional<size_t>는 위치가 있으면 인덱스를, 없으면 nullopt를 반환합니다.
3. readings.size()는 원소 수이고, readings[index]는 index 위치의 값입니다.
4. ||는 '또는', >=는 '크거나 같다', ++right는 right를 1 증가시킵니다.
5. 슬라이딩 윈도우는 새 값을 더하고 빠진 값을 빼서 매번 전체 합을 다시
   계산하지 않습니다.
6. auto는 오른쪽 식으로 타입을 추론하고, const는 이후 값을 바꾸지 않겠다는
   약속입니다.
7. assert(조건)는 조건이 거짓이면 테스트를 즉시 실패시켜 예상 동작을 검증합니다.
*/

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
