cmake_minimum_required(VERSION 3.16)

project(daily_exercise_2026_07_17 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(library_demo main.cpp)
add_executable(optional_problem problem.cpp)

foreach(target library_demo optional_problem)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()

enable_testing()
add_test(NAME library_example_runs COMMAND library_demo)
add_test(NAME optional_problem_runs COMMAND optional_problem)
