#ifndef LR_VALUE_HPP
#define LR_VALUE_HPP

#include <iostream>

namespace reference
{
	//L-R Value 
	int NextVal_1(int* p) { return *(p + 1); }
	int* NextVal_2(int* p) { return (p + 1); }

	/// <summary>
	/// L Value : Lvalue �� ��ü�� �����ϴ� ǥ�����̴� [�޸� ��ġ�� ������ �ִ�] [The C Programming Language - Kernighan and Ritchie].
	/// R Value : C++ ǥ���� r-value �����Ҷ�, ���� �������� ó���ϰ��ִ���, ������ ����.
	/// "��� ǥ������ Lvalue �ų� Rvalue�̴�" ���, Rvalue �� Lvalue �� �ƴ� �����̴�. 
	/// ��Ȯ�ϰ� �����ڸ�, ���а����� �޸� ������ ������ ��ü�� ��Ÿ�� �ʿ䰡 ���� ǥ�����̴�(�ӽ÷� �����ϴ°��ϼ� �ִ�).
	/// </summary>
	void LRValueTest()
	{
		int a[] = { 1,2,3,4,5 };
		//NextVal_1(a) = 9; //error 
		*NextVal_2(a) = 10;
		std::cout << *NextVal_2(a) << std::endl;
	}

	void LRValueTest2()
	{
		int a = 0;
		int b = ++a;
		std::cout << a << ", " << b << std::endl;
		++a = 4;
		std::cout << a << std::endl;


	}
}
#endif