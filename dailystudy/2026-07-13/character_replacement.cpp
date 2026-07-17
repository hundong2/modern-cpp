/*
Bonus Problem: Longest Repeating Character Replacement

Given an uppercase string s and an integer k, return the longest substring that
can be made of one repeated character after changing at most k characters.
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
    int best = 0;

    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
        const int r = s[right] - 'A';
        ++counts[r];
        max_frequency = std::max(max_frequency, counts[r]);

        while ((right - left + 1) - max_frequency > k) {
            const int l = s[left] - 'A';
            --counts[l];
            ++left;
        }

        best = std::max(best, right - left + 1);
    }

    return best;
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
