#include <iostream>
#include "ExampleAllocateArray.hpp"
#include "ExampleConst.hpp"
#include "ExampleReference.hpp"
#include "ExampleCasting.hpp"
#include "Employee.hpp"

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
    return 0;
}