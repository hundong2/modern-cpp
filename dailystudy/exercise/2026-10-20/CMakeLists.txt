cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_20 LANGUAGES CXX)

# 오늘의 라이브러리 예제와 제출 코드를 C++23 표준으로 고정하고 구현체 확장은 끈다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 세 소스는 각각 독립 main 함수를 가지므로 별도 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 경고를 오류로 승격하고 libstdc++의 디버그 전제 검사를 켠다.
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
    target_compile_definitions(${target} PRIVATE _GLIBCXX_ASSERTIONS)
  endforeach()
endif()

enable_testing()

# build tree에 입력·기대 출력을 만들고 helper가 종료 코드와 stdout 전체를 비교한다.
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

# Modern C++ 학습 예제 두 개의 관찰 가능한 결과를 고정한다.
add_exact_output_case(daily_main_runs daily_main "" "1 0 2")
add_exact_output_case(daily_problem_runs daily_problem "" "1 0 2")

# CSES 2079 공식 예제의 유일한 센트로이드는 3번이다.
add_exact_output_case(finding_centroid_official icpc_problem
  "5\n1 2\n2 3\n3 4\n3 5\n"
  "3")

# 정점 하나를 제거하면 남는 연결 요소가 없으므로 자기 자신이 센트로이드다.
add_exact_output_case(finding_centroid_single icpc_problem
  "1\n"
  "1")

# 홀수 길이 사슬의 유일한 가운데 정점을 찾는다.
add_exact_output_case(finding_centroid_chain icpc_problem
  "7\n1 2\n2 3\n3 4\n4 5\n5 6\n6 7\n"
  "4")

# 중심 번호가 1이 아니어도 부모 방향 조각까지 비교해 2번을 찾는다.
add_exact_output_case(finding_centroid_star icpc_problem
  "6\n2 1\n2 3\n2 4\n2 5\n2 6\n"
  "2")

# 짝수 길이 사슬에는 2와 3이 모두 답이며, 이 구현의 부모-선행 순서는 먼저 2를 선택한다.
add_exact_output_case(finding_centroid_two_centroids icpc_problem
  "4\n1 2\n2 3\n3 4\n"
  "2")
