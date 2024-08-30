#ifndef EXAMPLECONST_HPP
#define EXAMPLECONST_HPP
#include <string>
#include <iostream>

// Add your code here
class ExampleConst
{
    public:
        ExampleConst();
        ~ExampleConst();
    private:
        const int versionNumberMajor { 2 };
        const int versionNumverMinor { 1 };
        const std::string productName { "Super hyper Net Modulator"};
        const double PI { 3.141592653 };
        
        // This is a const member function, 
        //if you try to modify any member variable, the compiler will throw an error
        std::string getProductName() const; 
};

#endif // EXAMPLECONST_HPP