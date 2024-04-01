#pragma once

#include <iostream>
#include <compare>

namespace example::compare
{
	/// <summary>
	/// Three-way comparation operator
	/// </summary>
	void TestSpaceshipOperator()
	{
		// strong ordering 
		// integer  
		// strong_ordering::less A < B
		// strong_ordering::greater A > B
		// strong_ordering::equal A = B
		
		// floating point
		// partial_ordering::less A < B
		// partial ordering::preater A > B 
		// partial_ordering::equivalent A = B
		// partial_ordering::unordered A or B is not number 
		int i = { 11 };
		std::strong_ordering result{ 1 <=> 0 };
		if (result == std::strong_ordering::less) { std::cout << "less" << std::endl; }
		if (result == std::strong_ordering::greater) { std::cout << "greater" << std::endl; }
		if (result == std::strong_ordering::equal) { std::cout << "equal" << std::endl; }

		if (std::is_lt(result)) { std::cout << "less" << std::endl; }
		if (std::is_gt(result)) { std::cout << "greater" << std::endl; }
		if (std::is_eq(result)) { std::cout << "equal" << std::endl; }
	}
}