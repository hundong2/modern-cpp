// C++20 Concepts 예제
#include <iostream>
#include <concepts>
#include <string>
#include <vector>

// 개념 정의: 덧셈이 가능한 타입
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

// 개념 정의: 출력 가능한 타입
template<typename T>
concept Printable = requires(T t) {
    { std::cout << t } -> std::same_as<std::ostream&>;
};

// 개념 정의: 컨테이너 타입
template<typename T>
concept Container = requires(T t) {
    typename T::value_type;
    typename T::iterator;
    { t.begin() } -> std::same_as<typename T::iterator>;
    { t.end() } -> std::same_as<typename T::iterator>;
    { t.size() } -> std::convertible_to<std::size_t>;
};

// Addable 개념을 사용한 함수
template<Addable T>
T add(T a, T b) {
    return a + b;
}

// Printable 개념을 사용한 함수
template<Printable T>
void print(const T& value) {
    std::cout << "값: " << value << std::endl;
}

// Container 개념을 사용한 함수
template<Container C>
void print_container(const C& container) {
    std::cout << "컨테이너 크기: " << container.size() << std::endl;
    std::cout << "원소들: ";
    for (const auto& item : container) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

// requires 절을 사용한 고급 예제
template<typename T>
    requires std::integral<T> || std::floating_point<T>
T multiply(T a, T b) {
    return a * b;
}

int main() {
    std::cout << "=== C++20 Concepts 예제 ===" << std::endl << std::endl;
    
    // 1. Addable 개념 사용
    std::cout << "1. Addable 개념:" << std::endl;
    std::cout << "add(10, 20) = " << add(10, 20) << std::endl;
    std::cout << "add(3.14, 2.86) = " << add(3.14, 2.86) << std::endl;
    std::string s1 = "Hello, ";
    std::string s2 = "World!";
    std::cout << "add(\"Hello, \", \"World!\") = " << add(s1, s2) << std::endl;
    std::cout << std::endl;
    
    // 2. Printable 개념 사용
    std::cout << "2. Printable 개념:" << std::endl;
    print(42);
    print(3.14159);
    print("C++20 Concepts!");
    std::cout << std::endl;
    
    // 3. Container 개념 사용
    std::cout << "3. Container 개념:" << std::endl;
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    print_container(numbers);
    
    std::vector<std::string> words = {"Modern", "C++", "20"};
    print_container(words);
    std::cout << std::endl;
    
    // 4. requires 절 사용
    std::cout << "4. requires 절 (integral or floating_point):" << std::endl;
    std::cout << "multiply(5, 6) = " << multiply(5, 6) << std::endl;
    std::cout << "multiply(2.5, 4.0) = " << multiply(2.5, 4.0) << std::endl;
    
    return 0;
}
