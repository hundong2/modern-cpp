# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 C++만 사용한다는 사실을 선언한다.
project(daily_2026_08_24 LANGUAGES CXX)

# std::move_only_function은 C++23 표준 라이브러리 기능이다.
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

# Kattis 공개 예제는 같은 지점, 여러 방향 전환, 한 번의 전환을 함께 검증한다.
add_icpc_case(ocean_currents_official_sample
  "5 5\n04125\n03355\n64734\n72377\n02062\n3\n4 2 4 2\n4 5 1 4\n5 3 3 4\n"
  "0\n2\n1")

# 한 칸 격자에서는 출발지와 목적지가 같으므로 이동과 에너지가 모두 0이다.
add_icpc_case(ocean_currents_single_cell
  "1 1\n0\n1\n1 1 1 1\n"
  "0")

# 동쪽 해류를 따르면 비용 0, 반대로 서쪽으로 가면 비용 1임을 검사한다.
add_icpc_case(ocean_currents_zero_and_one_edges
  "1 2\n22\n2\n1 1 1 2\n1 2 1 1\n"
  "0\n1")

# 바로 남동쪽인 방향 3을 따르면 대각선 목적지까지 비용이 들지 않는다.
add_icpc_case(ocean_currents_diagonal
  "2 2\n33\n33\n1\n1 1 2 2\n"
  "0")

# 모든 해류가 북쪽이어도 격자 경계를 지키며 남동쪽 한 번에 비용 1로 도달한다.
add_icpc_case(ocean_currents_boundary
  "2 2\n00\n00\n1\n1 1 2 2\n"
  "1")
