cmake_minimum_required(VERSION 3.16)
project(algorithm_guide LANGUAGES CXX)

add_executable(algorithm_cpp examples/cpp/main.cpp)
target_compile_features(algorithm_cpp PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(algorithm_cpp PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(algorithm_cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()
