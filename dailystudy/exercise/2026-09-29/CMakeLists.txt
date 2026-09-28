cmake_minimum_required(VERSION 3.20)
project(daily_2026_09_29 LANGUAGES CXX)

# std::string::resize_and_overwrite는 C++23 기능이다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스는 각각 main을 가진 독립 실행 프로그램이다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 변환·shadowing까지 초보자가 조기에 발견하도록 높은 경고 수준을 세 target에 공통 적용한다.
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

# 입력·기대 출력은 build 폴더에만 만들고 helper가 종료 코드와 stdout 전체를 비교한다.
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

# 학습 예제는 콜백이 상한보다 짧은 실제 길이를 돌려주는 경로를 고정한다.
add_exact_output_case(daily_main_runs daily_main ""
  "key=API-v2-West,dropped=2")
add_exact_output_case(daily_problem_runs daily_problem ""
  "digits=2026,dropped=6")

# 공식 예제와 최소 길이·동률 주기·마지막 시작점·긴 공통 접두사 경계를 검증한다.
add_exact_output_case(minimal_rotation_official icpc_problem
  "acab\n"
  "abac")
add_exact_output_case(minimal_rotation_single icpc_problem
  "a\n"
  "a")
add_exact_output_case(minimal_rotation_equal icpc_problem
  "aaaaaa\n"
  "aaaaaa")
add_exact_output_case(minimal_rotation_periodic icpc_problem
  "bcabca\n"
  "abcabc")
add_exact_output_case(minimal_rotation_last_start icpc_problem
  "aaba\n"
  "aaab")
add_exact_output_case(minimal_rotation_long_prefix icpc_problem
  "aaaaabaaaa\n"
  "aaaaaaaaab")

# 공식 최대 길이에서도 마지막 문자만 유일한 최소 문자인 경계를 실제 CTest에 넣는다.
# 입력·기대 문자열은 source가 아니라 무시되는 build 폴더에만 기록된다.
string(REPEAT "z" 999999 maximum_prefix)
add_exact_output_case(minimal_rotation_maximum icpc_problem
  "${maximum_prefix}a\n"
  "a${maximum_prefix}")
unset(maximum_prefix)
