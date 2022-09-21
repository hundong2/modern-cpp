#include <vector>
#include <list>
#include <iostream>

template<typename T> requires std::input_iterator<T>
void advance(T p, int N)
{
    std::cout << "input_iterator" << std::endl;
    while(N--) ++p;
}

template<typename T> requires std::random_access_iterator<T>
void advance(T p, int N)
{
    std::cout << "random access iterator" << std::endl;
    p = p + N;
}

int main()
{
    std::vector<int> c = {1,2,3,4,5,6,7,8,9,10};
    //std::list<int> c = {1,2,3,4,5,6,7,8,9};
    auto p = c.begin();

    //p = p + 3;
    std::advance(p, 3);
}