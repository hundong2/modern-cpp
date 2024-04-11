#pragma once
#include <iostream>
#include <format>
#include <array>
#include <vector>

namespace example::loop
{
	/// <summary>
	/// Range loop 
	/// </summary>
	void TestRange()
	{
		std::array arr{ 1,2,3,4,5 };
		for (auto i : arr) 
		{
			std::cout << i << " ";
		}
		std::cout << std::endl;

		std::vector vec{ 1,2,3,4,5 };
		for (auto i : vec) {
			std::cout << i << " ";
		}
		std::cout <<  std::endl;
	}

	/// <summary>
	/// Range loop for c++20
	/// for ( <initailizer>; <for-range-declare>; <for-range-initializer> ) { <context> }
	/// </summary>
	void TestRange20()
	{
		for (std::array arr{ 1,2,3,4,5 }; auto i: arr)
		{
			std::cout << i << " ";
		}
		std::cout << std::endl;
	}
}