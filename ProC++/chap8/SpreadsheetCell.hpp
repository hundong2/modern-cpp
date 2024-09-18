#ifndef SPREADSHEETCELL_HPP
#define SPREADSHEETCELL_HPP

#include <string>
#include <string_view>

class SpreadsheetCell
{
public:
    SpreadsheetCell(double initializeValue);
    SpreadsheetCell(std::string_view initialValue);
    void setValue(double value);
    double getValue() const;

    void setString(std::string_view inString);
    std::string getString() const;
private:
    std::string doubleToString(double value) const;
    double stringToDouble(std::string_view inString) const;
    double m_value { 0 };
};

#endif // SPREADSHEETCELL_HPP