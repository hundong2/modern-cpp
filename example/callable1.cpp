#include <iostream>

int main()
{
	auto f = [](int a, int b){ std::cout << "a+b=" << a+b << std::endl; };
	f(3,5);
}
