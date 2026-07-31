cmake_minimum_required(VERSION 3.20)
project(daily_modern_cpp_2026_08_01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(command_architecture main.cpp)
add_executable(command_problem problem.cpp)

enable_testing()
add_test(NAME command_architecture_runs COMMAND command_architecture)
add_test(NAME command_problem_runs COMMAND command_problem)
