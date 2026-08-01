#include <iostream>
#include <concepts>

template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>;
};

template<Addable T>
T add(T a, T b) {
    return a + b;
}

int main() {
    int x = 5, y = 10;
    std::cout << "정수 덧셈: " << add(x, y) << std::endl;

    double m = 3.14, n = 2.71;
    std::cout << "실수 덧셈: " << add(m, n) << std::endl;

    //std::string s1 = "Hello, ", s2 = "World!";
    //std::cout << "문자열 덧셈: " << add(s1, s2) << std::endl; // 컴파일 에러 발생

    return 0;
}