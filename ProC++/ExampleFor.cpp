#include <iostream>
#include <array>

//for c++20
// for( <initializer>; <condition>; <increment> ) { <statement> }
int main()
{
    for( std::array myArray {1,2,3,4}; auto i : myArray )
    {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    return 0;
}