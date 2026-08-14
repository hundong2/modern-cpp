cmake_minimum_required(VERSION 3.20)

# project는 오늘 자료의 빌드 단위를 만들고 C++ 컴파일러를 사용한다고 선언한다.
project(daily_2026_08_15 LANGUAGES CXX)

# 세 실행 파일 모두 C++20 문법을 사용하며 컴파일러 확장 문법은 끈다.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 각 add_executable은 소스 파일 하나를 독립 실행 파일로 컴파일·링크한다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# CTest를 켜고 두 학습 프로그램의 정상 종료를 검증한다.
enable_testing()
add_test(NAME daily_main_runs COMMAND daily_main)
add_test(NAME daily_problem_runs COMMAND daily_problem)

# 함수 매개변수는 테스트 이름, 표준 입력 문자열, 기대 출력 문자열이다.
function(add_icpc_case name input expected)
  # 각 테스트 입력을 빌드 폴더에 만들므로 소스 트리는 오염되지 않는다.
  set(input_file "${CMAKE_CURRENT_BINARY_DIR}/${name}.txt")
  file(WRITE "${input_file}" "${input}")
  # 생성기 식은 빌드된 icpc_problem 경로를 CMake 검증 스크립트에 넘긴다.
  add_test(NAME "${name}" COMMAND ${CMAKE_COMMAND}
    -DEXECUTABLE=$<TARGET_FILE:icpc_problem>
    -DINPUT_FILE=${input_file}
    -DEXPECTED=${expected}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_icpc_test.cmake)
endfunction()

# 공식 예제는 두 번의 값 변경과 두 번의 구간 합을 검증한다.
add_icpc_case(icpc_example "5 2 2\n1\n2\n3\n4\n5\n1 3 6\n2 2 5\n1 5 2\n2 3 5\n" "17\n12")
# 한 원소 배열은 1 기반 인덱스의 최소 경계를 검증한다.
add_icpc_case(icpc_single "1 1 2\n7\n2 1 1\n1 1 10\n2 1 1\n" "7\n10")
# 음수로 바꾸고 부분 구간을 질의하여 차이 갱신을 검증한다.
add_icpc_case(icpc_negative "4 1 2\n5\n4\n3\n2\n2 2 4\n1 3 -7\n2 1 3\n" "9\n2")
