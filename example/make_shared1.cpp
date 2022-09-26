#include <iostream>
#include <memory>

int main()
{
    std::shared_ptr<int> sp1( new int );
    std::shared_ptr<int> sp2 = std::make_shared<int>();

    std::shared_ptr<int[]> sp3(new int[10]);// C++17
    std::shared_ptr<int[]> sp4 = std::make_shared<int[10]>(); //C++20

    auto p5 = std::make_shared<int[]>(3); // new int[3]
    auto p6 = std::make_shared<int[3]>(); // new int[3]
    auto p7 = std::make_shared<int[3]>(4); // vc : new int[3]{4,4,4,4} / g++ new int[3]{4,xx,xxx,xxx}

    //sp1[0] = 10; //error
    *sp1 = 10; //ok
    sp3[0] = 10; //ok
    //*sp3 = 10; //error
}