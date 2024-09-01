#include "ExampleReference.hpp"

ExampleReference::ExampleReference()
{

}

ExampleReference::~ExampleReference()
{

}
void ExampleReference::exampleReference()
{
    std::cout << "Example Reference" << std::endl;
    int z;
    const int& zRef { z }; //same means as int const& zRef { z };
    //zRef = 4; //compile error, error: assignment of read-only reference ‘zRef’
    z = 4; //ok
}

void ExampleReference::exampleReference2()
{
    std::cout << "example reference 2" << std::endl;
    //int& unnamedRef1 { 5 }; //compile error, error: invalid initialization of non-const reference of type ‘int&’ from an rvalue of type ‘int’
    const int& unnamedRef2 { 5 }; //ok
}

/**
 * @brief get string
 * @details get string
 */
std::string ExampleReference::getString()
{
    std::string str { "Hello World" };
    const std::string& strRef { str };
    //strRef = "Hello World"; //compile error, error: assignment of read-only reference ‘strRef’
    std::cout << strRef << std::endl;
    return strRef;
}

/**
 * @brief reference pointer
 * @details reference pointer example
 */
void ExampleReference::exampleReference3()
{
    int* intP { nullptr };
    int*& ptrRef { intP };
    ptrRef = new int;
    *ptrRef = 5;
    std::cout << "ptrRef: " << *ptrRef << std::endl;
    std::cout << "intP: " << *intP << std::endl;
}


void ExampleReference::exampleReference4()
{
    int x { 3 };
    int& xRef { x };
    int* xPtr { &xRef };
    std::cout << "xPtr: " << *xPtr << " -> ";
    *xPtr = 100;
    std::cout << "xPtr: " << *xPtr << std::endl;
}