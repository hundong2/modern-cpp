#include <iostream>
#include <ranges>
#include <vector>

int main()
{
    std::vector<int> v1 = {1,2,3,4,5};
    std::vector<int> v2 = {6,7,8,9,0};

    //c++ 참조는 const ( 이동 불가능한 참조 )
    //std::vector<int>& r1(v1); 
    //std::vector<int>& r2(v2);

    //c++ std::reference_wrapper의 range 버젼  
    std::ranges::ref_view r1(v1);
    std::ranges::ref_view r2(v2);
    r1 = r2; // 참조 사용 시 vector 자체를 복사 

    std::cout << v1[0] << std::endl;
    std::cout << v2[0] << std::endl;
    std::cout << r1[0] << std::endl;
    std::cout << r2[0] << std::endl;
}