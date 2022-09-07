#include <iostream>

struct Empty
{
	void foo();
};
//Empty Class
//non-static member data and no have virtual function class 
// sizeof 
struct Data
{
	[[no_unique_address]] Empty e;
	int n;
};

struct Data_Original
{
	Empty e;
	int n;
};

int main()
{
	std::cout << sizeof(Empty) << std::endl;
	std::cout << sizeof(Data_Original) << std::endl;
	std::cout << sizeof(Data) << std::endl;
}
