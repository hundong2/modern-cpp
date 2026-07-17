/*
Problem: Permutation in String

Given two lowercase strings s1 and s2, return true if s2 contains any contiguous
substring that is a permutation of s1.

Examples:
  s1 = "ab", s2 = "eidbaooo" -> true   ("ba")
  s1 = "ab", s2 = "eidboaoo" -> false

Goal:
  Learn std::string, std::vector frequency tables, fixed-size sliding windows,
  and bool output with std::boolalpha.
*/

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

bool check_inclusion(const std::string& s1, const std::string& s2) {
    if (s1.size() > s2.size()) {
        return false;
    }

    std::vector<int> target(26, 0);
    std::vector<int> window(26, 0);
    const int width = static_cast<int>(s1.size());

    for (int i = 0; i < width; ++i) {
        ++target[s1[i] - 'a'];
        ++window[s2[i] - 'a'];
    }

    if (target == window) {
        return true;
    }

    for (int right = width; right < static_cast<int>(s2.size()); ++right) {
        ++window[s2[right] - 'a'];

        const int left = right - width;
        --window[s2[left] - 'a'];

        if (target == window) {
            return true;
        }
    }

    return false;
}

void run_tests() {
    assert(check_inclusion("ab", "eidbaooo"));
    assert(!check_inclusion("ab", "eidboaoo"));
    assert(check_inclusion("adc", "dcda"));
}

int main() {
    run_tests();

    std::cout << std::boolalpha;
    std::cout << "ab / eidbaooo -> " << check_inclusion("ab", "eidbaooo") << '\n';
    std::cout << "ab / eidboaoo -> " << check_inclusion("ab", "eidboaoo") << '\n';
    std::cout << "[TESTS] permutation window tests passed\n";
    return 0;
}

/*
Execution result:
ab / eidbaooo -> true
ab / eidboaoo -> false
[TESTS] permutation window tests passed
*/
