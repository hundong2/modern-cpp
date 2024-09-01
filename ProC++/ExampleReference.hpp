#ifndef EXAMPLE_REFERENCE_HPP
#define EXAMPLE_REFERENCE_HPP
#include <iostream>
#include <string>
#include <vector>

class MyClass
{
    public:
        MyClass(int& ref) : m_ref(ref) {}

    private:
        int& m_ref; //reference member variable initialized with constructor initializer list
};
// Include any necessary headers here
class ExampleReference
{
    public:
        struct OddAndEvens { std::vector<int> odds, evens; };
    public:
        // Declare your functions here
        ExampleReference();
        ~ExampleReference();
        void exampleReference();
        void exampleReference2();
        std::string getString();
        void exampleReference3(); //reference pointer
        void exampleReference4(); //reference to reference
        void exampleReference5(); //reference of struct binding to reference
        void exampleReference6(); //reference using function parameter for const reference 
        void printString(const std::string& str);
        void exampleReference7(); //reference using function parameter for const reference
        OddAndEvens seperateOddsAndEvens(const std::vector<int>& numbers);
};
// Declare your classes, functions, or variables here

#endif // EXAMPLE_REFERENCE_HPP