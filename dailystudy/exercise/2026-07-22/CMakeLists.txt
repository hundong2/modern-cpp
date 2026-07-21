cmake_minimum_required(VERSION 3.20)
project(daily_modern_cpp_2026_07_22 LANGUAGES CXX)

# std::span과 concepts를 쓰기 위해 C++20을 요구한다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(span_architecture main.cpp)
add_executable(span_problem problem.cpp)

if(MSVC)
  target_compile_options(span_architecture PRIVATE /W4 /permissive-)
  target_compile_options(span_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(span_architecture PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(span_problem PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME architecture_demo COMMAND span_architecture)
add_test(NAME practice_solution COMMAND span_problem)
