#ifndef SPREADSHEETCELL_HPP
#define SPREADSHEETCELL_HPP

#include <cstddef>
#include <iostream>
#include <utility>
#include <format>

namespace ch9
{
    class SpreadsheetCell 
    {
        public:
            SpreadsheetCell() = default;
            SpreadsheetCell(size_t width, size_t height);
            ~SpreadsheetCell();
            SpreadsheetCell(const SpreadsheetCell& src); //need deep copy constructor
            SpreadsheetCell& operator=(const SpreadsheetCell& rhs); //need deep copy assignment operator
            void setCellAt(size_t x, size_t y, const SpreadsheetCell& cell);
            SpreadsheetCell& getCellAt(size_t x, size_t y);
            void swap(SpreadsheetCell& other) noexcept;
        private:
            bool inRange(size_t value, size_t upper) const;
            void verifyCoordinate(size_t x, size_t y) const;
            size_t m_width;
            size_t m_height;
            SpreadsheetCell** m_cells { nullptr };
    };
    void swap(SpreadsheetCell& first, SpreadsheetCell& second) noexcept;

}

#endif // SPREADSHEETCELL_HPP