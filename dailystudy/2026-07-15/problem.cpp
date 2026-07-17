/*
Problem: Minimum Window Substring

Given strings s and t, return the shortest contiguous substring of s containing
every character from t, including duplicate characters.
*/

#include <array>
#include <cassert>
#include <climits>
#include <iostream>
#include <string>
#include <string_view>

std::string min_window(std::string_view s, std::string_view t) {
    if (s.size() < t.size()) {
        return "";
    }

    std::array<int, 128> target{};
    std::array<int, 128> window{};

    for (unsigned char c : t) {
        ++target[c];
    }

    int required = 0;
    for (int count : target) {
        if (count > 0) {
            ++required;
        }
    }

    int formed = 0;
    int left = 0;
    int best_start = 0;
    int best_length = INT_MAX;

    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
        const unsigned char r = static_cast<unsigned char>(s[right]);
        ++window[r];

        if (target[r] > 0 && window[r] == target[r]) {
            ++formed;
        }

        while (formed == required) {
            const int current_length = right - left + 1;
            if (current_length < best_length) {
                best_length = current_length;
                best_start = left;
            }

            const unsigned char l = static_cast<unsigned char>(s[left]);
            if (target[l] > 0 && window[l] == target[l]) {
                --formed;
            }
            --window[l];
            ++left;
        }
    }

    if (best_length == INT_MAX) {
        return "";
    }

    return std::string(s.substr(best_start, best_length));
}

void run_tests() {
    assert(min_window("ADOBECODEBANC", "ABC") == "BANC");
    assert(min_window("a", "aa").empty());
    assert(min_window("aa", "aa") == "aa");
}

int main() {
    run_tests();

    std::cout << "ADOBECODEBANC / ABC -> " << min_window("ADOBECODEBANC", "ABC") << '\n';
    std::cout << "[TESTS] min window tests passed\n";
    return 0;
}
