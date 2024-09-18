#include <iostream>
#include "ExampleAllocateArray.hpp"
#include "ExampleConst.hpp"
#include "ExampleReference.hpp"
#include "ExampleCasting.hpp"
#include "Employee.hpp"
#include "ExampleString.hpp"
#include "ExampleKeyValue.hpp"
#include "ExampleArray339.hpp"

#include "ExamplePointer.hpp"

#include "SpreadsheetCell.hpp"

#include <memory>

void ExampleChap8()
{
    auto myCell = std::make_shared<SpreadsheetCell>( double { 5.0 });
    auto anotherCell = std::make_shared<SpreadsheetCell>("5.0");    

    myCell->setValue(5);
    anotherCell->setString("6.5");

    std::cout << "myCell: " << myCell->getValue() << std::endl;
    std::cout << "anotherCell: " << anotherCell->getValue() << std::endl;
}
void ExampleFormatter()
{
    using namespace std;
    KeyValue keyValue { "Key1", 11 };
    cout << format( "{}", keyValue) << endl;
    cout << format( "{:a}", keyValue) << endl;
    cout << format( "{:b}", keyValue) << endl;
    cout << format( "{:c}", keyValue) << endl;
    try
    {
        //cout << format("{:d}", keyValue) << endl;
    }
    catch(const format_error& ex)
    {
        cout << ex.what() << endl;
    }
    
}
void ExampleEmployee()
{
    std::cout << "Testing the Employee class." << std::endl;
    Records::Employee emp { "Jane", "Doe" };
    emp.setEmployeeNumber(71);
    emp.setSalary(50'000);
    emp.setFirstName("John");
    emp.setLastName("Doe");
    emp.promote();
    emp.promote(50);
    emp.hire();
    emp.display();
}
int main()
{
#ifdef _WIN32
    _CtrSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif //_WIN32
    auto *exampleAllocateArray = new ExampleAllocateArray();
    delete exampleAllocateArray;
    exampleAllocateArray = nullptr;
    
    std::cout << std::endl;
    auto *exampleConst = new ExampleConst();
    delete exampleConst;
    exampleConst = nullptr;

    auto* exampleRef = new ExampleReference();
    exampleRef->exampleReference();
    exampleRef->exampleReference2();
    //std::string& str { exampleRef->getString() }; //error: cannot bind non-const lvalue reference of type ‘std::string&’ {aka ‘std::__cxx11::basic_string<char>&’} to an rvalue of type ‘std::string’ {aka ‘std::__cxx11::basic_string<char>’}
    const std::string& str { exampleRef->getString() };
    std::cout << "const std::string& " << str << std::endl;
    exampleRef->exampleReference3();
    exampleRef->exampleReference4();
    exampleRef->exampleReference5();
    exampleRef->exampleReference6();
    exampleRef->exampleReference7();

    auto* exampleCast = new ExampleCasting();
    exampleCast->exampleCasting1();
    ExampleEmployee();


    //ExampleString();
    std::cout << "Example String Library"  << std::endl;
    exampleString();
    exampleString2();
    exampleString3();
    exampleString4();
    exampleString5();
    exampleString6();
    exampleString7();
    exampleString8();
    exampleString9();
    ExampleConvertToString();
    ExampleStringView();
    ExampleStringView2();
    ExampleStringView3();
    ExampleStringView4();
    ExampleStringFormat1();
    ExampleStringFormat2();
    ExampleStringFormat3();

    ExampleFormatter();
    exampleArray339();
    exampleArray341();
    ExampleArray344();
    ExamplePage349();

    //chapter 8

    ExampleChap8();
    return 0;
}