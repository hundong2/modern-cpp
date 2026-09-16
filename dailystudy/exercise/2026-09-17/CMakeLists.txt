cmake_minimum_required(VERSION 3.20)
project(daily_2026_09_17 LANGUAGES CXX)

# std::expected의 C++23 monadic API를 포함하므로 확장 없는 C++23으로 빌드한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 독립적인 실무 예제, 연습 예제, 온라인 저지 제출 프로그램이다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

if(MSVC)
  target_compile_options(daily_main PRIVATE /W4 /permissive-)
  target_compile_options(daily_problem PRIVATE /W4 /permissive-)
  target_compile_options(icpc_problem PRIVATE /W4 /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  endforeach()
endif()

enable_testing()

# build 폴더에 입력/정답을 기록하고 종료 코드와 표준 출력 전체를 비교한다.
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
  "active=8080\nerror=invalid\nafter-invalid=8080\nzero=ok\nsuffix=ok\nupper=ok\nhigh=ok\nhuge=ok\nplus=ok\nspace=ok")
add_exact_output_case(daily_problem_runs daily_problem ""
  "quota=12,label=worker\nrejected=negative")

# CSES 1751 공식 예제와 서로 다른 그래프 모양의 회귀 테스트다.
add_exact_output_case(planets_cycles_official icpc_problem
  "5\n2 4 3 1 4\n" "3 3 1 3 4")
add_exact_output_case(planets_cycles_single_self_loop icpc_problem
  "1\n1\n" "1")
add_exact_output_case(planets_cycles_tail_and_cycle icpc_problem
  "6\n2 3 4 5 3 6\n" "5 4 3 3 3 1")
add_exact_output_case(planets_cycles_two_cycles icpc_problem
  "7\n2 1 4 5 3 3 6\n" "2 2 3 3 3 4 5")
