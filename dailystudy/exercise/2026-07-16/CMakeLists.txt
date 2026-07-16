cmake_minimum_required(VERSION 3.16)

project(daily_exercise_2026_07_16 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(inventory_demo main.cpp)
add_executable(syntax_problem problem.cpp)

foreach(target inventory_demo syntax_problem)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
