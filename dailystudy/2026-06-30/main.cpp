#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

struct TestCase {
    std::string s;
    int k;
    int expected;
};

int lengthOfLongestSubstringKDistinct(std::string_view s, int k) {
    if (k == 0 || s.empty()) {
        return 0;
    }

    std::unordered_map<char, int> char_counts;
    int max_length = 0;
    int left = 0;

    for (int right = 0; right < static_cast<int>(s.length()); ++right) {
        ++char_counts[s[right]];

        while (static_cast<int>(char_counts.size()) > k) {
            const char left_char = s[left];
            auto it = char_counts.find(left_char);
            --it->second;

            if (it->second == 0) {
                char_counts.erase(it);
            }

            ++left;
        }

        max_length = std::max(max_length, right - left + 1);
    }

    return max_length;
}

void runTests() {
    const std::vector<TestCase> tests = {
        {"eceba", 2, 3},
        {"aa", 1, 2},
        {"a", 0, 0},
        {"abcadcacacaca", 3, 11},
        {"abaccc", 2, 4},
        {"abc", 50, 3},
    };

    for (const auto& test : tests) {
        const int actual = lengthOfLongestSubstringKDistinct(test.s, test.k);
        assert(actual == test.expected);
    }
}

int main(int argc, char* argv[]) {
    runTests();

    if (argc == 3) {
        try {
            const std::string_view s = argv[1];
            const int k = std::stoi(argv[2]);
            std::cout << lengthOfLongestSubstringKDistinct(s, k) << '\n';
            return 0;
        } catch (const std::exception& ex) {
            std::cerr << "Invalid argument: " << ex.what() << '\n';
            return 1;
        }
    }

    const std::vector<std::pair<std::string, int>> examples = {
        {"eceba", 2},
        {"aa", 1},
        {"abcadcacacaca", 3},
    };

    std::cout << "Longest substring with at most K distinct characters\n";
    std::cout << "All built-in tests passed.\n\n";

    for (const auto& [s, k] : examples) {
        std::cout << "s = \"" << s << "\", k = " << k
                  << " -> " << lengthOfLongestSubstringKDistinct(s, k) << '\n';
    }

    std::cout << "\nUsage:\n";
    std::cout << "  sliding_window_k_distinct <lowercase-string> <k>\n";
    std::cout << "Example:\n";
    std::cout << "  sliding_window_k_distinct eceba 2\n";

    return 0;
}
