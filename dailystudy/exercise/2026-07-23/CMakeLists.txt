cmake_minimum_required(VERSION 3.20)
project(daily_modern_cpp_2026_07_23 LANGUAGES CXX)

# std::jthread와 std::stop_token은 C++20 기능이다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
add_executable(jthread_architecture main.cpp)
add_executable(jthread_problem problem.cpp)
target_link_libraries(jthread_architecture PRIVATE Threads::Threads)
target_link_libraries(jthread_problem PRIVATE Threads::Threads)

if(MSVC)
  target_compile_options(jthread_architecture PRIVATE /W4 /permissive-)
  target_compile_options(jthread_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(jthread_architecture PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(jthread_problem PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME architecture_demo COMMAND jthread_architecture)
add_test(NAME practice_solution COMMAND jthread_problem)
