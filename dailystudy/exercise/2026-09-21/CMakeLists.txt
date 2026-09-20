cmake_minimum_required(VERSION 3.20)
project(daily_2026_09_21 LANGUAGES CXX)

# std::bind_back은 C++23 표준 기능이다. 세 파일은 서로 독립된 실행 프로그램이다.
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

# 입력과 기대 출력은 빌드 폴더에만 생성하고 전체 stdout과 종료 코드를 함께 검사한다.
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

add_exact_output_case(daily_main_runs daily_main "" "A-42:standard=11000")
add_exact_output_case(daily_problem_runs daily_problem ""
  "first=1,2\nsecond=2,5")
add_exact_output_case(counting_numbers_official icpc_problem "123 321\n" "171")
add_exact_output_case(counting_numbers_zero icpc_problem "0 0\n" "1")
add_exact_output_case(counting_numbers_small_prefix icpc_problem "0 10\n" "11")
add_exact_output_case(counting_numbers_equal_adjacent icpc_problem "11 11\n" "0")
add_exact_output_case(counting_numbers_leading_zero_boundary icpc_problem "98 102\n" "3")
add_exact_output_case(counting_numbers_non_adjacent_repeat icpc_problem "101 101\n" "1")
add_exact_output_case(counting_numbers_full_range icpc_problem
  "0 1000000000000000000\n" "168856464709124011")
