cmake_minimum_required(VERSION 3.20)

project(daily_coroutine_task LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(coroutine_architecture main.cpp)
add_executable(coroutine_problem problem.cpp)

enable_testing()
add_test(NAME coroutine_architecture_runs COMMAND coroutine_architecture)
add_test(NAME coroutine_problem_runs COMMAND coroutine_problem)

