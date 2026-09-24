cmake_minimum_required(VERSION 3.20)
project(daily_2026_09_25 LANGUAGES CXX)

# std::generator와 ranges 알고리즘은 오늘 학습 환경의 C++23 모드를 사용한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스는 각각 main을 가진 독립 실행 파일이다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 초보자가 위험한 변환·shadowing도 즉시 볼 수 있게 높은 경고 수준을 공통 적용한다.
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

# 입력과 기대 출력은 build 폴더에만 만들고 helper가 종료 코드와 stdout 전체를 비교한다.
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
  "slow=2,total=355")
add_exact_output_case(daily_problem_runs daily_problem ""
  "alerts=2,shortage=6")

add_exact_output_case(meet_middle_official icpc_problem
  "4 5\n1 2 3 2\n"
  "3")
add_exact_output_case(meet_middle_single_match icpc_problem
  "1 7\n7\n"
  "1")
add_exact_output_case(meet_middle_single_miss icpc_problem
  "1 8\n7\n"
  "0")
add_exact_output_case(meet_middle_duplicate_sums icpc_problem
  "4 2\n1 1 1 1\n"
  "6")
add_exact_output_case(meet_middle_large_values icpc_problem
  "3 1000000000\n1000000000 1000000000 1\n"
  "2")
add_exact_output_case(meet_middle_64_bit_sum icpc_problem
  "5 705032704\n1000000000 1000000000 1000000000 1000000000 1000000000\n"
  "0")
add_exact_output_case(meet_middle_64_bit_answer icpc_problem
  "40 20\n1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1\n"
  "137846528820")
