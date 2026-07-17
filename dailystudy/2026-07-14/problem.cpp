/*
Problem: Permutation in String

Return true if s2 contains a contiguous substring that is a permutation of s1.
The window size is fixed to s1.length(), so each step adds one new character
and removes one old character.

Examples:
  s1 = "ab", s2 = "eidbaooo" -> true
  s1 = "ab", s2 = "eidboaoo" -> false

Goal:
  Reinforce std::vector frequency tables and fixed-size sliding windows.
*/

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

bool contains_permutation(const std::string& pattern, const std::string& text) {
    if (pattern.size() > text.size()) {
        return false;
    }

    std::vector<int> pattern_counts(26, 0);
    std::vector<int> window_counts(26, 0);
    const int window_size = static_cast<int>(pattern.size());

    for (int i = 0; i < window_size; ++i) {
        ++pattern_counts[pattern[i] - 'a'];
        ++window_counts[text[i] - 'a'];
    }

    for (int right = window_size; right <= static_cast<int>(text.size()); ++right) {
        if (pattern_counts == window_counts) {
            return true;
        }

        if (right == static_cast<int>(text.size())) {
            break;
        }

        ++window_counts[text[right] - 'a'];
        --window_counts[text[right - window_size] - 'a'];
    }

    return false;
}

void run_tests() {
    assert(contains_permutation("ab", "eidbaooo"));
    assert(!contains_permutation("ab", "eidboaoo"));
    assert(contains_permutation("abc", "ccccbbbbaaaaacb"));
}

int main() {
    run_tests();

    std::cout << std::boolalpha;
    std::cout << "ab / eidbaooo -> " << contains_permutation("ab", "eidbaooo") << '\n';
    std::cout << "ab / eidboaoo -> " << contains_permutation("ab", "eidboaoo") << '\n';
    std::cout << "[TESTS] permutation tests passed\n";
    return 0;
}

/*
Execution result:
ab / eidbaooo -> true
ab / eidboaoo -> false
[TESTS] permutation tests passed
*/
