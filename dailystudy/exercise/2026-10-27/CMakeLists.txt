cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_27 LANGUAGES CXX)

# stop_source/stop_callback은 C++20 기능이며 오늘 세 소스는 확장 없는 C++23 모드로 함께 검사한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 각 소스가 독립 main 함수를 가지므로 서로 다른 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 변환·shadowing·비표준 확장 경고를 오류로 승격해 학습 자료의 이식성 실수를 조기에 막는다.
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

# helper가 입력을 전달하고 종료 코드와 줄 끝을 정규화한 stdout 전체를 기대 파일과 비교한다.
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

# 첫 요청·중복 요청·late registration을 두 학습 프로그램에서 결정적으로 고정한다.
add_exact_output_case(daily_main_runs daily_main ""
  "first=1 second=0 cleanup=1 open=0\nlate=1")
add_exact_output_case(daily_problem_runs daily_problem ""
  "first=1 second=0 score=111")

# CSES 1160 공식 예제: 도달 거리 1, 2와 도달 불가를 모두 포함한다.
add_exact_output_case(planets_queries_official icpc_problem
  "5 3\n2 3 2 3 2\n1 2\n1 3\n1 4\n"
  "1\n2\n-1")

# 자기 루프에서 자기 자신까지는 순간이동 0회이며 같은 질의를 반복해도 답은 변하지 않는다.
add_exact_output_case(planets_queries_self_loop icpc_problem
  "1 3\n1\n1 1\n1 1\n1 1\n"
  "0\n0\n0")

# 세 개의 독립 성분을 만들어 성분 불일치, 사이클 wrap-around, 고립 자기 루프를 검사한다.
add_exact_output_case(planets_queries_components icpc_problem
  "6 7\n2 1 4 5 3 6\n1 2\n2 1\n1 3\n3 5\n5 4\n6 6\n6 1\n"
  "1\n1\n-1\n2\n2\n0\n-1")

# 긴 꼬리와 합류 가지에서 조상 방향만 도달 가능하고, 사이클 안에서는 양의 방향 거리만 센다.
add_exact_output_case(planets_queries_tail_branches icpc_problem
  "8 9\n2 3 4 3 4 5 6 6\n7 5\n7 4\n7 3\n8 5\n8 7\n6 8\n3 5\n3 4\n4 3\n"
  "2\n3\n4\n2\n-1\n-1\n-1\n1\n1")

# 목적지가 트리 정점일 때 depth 차만 같아도 부족하고 실제로 lift한 정점이 같아야 한다.
add_exact_output_case(planets_queries_tree_identity icpc_problem
  "9 7\n2 3 4 3 4 5 6 6 8\n9 6\n9 5\n9 7\n7 6\n8 6\n1 2\n3 3\n"
  "2\n3\n-1\n1\n1\n1\n0")
