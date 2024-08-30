#include "ExampleAllocateArray.hpp"

ExampleAllocateArray::ExampleAllocateArray() 
{
    myVariableSizedArray = new int[arraySize] {};
    std::cout << "Example Allocate Array Initialized" << std::endl;
}

ExampleAllocateArray::~ExampleAllocateArray() 
{
    delete[] myVariableSizedArray;
    myVariableSizedArray = nullptr;
    std::cout << "Example Allocate Array Deleted" << std::endl;
}