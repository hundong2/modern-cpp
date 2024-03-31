#include <iostream>
#include <boost/algorithm/algorithm.hpp>
#include "reference.hpp"
#include "LRValue.hpp"
#include "Modern20.hpp"
#include "Literal.hpp"

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

	std::cout << "format expression" << std::endl;
	namespace myformat = print::expression::format;
	myformat::formatExpression();

	std::cout << "format expression" << std::endl;
	namespace myliteral = literal::chapter113;
	myliteral::Test();
}