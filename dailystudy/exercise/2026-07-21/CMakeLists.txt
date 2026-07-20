cmake_minimum_required(VERSION 3.16)
project(daily_expected_pipeline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(expected_demo main.cpp)
add_executable(expected_problem problem.cpp)

if(MSVC)
  target_compile_options(expected_demo PRIVATE /W4 /permissive-)
  target_compile_options(expected_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(expected_demo PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(expected_problem PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME expected_demo_test COMMAND expected_demo)
add_test(NAME expected_problem_test COMMAND expected_problem)
