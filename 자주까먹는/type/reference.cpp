#include <iostream>

template <typename T>
void wrapper(T&& args)
{
    std::cout << args << std::endl;
}

int main()
{
    int number = 10;
    wrapper(number); // lvalue 전달
    wrapper(20);     // rvalue 전달
    /*
    number는 lvalue이므로 T는 int&로 추론되고, args는 int&로 선언됩니다.
    최종 void wrapper(int& args) { ... }가 호출됩니다.
    
    rvalue 20은 T가 int로 추론되고, args는 int&&로 선언됩니다.
    최종 void wrapper(int&& args) { ... }가 호출됩니다.
    */
    std::cout << number << std::endl; // number는 여전히 10입니다.  
    std::cout << "--------------------" << std::endl;
}