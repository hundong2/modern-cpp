#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <string_view>

// Longest substring where every character appears at least k times.
// For lowercase English letters, the number of possible distinct characters is
// fixed at 26. We try each target distinct-count and run a sliding window.

int longest_substring_with_at_least_k(std::string_view s, int k) {
    int best = 0;

    for (int target_unique = 1; target_unique <= 26; ++target_unique) {
        std::array<int, 26> counts{};
        int left = 0;
        int unique = 0;
        int enough = 0;

        for (int right = 0; right < static_cast<int>(s.size()); ++right) {
            const int r = s[right] - 'a';
            if (counts[r] == 0) {
                ++unique;
            }
            ++counts[r];
            if (counts[r] == k) {
                ++enough;
            }

            while (unique > target_unique) {
                const int l = s[left] - 'a';
                if (counts[l] == k) {
                    --enough;
                }
                --counts[l];
                if (counts[l] == 0) {
                    --unique;
                }
                ++left;
            }

            if (unique == target_unique && unique == enough) {
                best = std::max(best, right - left + 1);
            }
        }
    }

    return best;
}

void run_tests() {
    assert(longest_substring_with_at_least_k("aaabb", 3) == 3);
    assert(longest_substring_with_at_least_k("ababbc", 2) == 5);
}

int main() {
    run_tests();

    std::cout << "aaabb, k=3  -> " << longest_substring_with_at_least_k("aaabb", 3) << '\n';
    std::cout << "ababbc, k=2 -> " << longest_substring_with_at_least_k("ababbc", 2) << '\n';
    std::cout << "[TESTS] at-least-k substring tests passed\n";
    return 0;
}
