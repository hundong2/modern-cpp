#include <iostream>
#include <format>

[[nodiscard]] int func()
{
    return 42;
}

/// @brief attribute example
/// @return 
int main()
{
    func();
    return 0;
}