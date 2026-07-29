#include <iostream>
#include <type_traits>

// 1. T가 정수형(int, long 등)일 때만 후보에 남는 함수
template <typename T>
typename std::enable_if<std::is_integral<T>::value>::type
print_value(T value) {
    std::cout << "정수입니다: " << value << '\n';
}

// 2. T가 부동소수점(float, double 등)일 때만 후보에 남는 함수
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value>::type
print_value(T value) {
    std::cout << "실수입니다: " << value << '\n';
}

int main() {
    print_value(10);    // 1번 함수 호출 (2번 함수는 치환 실패로 조용히 제외됨)
    print_value(3.14);  // 2번 함수 호출 (1번 함수는 치환 실패로 조용히 제외됨)
    
    // print_value("Hello"); 
    // 컴파일 에러! 문자열 포인터는 1번, 2번 조건 모두 치환 실패하여 남은 후보가 없음.
    return 0;
}