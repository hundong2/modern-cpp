cmake_minimum_required(VERSION 3.20)

project(raii_resource_lifetime LANGUAGES CXX)

find_package(Threads REQUIRED)

add_executable(raii_example example.cpp)
set_target_properties(raii_example PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_link_libraries(raii_example PRIVATE Threads::Threads)

add_executable(raii_exercise exercise.cpp)
set_target_properties(raii_exercise PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

if(MSVC)
    target_compile_options(raii_example PRIVATE /W4 /permissive-)
    target_compile_options(raii_exercise PRIVATE /W4 /permissive-)
else()
    target_compile_options(raii_example PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(raii_exercise PRIVATE -Wall -Wextra -Wpedantic)
endif()
