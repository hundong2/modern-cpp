#include <iostream>
#include <concepts>

template<std::integral T>
void print_value(T value) {
    std::cout << "정수입니다: " << value << '\n';
}

template<std::floating_point T>
void print_value(T value) {
    std::cout << "실수입니다: " << value << '\n';
}

template<typename T>
requires std::integral<T> || std::floating_point<T>
void print_value1(T value) {
    if constexpr (std::integral<T>) {
        std::cout << "정수입니다: " << value << '\n';
    } else if constexpr (std::floating_point<T>) {
        std::cout << "실수입니다: " << value << '\n';
    }
}

int main() {
    std::cout << "Hello, C++20!" << std::endl;
    print_value(10);    // 정수입니다: 10
    print_value(3.14);  // 실수입니다: 3.14
    print_value1(20);   // 정수입니다: 20
    print_value1(2.718); // 실수입니다: 2.718
    return 0;
}

// == execute: /modern-cpp/자주까먹는/build/type/concept_requires4 ==

// Hello, C++20!
// 정수입니다: 10
// 실수입니다: 3.14
// 정수입니다: 20
// 실수입니다: 2.718