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
#endif // EXAMPLEARRAY339_HPP