#include <iostream>
#include <utility>

int main()
{
    int sn = 0;
    unsigned int un = 0;

    //기존 관계 연산자는 signed value와 unsigned value를 비교할 경우 잘못된 결과가 나옴. 
    std::cout << ( -1 < sn ) << std::endl; // true 1 
    std::cout << ( -1 < un ) << std::endl;

    std::cout << std::cmp_less(-1, un) << std::endl; // 1
    // cmp_equal, cmp_not_equal, cmp_less, cmp_greater, cmp_less_equal, cmp_greater_equal
}