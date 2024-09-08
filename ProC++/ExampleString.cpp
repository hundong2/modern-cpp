#include "ExampleString.hpp"

void exampleString()
{
    char text1[] { "abcdef" };
    size_t s1 { sizeof(text1) }; // 7 "abcdef\0" -> NUL character
    size_t s2 { std::strlen(text1) }; //6

    std::cout << "Size of text1: " << s1 << ", strlen: " << s2 << std::endl;
}

void exampleString2()
{
    const char* text { "abcdef" };
    size_t s1 { sizeof(text) }; // differ each platform, 64 bit OS is 8, 32 bit OS is 4
    size_t s2 { std::strlen(text) }; //6
    std::cout << "Size of text: " << s1 << ", strlen: " << s2 << std::endl;

}

void exampleString3()
{
    std::cout << "Example std::string"    << std::endl;
    std::string a { "12" };
    std::string b { "34" };

    auto result { a.compare(b) };
    if( result < 0 )
    {
        std::cout << "a is less than b" << std::endl;
    }
    else if( result > 0 )
    {
        std::cout << "a is greater than b" << std::endl;
    }
    else
    {
        std::cout << "a is equal to b" << std::endl;
    }
}

void exampleString4()
{
    int a = 10;
    int b = 20;

    auto result = a <=> b;

    if (result < 0) {
        std::cout << "a is less than b" << std::endl;
    } else if (result > 0) {
        std::cout << "a is greater than b" << std::endl;
    } else {
        std::cout << "a is equal to b" << std::endl;
    }
}

void exampleString5()
{
    std::string strHello { "Hello!!" };
    std::string strWorld { "World...OMG..." };
    auto position { strHello.find("!!") };
    if( position != std::string::npos )
    {
        strHello.replace(position, 2, strWorld.substr(3, 6)); //"ld...O" 
    }
    std::cout << strHello << std::endl; //result : "Hellold...O"
}