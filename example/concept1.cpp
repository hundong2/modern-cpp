#include <iostream>
#include <type_traits>

// concept is "a named set of requirements"
//template <template-parameter-list>
//concept concept-name=onstraint-expression
template<typename T>
concept Integral = std::is_integral_v<T>;
//Concept prod 
// 여러개의 제약조건을 하나의 이름으로 표현
// 제약 조건을 표현하는 "requires expression 문법 제공"
// 조건을 만족하는 2개 이상의 템플릿이 있을 때 
// "ambiguous 에러에 대한 해결 방법"
template<typename T> requires Integral<T>
void foo(T a)
{

}

int main()
{
    bool b = Integral<int>;
    bool f = Integral<float>;
    std::cout << b << std::endl;
    std::cout << f << std::endl;
}