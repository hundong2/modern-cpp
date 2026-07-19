cmake_minimum_required(VERSION 3.25)

project(cpp_env_example LANGUAGES CXX)

add_executable(cpp_env_example
  src/main.cpp
  src/calculator.cpp
)

target_include_directories(cpp_env_example PRIVATE include)

enable_testing()
add_test(NAME cpp_env_example_runs COMMAND cpp_env_example)

