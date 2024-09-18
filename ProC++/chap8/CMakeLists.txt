cmake_minimum_required(VERSION 3.20)
project(SpreadsheetCellLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(spreadsheet_cell STATIC
    SpreadsheetCell.cpp
    SpreadsheetCell.hpp
)

add_library(foo_lib STATIC
    FooExample.cpp
    FooExample.hpp
)

add_library(evensequence_lib STATIC 
    EvenSequence.cpp
    EvenSequence.hpp
)

target_include_directories(spreadsheet_cell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(foo_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(evensequence_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})