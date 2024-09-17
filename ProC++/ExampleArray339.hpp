#ifndef EXAMPLEARRAY339_HPP
#define EXAMPLEARRAY339_HPP

#include <iostream>
#include <format>

using namespace std;

class Simple
{
    public:
        Simple() { cout << "Simple constructor called!" << endl; }
        ~Simple() { cout << "Simple destructor called!" << endl; }
};

void exampleArray339()
{
    cout << format("using {}", __func__) << endl;
    Simple* mySimpleArray { new Simple[4] };//called constructor 4 times 
    delete[] mySimpleArray; //called destructor 4 times 
    mySimpleArray = nullptr;
}

void exampleArray341()
{
    cout << format("using {}", __func__) << endl;
    const size_t size { 4 };
    Simple** mySimplePtrArray { new Simple * [size]};
    
    for ( size_t i {0}; i < size; i++) { mySimplePtrArray[i] = new Simple{}; }

    for( size_t i { 0 }; i < size; i++ )
    {
        delete mySimplePtrArray[i];
        mySimplePtrArray[i] = nullptr;
    }
    delete[] mySimplePtrArray;
    mySimplePtrArray = nullptr;
}
char** allocateCharacterBoard(size_t xDimension, size_t yDimension)
{
    std::cout << "allocateCharacterBoard" << std::endl;
    char** myArray { new char*[xDimension] };
    for( size_t i { 0 }; i < xDimension; i++ )
    {
        myArray[i] = new char[yDimension];
    }
    return myArray;
}
void releaseCharacterBoard(char** myArray, size_t xDimension)
{
    std::cout << "releaseCharacterBoard" << std::endl;
    for( size_t i { 0 }; i < xDimension; i++ )
    {
        delete[] myArray[i];
        myArray[i] = nullptr;
    }
    delete[] myArray;
    myArray = nullptr;
}
void ExampleArray344()
{
    std::cout << format("using {}", __func__) << std::endl; 
    auto exampleArray = allocateCharacterBoard(3, 3);
    releaseCharacterBoard(exampleArray, 3);
}
#endif // EXAMPLEARRAY339_HPP