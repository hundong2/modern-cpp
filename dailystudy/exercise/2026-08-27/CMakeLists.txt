# 이 날짜 폴더를 독립적으로 구성할 수 있는 최소 CMake 버전을 지정한다.
cmake_minimum_required(VERSION 3.20)

# 프로젝트 이름과 C++만 사용한다는 사실을 선언한다.
project(daily_2026_08_27 LANGUAGES CXX)

# std::barrier와 std::jthread는 C++20 표준 라이브러리 기능이다.
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

# BOJ 공개 예제는 여러 직원이 같은 일을 원하는 일반 충돌을 검증한다.
add_icpc_case(matching_official_sample
  "5 5\n2 1 2\n1 1\n2 2 3\n3 3 4 5\n1 1\n"
  "4")

# 앞 직원이 두 번째 일로 옮겨야 뒤 직원도 배정되는 증강 경로를 검증한다.
add_icpc_case(matching_requires_reassignment "2 2\n2 1 2\n1 1\n" "2")

# 완전한 3x3 이분 그래프에는 크기 3의 완전 매칭이 있다.
add_icpc_case(matching_dense "3 3\n3 1 2 3\n3 1 2 3\n3 1 2 3\n" "3")

# 가능한 일이 없는 직원만 있어도 빈 인접 목록을 안전하게 처리해야 한다.
add_icpc_case(matching_no_edges "2 3\n0\n0\n" "0")

# 직원보다 일이 많을 때 답은 직원 수를 넘지 않는다.
add_icpc_case(matching_more_jobs "2 4\n2 1 4\n2 2 3\n" "2")
