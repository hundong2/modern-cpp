#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

// Shortest substring that contains every distinct character appearing in s.

int find_min_substring_all_chars(std::string_view s) {
    std::unordered_set<char> total_chars(s.begin(), s.end());
    const auto target_unique_count = total_chars.size();

    std::unordered_map<char, int> window_counts;
    int left = 0;
    int best = std::numeric_limits<int>::max();

    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
        ++window_counts[s[right]];

        while (window_counts.size() == target_unique_count) {
            best = std::min(best, right - left + 1);

            const char c = s[left];
            --window_counts[c];
            if (window_counts[c] == 0) {
                window_counts.erase(c);
            }
            ++left;
        }
    }

    return best == std::numeric_limits<int>::max() ? 0 : best;
}

void run_tests() {
    assert(find_min_substring_all_chars("AABCBBCADEB") == 5);
    assert(find_min_substring_all_chars("aaaa") == 1);
    assert(find_min_substring_all_chars("abc") == 3);
}

int main() {
    run_tests();

    std::cout << "AABCBBCADEB -> " << find_min_substring_all_chars("AABCBBCADEB") << '\n';
    std::cout << "[TESTS] all unique chars window tests passed\n";
    return 0;
}
