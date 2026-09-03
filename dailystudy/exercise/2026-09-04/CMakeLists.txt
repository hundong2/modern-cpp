cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_04 LANGUAGES CXX)

# promise/future는 C++11부터이고 jthread는 C++20부터이므로, jthread를 위해 확장 없는 C++20 모드로 고정한다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 일부 플랫폼은 표준 thread 지원에 별도 링크 옵션이 필요하므로 CMake의 이식 가능한 Threads target을 사용한다.
find_package(Threads REQUIRED)
target_link_libraries(daily_main PRIVATE Threads::Threads)

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
  PROPERTIES PASS_REGULAR_EXPRESSION "orders-42 20 3 1 0")
add_test(NAME daily_problem_runs COMMAND daily_problem)
set_tests_properties(
  daily_problem_runs
  PROPERTIES PASS_REGULAR_EXPRESSION "deployment-ready deployment-ready 1 0")

function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# 공식 예제를 고정된 인접 입력 순서로 실행해 모든 8개 거리를 한 번씩 쓰는 결정적 경로를 확인한다.
add_icpc_case(mail_delivery_sample
  "6 8\n1 2\n1 3\n2 3\n2 4\n2 6\n3 5\n3 6\n4 5\n"
  "1 2 3 5 4 2 6 3 1")
# 삼각형은 가장 작은 비자명 오일러 회로이며 시작과 끝이 모두 1이어야 한다.
add_icpc_case(mail_delivery_triangle "3 3\n1 2\n2 3\n3 1\n" "1 2 3 1")
# 홀수 차수 정점이 두 개면 닫힌 회로가 존재하지 않는다.
add_icpc_case(mail_delivery_odd_degree "3 2\n1 2\n2 3\n" "IMPOSSIBLE")
# 모든 차수가 짝수여도 1과 분리된 간선 성분이 있으면 route 길이 검사가 실패해야 한다.
add_icpc_case(mail_delivery_disconnected
  "6 6\n1 2\n2 3\n3 1\n4 5\n5 6\n6 4\n"
  "IMPOSSIBLE")
# 우체국을 공유하는 두 작은 회로가 올바른 순서로 splice되는지 확인한다.
add_icpc_case(mail_delivery_two_cycles
  "5 6\n1 2\n2 3\n3 1\n1 4\n4 5\n5 1\n"
  "1 2 3 1 4 5 1")
