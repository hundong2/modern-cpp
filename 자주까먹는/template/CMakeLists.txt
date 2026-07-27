cmake_minimum_required(VERSION 3.24)

project(
    modern_cpp23_template
    VERSION 1.0.0
    DESCRIPTION "Layered and practical Modern C++23 learning template"
    LANGUAGES CXX
)

option(MODERN_CPP_BUILD_EXERCISES "Build the hands-on exercise executable" ON)
option(MODERN_CPP_ENABLE_BOOST "Build the optional Boost.Container example" OFF)
option(MODERN_CPP_WARNINGS_AS_ERRORS "Treat this project's compiler warnings as errors" OFF)

add_library(modern_cpp_project_options INTERFACE)
target_compile_features(modern_cpp_project_options INTERFACE cxx_std_23)

if(MSVC)
    target_compile_options(modern_cpp_project_options INTERFACE /W4 /permissive-)
    if(MODERN_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(modern_cpp_project_options INTERFACE /WX)
    endif()
else()
    target_compile_options(
        modern_cpp_project_options
        INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
    )
    if(MODERN_CPP_WARNINGS_AS_ERRORS)
        target_compile_options(modern_cpp_project_options INTERFACE -Werror)
    endif()
endif()

add_library(
    modern_cpp_core
    STATIC
        src/domain/task.cpp
        src/application/event_sink.cpp
        src/application/task_service.cpp
        src/infrastructure/in_memory_task_repository.cpp
)

target_include_directories(
    modern_cpp_core
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

target_link_libraries(modern_cpp_core PUBLIC modern_cpp_project_options)

add_executable(modern_cpp_demo src/main.cpp)
target_link_libraries(modern_cpp_demo PRIVATE modern_cpp_core)

if(MODERN_CPP_BUILD_EXERCISES)
    add_executable(modern_cpp_exercise src/exercise.cpp)
    target_link_libraries(modern_cpp_exercise PRIVATE modern_cpp_core)
endif()

if(MODERN_CPP_ENABLE_BOOST)
    find_package(Boost 1.82 CONFIG REQUIRED COMPONENTS headers)

    add_executable(modern_cpp_boost_demo src/boost_example.cpp)
    target_link_libraries(
        modern_cpp_boost_demo
        PRIVATE
            modern_cpp_project_options
            Boost::headers
    )
endif()
