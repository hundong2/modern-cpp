# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 사용 언어를 선언한다.
project(daily_2026_08_19 LANGUAGES CXX)

# std::byteswap을 포함한 오늘 예제는 C++23 표준 모드로 컴파일한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스 파일은 각각 독립적으로 실행되고 온라인 저지 풀이도 별도 제출할 수 있다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 암시 변환과 누락된 분기 같은 초보 실수를 일찍 발견하도록 높은 경고 수준을 적용한다.
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

# BOJ 공개 예제로 긴 실패 함수 이동 뒤 16번째의 한 번 일치를 검증한다.
add_icpc_case(boj_1786_official
  "ABC ABCDAB ABCDABCDABDE\nABCDABD\n"
  "1\n16")

# 접두사와 접미사가 겹치는 패턴에서 일치 세 개를 모두 찾는지 검증한다.
add_icpc_case(boj_1786_overlapping
  "AAAAA\nAAA\n"
  "3\n1 2 3")

# getline이 공백을 보존하며 KMP가 공백까지 문자로 비교하는지 검증한다.
add_icpc_case(boj_1786_spaces
  "a a a\na \n"
  "2\n1 3")

# 일치가 없을 때 개수 0과 빈 위치 줄을 출력하는 경계를 검증한다.
add_icpc_case(boj_1786_no_match
  "modern cpp\nrust\n"
  "0\n")
