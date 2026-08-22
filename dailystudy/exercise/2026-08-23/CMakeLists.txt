# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 C++만 사용한다는 사실을 선언한다.
project(daily_2026_08_23 LANGUAGES CXX)

# std::latch와 std::jthread를 사용하므로 C++20을 요구한다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 스레드 구현에 필요한 플랫폼별 컴파일·링크 설정을 가져온다.
find_package(Threads REQUIRED)

# 실무 예제, 직접 연습, 온라인 저지 제출 풀이를 각각 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 두 실무 예제만 C++ 표준 스레드 라이브러리를 사용한다.
target_link_libraries(daily_main PRIVATE Threads::Threads)
target_link_libraries(daily_problem PRIVATE Threads::Threads)

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

# BOJ 11657 공개 예제 1은 음수 간선이 있지만 음수 사이클은 없는 기본 최단 거리 사례다.
add_icpc_case(boj_11657_official_1
  "3 4\n1 2 4\n1 3 3\n2 3 -1\n3 1 -2\n"
  "4\n3")

# 1번 도시에서 도달 가능한 음수 사이클이 있으면 다른 거리 대신 -1 하나만 출력해야 한다.
add_icpc_case(boj_11657_reachable_negative_cycle
  "3 4\n1 2 4\n1 3 3\n2 3 -1\n3 1 -4\n"
  "-1")

# 시작점에서 닿지 못하는 음수 사이클은 정답을 망치지 않으며 두 도시는 도달 불가다.
add_icpc_case(boj_11657_unreachable_negative_cycle
  "3 2\n2 3 -1\n3 2 -1\n"
  "-1\n-1")

# 같은 두 도시 사이에 여러 간선이 있어도 완화 반복이 더 싼 비용을 선택한다.
add_icpc_case(boj_11657_parallel_edges
  "3 2\n1 2 4\n1 2 3\n"
  "3\n-1")

# 여러 음수 간선을 지나도 사이클이 아니면 유한한 최단 거리를 정확히 누적한다.
add_icpc_case(boj_11657_negative_path
  "4 4\n1 2 5\n2 3 -10\n3 4 2\n1 4 100\n"
  "5\n-5\n-3")

# 도시 하나의 음수 자기 루프는 시작점에서 즉시 도달 가능한 음수 사이클인 최소 경계다.
add_icpc_case(boj_11657_single_city_negative_loop
  "1 1\n1 1 -1\n"
  "-1")
