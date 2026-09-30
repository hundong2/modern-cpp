cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_01 LANGUAGES CXX)

# views::enumerate와 오늘 자료의 계약은 C++23을 기준으로 검증한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스는 각각 독립적인 main 함수를 가진 실행 프로그램이다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 변환, 이름 가림, 비표준 확장을 조기에 드러내도록 높은 경고 수준을 공통 적용한다.
if(MSVC)
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  endforeach()
endif()

enable_testing()

# 테스트별 입력과 기대 출력은 build 폴더에만 쓰고 stdout 전체를 helper에서 비교한다.
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

# enumerate 기반 소유 스냅숏 예제 두 개의 출력 순서와 1-based 번호를 고정한다.
add_exact_output_case(daily_main_runs daily_main ""
  "1:compile:ready\n2:test:blocked\n3:deploy:ready")
add_exact_output_case(daily_problem_runs daily_problem ""
  "2:cache\n3:docs")

# CSES 공식 예제로 LCA가 root인 조상-자손·서로 다른 가지 경로를 검증한다.
add_exact_output_case(counting_paths_official icpc_problem
  "5 3\n1 2\n1 3\n3 4\n3 5\n1 3\n2 5\n1 4\n"
  "3 1 3 1 1")

# 정점 하나, 자기 자신 경로, root의 parent 차분 예외 처리를 검증한다.
add_exact_output_case(counting_paths_single_vertex icpc_problem
  "1 3\n1 1\n1 1\n1 1\n"
  "3")

# 일자 트리에서 깊은 경로, 내부 경로, 길이 0 경로가 겹치는 누적을 검증한다.
add_exact_output_case(counting_paths_chain icpc_problem
  "5 3\n1 2\n2 3\n3 4\n4 5\n1 5\n2 4\n3 3\n"
  "1 2 3 2 1")

# 별 트리에서 서로 다른 두 LCA=root 경로와 한 단일 정점 경로를 검증한다.
add_exact_output_case(counting_paths_star icpc_problem
  "5 3\n1 2\n1 3\n1 4\n1 5\n2 3\n4 5\n2 2\n"
  "2 2 1 1 1")

# 균형 트리에서 형제, 서로 다른 서브트리, 조상-자손, root 자기 경로를 함께 검증한다.
add_exact_output_case(counting_paths_balanced icpc_problem
  "7 4\n1 2\n1 3\n2 4\n2 5\n3 6\n3 7\n4 5\n4 6\n2 7\n1 1\n"
  "3 3 2 2 1 1 1")
