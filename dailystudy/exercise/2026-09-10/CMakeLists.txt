cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_10 LANGUAGES CXX)

# std::out_ptr가 C++23 표준 기능이므로 세 실행 파일을 확장 없는 C++23 모드로 컴파일한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 암시 변환, 그림자 변수와 일반 실수를 학습 단계에서 빨리 찾도록 높은 경고를 켠다.
if(MSVC)
  target_compile_options(daily_main PRIVATE /W4 /permissive-)
  target_compile_options(daily_problem PRIVATE /W4 /permissive-)
  target_compile_options(icpc_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(daily_main PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  target_compile_options(daily_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  target_compile_options(icpc_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

enable_testing()

# 입력 파일을 만든 뒤 공통 드라이버로 종료 코드와 정규화한 전체 출력을 정확히 비교한다.
function(add_exact_output_case name target input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:${target}>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

add_exact_output_case(daily_main_runs daily_main "" "7 0")
add_exact_output_case(daily_problem_runs daily_problem "" "42 0")

function(add_icpc_case name input expected)
  add_exact_output_case("${name}" icpc_problem "${input}" "${expected}")
endfunction()

# 공식 예제: 가장 긴 홀수 팰린드롬은 bab이다.
add_icpc_case(manacher_official "aybabtu\n" "bab")

# 공식 최소 길이에서 홀수 반지름 1과 substr 경계를 확인한다.
add_icpc_case(manacher_single "a\n" "a")

# 짝수 반지름이 홀수 답보다 길어지는 기본 사례다.
add_icpc_case(manacher_even "cbbd\n" "bb")

# 전체 문자열이 짝수 팰린드롬이면 양 끝까지 정확히 확장해야 한다.
add_icpc_case(manacher_full_even "abba\n" "abba")

# 모든 문자가 같을 때 right 경계 재사용과 선형 상각 동작을 검증한다.
add_icpc_case(manacher_repeated "aaaaa\n" "aaaaa")

# 길이 3인 답이 둘일 때 구현은 먼저 발견한 bab을 유지한다. 문제는 어느 답도 허용한다.
add_icpc_case(manacher_tie "babad\n" "bab")

# 긴 짝수 답과 앞뒤 잡음을 함께 둬 even mirror 식의 +1 경계를 검증한다.
add_icpc_case(manacher_long_even "forgeeksskeegfor\n" "geeksskeeg")
