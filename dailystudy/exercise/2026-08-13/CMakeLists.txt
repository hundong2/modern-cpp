cmake_minimum_required(VERSION 3.20)
project(daily_2026_08_13 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

enable_testing()
add_test(NAME daily_main_runs COMMAND daily_main)
add_test(NAME daily_problem_runs COMMAND daily_problem)

function(add_icpc_case name input expected)
    set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
    file(WRITE "${input_file}" "${input}")
    add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
        -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
        -DINPUT_FILE=${input_file} -DEXPECTED=${expected}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

add_icpc_case(icpc_example "7 8\n0 1 3\n1 1 7\n0 7 6\n1 7 1\n0 3 7\n0 4 2\n0 1 1\n1 1 1\n" "NO\nNO\nYES")
add_icpc_case(icpc_chain "5 5\n0 1 2\n0 2 3\n1 1 3\n1 1 4\n1 4 5\n" "YES\nNO\nNO")

