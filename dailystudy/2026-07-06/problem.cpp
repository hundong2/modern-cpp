#include <cassert>
#include <iostream>
#include <span>
#include <vector>

// Count contiguous subarrays whose product is less than k.
//
// Because all numbers are positive, expanding right increases the product and
// moving left decreases it. That monotonic behavior enables a sliding window.

int num_subarray_product_less_than_k(std::span<const int> nums, int k) {
    if (k <= 1) {
        return 0;
    }

    int left = 0;
    int product = 1;
    int count = 0;

    for (int right = 0; right < static_cast<int>(nums.size()); ++right) {
        product *= nums[right];

        while (product >= k && left <= right) {
            product /= nums[left];
            ++left;
        }

        // Every subarray ending at right and starting from left..right is valid.
        count += right - left + 1;
    }

    return count;
}

void run_tests() {
    const std::vector<int> a{10, 5, 2, 6};
    assert(num_subarray_product_less_than_k(a, 100) == 8);
    assert(num_subarray_product_less_than_k(a, 0) == 0);
}

int main() {
    run_tests();

    const std::vector<int> nums{10, 5, 2, 6};
    std::cout << "count: " << num_subarray_product_less_than_k(nums, 100) << '\n';
    std::cout << "[TESTS] product window tests passed\n";
    return 0;
}
