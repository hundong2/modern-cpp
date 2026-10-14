cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_15 LANGUAGES CXX)

# atomic_ref와 jthread는 C++20 기능이다. 오늘 자료는 저장소의 최신 학습 기준인 C++23으로 검증한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 C++ 파일은 각자 main 함수를 가지므로 독립 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 일부 플랫폼은 표준 스레드에 별도 링크 옵션이 필요하므로 이식 가능한 CMake target을 연결한다.
find_package(Threads REQUIRED)
target_link_libraries(daily_main PRIVATE Threads::Threads)
target_link_libraries(daily_problem PRIVATE Threads::Threads)

# 변환 손실, 이름 가림, 비표준 확장을 학습 단계에서 조기에 발견한다.
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

# 입력과 기대 출력은 build 폴더에만 만들고 helper가 stdout 전체를 비교한다.
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
  set_tests_properties("${name}" PROPERTIES TIMEOUT 20)
endfunction()

# 두 worker가 같은 legacy 숫자를 원자 갱신한 뒤 모든 view를 파괴하고 plain 값과 대조한다.
add_exact_output_case(daily_main_runs daily_main "" "200000 200000")

# 두 발급 worker의 지역 합과 atomic_ref가 남긴 다음 ID를 결정적으로 검증한다.
add_exact_output_case(daily_problem_runs daily_problem "" "36 9")

# 공식 예제는 같은 최저 비용 4인 서로 다른 두 경로를 모두 보존해야 한다.
add_exact_output_case(flight_routes_official icpc_problem
  "4 6 3\n1 2 1\n1 3 3\n2 3 2\n2 4 6\n3 2 8\n3 4 1\n"
  "4 4 7")

# 평행 간선은 가격이 같아도 서로 다른 경로이므로 중복 답을 제거하면 안 된다.
add_exact_output_case(flight_routes_parallel icpc_problem
  "2 3 3\n1 2 5\n1 2 5\n1 2 7\n"
  "5 5 7")

# 양의 self-loop를 여러 번 사용한 walk가 비용 오름차순으로 열거되는지 확인한다.
add_exact_output_case(flight_routes_cycle icpc_problem
  "2 2 4\n1 1 1\n1 2 2\n"
  "2 3 4 5")

# 직접 경로가 반복 loop 경로들 사이에 끼는 순서와 k개 조기 종료를 함께 검사한다.
add_exact_output_case(flight_routes_mixed icpc_problem
  "3 4 5\n1 2 1\n2 3 1\n1 3 5\n2 2 2\n"
  "2 4 5 6 8")

# 비용 합은 32비트를 넘을 수 있으므로 64비트 누적과 출력 계약을 경계값으로 검증한다.
add_exact_output_case(flight_routes_64bit icpc_problem
  "2 2 10\n1 1 1000000000\n1 2 1000000000\n"
  "1000000000 2000000000 3000000000 4000000000 5000000000 6000000000 7000000000 8000000000 9000000000 10000000000")
