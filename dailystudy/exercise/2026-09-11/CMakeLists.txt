cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_11 LANGUAGES CXX)

# std::ranges::to는 C++23 표준 기능이므로 세 실행 파일 모두 확장 없는 C++23 모드로 컴파일한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 암시 변환·그림자 변수·비표준 확장을 학습 단계에서 빨리 발견하도록 높은 경고를 켠다.
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

# 입력·기대 출력을 build 디렉터리의 파일로 만든 뒤 공통 드라이버가 종료 코드와 전체 출력을 비교한다.
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

# 소유 snapshot은 저장소 변경 뒤에도 기존 이름 두 개를 유지하고, 새 snapshot만 한 행을 본다.
add_exact_output_case(daily_main_runs daily_main "" "2 Ada Linus 1 Linus")

# 연습 예제도 materialize 전후의 lazy/owning 경계를 정확한 한 줄 출력으로 고정한다.
add_exact_output_case(daily_problem_runs daily_problem "" "2 101 103 1 103")

function(add_icpc_case name input expected)
  add_exact_output_case("${name}" icpc_problem "${input}" "${expected}")
endfunction()

# CSES 공식 예제: 복사 직후 두 버전은 같은 합을 보지만 version 2 갱신 뒤 서로 갈라진다.
add_icpc_case(persistent_official
  "5 6\n2 3 1 2 5\n3 1\n2 1 1 5\n2 2 1 5\n1 2 2 5\n2 1 1 5\n2 2 1 5\n"
  "13\n13\n13\n15")

# n=1에서 복사·갱신을 연쇄해 leaf path copying과 버전 독립성을 검증한다.
add_icpc_case(persistent_single_element
  "1 9\n7\n2 1 1 1\n3 1\n1 2 1 100\n2 1 1 1\n2 2 1 1\n3 2\n1 3 1 1000000000\n2 2 1 1\n2 3 1 1\n"
  "7\n7\n100\n100\n1000000000")

# 한 뿌리에서 여러 번 copy한 뒤 각 버전을 다른 위치에서 갱신해 DAG 공유와 분기를 확인한다.
add_icpc_case(persistent_branching_versions
  "4 10\n1 2 3 4\n3 1\n1 1 1 10\n3 1\n1 2 4 40\n1 3 2 20\n2 1 1 4\n2 2 1 4\n2 3 1 4\n2 1 1 1\n2 2 1 1\n"
  "19\n46\n37\n10\n1")

# int 범위를 넘는 구간 합과 부분 구간을 넣어 long long 합 저장 계약을 검증한다.
add_icpc_case(persistent_large_sum
  "5 7\n1000000000 1000000000 1000000000 1000000000 1000000000\n2 1 1 5\n1 1 3 1\n2 1 1 5\n3 1\n1 2 5 2\n2 1 4 5\n2 2 1 5\n"
  "5000000000\n4000000001\n2000000000\n3000000003")
