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