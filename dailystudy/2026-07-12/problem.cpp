/*
Problem: Subarray Sum Equals K

Given an integer array nums and an integer k, return the number of contiguous
subarrays whose sum equals k. nums may contain negative values, so a simple
two-pointer window is not enough.

Examples:
  nums = [1, 1, 1], k = 2 -> 2
  nums = [1, 2, 3, -3, 1, 2], k = 3 -> 6

Goal:
  Learn std::vector, std::unordered_map, prefix sums, and O(N) counting.
*/

#include <cassert>
#include <iostream>
#include <unordered_map>
#include <vector>

int subarray_sum(const std::vector<int>& nums, int k) {
    int answer = 0;
    int prefix_sum = 0;

    // key = prefix sum value, value = how many times that prefix appeared.
    std::unordered_map<int, int> prefix_counts;
    prefix_counts[0] = 1;

    for (int num : nums) {
        prefix_sum += num;

        const int needed_past_prefix = prefix_sum - k;
        if (const auto it = prefix_counts.find(needed_past_prefix); it != prefix_counts.end()) {
            answer += it->second;
        }

        ++prefix_counts[prefix_sum];
    }

    return answer;
}

void run_tests() {
    assert(subarray_sum({1, 1, 1}, 2) == 2);
    assert(subarray_sum({1, 2, 3, -3, 1, 2}, 3) == 6);
    assert(subarray_sum({1, -1, 0}, 0) == 3);
}

int main() {
    run_tests();

    std::cout << "[1,1,1] / 2             -> " << subarray_sum({1, 1, 1}, 2) << '\n';
    std::cout << "[1,2,3,-3,1,2] / 3     -> " << subarray_sum({1, 2, 3, -3, 1, 2}, 3) << '\n';
    std::cout << "[TESTS] subarray sum tests passed\n";
    return 0;
}

/*
Execution result:
[1,1,1] / 2             -> 2
[1,2,3,-3,1,2] / 3     -> 6
[TESTS] subarray sum tests passed
*/
