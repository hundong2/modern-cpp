#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <string_view>

// Problem: length of the longest substring without repeating characters.
//
// Why this matters:
// - Sliding windows are common in parsing, streaming logs, and rate windows.
// - last_seen lets the left pointer jump instead of moving one step at a time.

int length_of_longest_substring(std::string_view s) {
    // ASCII table. -1 means the character has not appeared yet.
    std::array<int, 256> last_seen;
    last_seen.fill(-1);

    int left = 0;
    int best = 0;

    for (int right = 0; right < static_cast<int>(s.size()); ++right) {
        // Cast to unsigned char so characters with high-bit set do not become
        // negative indexes on platforms where char is signed.
        const auto c = static_cast<unsigned char>(s[right]);

        if (last_seen[c] >= left) {
            left = last_seen[c] + 1;
        }

        last_seen[c] = right;
        best = std::max(best, right - left + 1);
    }

    return best;
}

void run_tests() {
    assert(length_of_longest_substring("abcabcbb") == 3);
    assert(length_of_longest_substring("bbbbb") == 1);
    assert(length_of_longest_substring("pwwkew") == 3);
    assert(length_of_longest_substring("") == 0);
}

int main() {
    run_tests();

    std::cout << "abcabcbb -> " << length_of_longest_substring("abcabcbb") << '\n';
    std::cout << "bbbbb    -> " << length_of_longest_substring("bbbbb") << '\n';
    std::cout << "[TESTS] longest substring tests passed\n";
    return 0;
}
