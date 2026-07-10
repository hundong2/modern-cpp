#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

// Dutch national flag problem: sort an array containing only 0, 1, and 2.
//
// Why this matters:
// - It practices in-place partitioning.
// - It avoids std::sort when the value domain is tiny and known.

void sort_colors(std::vector<int>& nums) {
    int low = 0;                         // next position for 0
    int mid = 0;                         // current element under inspection
    int high = static_cast<int>(nums.size()) - 1; // next position for 2

    while (mid <= high) {
        if (nums[mid] == 0) {
            std::swap(nums[low], nums[mid]);
            ++low;
            ++mid;
        } else if (nums[mid] == 1) {
            ++mid;
        } else {
            // Do not increment mid here. The value swapped from high has not
            // been inspected yet.
            std::swap(nums[mid], nums[high]);
            --high;
        }
    }
}

void print_vector(const std::vector<int>& nums) {
    for (int value : nums) {
        std::cout << value << ' ';
    }
    std::cout << '\n';
}

void run_tests() {
    std::vector<int> a{2, 0, 2, 1, 1, 0};
    sort_colors(a);
    assert((a == std::vector<int>{0, 0, 1, 1, 2, 2}));

    std::vector<int> b{2, 0, 1};
    sort_colors(b);
    assert((b == std::vector<int>{0, 1, 2}));
}

int main() {
    run_tests();

    std::vector<int> nums{2, 0, 2, 1, 1, 0};
    sort_colors(nums);

    std::cout << "sorted: ";
    print_vector(nums);
    std::cout << "[TESTS] sort colors tests passed\n";
    return 0;
}
