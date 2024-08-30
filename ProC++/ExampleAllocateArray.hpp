#ifndef EXAMPLE_ALLOCATE_ARRAY_HPP
#define EXAMPLE_ALLOCATE_ARRAY_HPP

// Include any necessary headers here
class ExampleAllocateArray
{
    public:
        ExampleAllocateArray();
        ~ExampleAllocateArray();
    private:
        int arraySize { 8 };
        int* myVariableSizedArray { nullptr };
};
// Declare your functions or classes here

#endif // EXAMPLE_ALLOCATE_ARRAY_HPP