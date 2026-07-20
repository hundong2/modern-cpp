/*
[기초 문법부터 읽는 순서]
1. 함수의 std::optional<size_t> 반환형은 '인덱스가 있을 수도, 없을 수도 있다'는
   의미입니다. 찾지 못하면 std::nullopt를 반환합니다.
2. for(초기값; 조건; 증감)는 조건이 참인 동안 반복합니다.
3. values[index]는 해당 위치의 원소이고 >=는 왼쪽이 오른쪽 이상인지 검사합니다.
4. 범위 for의 `for (int value : values)`는 모든 원소를 앞에서부터 하나씩 읽습니다.
5. 정수끼리 나누면 소수점이 사라지므로 static_cast<double>로 실수형으로 바꾼 뒤
   평균을 계산합니다.
6. if(optional)은 값 존재 여부를 검사하고 *optional은 들어 있는 값을 꺼냅니다.
7. fixed와 setprecision(1)은 소수점 아래 한 자리로 출력 형식을 정합니다.
*/

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
