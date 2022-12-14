#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <iterator>
using namespace std;
int main()
{
    std::vector<int> v = {1,2,3,7,8,9,4,5,6,10};
    auto r = v | std::views::filter([](int a){ return a % 2 == 0;}) | std::views::take(3);


    for( auto n : r)
        std::cout << n << std::endl;
    std::ranges::reverse(r);

    for( auto n: v)
        std::cout << n << ", ";
    std::cout << std::endl;
}