cmake_minimum_required(VERSION 3.20)
project(daily_modern_cpp_2026_07_30 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(chrono_architecture main.cpp)
add_executable(chrono_problem problem.cpp)

enable_testing()
add_test(NAME chrono_architecture_runs COMMAND chrono_architecture)
add_test(NAME chrono_problem_runs COMMAND chrono_problem)
