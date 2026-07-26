cmake_minimum_required(VERSION 3.20)

project(daily_cpp_2026_07_27 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(architecture_demo main.cpp)
add_executable(beginner_problem problem.cpp)

if(MSVC)
  target_compile_options(architecture_demo PRIVATE /W4 /permissive-)
  target_compile_options(beginner_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(architecture_demo PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(beginner_problem PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME architecture_demo_runs COMMAND architecture_demo)
add_test(NAME beginner_problem_runs COMMAND beginner_problem)
