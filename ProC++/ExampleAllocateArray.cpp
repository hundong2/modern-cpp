#include "ExampleAllocateArray.hpp"

ExampleAllocateArray::ExampleAllocateArray() 
{
    myVariableSizedArray = new int[arraySize] {};
}

ExampleAllocateArray::~ExampleAllocateArray() 
{
    delete[] myVariableSizedArray;
    myVariableSizedArray = nullptr;
}