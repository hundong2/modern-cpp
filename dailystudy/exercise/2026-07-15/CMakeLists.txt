cmake_minimum_required(VERSION 3.16)

project(daily_exercise_2026_07_15 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(exercise_router main.cpp)
add_executable(exercise_problem problem.cpp)

if (MSVC)
    target_compile_options(exercise_router PRIVATE /W4)
    target_compile_options(exercise_problem PRIVATE /W4)
else()
    target_compile_options(exercise_router PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(exercise_problem PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 두 프로그램의 종료 코드가 0인지 CTest에서 반복 검증한다.
enable_testing()
add_test(NAME router_example_runs COMMAND exercise_router)
add_test(NAME load_problem_runs COMMAND exercise_problem)
