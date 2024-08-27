#ifndef EXAMPLE_DESIGNATED_H
#define EXAMPLE_DESIGNATED_H
#include <string>
// Include any necessary headers here
struct Employee
{
    char firstInitial;
    char lastInitial;
    int employeeNumber;
    int salary {75'000};

    std::string toString() const {
        return "Employee: " + std::string(1, firstInitial) + " " + std::string(1, lastInitial) +
               ", Number: " + std::to_string(employeeNumber) +
               ", Salary: " + std::to_string(salary);
    }
};
// Declare your classes, functions, and variables here

#endif // EXAMPLE_DESIGNATED_H