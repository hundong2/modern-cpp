cmake_minimum_required(VERSION 3.20)
project(daily_2026_09_23 LANGUAGES CXX)

# std::views::zip은 C++23 기능이다. 세 소스는 서로 독립된 실행 프로그램이다.
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
  "breaches=2,billing:12,cache:10")
add_exact_output_case(daily_problem_runs daily_problem ""
  "valid=2,total=11,width=rejected,domain=rejected")

add_exact_output_case(projects_official icpc_problem
  "4\n2 4 4\n3 6 6\n6 8 2\n5 7 3\n"
  "7")
add_exact_output_case(projects_single icpc_problem
  "1\n1 1 100\n"
  "100")
add_exact_output_case(projects_touching_days_conflict icpc_problem
  "2\n1 2 5\n2 3 7\n"
  "7")
add_exact_output_case(projects_disjoint_chain icpc_problem
  "3\n1 1 5\n2 2 6\n3 3 7\n"
  "18")
add_exact_output_case(projects_same_end_and_followup icpc_problem
  "4\n1 4 5\n2 4 9\n5 5 2\n1 10 10\n"
  "11")
add_exact_output_case(projects_64_bit_sum icpc_problem
  "3\n1 1 1000000000\n2 2 1000000000\n3 3 1000000000\n"
  "3000000000")
