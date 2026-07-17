/*
Problem: Longest Subarray With Sum <= K

nums contains positive integers. Find the maximum length of a contiguous
subarray whose sum is less than or equal to k.
*/

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

int max_subarray_len_less_equal_k(const std::vector<int>& nums, long long k) {
    if (k <= 0) {
        return 0;
    }

    int left = 0;
    long long current_sum = 0;
    int best = 0;

    for (int right = 0; right < static_cast<int>(nums.size()); ++right) {
        current_sum += nums[right];

        while (current_sum > k && left <= right) {
            current_sum -= nums[left];
            ++left;
        }

        best = std::max(best, right - left + 1);
    }

    return best;
}

void run_tests() {
    assert(max_subarray_len_less_equal_k({3, 1, 2, 1, 4, 5}, 5) == 3);
    assert(max_subarray_len_less_equal_k({1, 2, 3, 4}, 0) == 0);
    assert(max_subarray_len_less_equal_k({1, 1, 1, 1}, 2) == 2);
}

int main() {
    run_tests();

    std::cout << "[3,1,2,1,4,5] / 5 -> "
              << max_subarray_len_less_equal_k({3, 1, 2, 1, 4, 5}, 5) << '\n';
    std::cout << "[TESTS] longest sum window tests passed\n";
    return 0;
}
