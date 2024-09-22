#include "SpreadsheetCell.hpp"

using namespace std;
namespace ch9
{
    SpreadsheetCell::SpreadsheetCell(size_t width, size_t height)
        : m_width{ width }, m_height{ height }
    {
        m_cells = new SpreadsheetCell*[m_width];
        for (size_t i = 0; i < m_width; i++)
        {
            m_cells[i] = new SpreadsheetCell[m_height];
        }
    }
    SpreadsheetCell::SpreadsheetCell(const SpreadsheetCell& src) :
        m_width{ src.m_width }, m_height{ src.m_height }
    {
        for( size_t i { 0 }; i < m_width; i++ )
        {
            for( size_t j { 0 }; j < m_height; j++ )
            {
                m_cells[i][j] = src.m_cells[i][j];
            }
        }
    }
    // Deep copy assignment operator
    // this code is not exception safety. 
    /*
    SpreadsheetCell::operator=(const SpreadsheetCell& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        for (size_t i = 0; i < m_width; i++)
        {
            delete[] m_cells[i];
        }
        delete[] m_cells;
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        m_cells = new SpreadsheetCell*[m_width];
        for (size_t i = 0; i < m_width; i++)
        {
            m_cells[i] = new SpreadsheetCell[m_height];
        }
        for (size_t i = 0; i < m_width; i++)
        {
            for (size_t j = 0; j < m_height; j++)
            {
                m_cells[i][j] = rhs.m_cells[i][j];
            }
        }
        return *this;
    }
    */
    //Type safe deep copy assignment operator
    //If exception is thrown, the object is left in a valid state. ( no memory leak )
    SpreadsheetCell& SpreadsheetCell::operator=(const SpreadsheetCell& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        SpreadsheetCell temp{ rhs };
        swap(temp);
        return *this;
    }
    SpreadsheetCell::~SpreadsheetCell()
    {
        for (size_t i = 0; i < m_width; i++)
        {
            delete[] m_cells[i];
        }
        delete[] m_cells;
    }
    void SpreadsheetCell::swap(SpreadsheetCell& other) noexcept
    {
        std::swap(m_width, other.m_width);
        std::swap(m_height, other.m_height);
        std::swap(m_cells, other.m_cells);
    }

    /*
    SpreadsheetCell::setCellAt(size_t x, size_t y, const SpreadsheetCell& cell)
    {
        if (!inRange(x, m_width) || !inRange(y, m_height))
        {
            throw std::out_of_range(format("Cell index out of range: %d, %d", x, y));
        }
        m_cells[x][y] = cell;
    }
    SpreadsheetCell& SpreadsheetCell::getCellAt(size_t x, size_t y)
    {
        if (!inRange(x, m_width) || !inRange(y, m_height))
        {
            throw std::out_of_range(format("Cell index out of range: %d, %d", x, y));
        }
        return m_cells[x][y];
    }
    */
    bool SpreadsheetCell::inRange(size_t value, size_t upper) const
    {
        return value < upper;
    }
    void SpreadsheetCell::verifyCoordinate(size_t x, size_t y) const
    {
        if (x >= m_width)
        {
            throw std::out_of_range(format("Cell index out of range: %d, %d", x, y));
        }
        if (y >= m_height)
        {
            throw std::out_of_range(format("Cell index out of range: %d, %d", x, y));
        }
    }
    void SpreadsheetCell::setCellAt(size_t x, size_t y, const SpreadsheetCell& cell)
    {
        verifyCoordinate(x, y);
        m_cells[x][y] = cell;
    }
    SpreadsheetCell& SpreadsheetCell::getCellAt(size_t x, size_t y)
    {
        verifyCoordinate(x, y);
        return m_cells[x][y];
    }
    void swap(SpreadsheetCell& first, SpreadsheetCell& second) noexcept
    {
        first.swap(second);
    }

}