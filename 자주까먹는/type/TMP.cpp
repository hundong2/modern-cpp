#include <iostream>

//1. 재귀 템플릿 메타프로그래밍으로 팩토리얼 계산
template <int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

//템플릿 특수화 ( 종료 조건 0! = 1)
template <>
struct Factorial<0> {
    static constexpr int value = 1;
};

// C++14 이상 환경에서 허용되는 모던한 컴파일 타임 팩토리얼
constexpr int modern_factorial(int n) {
    int result = 1;
    for (int i = 1; i <= n; ++i) { // 이제 템플릿 없이 for문도 맘껏 쓸 수 있습니다!
        result *= i;
    }
    return result;
}

int main()
{
    std::cout << "5! = " << Factorial<5>::value << std::endl; // 120
    std::cout << "0! = " << Factorial<0>::value << std::endl; // 1
    std::cout << "modern_factorial(5) = " << modern_factorial(5) << std::endl; // 120
    std::cout << "modern_factorial(0) = " << modern_factorial(  0) << std::endl; // 1
}


