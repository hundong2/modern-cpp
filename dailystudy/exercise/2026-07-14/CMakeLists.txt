cmake_minimum_required(VERSION 3.16)

project(daily_exercise_2026_07_14 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(exercise_architecture main.cpp)
add_executable(exercise_window problem.cpp)

if (MSVC)
    target_compile_options(exercise_architecture PRIVATE /W4)
    target_compile_options(exercise_window PRIVATE /W4)
else()
    target_compile_options(exercise_architecture PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(exercise_window PRIVATE -Wall -Wextra -Wpedantic)
endif()

# 실행 파일이 단순히 컴파일되는 데서 끝나지 않고 내부 assert까지 통과하는지 자동 검증한다.
enable_testing()
add_test(NAME architecture_example_runs COMMAND exercise_architecture)
add_test(NAME window_problem_runs COMMAND exercise_window)
