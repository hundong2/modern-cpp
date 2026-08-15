# 이 날짜 폴더만 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# project는 빌드 단위의 이름과 C++ 언어 사용을 선언한다.
project(daily_2026_08_16 LANGUAGES CXX)

# std::filesystem과 오늘 예제에 필요한 C++20 문법을 모든 타깃에 적용한다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 컴파일러별 확장 대신 이식 가능한 표준 문법만 허용한다.
set(CMAKE_CXX_EXTENSIONS OFF)

# 각 소스 파일을 서로 독립적인 실행 파일로 컴파일한다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# CTest를 활성화하고 두 Modern C++ 학습 프로그램의 정상 종료를 확인한다.
enable_testing()
add_test(NAME daily_main_runs COMMAND daily_main)
add_test(NAME daily_problem_runs COMMAND daily_problem)

# name은 테스트명, input은 표준 입력, expected는 기대 표준 출력을 받는 함수 매개변수다.
function(add_icpc_case name input expected)
  # 테스트마다 입력 파일을 빌드 폴더에 생성해 테스트끼리 입력 상태를 공유하지 않는다.
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  # 생성기 식으로 실제 icpc_problem 경로를 얻어 공용 비교 스크립트에 전달한다.
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# BOJ 2150의 공식 예제 형태로 여러 SCC와 단일 정점 SCC의 정렬 출력을 함께 검증한다.
add_icpc_case(icpc_example "7 9\n1 4\n4 5\n5 1\n1 6\n6 7\n2 7\n7 3\n3 7\n7 2\n" "3\n1 4 5 -1\n2 3 7 -1\n6 -1")
# 모든 정점이 서로 왕복할 수 있을 때 하나의 SCC로 합쳐지는지 검증한다.
add_icpc_case(icpc_single_component "3 3\n1 2\n2 3\n3 1\n" "1\n1 2 3 -1")
# 간선이 없으면 각 정점이 자기 자신만 포함하는 SCC가 되는 경계를 검증한다.
add_icpc_case(icpc_isolated_vertices "4 0\n" "4\n1 -1\n2 -1\n3 -1\n4 -1")
