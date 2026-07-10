#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
concept Number = std::integral<T> || std::floating_point<T>;

template <Number T>
T Clamp(T value, T low, T high) {
    // Concepts를 쓰면 이 함수가 숫자 타입에만 열립니다.
    // 잘못된 타입을 넣었을 때 템플릿 내부의 긴 에러 대신
    // "Number 조건을 만족하지 않는다"는 식의 에러를 기대할 수 있습니다.
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

template <typename T>
void PrintStorageHint(const T& value) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        // if constexpr는 컴파일 타임 분기입니다.
        // 조건이 false인 분기는 컴파일 대상에서 제외됩니다.
        std::cout << "cheap/simple copy candidate: " << value << '\n';
    } else {
        std::cout << "prefer const reference for heavy type\n";
    }
}

int main() {
    std::cout << Clamp(15, 0, 10) << '\n';
    std::cout << Clamp(3.14, 0.0, 2.0) << '\n';

    PrintStorageHint(42);

    std::string text = "hello";
    PrintStorageHint(text);

    std::vector<int> values{1, 2, 3};
    PrintStorageHint(values);
}

