# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 사용 언어를 선언한다.
project(daily_2026_08_20 LANGUAGES CXX)

# std::expected와 C++20 지정 초기화를 함께 쓰므로 C++23 표준 모드로 컴파일한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 학습 예제 둘과 온라인 저지 제출용 풀이를 서로 독립적인 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 암시 변환·부호 변환·누락 분기 같은 초보 실수를 일찍 발견하도록 높은 경고 수준을 적용한다.
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

# name은 테스트명, input은 줄 입력, expected는 양 끝 공백을 제외한 기대 출력이다.
function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# BOJ 공개 예제로 A-B가 전체 흐름을 3으로 제한하는지 검증한다.
add_icpc_case(boj_6086_official
  "5\nA B 3\nB C 3\nC D 5\nD Z 4\nB Z 6\n"
  "3")

# 같은 두 정점의 평행 파이프 용량을 더하고 B-Z 병목 5를 고르는지 검증한다.
add_icpc_case(boj_6086_parallel
  "3\nA B 4\nA B 3\nB Z 5\n"
  "5")

# 대소문자를 다른 정점으로 매핑하고 소문자 경로를 처리하는지 검증한다.
add_icpc_case(boj_6086_lowercase
  "2\nA a 5\na Z 4\n"
  "4")

# 두 갈래와 중간 간선을 가진 네트워크에서 전체 5를 보내는지 검증한다.
add_icpc_case(boj_6086_split_routes
  "5\nA B 3\nA C 2\nB C 1\nB Z 2\nC Z 3\n"
  "5")
