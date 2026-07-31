#include <iostream>
#include <concepts>
#include <string>

template<typename T>
concept InContainer = requires {
    typename T::value_type; 
    requires std::integral<typename T::value_type>;
};

struct MyContainer {
    using value_type = int;
};
struct MyContainer2 {
    using value_type = double;
};

int main() {
    MyContainer c1;
    MyContainer2 c2;

    static_assert(InContainer<MyContainer>);
    //static_assert(InContainer<MyContainer2>); // error: double is not integral
    return 0;
}