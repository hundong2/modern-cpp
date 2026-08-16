# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 C++ 언어 사용을 선언한다.
project(daily_2026_08_17 LANGUAGES CXX)

# std::atomic<std::shared_ptr<T>> 특수화는 C++20 기능이다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 컴파일러별 확장 문법 대신 이식 가능한 표준 C++만 허용한다.
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 C++ 파일을 각각 독립 실행 파일로 컴파일한다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 초보자가 실수를 조기에 찾도록 세 실행 파일에 경고 옵션을 적용한다.
if(MSVC)
  target_compile_options(daily_main PRIVATE /W4 /permissive-)
  target_compile_options(daily_problem PRIVATE /W4 /permissive-)
  target_compile_options(icpc_problem PRIVATE /W4 /permissive-)
else()
  target_compile_options(daily_main PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(daily_problem PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(icpc_problem PRIVATE -Wall -Wextra -Wpedantic)
endif()

# CTest를 켜고 두 Modern C++ 예제가 정상 종료하는지 확인한다.
enable_testing()
add_test(NAME daily_main_runs COMMAND daily_main)
add_test(NAME daily_problem_runs COMMAND daily_problem)

# name은 테스트 이름, input은 표준 입력, expected는 기대 표준 출력이다.
function(add_icpc_case name input expected)
  # 테스트마다 입력 파일을 빌드 폴더에 만들어 테스트 상태를 서로 분리한다.
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  # 생성기 식으로 실제 실행 파일 경로를 얻어 공용 출력 비교 스크립트에 전달한다.
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# 작은 설명 예제에서 대입, 반전, 질의가 차례로 합성되는지 검증한다.
add_icpc_case(uva_11402_small_example
  "1\n1\n1\n00101\n7\nS 0 4\nF 1 3\nS 0 4\nI 0 2\nS 0 4\nE 3 4\nS 0 4\n"
  "Case 1:\nQ1: 2\nQ2: 4\nQ3: 3\nQ4: 1")

# 같은 구간의 반전을 두 번 합성하면 원래 상태가 되는지 검증한다.
add_icpc_case(uva_11402_double_invert
  "1\n1\n1\n0101\n5\nI 0 3\nS 0 3\nI 0 3\nS 0 3\nS 1 2\n"
  "Case 1:\nQ1: 2\nQ2: 2\nQ3: 1")

# 여러 블록 반복 입력과 부분 구간 대입이 경계 인덱스를 정확히 처리하는지 검증한다.
add_icpc_case(uva_11402_repeated_blocks
  "1\n2\n2\n01\n1\n111\n4\nS 0 6\nE 1 5\nS 0 6\nS 6 6\n"
  "Case 1:\nQ1: 5\nQ2: 1\nQ3: 1")
