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

void exampleString6()
{
    const std::string toParse { " 123USD" };
    size_t index { 0 };
    int value { std::stoi(toParse, &index) };
    std::cout << std::format("Parsed value: {}", value) << std::endl;
    std::cout << std::format("First non-parsed charater: '{}'", toParse[index]) << std::endl;
}

void exampleString7()
{
    std::cout << std::format("Function name: {}", __func__ ) << std::endl;
    const size_t BufferSize { 50 };
    std::string out(BufferSize, ' ');
    auto result { std::to_chars(out.data(), out.data() + out.size(), 12345) };
    if( result.ec == std::errc{})
    {
        std::cout << std::format("out size() : {}\n", out.size());
        std::cout << out << std::endl; //12345
    }
}

/**
 * @brief structure binding example for exampleString7
 */
void exampleString8()
{
    const size_t BufferSize { 50 }; //if 1 is not enough
    std::string out( BufferSize, ' ');
    auto [ptr, error] { std::to_chars(out.data(), out.data() + out.size(), 12345) };
    if( error == std::errc{} )
    {
        std::cout << std::format("out size() : {}\n", out.size());
        std::cout << out << std::endl; //12345
    }
    else
    {
        std::cout << "Error occurred" << std::endl;
    }
}

void exampleString9()
{
    const size_t BufferSize { 50 };
    double value { 0.314 };
    std::string out( BufferSize, ' ');
    auto [ptr, error] { std::to_chars(out.data(), out.data() + out.size(), value) };
    if( error == std::errc{})
    {
        std::cout << out << std::endl;
    }
}

void ExampleConvertToString()
{
    using namespace std;
    cout << __func__ << endl;
    const size_t BufferSize { 50 };
    double value1 { 0.314 };
    string out( BufferSize, ' ' );
    auto [ptr1, error1] { to_chars(out.data(), out.data() + out.size(), value1) };
    if( error1 == errc{} ) { cout << out << endl; }

    double value2;
    auto [ptr2, error2] { from_chars(out.data(), out.data() + out.size(), value2) };
    if( error2 == errc{})
    {
        if( value1 == value2)
        {
            cout << "Perfect roundtrip" << endl;
        }
        else
        {
            cout << "No Perfect roundtrip" << endl;

        }
    }
}

std::string_view extractExtension(std::string_view filename)
{
    return filename.substr(filename.rfind('.'));
}

void ExampleStringView()
{
    using namespace std;
    cout << format("Example string view {}", __func__) << endl;
    string filename { R"(c:\temp\example.txt)" };
    cout << format("C++ string: {}", extractExtension(filename)) << endl;

    const char* cString {  R"(c:\temp\example.txt)" };
    cout << format("C string: {}", extractExtension(cString)) << endl;

    cout << format("Literal: {}", extractExtension(R"(c:\temp\example.txt)")) << endl;
}

void handleExtension(const std::string& extension)
{
    std::cout << std::format("handleExtension: {}", extension) << std::endl;
}
void ExampleStringView2()
{
    //std::string and std::string_view are not the same type, not convcatenate directly
    //handleExtension(extractExtension(R"(c:\temp\example.txt)")); ///error: invalid initialization of reference of type ‘const std::string&’ {aka ‘const std::__cxx11::basic_string<char>&’} from expression of type ‘std::string_view’ {aka ‘std::basic_string_view<char>’}
    handleExtension(extractExtension("c:\\temp\\example.txt").data());
    handleExtension(std::string{extractExtension("c:\\temp\\example.txt")}); //uniform initialization syntax
}

void ExampleStringView3()
{
    //string -> string_view ok, string_view -> string not ok.
    std::string value { "test string"};
    std::string_view view { value };

    std::cout << view << std::endl;
    value[2] = 'x';
    std::cout << view << std::endl;

    constexpr std::string_view unicode[]{"▀▄─", "▄▀─", "▀─▄", "▄─▀"};
    //unicode[0] = "▄▀▄▀";
    for (int y{}, p{}; y != 6; ++y, p = ((p + 1) % 4))
    {
        for (int x{}; x != 16; ++x)
            std::cout << unicode[p];
        std::cout << '\n';
    }

}