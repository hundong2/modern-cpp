cmake_minimum_required(VERSION 3.20)
project(daily_ranges_pipeline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ranges_demo main.cpp)
add_executable(ranges_problem problem.cpp)

enable_testing()
add_test(NAME ranges_demo_runs COMMAND ranges_demo)
add_test(NAME ranges_problem_runs COMMAND ranges_problem)
