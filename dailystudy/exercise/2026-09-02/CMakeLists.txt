cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_02 LANGUAGES CXX)

# 템플릿 추론·람다·RAII 예제는 C++20에서 충분하며 온라인 저지 호환성도 유지한다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

if(MSVC)
  target_compile_options(daily_main PRIVATE /W4 /permissive-)
  target_compile_options(daily_problem PRIVATE /W4 /permissive-)
  target_compile_options(icpc_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(daily_main PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
  target_compile_options(daily_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
  target_compile_options(icpc_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

enable_testing()
add_test(NAME daily_main_runs COMMAND daily_main)
set_tests_properties(daily_main_runs PROPERTIES PASS_REGULAR_EXPRESSION "7 3 1.*7 3 0")
add_test(NAME daily_problem_runs COMMAND daily_problem)
set_tests_properties(daily_problem_runs PROPERTIES PASS_REGULAR_EXPRESSION "100 20 0")

function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# CSES 공개 예제는 두 단절점 4, 5와 역방향 간선 low 갱신을 함께 확인한다.
add_icpc_case(necessary_cities_sample "5 5\n1 2\n1 4\n2 4\n3 5\n4 5\n" "2\n4 5")
# 일자 그래프에서는 양 끝을 뺀 모든 정점이 단절점이어야 한다.
add_icpc_case(necessary_cities_chain "5 4\n1 2\n2 3\n3 4\n4 5\n" "3\n2 3 4")
# DFS 루트는 low 비교가 아니라 서로 다른 DFS 자식이 둘 이상일 때만 단절점이라는 예다.
add_icpc_case(necessary_cities_root "4 3\n1 2\n1 3\n1 4\n" "1\n1")
# 사이클 뒤 꼬리는 cycle 내부의 low가 조상으로 내려가고 연결 경계 정점 하나만 남아야 한다.
add_icpc_case(necessary_cities_cycle_tail "5 5\n1 2\n2 3\n3 1\n3 4\n4 5\n" "2\n3 4")
