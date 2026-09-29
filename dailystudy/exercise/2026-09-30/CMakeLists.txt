cmake_minimum_required(VERSION 3.20)
project(daily_2026_09_30 LANGUAGES CXX)

# 세 학습 소스 모두 C++23 문법/표준 라이브러리 계약을 같은 조건에서 확인한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스는 각각 main 함수를 가진 독립 실행 프로그램이다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 변환과 이름 가림까지 조기에 드러나도록 세 target에 높은 경고 수준을 공통 적용한다.
if(MSVC)
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  endforeach()
endif()

enable_testing()

# 입력·기대 출력은 build 폴더에만 만들고 helper가 종료 코드와 stdout 전체를 비교한다.
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

# main은 연속 부서별 인원·급여 합, problem은 연속 상태별 처음/끝/표본 수를 고정 출력한다.
add_exact_output_case(daily_main_runs daily_main ""
  "engineering:3:22000\nsales:2:10700\nsupport:3:15000")
add_exact_output_case(daily_problem_runs daily_problem ""
  "1:0-1:2\n0:2-4:3\n1:5-6:2")

# 공식 예제로 기본 갱신 뒤 범위 빈도가 바뀌는지 검증한다.
add_exact_output_case(salary_queries_official icpc_problem
  "5 3\n3 7 2 2 5\n? 2 3\n! 3 6\n? 2 3\n"
  "3\n2")

# 같은 급여 여러 명을 한 압축 좌표의 빈도로 합치고, 갱신 뒤 정확히 한 명만 이동하는지 검증한다.
add_exact_output_case(salary_queries_duplicates icpc_problem
  "4 5\n10 10 10 20\n? 10 10\n! 4 10\n? 10 10\n! 2 20\n? 10 10\n"
  "3\n4\n3")

# 최솟값·최댓값과 압축 목록에 없는 질의 경계에서 lower/upper 삽입 위치를 검증한다.
add_exact_output_case(salary_queries_boundaries icpc_problem
  "3 5\n1 1000000000 500000000\n? 1 1000000000\n? 2 499999999\n? 1000000000 1000000000\n! 1 1000000000\n? 1000000000 1000000000\n"
  "3\n0\n1\n2")

# 같은 값으로의 무효 갱신과 같은 새 값으로의 반복 갱신도 빈도 불변식을 깨지 않는지 검증한다.
add_exact_output_case(salary_queries_repeat_updates icpc_problem
  "2 6\n5 8\n! 1 5\n? 5 5\n! 1 8\n? 5 5\n! 1 8\n? 8 8\n"
  "1\n0\n2")
