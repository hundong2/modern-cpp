# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 C++만 사용한다는 사실을 선언한다.
project(daily_2026_08_26 LANGUAGES CXX)

# std::mdspan은 C++23 표준 라이브러리 기능이다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 실무 예제, 직접 연습, 온라인 저지 제출 풀이를 각각 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 암시 변환·부호 변환·누락 분기 같은 실수를 높은 경고 수준에서 찾는다.
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

# name은 테스트 이름, input은 줄 입력, expected는 양 끝 공백을 제외한 기대 출력이다.
function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# BOJ 공개 예제는 일반 높이와 모든 높이가 같은 두 테스트 케이스를 함께 검증한다.
add_icpc_case(histogram_official_sample
  "7 2 1 4 5 1 3 3\n4 1000 1000 1000 1000\n0\n"
  "8\n4000")

# 막대 하나의 최대 직사각형은 그 막대 자체다.
add_icpc_case(histogram_single_bar "1 7\n0\n" "7")

# 엄격히 증가하는 입력은 끝 센티널이 모든 오른쪽 경계를 차례로 닫는지 검사한다.
add_icpc_case(histogram_increasing "5 1 2 3 4 5\n0\n" "9")

# 엄격히 감소하는 입력은 매 위치에서 pop하며 start를 왼쪽으로 전달하는지 검사한다.
add_icpc_case(histogram_decreasing "5 5 4 3 2 1\n0\n" "9")

# 높이 0만 있으면 양의 넓이가 없고 빈 단조 스택 처리가 안전해야 한다.
add_icpc_case(histogram_all_zero "5 0 0 0 0 0\n0\n" "0")

# 32비트 int를 넘는 넓이를 long long으로 계산하는지 검증한다.
add_icpc_case(histogram_requires_int64
  "3 1000000000 1000000000 1000000000\n0\n"
  "3000000000")
