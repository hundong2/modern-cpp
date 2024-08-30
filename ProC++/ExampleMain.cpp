#include <iostream>
#include "ExampleAllocateArray.hpp"
#include "ExampleConst.hpp"

int main()
{
    auto *exampleAllocateArray = new ExampleAllocateArray();
    delete exampleAllocateArray;
    exampleAllocateArray = nullptr;
    
    std::cout << std::endl;
    auto *exampleConst = new ExampleConst();
    delete exampleConst;
    exampleConst = nullptr;
    return 0;
}