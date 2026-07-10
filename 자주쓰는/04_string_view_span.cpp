#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

std::size_t CountChar(std::string_view text, char target) {
    // string_view는 문자열을 소유하지 않습니다.
    // 내부적으로 대개 const char*와 길이만 들고 있습니다.
    // std::string, 문자열 리터럴, char 배열을 복사 없이 받을 수 있습니다.
    std::size_t count = 0;
    for (char ch : text) {
        if (ch == target) {
            ++count;
        }
    }
    return count;
}

int Sum(std::span<const int> values) {
    // span도 배열을 소유하지 않는 뷰입니다.
    // vector, array, C 배열을 복사 없이 같은 함수로 받을 수 있습니다.
    int result = 0;
    for (int value : values) {
        result += value;
    }
    return result;
}

int main() {
    std::string name = "modern cpp";
    std::cout << CountChar(name, 'p') << '\n';
    std::cout << CountChar("literal text", 't') << '\n';

    std::vector<int> numbers{1, 2, 3, 4, 5};
    std::cout << Sum(numbers) << '\n';

    int raw_numbers[] = {10, 20, 30};
    std::cout << Sum(raw_numbers) << '\n';

    // 주의: string_view/span은 원본 메모리보다 오래 살아 있으면 안 됩니다.
    // 아래 같은 코드는 위험합니다.
    // std::string_view dangling = std::string{"temporary"};
    // 임시 string은 줄이 끝나면 파괴되고, dangling은 해제된 메모리를 보게 됩니다.
}

