cmake_minimum_required(VERSION 3.20)
project(daily_cpp_2026_07_19 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(add_lesson target source)
  add_executable(${target} ${source})
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endfunction()

add_lesson(ownership_demo main.cpp)
add_lesson(ownership_problem problem.cpp)

enable_testing()
add_test(NAME ownership_demo_runs COMMAND ownership_demo)
add_test(NAME ownership_problem_runs COMMAND ownership_problem)
