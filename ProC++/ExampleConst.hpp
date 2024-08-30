#ifndef EXAMPLECONST_HPP
#define EXAMPLECONST_HPP
#include <string>
#include <iostream>

class Rect 
{
    public:
        constexpr Rect(size_t width, size_t height) : width(width), height(height) {}
        constexpr size_t getArea() const { return width * height; }
    private:
        size_t width { 0 };
        size_t height { 0 };
};
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
        constexpr int getArraySize();
        constexpr void SetConstExpr();
};

#endif // EXAMPLECONST_HPP