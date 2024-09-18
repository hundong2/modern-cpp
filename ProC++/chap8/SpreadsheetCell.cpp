#include "SpreadsheetCell.hpp"
#include <charconv>


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

