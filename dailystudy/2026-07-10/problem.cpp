#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

// Minimum window substring.

std::string min_window(std::string_view s, std::string_view t) {
    if (s.size() < t.size()) {
        return "";
    }

    std::vector<int> target(128, 0);
    std::vector<int> window(128, 0);

    for (char c : t) {
        ++target[static_cast<unsigned char>(c)];
    }

    int required = 0;
    for (int count : target) {
        if (count > 0) {
            ++required;
        }
    }

    int formed = 0;
    int left = 0;
    int best_len = std::numeric_limits<int>::max();
    int best_start = 0;

    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
        const auto r = static_cast<unsigned char>(s[right]);
        ++window[r];
        if (target[r] > 0 && window[r] == target[r]) {
            ++formed;
        }

        while (formed == required) {
            if (right - left + 1 < best_len) {
                best_len = right - left + 1;
                best_start = left;
            }

            const auto l = static_cast<unsigned char>(s[left]);
            if (target[l] > 0 && window[l] == target[l]) {
                --formed;
            }
            --window[l];
            ++left;
        }
    }

    if (best_len == std::numeric_limits<int>::max()) {
        return "";
    }

    return std::string{s.substr(best_start, best_len)};
}

void run_tests() {
    assert(min_window("ADOBECODEBANC", "ABC") == "BANC");
    assert(min_window("a", "a") == "a");
    assert(min_window("a", "aa") == "");
}

int main() {
    run_tests();

    std::cout << "ADOBECODEBANC / ABC -> " << min_window("ADOBECODEBANC", "ABC") << '\n';
    std::cout << "[TESTS] min window tests passed\n";
    return 0;
}
