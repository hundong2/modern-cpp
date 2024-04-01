#pragma once
#include <iostream>

namespace example::function
{
	/// <summary>
	/// return auto 
	/// </summary>
	/// <param name="a"></param>
	/// <param name="b"></param>
	/// <returns>summary a and b</returns>
	auto TestAutoReturn(int a, int b)
	{
		std::cout << "current function name : " << __func__ << std::endl;
		return a + b;
	}
	namespace attribute
	{
		// old version : __attribute__, __declspec 
		// new version : [[attribute]]
		// example : [[fallthrough]] in swtich, case expression 
		[[nodiscard("Some Explanation")]] int func() //not used return value then warning message C4834 discarding return value of function with 'nodiscard' attribute
		{
			return 42;
		}
		int func(int a,[[maybe_unused]] int b) //C4100 warning ( not used parameter ) 
		{
			return 1;
		}
		[[deprecated("Unsafe method, please use other function")]]int func(double a) //error message C4996 
		{
			return 1;
		}
	}
}