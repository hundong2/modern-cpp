#include <set>
#include <iostream>

int main()
{
    std::set<int> ss = {1,2,3,4,5,6,7,8,9,10};
    std::multiset<int> ms = {1,2,3,4,5,6,7,8,9,10};

    //associative container ㅇ에서 요소가 있는지 확인하는 방법
    //until C++20
    auto ret = ss.find(3);
    if( ret != ss.end() ){}

    if( ms.count(3) ) {}

    //since C++20
    // accociative container 
    // set, multiset, multimap, unordered_set, unordered_multiset, unordered_map, unordered_multimap
    bool s1 = ss.contains(3);
    bool s2 = ms.contains(3);

    std::cout << std::boolalpha;
    std::cout << s1 << std::endl;
    std::cout << s2 << std::endl;

}