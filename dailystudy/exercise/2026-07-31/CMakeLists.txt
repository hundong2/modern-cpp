cmake_minimum_required(VERSION 3.20)
project(daily_modern_cpp_2026_07_31 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(area_architecture main.cpp)
add_executable(area_problem problem.cpp)

enable_testing()
add_test(NAME area_architecture_runs COMMAND area_architecture)
add_test(NAME area_problem_runs COMMAND area_problem)
