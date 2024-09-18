#include "ExampleConst.hpp"
#include <array>
ExampleConst::ExampleConst()
{
    std::cout << "Example Const Initialized" << std::endl;  
    std::cout << "Version Number Major: " << versionNumberMajor << std::endl;
    std::cout << "Version Number Minor: " << versionNumverMinor << std::endl;
    std::cout << "constexpr exmple " << getArraySize() << std::endl;
    std::cout << "constexpr class example " << std::endl;
    SetConstExpr();
}

ExampleConst::~ExampleConst()
{
    std::cout << "Example Const Deleted" << std::endl;
}

std::string ExampleConst::getProductName() const
{
    // This will throw an error
    // productName = "New Product";
    return productName;
}
constexpr int ExampleConst::getArraySize()
{
    return 10;
}
constexpr void ExampleConst::SetConstExpr()
{
    // This will throw an error
    // PI = 3.14;
    constexpr Rect r { 8, 2};
    //int myArray[r.getArea()];
    std::array<int, r.getArea()> myArray;
    //std::cout << "Array Size: " << r.getArea() << std::endl;
}
constexpr double ExampleConst::inchToMm(double inch)
{
    return inch * 25.4;
}
constexpr void ExampleConst::ExampleConstexpr() 
{
    //constexpr 
    constexpr double const_inch { 6.0 };
    constexpr double mm1 { inchToMm(const_inch)}; //compile time evaluation

    //dynamic evaluation
    constexpr double dynamic_inch { 8.0 };
    double mm2 { inchToMm(dynamic_inch)}; //execute at runtime

}
consteval double ExampleConst::inchToMmConsteval(double inch)
{
    return inch * 25.4;
}