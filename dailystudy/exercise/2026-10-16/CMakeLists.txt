cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_16 LANGUAGES CXX)

# 오늘 예제는 C++23 표준으로 컴파일하고 구현체 확장은 허용하지 않는다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스는 각각 main 함수를 가지므로 독립 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 경고를 오류로 승격해 학습 코드의 변환 손실, 이름 가림, 비표준 의존을 빌드에서 차단한다.
if(MSVC)
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE /W4 /permissive- /WX)
  endforeach()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  foreach(target daily_main daily_problem icpc_problem)
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wshadow
      -Werror
    )
    # libstdc++의 범위·선행조건 검사를 활성화해 잘못된 컨테이너 접근을 테스트에서 조기에 찾는다.
    target_compile_definitions(${target} PRIVATE _GLIBCXX_ASSERTIONS)
  endforeach()
endif()

enable_testing()

# 입력과 기대 출력은 무시되는 build 트리에 만들고 helper가 stdout 전체와 종료 상태를 검증한다.
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
  set_tests_properties("${name}" PROPERTIES TIMEOUT 25)
endfunction()

# Modern C++ 예제들의 관찰 가능한 결과를 정확히 고정한다.
add_exact_output_case(daily_main_runs daily_main "" "1 0 70 50")
add_exact_output_case(daily_problem_runs daily_problem "" "1 0 3")

# CSES 1139 공식 예제: 각 정점을 루트로 하는 서브트리의 서로 다른 색 수를 검증한다.
add_exact_output_case(distinct_colors_official icpc_problem
  "5\n2 3 2 2 1\n1 2\n1 3\n3 4\n3 5\n"
  "3 1 2 1 1")

# 최소 트리는 루트 자신만 포함하므로 서로 다른 색 수가 정확히 1이다.
add_exact_output_case(distinct_colors_single_node icpc_problem
  "1\n42\n"
  "1")

# 모든 색이 같은 긴 체인은 병합 순서와 무관하게 모든 서브트리 답이 1이어야 한다.
add_exact_output_case(distinct_colors_same_color_chain icpc_problem
  "5\n7 7 7 7 7\n1 2\n2 3\n3 4\n4 5\n"
  "1 1 1 1 1")

# 모든 색이 다른 별 트리는 루트만 n개 색을 보고 각 잎은 자기 색 하나만 본다.
add_exact_output_case(distinct_colors_distinct_star icpc_problem
  "5\n1 2 3 4 5\n1 2\n1 3\n1 4\n1 5\n"
  "5 1 1 1 1")

# 중복 색과 여러 깊이가 섞인 트리로 작은 쪽을 큰 쪽에 합치는 구현의 일반 병합을 검증한다.
# 문제 입력은 항상 연결 트리이므로 disconnected case는 유효 입력이 아니어서 만들지 않는다.
add_exact_output_case(distinct_colors_mixed_tree icpc_problem
  "8\n1 2 1 3 2 4 3 4\n1 2\n1 3\n2 4\n2 5\n3 6\n6 7\n6 8\n"
  "4 2 3 1 1 2 1 1")
