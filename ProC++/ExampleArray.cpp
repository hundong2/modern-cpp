#include <iostream>
#include <format>

void ExampleArraySize()
{
    int myArray[] {1, 2, 3, 4};
    //old version
    size_t arraysize { sizeof(myArray)/ sizeof(myArray[0])};
    size_t arraysize2 { std::size(myArray)};
    std::cout << std::format("old version : {}, latest version : {}", arraysize, arraysize2) << std::endl;
}
/// @brief just one initialize using initializer list
void ExampleArray5()
{
    int myArray[3] = {2};
    std::cout << std::format("2, 0, 0 = {} {} {}", myArray[0], myArray[1], myArray[2]) << std::endl;
}
/// @brief initializer list
void ExampleArray4()
{
    int myArray[] {1,2,3,4};
    std::cout << std::format("initializer list, {} {}", myArray[0], myArray[1]) << std::endl;
}
/**
 * @brief (=) delete
 * 
 */
void ExampleArray3()
{
    int myArray[3] {};
    std::cout << std::format("same means ExampleArray1, 2 = {} {} ", myArray[0], myArray[1]) << std::endl;
}
/**
 * @brief initialize Array
 * 
 */
void ExampleArray2()
{
    int myArray[3] = {};
    std::cout << std::format("same means ExampleArray1(), {} {}", myArray[0], myArray[1]) << std::endl;
}
/**
 * @brief Example Array initialize 
 */
void ExampleArray1()
{
    int myArray[3] = {0};
    std::cout << std::format("array : {} {}", myArray[0], myArray[1]) << std::endl;
}
//original c style 
void ExampleArray()
{
    int myArray[3];
    myArray[0] = 0;
    myArray[1] = 1;
    myArray[2] = 2;
    std::cout << std::format("{} {}", myArray[0], myArray[1]) << std::endl;
}

int main()
{
    ExampleArray();
    ExampleArray1();
    ExampleArray2();
    ExampleArray3();
    ExampleArray4();
    ExampleArray5();
    ExampleArraySize();
    return 0;
}