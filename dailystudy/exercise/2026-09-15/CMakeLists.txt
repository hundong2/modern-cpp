cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_15 LANGUAGES CXX)

# 오늘의 실무 예제는 C++23 기능을 사용하므로 세 실행 파일을 확장 없는 C++23 모드로 빌드한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 학습 예제 두 개와 온라인 저지에 제출 가능한 풀이를 서로 독립된 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 컴파일러별 높은 경고 수준으로 암시 변환, 그림자 변수, 비표준 확장을 학습 단계에서 찾는다.
if(MSVC)
  target_compile_options(daily_main PRIVATE /W4 /permissive-)
  target_compile_options(daily_problem PRIVATE /W4 /permissive-)
  target_compile_options(icpc_problem PRIVATE /W4 /permissive-)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  target_compile_options(daily_main PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  target_compile_options(daily_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  target_compile_options(icpc_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

enable_testing()

# 입력과 기대 출력을 build 디렉터리의 파일로 만든 뒤 공통 드라이버가 종료 코드와 전체 출력을 비교한다.
# 파일 전달 방식은 여러 줄 출력과 CMake 목록 구분자인 세미콜론도 문자열 그대로 보존한다.
function(add_exact_output_case name target input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.input.txt")
  set(expected_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.expected.txt")
  file(WRITE "${input_file}" "${input}")
  file(WRITE "${expected_file}" "${expected}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    "-DEXECUTABLE=$<TARGET_FILE:${target}>"
    "-DINPUT_FILE=${input_file}"
    "-DEXPECTED_FILE=${expected_file}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake")
  set_tests_properties("${name}" PROPERTIES TIMEOUT 15)
endfunction()

# main.cpp는 const 차용값과 rvalue 소비 뒤의 새 소유값이 같은 논리 데이터를 담는지 두 줄로 확인한다.
add_exact_output_case(daily_main_runs daily_main ""
  "audit=search-api:2\npublished=search-api:2")

# problem.cpp는 복사한 값과 이동한 값이 각각 기대한 문자열을 소유하는지 확인한다.
add_exact_output_case(daily_problem_runs daily_problem ""
  "copied=trace\nmoved=trace")

# ICPC 풀이용 작은 래퍼는 모든 트리 테스트를 같은 exact-output 비교 경계에 등록한다.
function(add_icpc_case name input expected)
  add_exact_output_case("${name}" icpc_problem "${input}" "${expected}")
endfunction()

# CSES 1133 공식 예제: 각 정점에서 다른 모든 정점까지의 거리 합을 순서대로 검증한다.
add_icpc_case(tree_distances_official
  "5\n1 2\n1 3\n3 4\n3 5\n"
  "6 9 5 8 8")

# 정점 하나뿐인 트리는 다른 정점까지 이동하지 않으므로 거리 합이 0이다.
add_icpc_case(tree_distances_single_vertex
  "1\n"
  "0")

# 네 정점 사슬은 양 끝의 합이 6, 가운데 두 정점의 합이 4여야 한다.
add_icpc_case(tree_distances_chain
  "4\n1 2\n2 3\n3 4\n"
  "6 4 4 6")

# 1번이 중심인 다섯 정점 별에서는 중심의 합이 4이고 각 잎의 합이 7이다.
add_icpc_case(tree_distances_star
  "5\n1 2\n1 3\n1 4\n1 5\n"
  "4 7 7 7 7")
