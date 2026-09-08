cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_09 LANGUAGES CXX)

# 오늘의 소유권/범위 예제와 센트로이드 분할 풀이를 확장 없는 C++20 규칙으로 컴파일한다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 학습 중 위험한 암시 변환과 그림자 변수까지 빨리 발견하도록 지원 컴파일러의 높은 경고를 켠다.
if(MSVC)
  target_compile_options(daily_main PRIVATE /W4 /permissive-)
  target_compile_options(daily_problem PRIVATE /W4 /permissive-)
  target_compile_options(icpc_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(daily_main PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  target_compile_options(daily_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  target_compile_options(icpc_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

enable_testing()

add_test(NAME daily_main_runs COMMAND daily_main)
set_tests_properties(
  daily_main_runs
  PROPERTIES PASS_REGULAR_EXPRESSION "2 50 4")

add_test(NAME daily_problem_runs COMMAND daily_problem)
set_tests_properties(
  daily_problem_runs
  PROPERTIES PASS_REGULAR_EXPRESSION "3 90")

# 입력 파일을 만든 뒤 공통 드라이버로 종료 코드와 정규화한 전체 출력을 정확히 비교한다.
function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# 공식 예제: 길이 2인 무방향 단순 경로는 네 개다.
add_icpc_case(centroid_official
  "5 2\n1 2\n2 3\n3 4\n3 5\n"
  "4")

# 정점 하나에는 길이 1인 경로가 없다. 공식 k>=1, k<=n 범위 안의 최소 입력이다.
add_icpc_case(centroid_single_node
  "1 1\n"
  "0")

# k=1이면 모든 간선이 정확히 한 경로이므로 다섯 정점 일자 트리의 답은 네 개다.
add_icpc_case(centroid_chain_adjacent
  "5 1\n1 2\n2 3\n3 4\n4 5\n"
  "4")

# k=1인 별에서도 각 중심-잎 간선만 세어야 한다.
add_icpc_case(centroid_star_adjacent
  "6 1\n1 2\n1 3\n1 4\n1 5\n1 6\n"
  "5")

# 별의 서로 다른 잎 두 개는 거리가 2이고 C(5,2)=10이다. 자식 컴포넌트 간 쌍 세기를 검증한다.
add_icpc_case(centroid_star_leaf_pairs
  "6 2\n1 2\n1 3\n1 4\n1 5\n1 6\n"
  "10")

# 여섯 정점 일자 트리에서 간선 세 개짜리 구간의 시작점은 1,2,3 세 곳이다.
add_icpc_case(centroid_chain_distance_three
  "6 3\n1 2\n2 3\n3 4\n4 5\n5 6\n"
  "3")

# n개 정점 트리의 최대 거리는 n-1이므로 허용 입력 k=n의 답은 반드시 0이다.
add_icpc_case(centroid_distance_too_large
  "5 5\n1 2\n2 3\n3 4\n4 5\n"
  "0")
