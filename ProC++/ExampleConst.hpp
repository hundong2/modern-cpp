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
};

#endif // EXAMPLECONST_HPP