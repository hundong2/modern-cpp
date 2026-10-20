cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_21 LANGUAGES CXX)

# 오늘의 수명 경계 예제와 제출 코드를 C++23 표준으로 고정하고 구현체 확장은 끈다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스는 각각 독립 main 함수를 가지므로 별도 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 경고를 오류로 승격하고 libstdc++의 런타임 전제 검사를 켠다.
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

# build tree에 입력·기대 출력을 만들고 helper가 종료 코드와 stdout 전체를 비교한다.
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

# Modern C++ 학습 예제는 생존 중 성공(1), 마지막 owner 파괴 뒤 안전한 취소(0)를 출력한다.
add_exact_output_case(daily_main_runs daily_main "" "1 0")
add_exact_output_case(daily_problem_runs daily_problem "" "1 0")

# 공식 예제는 최적 배정이 유일하며 비용 33을 만든다.
add_exact_output_case(task_assignment_official icpc_problem
  "4\n17 8 16 9\n7 15 12 19\n6 9 10 11\n14 7 13 10\n"
  "33\n1 4\n2 1\n3 3\n4 2")

# 한 직원·한 작업에서는 유일한 간선 하나가 곧 최적 배정이다.
add_exact_output_case(task_assignment_single icpc_problem
  "1\n7\n"
  "7\n1 1")

# 대각선 비용만 작게 만들어 복원된 직원-작업 쌍의 방향까지 검증한다.
add_exact_output_case(task_assignment_diagonal icpc_problem
  "3\n1 50 50\n50 2 50\n50 50 3\n"
  "6\n1 1\n2 2\n3 3")

# 순환 이동 배정이 유일한 최적해인 경우에도 모든 열을 정확히 한 번 사용해야 한다.
add_exact_output_case(task_assignment_cycle icpc_problem
  "3\n50 1 50\n50 50 2\n3 50 50\n"
  "6\n1 2\n2 3\n3 1")

# 공식 최대 단일 비용을 포함한 유일 최적해로 합계와 잠재치의 경계를 확인한다.
add_exact_output_case(task_assignment_max_cost icpc_problem
  "3\n998 1000 1000\n1000 999 1000\n1000 1000 1000\n"
  "2997\n1 1\n2 2\n3 3")
