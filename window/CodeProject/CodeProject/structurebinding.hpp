#pragma once
#include <iostream>
#include <format>
#include <array>


namespace example::structurebinding
{
	void TestBinding()
	{
		std::array values{ 11, 22, 33 };
		
	}
	void TestStruct()
	{
		std::array values{ 11, 22, 33};
		auto [a, b, c] { values };//error : int [a, b, c] { values };
	}

	void TestStaticStructBinding()
	{
		struct Point { double m_x, m_y, m_z; };
		Point point;
		point.m_x = 1.0; point.m_y = 2.0; point.m_z = 3.0;
		auto [x, y, z] {point};

		std::cout << std::format("x: {}, y: {}, z: {}", x, y, z) << std::endl;
	}

	void TestPair()
	{
		std::pair myPair{ "hello", 5 };
		auto [testString, theInt] {myPair};

		std::cout << std::format("pair String {}, pair Integer {}", testString, theInt) << std::endl;
	}

}