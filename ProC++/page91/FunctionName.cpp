#include <iostream>
#include <format>

int addNumbers(int number1, int number2)
{
    std::cout << "Entering function name: " << __func__ << std::endl;
    return number1 + number2;
}
int main()
{
    std::cout << std::format("1+2={}", addNumbers(1,2)) << std::endl;    
    return 0;
}