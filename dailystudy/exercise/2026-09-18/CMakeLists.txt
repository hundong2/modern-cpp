cmake_minimum_required(VERSION 3.20)
project(daily_2026_09_18 LANGUAGES CXX)

# flat_map은 C++23 표준 타입이다. 확장 문법 없이 세 독립 프로그램을 검증한다.
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

# 입력·정답을 빌드 폴더에만 기록하며 종료 코드와 전체 출력을 비교한다.
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

add_exact_output_case(daily_main_runs daily_main "" "retries=3\nmissing=absent")
add_exact_output_case(daily_problem_runs daily_problem ""
  "before=on\ndeployed=off\nmissing=unknown")
add_exact_output_case(coprime_official icpc_problem
  "8\n5 4 20 1 16 17 5 15\n" "19")
add_exact_output_case(coprime_single icpc_problem "1\n1\n" "0")
add_exact_output_case(coprime_duplicate_ones icpc_problem "2\n1 1\n" "1")
add_exact_output_case(coprime_no_pair icpc_problem "3\n2 4 8\n" "0")
add_exact_output_case(coprime_mixed icpc_problem "4\n2 3 4 9\n" "4")
add_exact_output_case(coprime_one_and_duplicates icpc_problem "3\n1 2 2\n" "2")
