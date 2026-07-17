cmake_minimum_required(VERSION 3.20)
project(daily_modern_cpp_2026_07_18 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(checkout_demo main.cpp)
add_executable(checkout_problem problem.cpp)

if(MSVC)
  target_compile_options(checkout_demo PRIVATE /W4 /permissive-)
  target_compile_options(checkout_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(checkout_demo PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(checkout_problem PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME checkout_demo_runs COMMAND checkout_demo)
add_test(NAME checkout_problem_runs COMMAND checkout_problem)
