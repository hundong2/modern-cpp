#include <iostream>
#include <concepts>

//C++11 typename을 통해 type이 type형식이라는 것을 명시 
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value>::type
print_value(T value){
    std::cout << "C++11 typename example: " << value << std::endl;
}
//C++14 _v, _t 헬퍼가 추가 되어 ::value, typename ... :: type을 생략 할 수 있음. 
template<typename T>
std::enable_if_t<std::is_floating_point_v<T>> print_value_14(T value){
    std::cout << "C++14 _v, _t example: " << value << std::endl;
}

//C++20( concepts ) enable_if는 버리고 Concepts라는 기능이 도입 됨. 
//직관적으로 T는 실수여야 해 라고 선언할 수 있다. 
//#include<concepts>
// 1. template parameter direct writting 
template<std::floating_point T>
void print_value_20(T value) {
    std::cout << "C++20 concepts example: " << value << std::endl;
}
// 2. using auto 
void print_value_20_1(std::floating_point auto value) {
    std::cout << "C++20 concept example(using auto): " << value << std::endl; 
}

int main() {
    print_value(3.14);
    print_value_14(3.14);
    print_value_20(3.14);
    print_value_20_1(3.14);
    //print_value(10); //build error 
    std::cout << std::is_floating_point<float>::value << std::endl;
    std::cout << std::is_floating_point<bool>::value;
}

// == execute: /modern-cpp/자주까먹는/build/SFINAE ==

// C++11 typename example: 3.14
// C++14 _v, _t example: 3.14
// C++20 concepts example: 3.14
// C++20 concept example(using auto): 3.14
// 1
// 0