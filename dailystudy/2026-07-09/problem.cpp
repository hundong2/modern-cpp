#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// Find all start indexes of p's anagrams in s.

std::vector<int> find_anagrams(std::string_view s, std::string_view p) {
    std::vector<int> result;
    if (s.size() < p.size()) {
        return result;
    }

    std::array<int, 26> target{};
    std::array<int, 26> window{};

    for (std::size_t i = 0; i < p.size(); ++i) {
        ++target[p[i] - 'a'];
        ++window[s[i] - 'a'];
    }

    if (target == window) {
        result.push_back(0);
    }

    for (std::size_t i = p.size(); i < s.size(); ++i) {
        ++window[s[i] - 'a'];
        --window[s[i - p.size()] - 'a'];

        if (target == window) {
            result.push_back(static_cast<int>(i - p.size() + 1));
        }
    }

    return result;
}

void run_tests() {
    assert((find_anagrams("cbaebabacd", "abc") == std::vector<int>{0, 6}));
    assert((find_anagrams("abab", "ab") == std::vector<int>{0, 1, 2}));
}

int main() {
    run_tests();

    const auto answer = find_anagrams("cbaebabacd", "abc");
    std::cout << "indexes: ";
    for (int index : answer) {
        std::cout << index << ' ';
    }
    std::cout << "\n[TESTS] anagram tests passed\n";
    return 0;
}
