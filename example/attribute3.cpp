#include <thread>
#include <future>
#include <iostream>

int foo(int i)
{
	//compiler의 최ㅏ적화에 관련된 것. 
	if( __builtin_expect(i>0, 1)) //linux
	//if(i > 0) [[likely]]
		i += 2;
	else
		i -= 2;
	return i;
}

int main()
{
	foo(10);
	return 0;
}
