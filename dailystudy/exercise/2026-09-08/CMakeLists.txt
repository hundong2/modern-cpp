cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_08 LANGUAGES CXX)

# std::once_flag와 std::call_once를 포함해 확장 없는 C++20 규칙으로 학습한다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# call_once의 플랫폼별 동기화 구현에 필요한 컴파일·링크 설정을 가져온다.
find_package(Threads REQUIRED)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

target_link_libraries(daily_main PRIVATE Threads::Threads)
target_link_libraries(daily_problem PRIVATE Threads::Threads)

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
set_tests_properties(
  daily_main_runs
  PROPERTIES PASS_REGULAR_EXPRESSION "config.internal 42 1 1")
add_test(NAME daily_problem_runs COMMAND daily_problem)
set_tests_properties(
  daily_problem_runs
  PROPERTIES PASS_REGULAR_EXPRESSION "1 73 1")

function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# 공식 예제: 두 중간 도시를 방문하는 순서 두 가지를 모두 센다.
add_icpc_case(hamiltonian_official
  "4 6\n1 2\n1 3\n2 3\n3 2\n2 4\n3 4\n"
  "2")

# 도시가 둘뿐이면 시작에서 도착으로 가는 한 간선이 곧 유일한 Hamiltonian 경로다.
add_icpc_case(hamiltonian_minimum
  "2 1\n1 2\n"
  "1")

# 모든 도시를 방문하기 전에 막히면 부분 경로가 있어도 답은 0이어야 한다.
add_icpc_case(hamiltonian_unreachable
  "4 4\n1 2\n2 4\n1 3\n3 1\n"
  "0")

# 공식 설명은 중복 연결의 식별을 따로 명시하지 않는다. 이 테스트는 입력의 각 edge occurrence를
# 별도 항공편 선택으로 합산한다는 이 구현의 명시적 multigraph 계약을 고정한다.
add_icpc_case(hamiltonian_parallel_edges
  "2 2\n1 2\n1 2\n"
  "2")

# 완전 방향 그래프에서 시작·도착을 고정하면 중간 두 도시의 2! 순서가 답이다.
add_icpc_case(hamiltonian_complete_four
  "4 12\n1 2\n1 3\n1 4\n2 1\n2 3\n2 4\n3 1\n3 2\n3 4\n4 1\n4 2\n4 3\n"
  "2")
