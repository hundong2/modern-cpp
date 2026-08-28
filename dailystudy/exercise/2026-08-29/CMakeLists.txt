# 이 날짜 폴더를 독립 구성할 수 있는 최소 CMake 버전이다.
cmake_minimum_required(VERSION 3.20)

project(daily_2026_08_29 LANGUAGES CXX)

# semaphore와 jthread는 C++20 표준 라이브러리 기능이다.
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

# BOJ 공개 예제: 반복 접두사가 있는 banana의 SA와 LCP를 함께 검증한다.
add_icpc_case(suffix_official_sample "banana\n" "6 4 2 1 5 3\nx 1 3 0 0 2")
# 길이 1은 sentinel 제거, 빈 이전 접미사 표시만 남기는 최소 경계다.
add_icpc_case(suffix_single "a\n" "1\nx")
# 모두 같은 문자는 짧은 접미사부터 정렬되고 LCP가 1씩 증가한다.
add_icpc_case(suffix_all_equal "aaaa\n" "4 3 2 1\nx 1 2 3")
# 두 주기가 반복될 때 쌍 순위 갱신과 Kasai의 감소 불변식을 확인한다.
add_icpc_case(suffix_periodic "abab\n" "3 1 4 2\nx 2 0 1")
# 이미 오름차순 문자는 원래 시작 위치 순서를 유지하고 모든 LCP가 0이다.
add_icpc_case(suffix_distinct "abcde\n" "1 2 3 4 5\nx 0 0 0 0")
