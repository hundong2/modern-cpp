cmake_minimum_required(VERSION 3.10)

# Set the project name
project(SpreadsheetProject)

file(GLOB_RECURSE SOURCES "*.cpp")
# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER clang++)
# Add the executable
add_library(SpreadsheetLibrary SpreadsheetCell.cpp)
# Only build the library, no executable
#target_include_directories(SpreadsheetLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)