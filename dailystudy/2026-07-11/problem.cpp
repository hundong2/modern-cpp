/*
Problem: Longest Repeating Character Replacement

Given an uppercase string s and an integer k, return the length of the longest
substring that can be made of the same character after changing at most k
characters.

Examples:
  s = "ABAB",    k = 2 -> 4
  s = "AABABBA", k = 1 -> 4

Goal:
  Learn std::string, std::vector frequency arrays, and sliding-window resizing.
*/

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int character_replacement(const std::string& s, int k) {
    std::vector<int> counts(26, 0);
    int left = 0;
    int max_frequency = 0;
    int best_length = 0;

    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
        const int index = s[right] - 'A';
        ++counts[index];
        max_frequency = std::max(max_frequency, counts[index]);

        // Window length minus the most frequent character count equals the
        // number of characters that must be replaced.
        while ((right - left + 1) - max_frequency > k) {
            --counts[s[left] - 'A'];
            ++left;
        }

        best_length = std::max(best_length, right - left + 1);
    }

    return best_length;
}

void run_tests() {
    assert(character_replacement("ABAB", 2) == 4);
    assert(character_replacement("AABABBA", 1) == 4);
    assert(character_replacement("AAAA", 0) == 4);
}

int main() {
    run_tests();

    std::cout << "ABAB / 2    -> " << character_replacement("ABAB", 2) << '\n';
    std::cout << "AABABBA / 1 -> " << character_replacement("AABABBA", 1) << '\n';
    std::cout << "[TESTS] character replacement tests passed\n";
    return 0;
}

/*
Execution result:
ABAB / 2    -> 4
AABABBA / 1 -> 4
[TESTS] character replacement tests passed
*/
