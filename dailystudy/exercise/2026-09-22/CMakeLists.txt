cmake_minimum_required(VERSION 3.20)
project(daily_2026_09_22 LANGUAGES CXX)

# std::ranges::fold_left는 C++23 기능이다. 세 소스는 각각 독립된 실행 프로그램이다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

if(MSVC)
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  endforeach()
endif()

enable_testing()

# 입력과 기대 출력은 build 폴더에만 만들고, helper가 종료 코드와 stdout 전체를 비교한다.
function(add_exact_output_case name target input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.input.txt")
  set(expected_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.expected.txt")
  file(WRITE "${input_file}" "${input}")
  file(WRITE "${expected_file}" "${expected}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    "-DEXECUTABLE=$<TARGET_FILE:${target}>"
    "-DINPUT_FILE=${input_file}"
    "-DEXPECTED_FILE=${expected_file}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake")
  set_tests_properties("${name}" PROPERTIES TIMEOUT 15)
endfunction()

add_exact_output_case(daily_main_runs daily_main ""
  "net=500,count=3,last=refund")
add_exact_output_case(daily_problem_runs daily_problem ""
  "passed=2,failed=2,first=database")

add_exact_output_case(longest_route_official icpc_problem
  "5 5\n1 2\n2 5\n1 3\n3 4\n4 5\n"
  "4\n1 3 4 5")
add_exact_output_case(longest_route_impossible icpc_problem
  "4 2\n1 2\n3 4\n"
  "IMPOSSIBLE")
add_exact_output_case(longest_route_direct icpc_problem
  "2 1\n1 2\n"
  "2\n1 2")
add_exact_output_case(longest_route_ignores_unreachable icpc_problem
  "6 5\n1 2\n2 6\n3 4\n4 5\n5 6\n"
  "3\n1 2 6")
add_exact_output_case(longest_route_prefers_more_cities icpc_problem
  "6 6\n1 2\n2 6\n1 3\n3 4\n4 5\n5 6\n"
  "5\n1 3 4 5 6")
add_exact_output_case(longest_route_equal_length_tie icpc_problem
  "5 5\n1 2\n1 3\n2 4\n3 4\n4 5\n"
  "4\n1 2 4 5")
