# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 사용 언어를 선언한다.
project(daily_2026_08_21 LANGUAGES CXX)

# 참조 한정자와 오늘 사용한 표준 도구를 충분히 지원하는 C++20 모드로 컴파일한다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 실무 예제, 직접 연습, 온라인 저지 제출 풀이를 독립 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 암시 변환·부호 변환·누락 분기 같은 실수를 높은 경고 수준에서 발견한다.
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

# BOJ 공개 예제는 짧은 번호가 먼저인 충돌과 충돌 없는 공통 접두사를 함께 검증한다.
add_icpc_case(boj_5052_official
  "2\n3\n911\n97625999\n91125426\n5\n113\n12340\n123440\n12345\n98346\n"
  "NO\nYES")

# 긴 번호가 먼저 들어와도 나중의 짧은 번호가 기존 자식을 보고 충돌을 찾아야 한다.
add_icpc_case(boj_5052_reverse_prefix "1\n2\n12345\n123\n" "NO")

# 같은 번호 두 개는 서로 접두사이므로 NO다.
add_icpc_case(boj_5052_duplicate "1\n2\n42\n42\n" "NO")

# 모든 번호가 접두사 12를 공유해도 어느 번호도 다른 번호의 끝이 아니면 YES다.
add_icpc_case(boj_5052_shared_prefix "1\n4\n120\n121\n122\n1299\n" "YES")

# 한 자리 번호가 더 긴 번호보다 먼저 나오면 terminal 조기 검사가 충돌을 찾는다.
add_icpc_case(boj_5052_single_digit_prefix "1\n3\n7\n70\n8\n" "NO")
