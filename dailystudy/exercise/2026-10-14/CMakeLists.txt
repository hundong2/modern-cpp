cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_14 LANGUAGES CXX)

# std::to_underlying와 오늘 자료의 호출 계약은 C++23을 기준으로 검증한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 C++ 파일은 각각 독립적인 main 함수를 가지므로 실행 파일도 분리한다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 변환 손실, 이름 가림, 비표준 확장을 학습 단계에서 조기에 발견한다.
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

# 입력과 기대 출력은 build 폴더에만 만들고 helper가 stdout 전체를 비교한다.
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

# 실무 예제는 enum 값과 wire code의 순서·값을 정확히 고정한다.
add_exact_output_case(daily_main_runs daily_main ""
  "PKG-101:10\nPKG-102:20\nPKG-103:30")
add_exact_output_case(daily_problem_runs daily_problem ""
  "read=yes\ndeploy=yes\nwire=5")

# CSES 공식 예제: abaa의 서로 다른 부분 문자열은 8개다.
add_exact_output_case(distinct_substrings_official icpc_problem "abaa\n" "8")

# 가장 작은 입력과 모든 문자가 같은 경우는 서로 다른 길이만 답에 기여한다.
add_exact_output_case(distinct_substrings_single icpc_problem "a\n" "1")
add_exact_output_case(distinct_substrings_repeated icpc_problem "aaaa\n" "4")

# 모든 문자가 다르면 모든 구간이 서로 달라 n(n+1)/2개다.
add_exact_output_case(distinct_substrings_all_distinct icpc_problem "abcd\n" "10")

# 주기 문자열과 잘 알려진 중복 문자열로 clone·suffix link 갱신을 검증한다.
add_exact_output_case(distinct_substrings_periodic icpc_problem "ababa\n" "9")
add_exact_output_case(distinct_substrings_banana icpc_problem "banana\n" "15")

# a 뒤에 b가 길게 이어지는 입력은 clone을 많이 만들어 2n 안전 용량 경계를 검증한다.
string(REPEAT "b" 99999 clone_tail)
set(clone_heavy "a${clone_tail}")
add_exact_output_case(distinct_substrings_clone_heavy icpc_problem
  "${clone_heavy}\n" "199999")

# a^50000 b^50000의 서로 다른 부분 문자열 수는 m^2+2m이라 32비트 signed 범위를 넘는다.
string(REPEAT "a" 50000 wide_left)
string(REPEAT "b" 50000 wide_right)
set(wide_answer_input "${wide_left}${wide_right}")
add_exact_output_case(distinct_substrings_64bit_answer icpc_problem
  "${wide_answer_input}\n" "2500100000")
