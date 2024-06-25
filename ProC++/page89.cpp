//function 
#include <iostream>
#include <format>

void FormatPrintExample()
{
    int i = 0;
    std::cout << std::format("the value of i is {}", i) << std::endl;
}
int main()
{
    FormatPrintExample();
    return 0;
}