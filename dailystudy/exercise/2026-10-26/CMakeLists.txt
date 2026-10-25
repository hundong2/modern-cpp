cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_26 LANGUAGES CXX)

# optional::or_else는 C++23 기능이므로 세 실행 파일을 확장 없는 C++23으로 검사한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 각 소스가 독립 main 함수를 가지므로 실행 파일도 세 개로 분리한다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 경고를 오류로 승격해 변환·shadowing·이식성 실수를 학습 단계에서 막는다.
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

# 입력과 기대 출력을 build tree에 기록하고 helper가 종료 코드와 줄 끝을 정규화한 stdout 전체를 비교한다.
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

# Modern C++ 예제의 성공/대체 경로를 모두 고정한다.
add_exact_output_case(daily_main_runs daily_main ""
  "billing https://billing.internal\nsearch https://fallback.internal")
add_exact_output_case(daily_problem_runs daily_problem "" "admin guest")

# CSES 2074 공식 예제: 첫 합은 1+3+4, 뒤집은 뒤 두 번째 합은 1+3+5다.
add_exact_output_case(reversals_sums_official icpc_problem
  "8 3\n2 1 3 4 5 3 4 4\n2 2 4\n1 3 6\n2 2 4\n"
  "8\n9")

# 길이 1 구간의 reverse는 값과 합을 바꾸지 않는다.
add_exact_output_case(reversals_sums_single icpc_problem
  "1 4\n7\n2 1 1\n1 1 1\n2 1 1\n2 1 1\n"
  "7\n7\n7")

# 전체 뒤집기와 부분 뒤집기를 겹쳐 lazy flag의 push 순서를 검사한다.
add_exact_output_case(reversals_sums_nested icpc_problem
  "5 6\n1 2 3 4 5\n2 1 5\n1 1 5\n2 1 2\n1 2 4\n2 2 4\n2 1 5\n"
  "15\n9\n9\n15")

# 0과 1e9를 더해 32비트 범위를 넘는 합은 int64_t로 정확히 보존한다.
add_exact_output_case(reversals_sums_wide icpc_problem
  "5 3\n1000000000 0 1000000000 0 1000000000\n2 1 5\n1 2 5\n2 1 5\n"
  "3000000000\n3000000000")

# 같은 구간을 두 번 뒤집고 비대칭 단일 위치를 읽어 변화와 복원을 모두 검증한다.
add_exact_output_case(reversals_sums_double_reverse icpc_problem
  "6 6\n4 1 9 2 8 3\n2 2 5\n1 2 5\n2 2 2\n1 2 5\n2 2 2\n2 1 6\n"
  "20\n8\n1\n27")
