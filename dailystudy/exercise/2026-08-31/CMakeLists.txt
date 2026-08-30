cmake_minimum_required(VERSION 3.20)

project(daily_2026_08_31 LANGUAGES CXX)

# std::format과 designated initializer는 C++20 기능이므로 C++20 모드로 빌드한다.
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
add_test(NAME daily_problem_runs COMMAND daily_problem)

function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# 공개 예제를 재서술한 최소 트리: 갱신 전후 같은 간선의 최댓값이 바뀐다.
add_icpc_case(hld_sample "3\n1 2 1\n2 3 2\n3\n2 1 2\n1 1 3\n2 1 2\n" "1\n3")
# 여러 heavy/light 체인을 건너는 경로와 중간 간선 갱신을 검증한다.
add_icpc_case(hld_cross_chains "7\n1 2 4\n1 3 7\n2 4 5\n2 5 1\n3 6 6\n3 7 2\n4\n2 4 6\n1 2 3\n2 4 6\n2 5 4\n" "7\n6\n5")
# 두 정점이 같은 경우 빈 경로의 항등값 0을 반환하는 구현 경계를 확인한다.
add_icpc_case(hld_same_vertex "2\n1 2 9\n2\n2 1 1\n2 1 2\n" "0\n9")
# 단일 간선을 여러 번 갱신해 edge index에서 더 깊은 정점 위치로의 매핑을 확인한다.
add_icpc_case(hld_repeated_update "2\n1 2 5\n5\n2 1 2\n1 1 11\n2 2 1\n1 1 1\n2 1 2\n" "5\n11\n1")
