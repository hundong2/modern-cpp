#include <iostream>
#include <concepts>

//requires 키워드를 사용하면, 
//템플릿 인자에 대한 제약 조건을 걸 수 있습니다.
//C++11에서는 SFINAE를 사용했지만, C++20에서는 requires를 사용하여 더 직관적으로 제약 조건을 걸 수 있습니다.
//C++14에서는 requires를 template parameter에 붙여서 사용했지만,
//C++17에서는 requires를 template parameter에 붙여서 사용했지만,
//C++20에서는 함수 parameter에 바로 requires를 붙일 수도 있습니다.
template<typename T>
requires std::same_as<T, int>
void print_int(T val){
    std::cout << "this is exactly an int: " << val << "\n";
}

//template parameter에 requires를 붙여서 제약 조건을 걸 수도 있다.
template<std::same_as<int> T>
void print_int_shorter(T val){
    std::cout << "Also exactly an int: " << val << "\n";
}

//C++20에서는 template parameter에 requires를 붙이지 않고, 
//함수 parameter에 바로 requires를 붙일 수도 있다.
void print_int_auto(std::same_as<int> auto val) {
    std::cout << "Modern exact int: " << val << "\n";
}

int main() {
    print_int(10);
    print_int_shorter(10);
    print_int_auto(10);
  
    
}
