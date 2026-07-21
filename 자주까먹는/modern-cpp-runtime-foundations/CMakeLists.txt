cmake_minimum_required(VERSION 3.20)

project(modern_cpp_runtime_foundations LANGUAGES CXX)

find_package(Threads REQUIRED)

add_executable(runtime_foundations example.cpp)
set_target_properties(runtime_foundations PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_link_libraries(runtime_foundations PRIVATE Threads::Threads)

add_executable(runtime_exercise exercise.cpp)
set_target_properties(runtime_exercise PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
target_link_libraries(runtime_exercise PRIVATE Threads::Threads)

add_executable(coroutine_lifecycle coroutine_example.cpp)
set_target_properties(coroutine_lifecycle PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

if(MSVC)
    target_compile_options(runtime_foundations PRIVATE /W4 /permissive-)
    target_compile_options(runtime_exercise PRIVATE /W4 /permissive-)
    target_compile_options(coroutine_lifecycle PRIVATE /W4 /permissive-)
else()
    target_compile_options(runtime_foundations PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(runtime_exercise PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(coroutine_lifecycle PRIVATE -Wall -Wextra -Wpedantic)
endif()
