module employee;
import <iostream>;
import <format>;
using namespace std;

namespace Records
{
    Employee::Employee(const std::string& firstName, const std::string& lastName)
        : m_firstName { firstName }, m_lastName { lastName }
    {
    }
}