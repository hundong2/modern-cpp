#ifndef REFERENCE_HPP
#define REFERENCE_HPP

#include <iostream>
#include <vector>
using T = std::vector<int>;
using CONSTINT = const int&;

namespace reference
{
	CONSTINT SIZE = 5;
	void test1()
	{
		int number = 3;
		int& reference_number = number;

		reference_number = 5;
		std::cout << "number : " << number << std::endl;
		std::cout << "reference number : " << reference_number << std::endl;
	}

	void test2()
	{
		std::vector<int> temp = { 1, 2, 3, 4, 5, 6 };

		for (const auto& e: temp)
		{
			std::cout << e << std::endl;
		}
	}
	/**
	* test 3
	*/
	void test3()
	{
		T test = { 1,2,3,4,5 };
		for (const auto& e : test)
		{
			std::cout << e << std::endl;
		}
	}

	void ReferenceTest1()
	{
		int number = 3;
		int& ref = number;
		std::cout << ref << std::endl;
		int number2 = 4;
		ref = number2; 
		std::cout << ref << std::endl;
	}

	void ReferenceArray()
	{
		int test[3] = { 1,2,3 };
		int(&testRef)[3] = test;
		for( const auto& i : testRef )
			std::cout << "test: " << i << std::endl;
	}

	int ReferenceReturn1()
	{
		int a = 2;
		return a;
	}

	//dangling reference 
	int& ReferenceReturn2()
	{
		int b = 3;
		return b;
	}

	int ReferenceReturn3()
	{
		int c = 4;
		return c;
	}
	void ReferenceFunctionTest()
	{
		int a = ReferenceReturn1();
		int b = ReferenceReturn2();
		b = 3;
		//int &c = ReferenceReturn3(); //const refernce is lvalue
		const int& c = ReferenceReturn3();
		std::cout << a << " , " << b << ", " << c << std::endl;
	}
	/// <summary>
	/// new, delete example 
	/// T* pointer = new T;
	/// </summary>
	void deletenewTest()
	{
		int* p = new int; //heap 
		*p = 10;

		std::cout << *p << std::endl;

		delete p;
	}

	/// <summary>
	/// new
	/// T* Value = new T[array size]
	/// </summary>
	void newArray()
	{
		int arraySize = SIZE;
		int* list = new int[SIZE];
		for (int i = 0; i < SIZE; i++)
		{
			*(list + i) = i;
		}
		for (int i = 0; i < SIZE; i++ )
		{
			std::cout << list[i] << std::endl;
		}
		
		delete[] list;
	}



}


#endif
