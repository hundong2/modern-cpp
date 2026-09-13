cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_14 LANGUAGES CXX)

# std::ospanstream은 C++23에서 도입됐으므로 세 실행 파일 모두 확장 없는 C++23 모드로 빌드한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 암시 변환, 그림자 변수, 비표준 확장을 학습 단계에서 일찍 찾도록 높은 경고를 적용한다.
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

# 입력과 기대 출력을 build 디렉터리 파일로 만든 뒤 공통 드라이버가 종료 코드와 전체 출력을 비교한다.
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

# 실무 예제는 호출자가 제공한 버퍼에 접근 감사 레코드 한 줄을 직렬화한다.
add_exact_output_case(daily_main_runs daily_main "" "gateway|17|kim|ALLOW")

# 연습 예제는 같은 비소유 버퍼 경계를 메트릭 한 줄 직렬화로 재현한다.
add_exact_output_case(daily_problem_runs daily_problem "" "latency_ms=37")

function(add_icpc_case name input expected)
  add_exact_output_case("${name}" icpc_problem "${input}" "${expected}")
endfunction()

# CSES 공식 예제: 1->2->4로 한 개, 1->3->4로 두 개를 보내 총비용 750이다.
add_icpc_case(parcel_official
  "4 5 3\n1 2 5 100\n1 3 10 50\n1 4 7 500\n2 4 8 350\n3 4 2 100\n"
  "750")

# 두 평행 간선의 싼 용량 2를 먼저 쓰고 남은 3개를 비싼 간선으로 보내 2*3+3*5=21이다.
add_icpc_case(parcel_direct_capacity
  "2 2 5\n1 2 2 3\n1 2 3 5\n"
  "21")

# 출발지에서 목적지로 가는 총 용량이 k보다 작으면 완전한 운송이 불가능하다.
add_icpc_case(parcel_impossible
  "3 1 2\n1 3 1 7\n"
  "-1")

# 첫 최단 경로를 취소하는 음수 비용 역잔여 간선을 두 번째 증대 경로가 사용해야 최적비용 8이다.
add_icpc_case(parcel_reverse_residual_reroute
  "6 8 2\n1 2 1 1\n1 3 1 1\n2 4 1 1\n2 5 1 2\n3 4 1 2\n3 5 1 100\n4 6 1 1\n5 6 1 1\n"
  "8")
