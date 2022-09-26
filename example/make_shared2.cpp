#include <iostream>
#include <memory>

struct Point { int x, y; };

int main()
{
    Point* p1 = new Point;  //default initialization
    Point* p2 = new Point{ .x = 1, .y = 2};//value initialization

    std::cout << p1->x << std::endl; // 쓰레기 값
    std::cout << p2->x << std::endl; // int 의 경우 0으로 초기화 

    // new Point, new Point{} => make_shared -> new Point{ std::forward<ARGS>(args) ...  perfect forwarding}
    std::shared_ptr<Point> sp1 = std::make_shared<Point>(); // 내부적으로 value 초기화 
    std::cout << sp1->x << std::endl;

    //C++ 20
    std::shared_ptr<Point> sp2 = std::make_shared_for_overwrite<Point>(); // new Point;, default 
    std::shared_ptr<Point[]> sp3 = std::make_shared_for_overwrite<Point[]>(3);

}