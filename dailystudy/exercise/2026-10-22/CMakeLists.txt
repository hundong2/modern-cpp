cmake_minimum_required(VERSION 3.20)
project(daily_2026_10_22 LANGUAGES CXX)

# optional::transform은 C++23 기능이므로 세 실행 파일 모두 같은 언어 표준으로 고정한다.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 각 소스는 독립된 main 함수를 가지므로 별도 실행 파일로 만든다.
add_executable(daily_main main.cpp)
add_executable(daily_problem problem.cpp)
add_executable(icpc_problem icpc_problem.cpp)

# 이식성 문제를 일찍 찾도록 경고를 오류로 승격하고 libstdc++ 전제 검사도 켠다.
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

# 입력과 기대 출력을 build tree에 만들고 helper가 종료 코드와 stdout 전체를 비교한다.
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

# Modern C++ 소유 DTO 예제와 초보자 연습의 관찰 가능한 출력을 고정한다.
add_exact_output_case(daily_main_runs daily_main "" "billing 3\n0")
add_exact_output_case(daily_problem_runs daily_problem "" "42 -1")

# CSES 1735 공식 예제: 부분 합이 7, 11, 15로 변한다.
add_exact_output_case(range_updates_official icpc_problem
  "6 5\n2 3 1 1 5 3\n3 3 5\n1 2 4 2\n3 3 5\n2 2 4 5\n3 3 5\n"
  "7\n11\n15")

# 원소 하나에서도 build, add, assign, query가 같은 잎에 안전하게 합성되어야 한다.
add_exact_output_case(range_updates_single icpc_problem
  "1 6\n7\n3 1 1\n1 1 1 5\n3 1 1\n2 1 1 3\n1 1 1 4\n3 1 1\n"
  "7\n12\n7")

# add 뒤 assign은 과거 add를 덮고, 그 뒤 add는 새 대입값 위에 적용되어야 한다.
add_exact_output_case(range_updates_composition_order icpc_problem
  "4 5\n1 2 3 4\n1 1 4 5\n2 2 3 1\n1 3 4 2\n3 1 4\n3 2 4\n"
  "21\n15")

# 부분 구간 질의는 부모 tag를 자식에게 내려보낸 뒤 정확한 두 합을 결합해야 한다.
add_exact_output_case(range_updates_push_partial icpc_problem
  "5 6\n1 1 1 1 1\n2 1 5 10\n1 2 4 3\n3 1 1\n3 2 4\n2 3 3 2\n3 1 5\n"
  "10\n39\n48")
