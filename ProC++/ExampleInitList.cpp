#include <initializer_list>
#include <iostream>
#include <format>
/**
 * @brief sum of initializer list element
 * 
 * @param list integer list
 * @return int sum of list
 */
int makeSum(std::initializer_list<int> list) //type safety 
{
    int sum { 0 };
    for( auto i : list )
    {
        sum += i;
    }
    return sum;
}
int main()
{
    int a { makeSum({1,2,3,4,5}) };
    int b { makeSum({1,2,3,4,5,6,7,8,9,10}) };
    std::cout << std::format("a = {}, b = {}", a, b) << std::endl;
    //int c { makeSum({ 1, 2, 3.0 })}; //Error initializer list is type safe 
    return 0;
}