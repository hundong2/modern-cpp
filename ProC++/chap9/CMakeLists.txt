cmake_minimum_required(VERSION 3.20)
project(SpreadsheetCellchap9Lib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ch9_spreadsheet_cell STATIC
    SpreadsheetCell.cpp
    SpreadsheetCell.hpp
)


target_include_directories(ch9_spreadsheet_cell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(ch9_spreadsheet_cell PRIVATE -fmodules-ts)
