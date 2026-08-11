cmake_minimum_required(VERSION 3.20)
project(daily_2026_08_12 LANGUAGES CXX)

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

add_icpc_case(icpc_example "10 15\n5 1 3 5 10 7 4 9 2 8\n" "2")
add_icpc_case(icpc_single "1 7\n7\n" "1")
add_icpc_case(icpc_impossible "4 100\n1 2 3 4\n" "0")
