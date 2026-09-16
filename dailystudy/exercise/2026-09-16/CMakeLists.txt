cmake_minimum_required(VERSION 3.20)

project(daily_2026_09_16 LANGUAGES CXX)

# std::any 자체는 C++17부터 사용할 수 있지만, 저장소의 최신 학습 환경과 일관되게
# 세 실행 파일을 확장 없는 C++23 모드로 빌드한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 실무 예제 두 개와 온라인 저지 제출 풀이를 서로 독립된 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 암시 변환, 그림자 변수, 비표준 확장을 학습 단계에서 빨리 찾도록 높은 경고 수준을 켠다.
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
# 파일 전달은 여러 줄 출력과 CMake 목록 구분자인 세미콜론도 문자열 그대로 보존한다.
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

# main.cpp는 존재하는 두 타입, 누락 키, 타입 불일치의 네 조회 경계를 검증한다.
add_exact_output_case(daily_main_runs daily_main ""
  "trace=req-42\nretries=3\nmissing=true\nwrong-type=true")

# problem.cpp는 any 깊은 복사·이동 결과와 명시적 reset 뒤 원본의 빈 상태를 검증한다.
add_exact_output_case(daily_problem_runs daily_problem ""
  "copied=trace\nmoved=trace\nsource-reset=true")

# ICPC 풀이용 작은 래퍼는 모든 그래프 테스트를 같은 exact-output 비교 경계에 등록한다.
function(add_icpc_case name input expected)
  add_exact_output_case("${name}" icpc_problem "${input}" "${expected}")
endfunction()

# CSES 1724 공식 예제: 여덟 간선 walk의 최소 비용은 27이다.
add_icpc_case(graph_paths_official
  "3 4 8\n1 2 5\n2 3 4\n3 1 1\n3 2 2\n"
  "27")

# 정확히 두 간선이어야 하므로 더 싼 1->3 직접 간선을 고를 수 없고, 평행 간선 중 비용 3을 써야 한다.
add_icpc_case(graph_paths_parallel_exact_length
  "3 4 2\n1 2 10\n1 2 3\n2 3 4\n1 3 1\n"
  "7")

# 1에서 3으로 가는 간선은 있지만 정확히 두 간선인 walk는 존재하지 않는다.
add_icpc_case(graph_paths_unreachable_length
  "3 2 2\n1 2 5\n1 3 1\n"
  "-1")

# 큰 홀수 k는 로그 지수승과 64비트 누적을 함께 검증한다.
add_icpc_case(graph_paths_large_k_cycle
  "2 2 999999999\n1 2 7\n2 1 4\n"
  "5499999996")
