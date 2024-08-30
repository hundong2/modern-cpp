#include <iostream>
#include "ExampleAllocateArray.hpp"

int main()
{
    auto *exampleAllocateArray = new ExampleAllocateArray();
    delete exampleAllocateArray;
    exampleAllocateArray = nullptr;
    return 0;
}