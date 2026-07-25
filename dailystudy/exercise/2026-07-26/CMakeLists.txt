cmake_minimum_required(VERSION 3.20)
project(daily_concepts LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(concept_demo main.cpp)
add_executable(concept_problem problem.cpp)

enable_testing()
add_test(NAME concept_demo_runs COMMAND concept_demo)
add_test(NAME concept_problem_runs COMMAND concept_problem)

