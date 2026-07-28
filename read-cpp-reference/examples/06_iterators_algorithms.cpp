#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

int main() {
    std::vector<int> values{5, 1, 4, 2, 3, 2};

    // [begin, end)는 begin은 포함하고 end 자체는 포함하지 않는 범위다.
    std::sort(values.begin(), values.end());

    const auto found = std::find(values.cbegin(), values.cend(), 4);
    if (found != values.cend()) {
        std::cout << "found 4 at offset "
                  << std::distance(values.cbegin(), found) << '\n';
    }

    // C++17 erase-remove idiom:
    // remove는 실제 크기를 줄이지 않고 남길 값을 앞으로 이동시킨다.
    const auto new_end = std::remove(values.begin(), values.end(), 2);
    values.erase(new_end, values.end());

    const int sum = std::accumulate(values.cbegin(), values.cend(), 0);
    std::cout << "sum = " << sum << '\n';

    for (const int value : values) {
        std::cout << value << ' ';
    }
    std::cout << '\n';

    // iterator invalidation 예시:
    // push_back이 재할당하면 이전 iterator/reference/pointer는 무효화된다.
    const auto old_capacity = values.capacity();
    values.push_back(9);
    if (values.capacity() != old_capacity) {
        std::cout << "reallocation happened\n";
    }
}
