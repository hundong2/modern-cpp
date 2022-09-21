#include <iostream>
#include <algorithm>
#include <iterator>

//T2가 인자로 T1::value type 받을수 있는 함수인 경우 
template<typename T1, typename T2> requires std::predicate<T2, typename std::iterator_traits<T1>::value_type>
T1 find(T1 first, T1 last, T2 pred)
{
    std::cout << "predicate version" << std::endl;
    return first;
}

template<typename T1, typename T2> requires (!std::predicate<T2, typename std::iterator_traits<T1>::value_type>)
T1 find(T1 first, T1 last, T2 pred)
{
    std::cout << "value version" << std::endl;
    return first;
}

int main()
{
    int x[10]{1,2,3,4,5,6,7,8,9,10};

    //auto p1 = std::find(std::begin(x), std::end(x), 3);
    //auto p2 = std::find_if(std::begin(x), std::end(x), [](int n) { return n % 2 == 0;});

    auto p1 = find(std::begin(x), std::end(x), 3);
    auto p2 = find(std::begin(x), std::end(x), [](int n) { return n % 2 == 0;});
}
