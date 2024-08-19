#include <iostream>
#include <array>
#include <format>
#include <vector>
#include <utility>

/// @brief pair example - <utility>
void ExamplePair()
{
    //std::pair<double, int> myPair {1.1, 2};
    std::pair myPair {1.1 , 2}; //using CTAD
    std::cout << std::format("{} {}", myPair.first, myPair.second) << std::endl;
}
/// @brief using vector 
void ExampleVector()
{
    //std::vector<int> myVector { 11, 22};
    std::vector myVector {11, 22}; //using CTAD 
    myVector.push_back(33);
    myVector.push_back(44);
    for( auto i : myVector )
    {
        std::cout << std::format("{} ", i);
    }
    std::cout << std::endl;
}
/// @brief Example Array ( std::array )
void ExampleArray()
{
    //std::array<int, 3> myArray {1, 2, 3};
    std::array myArray {1,2,3}; //using CTAD ( class template argument deduction )
    std::cout << std::format("array size ( myArray.size() ) = {}", myArray.size()) << std::endl;
    for(auto i: myArray)
    {
        std::cout << std::format("{} ", i);
    }
    std::cout << std::endl;
}
int main()
{
    ExampleArray();
    ExampleVector();
    ExamplePair();
    return 0;
}