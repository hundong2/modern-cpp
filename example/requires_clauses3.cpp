#include <iostream>

template<typename T> requires (sizeof(T) > 1)
void foo(T a)
{
    std::cout << "A" << std::endl;
}

template<typename T> requires (sizeof(T) > 1 && sizeof(T) < 8)
void foo(T a)
{
    std::cout << "B" << std::endl;
}

int main()
{
    foo(3.4); // A만 조건을 만족
    foo(3); //sizeof(int) = 4 -> concept 라는 새로운 문법 필요. 
}