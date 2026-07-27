cmake_minimum_required(VERSION 3.20)
project(daily_cpp_2026_07_28 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_demo main.cpp)
add_executable(daily_problem problem.cpp)

enable_testing()
add_test(NAME daily_demo_runs COMMAND daily_demo)
add_test(NAME daily_problem_runs COMMAND daily_problem)
