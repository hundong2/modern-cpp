#include <iostream>
#include <boost/algorithm/algorithm.hpp>
#include "reference.hpp"
#include "LRValue.hpp"
#include "Modern20.hpp"
#include "Literal.hpp"
#include "Enum.hpp"
#include "Compare.hpp"
#include "function.hpp"
#include "array.hpp"

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
	myliteral::Test2();
	myliteral::Test3();
	myliteral::TestNuemericThreshold();
	myliteral::TestUniformInit();
	myliteral::TestFloatingPoint();

	std::cout << "enum test" << std::endl;
	namespace enumtest = example::enumclass;
	enumtest::TestEnum();
	enumtest::TestModule();
	enumtest::TestSwitch();
	
	std::cout << "compare test" << std::endl;
	namespace myCompare = example::compare;
	myCompare::TestSpaceshipOperator();

	std::cout << "function test" << std::endl;
	namespace myFunction = example::function;
	std::cout << std::format( "1 + 2 = {} ", myFunction::TestAutoReturn(1, 2)) << std::endl;
	myFunction::attribute::func(); // warning 
	auto test = myFunction::attribute::func(); // not warning 

	auto value = myFunction::attribute::func(1,2);
	//auto value2 = myFunction::attribute::func(3.14); //C4996 error 

	namespace myArray = example::testarray;
	myArray::TestArray();
	myArray::TestArrayStandard();
	myArray::TestVector();


}