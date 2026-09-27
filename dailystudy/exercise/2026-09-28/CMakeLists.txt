cmake_minimum_required(VERSION 3.20)
project(daily_2026_09_28 LANGUAGES CXX)

# std::views::slide와 std::ranges::slide_view는 C++23 기능이다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스는 각각 main을 가진 독립 실행 프로그램이다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 변환·shadowing까지 초보자가 조기에 발견하도록 높은 경고 수준을 세 target에 공통 적용한다.
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

# 입력·기대 출력은 build 폴더에만 만들고 helper가 종료 코드와 stdout 전체를 비교한다.
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

# 학습 예제의 고정 출력도 회귀 테스트로 잠근다.
add_exact_output_case(daily_main_runs daily_main ""
  "breaches=3,worst_total_ms=176")
add_exact_output_case(daily_problem_runs daily_problem ""
  "unstable=2,widest_span_mg=28")

# 공식 예제와 창 경계·중복·64비트 비용을 서로 독립된 exact-output 사례로 검증한다.
add_exact_output_case(sliding_cost_official icpc_problem
  "8 3\n2 4 3 5 8 1 2 1\n"
  "2 2 5 7 7 1")
add_exact_output_case(sliding_cost_k_one icpc_problem
  "4 1\n9 1 9 5\n"
  "0 0 0 0")
add_exact_output_case(sliding_cost_k_equals_n icpc_problem
  "5 5\n5 1 9 2 7\n"
  "13")
add_exact_output_case(sliding_cost_duplicates icpc_problem
  "6 4\n3 3 3 3 9 3\n"
  "0 6 6")
add_exact_output_case(sliding_cost_64_bit_boundary icpc_problem
  "7 7\n1 1 1 1 1000000000 1000000000 1000000000\n"
  "2999999997")
