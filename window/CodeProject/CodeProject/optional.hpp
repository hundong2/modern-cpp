#pragma once
#include <optional>

namespace example::optional
{
	std::optional<int> TestOptional(bool giveIt)
	{
		if (giveIt)
		{
			return 42;
		}
		return std::nullopt; //or nothing 
	}

	void TestOptional2()
	{
		std::optional<int> data1 = example::optional::TestOptional(true);
		std::optional<int> data2 = example::optional::TestOptional(false);
		std::cout << std::format("has value = {} ", data1.has_value()) << std::endl;
		std::cout << std::format("has value = {} ", data2.has_value()) << std::endl;

		std::cout << std::format("data1.value : {}", data1.value()) << std::endl;
		std::cout << std::format("data1.value : {}", *data1) << std::endl;
		std::cout << std::format("data2.value : {}", data2.value_or(0)) << std::endl;

		//std::cout << std::format("data2.value : {}", data2.value()) << std::endl; nullptr execption
		//std::cout << std::format("data2.value : {}", *data2) << std::endl; nullptr exception
		//don't use optional<&T>, but, optional<*T> possible
	}
}