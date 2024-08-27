#include "ExampleInit.h"
#include "ExampleDesignated.h"
#include <iostream>
#include <format>
#include <ranges>
#include <vector>


void func(int i)
{
    //narrawing conversion test 
    std::cout << std::format("test narrowing conversion {}", i) << std::endl;
}
int main()
{
    auto printArray = [](const int* array, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
    };
    //Previous code differ struct and class initializer
    CircleStruct myCircle1 = {10, 10, 2.5};
    CircleClass myCircle2 (10, 10, 2.5);
    //Uniform initialization for both struct and class after C++11
    CircleStruct myCircle3 = {10, 10, 2.5};
    CircleClass myCircle4 = {10, 10, 2.5}; // = is optional 

    CircleStruct myCircle5 {10, 10, 2.5};
    CircleClass myCircle6 {10, 10, 2.5};

    //inteager 
    int a = 3;
    int b(3);
    //uniform initialization
    int c = {3};
    int d {3};
    int e {}; //e is 0 
    char ch {}; //ch is '\0'
    double dou {}; //dou is 0.0
    int *p {}; //p is nullptr

    std::cout << std::format("e is {}", e) << std::endl;

    //using uniform initialization you can prevent narrowing conversion
    int x = 3.14;;
    func(3.14); //narrowing conversion, compiler warning is not expressed 

    //int xx {3.14}; //error occurs 
    //func({3.14}); //error occurs

    //if using narrowing conversion, you can use gsl::narrow_cast()
    //GSL : guideline Support Library

    //Previous C++20 
    int* pArray = new int[4] { 0, 2, 4, 6};
    //After C++20
    int* mArray2 = new int[] { 0, 2, 4, 6};

    printArray(pArray, 4);
    
    delete[] pArray;
    delete[] mArray2;

    std::vector vec {1, 2, 3, 4, 5};

    for( auto i : vec )
    {
        std::cout << i << " ";
    }

    //Initialize Designated Initializer
    //Old aggregate type 
    Employee emp1 = {'A', 'B', 1, 100'000};
    //New designated initializer
    Employee emp2 { 
        .firstInitial = 'A', 
        .lastInitial = 'B',
        //.employeeNumber = 1, 
        //.salary = 100'000
        };
    std::cout << std::format("emp2 : {}", emp2.toString()) << std::endl;    
    return 0;
}