#pragma once

#include <utility>
#include <iostream>
#include <format>

namespace example::utility
{
	void TestPair()
	{
		std::pair<double, int> myPair{ 1.23, 5 }; //CTAD ( Class template argument deduction )
		std::cout << std::format("{0}, {1}", myPair.first, myPair.second) << std::endl;
	}


}