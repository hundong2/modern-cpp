#pragma once

#include <iostream>
#include <format>

namespace print::expression::format
{
	void formatExpression()
	{
		std::cout << std::format("test {} ", 123) << std::endl;
		std::cout << std::format(" {0} + {0} = {1}", 50, 100) << std::endl;
	}
}
