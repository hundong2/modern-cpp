#include "ExampleConst.hpp"

ExampleConst::ExampleConst()
{
    std::cout << "Example Const Initialized" << std::endl;  
    std::cout << "Version Number Major: " << versionNumberMajor << std::endl;
    std::cout << "Version Number Minor: " << versionNumverMinor << std::endl;
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