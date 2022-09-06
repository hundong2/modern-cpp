#include <functional>
#include <iostream>
#include <string>

int some_func1(const std::string& a)
{
	std::cout << "func1 call " << a << std::endl;
	return 0;
}

struct S
{
	void operator()(char c){ std::cout << "func2 call " << c << std::endl; }
};

int main()
{
	std::function<int(const std::string&)> f1 = some_func1;
	std::function<void(char)> f2 = S();
	std::function<void()> f3 = [](){ std::cout << "function3 call " << std::endl; };

	f1("hello");
	f2('c');
	f3();
}

