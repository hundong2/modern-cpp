#include "SpreadsheetCell.hpp"
#include <charconv>

SpreadsheetCell::SpreadsheetCell(const SpreadsheetCell& src) : m_value { src.m_value }
{
    std::cout << "called copy constructor" << std::endl;
}
SpreadsheetCell& SpreadsheetCell::operator=(const SpreadsheetCell& rhs)
{
    std::cout << "called assignment constructor operator" << std::endl;
    if (this == &rhs)
    {
        return *this;
    }
    m_value = rhs.m_value;
    return *this;
}
SpreadsheetCell::SpreadsheetCell(double initializeValue)
{
    setValue(initializeValue);
}
SpreadsheetCell::SpreadsheetCell(std::string_view initialValue)
{
    setString(initialValue);
}
void SpreadsheetCell::setValue(double value)
{
    m_value = value;
}
double SpreadsheetCell::getValue() const
{
    return m_value;
}

void SpreadsheetCell::setString(std::string_view inString)
{
    m_value = stringToDouble(inString);
}
std::string SpreadsheetCell::getString() const
{
    return doubleToString(m_value);
}

std::string SpreadsheetCell::doubleToString(double value) const
{
    return std::to_string(value);
}
double SpreadsheetCell::stringToDouble(std::string_view inString) const
{
    double number { 0 };
    std::from_chars(inString.data(), inString.data() + inString.size(), number);
    return number;
}

