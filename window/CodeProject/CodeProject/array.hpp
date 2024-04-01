#pragma once
#include <iostream>
#include <array>

namespace example::testarray
{
	/// <summary>
	/// Array Test Example
	/// </summary>
	void TestArray()
	{
		int arrayValue1[3] = { 0 };
		int arrayValue2[3] = {};
		int arrayValue3[3]{};
		int arrayValue4[]{ 1, 2, 3 };

		std::size_t arraySize{ std::size(arrayValue4) };
		std::size_t arraySizeofStatck{ sizeof(arrayValue4) / sizeof(arrayValue4[0]) };
		std::cout << "array std::size(): " << arraySize << std::endl;
		std::cout << "array sizeof(arr)/sizeof(arr[0]) : " << arraySizeofStatck << std::endl;
	}

	/// <summary>
	/// standard array 
	/// </summary>
	void TestArrayStandard()
	{
		std::array arrayValue{ 1,2,3 };
		std::cout << std::format("array size = {}", arrayValue.size()) << std::endl;
	}


	/// <summary>
	/// vector test 
	/// </summary>
	void TestVector()
	{
		std::vector<std::string> strVector{ "hello", "world" };
		strVector.push_back("my");
		strVector.push_back("is test function");
		std::cout << std::format("1st element : {}", strVector[0]);
	}
}