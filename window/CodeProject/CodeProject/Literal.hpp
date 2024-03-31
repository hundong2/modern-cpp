#pragma once

#include <iostream>
#include <format> //c++20

namespace literal::chapter113
{
	/// <summary>
	/// literal Test with format 
	/// https://en.cppreference.com/w/cpp/utility/format/format
	/// </summary>
	void Test()
	{
		auto _decimal = 123;
		auto _octal = 0173;
		auto _hexadecimal = 0x7B;
		auto _binary = 0b1111011;

		std::cout << "decimal : "	<< std::format("{:d}", _decimal) << std::endl;
		std::cout << "octal : "		<< std::format("{:o}", _octal) << std::endl;
		std::cout << "hexa {:x}, {:X}, {:#x}, {:#X} : "	<< std::format("{0:x}, {0:X}, {0:#x}, {0:#X}", _hexadecimal) << std::endl;
		std::cout << "binary {:b}, {:#b}, {:B}: "	<< std::format("{0:b}, {0:#b}, {0:#B} ", _binary) << std::endl;
		
	}
}