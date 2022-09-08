#include <iostream>

int main()
{
	int a = 10, b = 20;
	bool ret1 = a < b;
	auto ret2 = a <=> b; //after C++20, three-way comparison operator 
	// ret2 < 0 -> a < b, ret > 0 -> a > b, ret = 0 -> a == b
	// int : ret2 return type : strong_ordering 
	// partial_ordering ( double )
	if	( ret2 > 0 ) std::cout << "a > b" << std::endl;
	else if ( ret2 < 0 ) std::cout << "a < b" << std::endl;
	else if ( ret2 == 0) std::cout << "a==b"  << std::endl;

	std::cout << typeid(ret2).name() << std::endl;
}
