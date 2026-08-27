# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 C++만 사용한다는 사실을 선언한다.
project(daily_2026_08_28 LANGUAGES CXX)

# std::osyncstream과 std::jthread는 C++20 표준 라이브러리 기능이다.
set(CMAKE_CXX_STANDARD 20)
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

# BOJ 공개 예제는 정사각형 네 꼭짓점의 최소 거리 제곱을 검증한다.
add_icpc_case(closest_official_sample
  "4\n0 0\n10 10\n0 10\n10 0\n"
  "100")

# 같은 좌표가 둘 이상이면 가능한 최소값 0을 즉시 얻어야 한다.
add_icpc_case(closest_duplicate "3\n1 1\n5 5\n1 1\n" "0")

# 좌우 부분의 내부 최솟값보다 중앙 띠를 가로지르는 두 점이 더 가까운 사례다.
add_icpc_case(closest_cross_strip
  "6\n-10 0\n-5 10\n-1 1\n0 0\n5 10\n10 0\n"
  "2")

# 좌표 절댓값 경계의 두 점은 64비트 거리 계산을 확인한다.
add_icpc_case(closest_coordinate_boundary
  "2\n-10000 -10000\n10000 10000\n"
  "800000000")

# y좌표가 모두 같아도 띠 안에서 인접 후보만 비교해 최소 간격 1을 찾는다.
add_icpc_case(closest_same_y
  "5\n100 7\n3 7\n8 7\n4 7\n20 7\n"
  "1")
