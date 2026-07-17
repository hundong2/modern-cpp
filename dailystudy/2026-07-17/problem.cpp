/*
Problem: Longest Substring With At Most K Distinct Characters

Given a lowercase string s and an integer k, return the length of the longest
contiguous substring containing at most k distinct characters.
*/

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int length_of_longest_substring_k_distinct(const std::string& s, int k) {
    if (k == 0 || s.empty()) {
        return 0;
    }

    std::vector<int> counts(26, 0);
    int left = 0;
    int distinct = 0;
    int best = 0;

    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
        const int r = s[right] - 'a';
        if (counts[r] == 0) {
            ++distinct;
        }
        ++counts[r];

        while (distinct > k) {
            const int l = s[left] - 'a';
            --counts[l];
            if (counts[l] == 0) {
                --distinct;
            }
            ++left;
        }

        best = std::max(best, right - left + 1);
    }

    return best;
}

void run_tests() {
    assert(length_of_longest_substring_k_distinct("eceba", 2) == 3);
    assert(length_of_longest_substring_k_distinct("aa", 1) == 2);
    assert(length_of_longest_substring_k_distinct("abc", 0) == 0);
}

int main() {
    run_tests();

    std::cout << "eceba / 2 -> "
              << length_of_longest_substring_k_distinct("eceba", 2) << '\n';
    std::cout << "aa / 1    -> "
              << length_of_longest_substring_k_distinct("aa", 1) << '\n';
    std::cout << "[TESTS] k distinct window tests passed\n";
    return 0;
}
