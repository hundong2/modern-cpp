#pragma once

#include <iostream>
import EmployeeModule;
namespace example::enumclass
{
	enum class TestEnum : std::uint8_t
	{
		King = 0x01,
		Queen = 0x02,
		I = 0x33
	};
	/// <summary>
	/// Test Enum
	/// </summary>
	void TestEnum()
	{
		if (static_cast<std::uint8_t>(TestEnum::King) == 0x01)
		{
			std::cout << "enum class is OK " << std::endl;
		}
	}

	/// <summary>
	/// Module test
	/// </summary>
	void TestModule()
	{
		Employee employee;
		employee.employeeNumber = 1;
		employee.firstInitial = 'a';
		employee.lastInitial = 'b';
		employee.salary = 100;
		std::cout << std::format(" {0}, {1}, {2}, {3}", employee.employeeNumber, employee.firstInitial, employee.lastInitial, employee.salary) << std::endl;
	}

	/// <summary>
	/// Switch case 
	/// </summary>
	void TestSwitch()
	{
		enum class TestEnum : std::uint8_t
		{
			King = 0x01,
			Queen = 0x02,
			I = 0x33
		};
		int value = 0;
		TestEnum myEnum = TestEnum::King;

		switch (int count = 0; myEnum)
		{
		case TestEnum::King:
			[[fallthrough]];
		case TestEnum::Queen:
			[[fallthrough]];
		case TestEnum::I:
			value = 0;
			count++;
			break;
		default:
			count--;
			break;
		}
		//not used count
	}

}