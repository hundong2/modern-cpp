cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_03 LANGUAGES CXX)

# shared_ptr 별칭 생성자는 C++11부터 가능하고, prvalue 직접 구성 규칙을 명확히 보기 위해 C++20으로 고정한다.
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
set_tests_properties(daily_main_runs PROPERTIES PASS_REGULAR_EXPRESSION "release-notes 7 2.*release-notes 1")
add_test(NAME daily_problem_runs COMMAND daily_problem)
set_tests_properties(daily_problem_runs PROPERTIES PASS_REGULAR_EXPRESSION "api-v2 30 2")

function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# 공식 예제는 implication 두 방향과 자유 변수까지 함께 검사한다.
add_icpc_case(giant_pizza_sample "3 5\n+ 1 + 2\n- 1 + 3\n+ 4 - 2\n" "+ + + + +")
# 같은 토핑을 반드시 넣고 반드시 빼야 한다는 두 절은 모순이다.
add_icpc_case(giant_pizza_impossible "2 1\n+ 1 + 1\n- 1 - 1\n" "IMPOSSIBLE")
# 단위 절을 두 번 쓴 형태로 첫 토핑 true, 둘째 토핑 false를 강제한다.
add_icpc_case(giant_pizza_forced "2 2\n+ 1 + 1\n- 2 - 2\n" "+ -")
# 한 토핑의 선택이 implication 연쇄를 따라 나머지 둘을 강제한다.
add_icpc_case(giant_pizza_chain "3 3\n- 1 + 2\n- 2 + 3\n+ 1 + 1\n" "+ + +")
