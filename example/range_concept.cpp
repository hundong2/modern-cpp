#include <iostream>
#include <ranges>
#include <vector>


// range concept 
// abstraction of "a collection of items"
// something iterable
// 최소의 요구조건 - "begin(), end() 사용할 수 있어야 한다. "

/*
template<typename T>
concept range = requires(T& t)
{
    std::ranges::begin(t)
    std::ranges::end(t)
};
*/
int main()
{
    bool b1 = std::ranges::range<int>; //false
    bool b2 = std::ranges::range<int[5]>; //true
    bool b3 = std::ranges::range<std::vector<int>>; //true

    std::cout << b1 << std::endl;
    std::cout << b2 << std::endl;
    std::cout << b3 << std::endl;

    std::vector<int> v  = {1,2,3,4,5,6,7,8,9,10};

    auto r = v | std::views::take(3);

    bool b4 = std::ranges::range<decltype(r)>;
    std::cout << b4 << std::endl;

}