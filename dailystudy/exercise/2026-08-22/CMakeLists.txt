# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 사용 언어를 선언한다.
project(daily_2026_08_22 LANGUAGES CXX)

# unordered_map::contains와 노드 핸들을 함께 쓰기 위해 C++20 모드로 컴파일한다.
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

# BOJ 11404 공개 예제로 병렬 간선, 여러 중간 도시, 도달 불가 0 출력을 함께 검증한다.
add_icpc_case(boj_11404_official
  "5\n14\n1 2 2\n1 3 3\n1 4 1\n1 5 10\n2 4 2\n3 4 1\n3 5 1\n4 5 3\n3 5 10\n3 1 8\n1 4 2\n5 1 7\n3 4 2\n5 2 4\n"
  "0 2 3 1 4\n12 0 15 2 5\n8 5 0 1 1\n10 7 13 0 3\n7 4 10 6 0")

# 같은 방향 버스가 여러 개면 가장 싼 직접 간선만 초기값으로 남겨야 한다.
add_icpc_case(boj_11404_parallel_edges
  "3\n4\n1 2 9\n1 2 4\n2 3 5\n1 3 20\n"
  "0 4 9\n0 0 5\n0 0 0")

# 중간 도시를 거치는 경로가 직접 간선보다 싸면 점화식이 값을 낮춰야 한다.
add_icpc_case(boj_11404_intermediate
  "4\n4\n1 2 3\n2 3 4\n3 4 5\n1 4 20\n"
  "0 3 7 12\n0 0 4 9\n0 0 0 5\n0 0 0 0")

# 도시가 하나뿐이고 버스가 없어도 자기 자신 거리는 0이다.
add_icpc_case(boj_11404_single_city "1\n0\n" "0")
