cmake_minimum_required(VERSION 3.16)
project(daily_variant_state LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(variant_demo main.cpp)
add_executable(variant_problem problem.cpp)

if(MSVC)
  target_compile_options(variant_demo PRIVATE /W4 /permissive-)
  target_compile_options(variant_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(variant_demo PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(variant_problem PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME variant_demo_test COMMAND variant_demo)
add_test(NAME variant_problem_test COMMAND variant_problem)
