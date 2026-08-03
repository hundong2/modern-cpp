cmake_minimum_required(VERSION 3.20)
project(daily_2026_08_04 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

enable_testing()
add_test(NAME daily_main_runs COMMAND daily_main)
add_test(NAME daily_problem_runs COMMAND daily_problem)

set(ICPC_INPUT "${CMAKE_CURRENT_BINARY_DIR}/icpc_input.txt")
file(WRITE "${ICPC_INPUT}" "3 3\n1 2 1\n2 3 2\n1 3 3\n")
add_test(NAME icpc_example
    COMMAND ${CMAKE_COMMAND}
        -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
        -DINPUT_FILE=${ICPC_INPUT}
        -DEXPECTED=3
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
