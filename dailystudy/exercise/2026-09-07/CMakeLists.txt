cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_07 LANGUAGES CXX)

# std::apply는 C++17부터지만 최근 저장소의 공통 학습 기준과 일관되게 확장 없는 C++20 모드로 고정한다.
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
set_tests_properties(
  daily_main_runs
  PROPERTIES PASS_REGULAR_EXPRESSION "billing 4 1 canary")
add_test(NAME daily_problem_runs COMMAND daily_problem)
set_tests_properties(
  daily_problem_runs
  PROPERTIES PASS_REGULAR_EXPRESSION "search 2 3 0")

function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# 공식 예제: 꼭짓점 3개와 그 사이 공선점 1개인 네 경계점을 반시계 순서로 확인한다.
add_icpc_case(convex_hull_official
  "6\n2 1\n2 5\n3 3\n4 3\n4 4\n6 3\n"
  "4\n2 1\n6 3\n4 4\n2 5")
# 직사각형 네 변의 공선 경계점은 모두 답에 남고 내부 중앙점만 빠져야 한다.
add_icpc_case(convex_hull_boundary_collinear
  "9\n0 0\n1 0\n2 0\n2 1\n2 2\n1 2\n0 2\n0 1\n1 1\n"
  "8\n0 0\n1 0\n2 0\n2 1\n2 2\n1 2\n0 2\n0 1")
# 최소 비자명 입력인 삼각형은 세 점을 모두 정확히 한 번 출력한다.
add_icpc_case(convex_hull_triangle
  "3\n0 0\n2 0\n0 2\n"
  "3\n0 0\n2 0\n0 2")
# 음수 좌표와 내부점을 함께 넣어 사전순 정렬·외적 부호가 좌표 부호에 의존하지 않음을 검증한다.
add_icpc_case(convex_hull_negative_coordinates
  "5\n-3 -1\n0 -2\n3 -1\n0 3\n0 0\n"
  "4\n-3 -1\n0 -2\n3 -1\n0 3")
