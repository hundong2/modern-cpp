#pragma once
#include <initializer_list>
#include <iostream>
#include <optional>
#include <format>

namespace example::initialize::list
{
	std::optional<int> TestInitialize(std::initializer_list<int> test)
	{
		auto total{0};
		for (auto i : test) {
			total += i;
		}
		return total;
	}

	void TestMain()
	{
		int a{ TestInitialize({1,2,3}).value() };
		int b{ TestInitialize({10,20,30,40,50,60}).value() };
		std::cout << std::format("sum a {}, sum b {}", a, b) << std::endl;
	}
}