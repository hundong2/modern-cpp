cmake_minimum_required(VERSION 3.20)

project(daily_2026_08_30 LANGUAGES CXX)

# condition_variable, jthread와 코드의 값 범주 규칙은 C++20으로 학습한다.
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
add_test(NAME daily_problem_runs COMMAND daily_problem)

function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# CSES 공개 예제: 포함 패턴, 부재 패턴, 시작 위치 패턴을 함께 검증한다.
add_icpc_case(aho_official_sample "aybabtu\n3\nbab\nabc\nayba\n" "YES\nNO\nYES")
# 실패 링크를 여러 단계 따라가며 접미 패턴 he/she/hers/his가 모두 표시되는지 확인한다.
add_icpc_case(aho_failure_suffixes "ahishers\n4\nhe\nshe\nhers\nhis\n" "YES\nYES\nYES\nYES")
# 같은 문자의 중첩 패턴과 text보다 긴 부재 패턴을 검증한다.
add_icpc_case(aho_repeated "aaaa\n4\na\naa\naaa\naaaaa\n" "YES\nYES\nYES\nNO")
# 패턴 중복은 같은 terminal 상태를 공유해도 입력 행마다 같은 답을 내야 한다.
add_icpc_case(aho_duplicates "xyz\n3\nxyz\nxyz\nyz\n" "YES\nYES\nYES")
# 전혀 일치하지 않는 최소 검색 경계를 검증한다.
add_icpc_case(aho_none "a\n2\nb\nzz\n" "NO\nNO")
