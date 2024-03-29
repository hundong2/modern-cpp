#ifndef LR_VALUE_HPP
#define LR_VALUE_HPP

#include <iostream>

namespace reference
{
	//L-R Value 
	int NextVal_1(int* p) { return *(p + 1); }
	int* NextVal_2(int* p) { return (p + 1); }

	/// <summary>
	/// L Value : Lvalue 는 객체를 참조하는 표현식이다 [메모리 위치를 가지고 있다] [The C Programming Language - Kernighan and Ritchie].
	/// R Value : C++ 표준은 r-value 정의할때, 제외 개념으로 처리하고있느데, 다음과 같다.
	/// "모든 표현식은 Lvalue 거나 Rvalue이다" 고로, Rvalue 는 Lvalue 가 아닌 모든것이다. 
	/// 정확하게 말하자면, 구분가능한 메모리 영역을 가지는 객체를 나타낼 필요가 없는 표현식이다(임시로 존재하는것일수 있다).
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