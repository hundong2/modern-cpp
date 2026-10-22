cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_23 LANGUAGES CXX)

# std::popcount와 오늘의 값 타입 예제를 C++23 규칙으로 일관되게 검사한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스는 각각 main 함수를 가지므로 독립 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 경고를 오류로 올려 signed/unsigned 변환과 shadowing 같은 대회·실무 결함을 일찍 찾는다.
if(MSVC)
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE /W4 /permissive- /WX)
  endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wshadow
      -Werror
    )
    target_compile_definitions(${target} PRIVATE _GLIBCXX_ASSERTIONS)
  endforeach()
endif()

enable_testing()

# 입력과 기대 출력을 build tree에 기록하고 helper로 종료 코드와 stdout 전체를 비교한다.
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
  set_tests_properties("${name}" PROPERTIES TIMEOUT 25)
endfunction()

# Modern C++ 값 타입과 초보자용 비트 마스크의 관찰 가능한 출력을 고정한다.
add_exact_output_case(daily_main_runs daily_main "" "operator 2 1")
add_exact_output_case(daily_problem_runs daily_problem "" "3 1")

# CSES 2138 공식 예제다.
add_exact_output_case(reachable_nodes_official icpc_problem
  "5 6\n1 2\n1 3\n1 4\n2 3\n3 5\n4 5\n"
  "5 3 2 2 1")

# 긴 경로에서는 각 행이 뒤 suffix 전체를 정확히 포함해야 한다.
add_exact_output_case(reachable_nodes_chain icpc_problem
  "6 5\n1 2\n2 3\n3 4\n4 5\n5 6\n"
  "6 5 4 3 2 1")

# 합류 경로의 같은 정점은 두 번 더하지 않고 bit OR로 한 번만 센다.
add_exact_output_case(reachable_nodes_diamond icpc_problem
  "5 5\n1 2\n1 3\n2 4\n3 4\n4 5\n"
  "5 3 3 2 1")

# 서로 끊긴 DAG component들은 도달 집합을 섞지 않는다.
add_exact_output_case(reachable_nodes_disconnected icpc_problem
  "6 4\n1 3\n2 3\n3 4\n5 6\n"
  "3 3 2 1 2 1")

# 중복 간선이 들어와도 indegree 감소 횟수와 OR가 대응해 정점 수는 중복 집계하지 않는다.
add_exact_output_case(reachable_nodes_duplicate_edge icpc_problem
  "3 3\n1 2\n1 2\n2 3\n"
  "3 2 1")
