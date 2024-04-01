#pragma once

#include <iostream>
#include <format> //c++20
#include <cstddef> //std::byte 
#include <limits> //limit varirable 
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

		std::cout << "Exampe1" << std::endl;
		std::cout << "decimal : "	<< std::format("{:d}", _decimal) << std::endl;
		std::cout << "octal : "		<< std::format("{:o}", _octal) << std::endl;
		std::cout << "hexa {:x}, {:X}, {:#x}, {:#X} : "	<< std::format("{0:x}, {0:X}, {0:#x}, {0:#X}", _hexadecimal) << std::endl;
		std::cout << "binary {:b}, {:#b}, {:B}: "	<< std::format("{0:b}, {0:#b}, {0:#B} ", _binary) << std::endl;
		
	}

	void Test2()
	{
		int randomVariable;
		int notRandomVariable{ 2 }; //uniform initialization it same that expression "int notRandomVariable = 2"
		std::cout << "Example2" << std::endl;
		std::cout << std::format(" random variable : {0}, not random variable {1}", randomVariable, notRandomVariable) << std::endl;
	}
	/// <summary>
	/// C++ is strong type
	/// </summary>
	void Test3()
	{
		long l { -7L };
		long long LL{ 17LL };
		std::cout << "Example3" << std::endl;
		std::cout << std::format("long : {0}, long long : {1}", l, LL);
	}

	/// <summary>
	/// over C++20
	/// </summary>
	void TestUnicode()
	{
		// setting single n bit n - encoding 
		// Unicode area https://ko.wikipedia.org/wiki/%EC%9C%A0%EB%8B%88%EC%BD%94%EB%93%9C_%EC%98%81%EC%97%AD

		char8_t c8 { u8'm'};
		char16_t c16{ u'm' };
		char32_t c32{ U'm' };
		std::byte ubyte{ 42 }; //enum class std::byte = unsigned char ( 1byte )
	}

	/// <summary>
	/// threshold for variable min, max, lowest 
	/// min variable at double is the smallest amount that can be expressed 
	/// lowest is -max()
	/// </summary>
	void TestNuemericThreshold()
	{
		std::cout << "integer :" << std::endl;
		std::cout << std::format("Max int value : {}", std::numeric_limits<int>::max()) << std::endl;
		std::cout << std::format("Min int value : {}", std::numeric_limits<int>::min()) << std::endl;
		std::cout << std::format("Lowest int value {}", std::numeric_limits<int>::lowest()) << std::endl;

		std::cout << "double :" << std::endl;
		std::cout << std::format("Max double value : {}", std::numeric_limits<double>::max()) << std::endl;
		std::cout << std::format("Min double value : {}", std::numeric_limits<double>::min()) << std::endl;
		std::cout << std::format("Lowest double value {}", std::numeric_limits<double>::lowest()) << std::endl;
	}

	/// <summary>
	/// zero initializer 
	/// </summary>
	void TestUniformInit()
	{
		float myFloat {};//0.0
		int myInt{}; //0
		int* myPointer{}; //nullptr
	}

	/// <summary>
	/// type casting 
	/// recommand static_cast
	/// </summary>
	void TestTypeCasting()
	{
		float myfloat {3.14f};
		int i1{ (int)myfloat }; //C 
		int i2{ int(myfloat) }; //not used
		int i3{ static_cast<int>(myfloat) }; //recommand 
		long lo = myfloat; // no need type casting long type size larger than float type 
	}

	/// <summary>
	/// floating point test
	/// </summary>
	void TestFloatingPoint()
	{
		double zero = 0.0;
		double posinf = 5.0 / zero;
		double neginf = -5.0 / zero;
		double nan = zero / zero;
		auto inf = std::numeric_limits<double>::infinity();
		//infinity
		std::cout << std::format("infinity value : {} ", inf) << std::endl;
		std::cout << "nan value : " << std::isnan(nan) << std::endl; //nan = 1
		std::cout << "is not nan value : " << std::isnan(zero) << std::endl; //not nan = 0
		std::cout << "inf value : " << std::isinf(inf) << std::endl; //infinity = 1
		std::cout << "is not inf value : " << std::isinf(zero) << std::endl; //not inf = 0
	}
}