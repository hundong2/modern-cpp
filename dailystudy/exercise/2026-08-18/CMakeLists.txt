# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 사용 언어를 선언한다.
project(daily_2026_08_18 LANGUAGES CXX)

# 오늘 예제는 C++20 표준 모드로 컴파일하고 구현체 확장 문법은 사용하지 않는다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 각 소스 파일을 서로 독립적인 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 초보자가 타입 변환·누락된 분기 같은 실수를 조기에 찾도록 높은 경고 수준을 적용한다.
if(MSVC)
  target_compile_options(daily_main PRIVATE /W4 /permissive-)
  target_compile_options(daily_problem PRIVATE /W4 /permissive-)
  target_compile_options(icpc_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(daily_main PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
  target_compile_options(daily_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
  target_compile_options(icpc_problem PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

# CTest에 Modern C++ 예제와 ICPC 정답 검증을 등록한다.
enable_testing()
add_test(NAME daily_main_runs COMMAND daily_main)
add_test(NAME daily_problem_runs COMMAND daily_problem)

# name은 테스트명, input은 표준 입력, expected는 마지막 개행을 제외한 기대 출력이다.
function(add_icpc_case name input expected)
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# 가지가 갈라지는 트리에서 같은 하위 트리와 루트 LCA를 함께 검증한다.
add_icpc_case(boj_11438_branching
  "5\n1 2\n1 3\n2 4\n2 5\n3\n4 5\n4 3\n2 4\n"
  "2\n1\n2")

# 편향 트리에서 한 정점이 다른 정점의 조상인 경우를 검증한다.
add_icpc_case(boj_11438_chain
  "5\n1 2\n2 3\n3 4\n4 5\n3\n4 5\n2 5\n1 3\n"
  "4\n2\n1")

# 루트의 여러 자식과 자기 자신 질의로 경계 동작을 검증한다.
add_icpc_case(boj_11438_root_and_same
  "6\n1 2\n1 3\n1 4\n4 5\n4 6\n4\n2 3\n5 6\n4 4\n2 6\n"
  "1\n4\n4\n1")

# 온라인 저지 공개 예제로 더 큰 트리의 여러 가지 깊이 관계를 검증한다.
add_icpc_case(boj_11438_official_example
  "15\n1 2\n1 3\n2 4\n3 7\n6 2\n3 8\n4 9\n2 5\n5 11\n7 13\n10 4\n11 15\n12 5\n14 7\n6\n6 11\n10 9\n2 6\n7 6\n8 13\n8 15\n"
  "2\n4\n2\n1\n3\n1")
