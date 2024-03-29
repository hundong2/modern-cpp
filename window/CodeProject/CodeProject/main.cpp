#include <iostream>
#include <boost/algorithm/algorithm.hpp>
#include "reference.hpp"
#include "LRValue.hpp"

using namespace reference;
int main()
{
	std::cout << "ReferenceArray" << std::endl;
	ReferenceArray();
	std::cout << "ReferenceFunctionTest" << std::endl;
	ReferenceFunctionTest();
	std::cout << "deletenewTest T* value = new T" << std::endl;
	deletenewTest();

	std::cout << "newArray T* pointer = new int[array isze]" << std::endl;
	newArray();

	std::cout << "LRValueTest" << std::endl;
	LRValueTest();

	std::cout << "LRValueTest2" << std::endl;
	LRValueTest2();
}