#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

std::vector<std::string> long_words(std::vector<std::string> words, std::size_t min_size)
{
    words.erase(
        std::remove_if(words.begin(), words.end(), [min_size](const std::string& word) {
            return word.size() < min_size;
        }),
        words.end());

    std::sort(words.begin(), words.end());
    return words;
}

std::unordered_map<std::string, int> count_words(const std::vector<std::string>& words)
{
    std::unordered_map<std::string, int> counts;
    for (const auto& word : words) {
        ++counts[word];
    }
    return counts;
}

int main()
{
    const std::vector<std::string> words = {
        "cpp", "modern", "cpp", "vector", "lambda", "algorithm", "cpp"
    };

    const auto filtered = long_words(words, 5);
    const auto counts = count_words(words);

    std::cout << "long words: ";
    for (const auto& word : filtered) {
        std::cout << word << ' ';
    }
    std::cout << '\n';

    std::cout << "cpp count: " << counts.at("cpp") << '\n';
    std::cout << "total words: " << words.size() << '\n';

    return 0;
}
