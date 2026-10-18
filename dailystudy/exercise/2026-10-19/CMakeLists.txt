cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_19 LANGUAGES CXX)

# std::ranges::contains를 포함한 C++23 표준을 요구하고 구현체 확장은 끈다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 각 소스는 독립 main 함수를 가지므로 실행 파일도 분리한다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 변환 손실·이름 가림·비표준 의존을 경고 단계에서 실패시키고 디버그 전제 검사를 켠다.
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

# build tree에 입력/기대 출력을 만들고 helper가 종료 코드와 stdout 전체를 비교한다.
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

# Modern C++ 예제의 관찰 가능한 정책 결과를 고정한다.
add_exact_output_case(daily_main_runs daily_main "" "1 0 0")
add_exact_output_case(daily_problem_runs daily_problem "" "1 0 1")

# CSES 1686 공식 예제: {1,2} SCC의 9개와 방 4의 7개를 모아 16이다.
add_exact_output_case(coin_collector_official icpc_problem
  "4 4\n4 5 2 7\n1 2\n2 1\n1 3\n2 4\n"
  "16")

# 방 하나는 시작과 끝을 그 방으로 선택하므로 그 방의 동전을 모두 얻는다.
add_exact_output_case(coin_collector_single icpc_problem
  "1 1\n1000000000\n1 1\n"
  "1000000000")

# 모든 정점이 하나의 SCC라면 동전을 정확히 한 번씩 모두 더한다.
add_exact_output_case(coin_collector_one_scc icpc_problem
  "5 6\n1 2 3 4 5\n1 2\n2 3\n3 1\n3 4\n4 5\n5 3\n"
  "15")

# 분기 DAG에서는 지역적으로 큰 다음 정점이 아니라 전체 경로 합이 큰 쪽을 선택해야 한다.
add_exact_output_case(coin_collector_branching_dag icpc_problem
  "6 6\n5 20 6 7 8 30\n1 2\n1 3\n3 4\n4 5\n5 6\n2 6\n"
  "56")

# 같은 SCC 쌍 사이 중복 간선을 제거하지 않아도 indegree를 같은 횟수 감소시키면 정답이 같다.
add_exact_output_case(coin_collector_parallel_edges icpc_problem
  "4 6\n3 4 5 6\n1 2\n2 1\n1 3\n2 3\n2 3\n3 4\n"
  "18")

# 32-bit signed 범위를 넘는 5*10^9 합계를 64-bit로 보존하는 경계를 검증한다.
add_exact_output_case(coin_collector_int64_boundary icpc_problem
  "5 4\n1000000000 1000000000 1000000000 1000000000 1000000000\n1 2\n2 3\n3 4\n4 5\n"
  "5000000000")
