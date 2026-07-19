cmake_minimum_required(VERSION 3.16)

project(BasicCppPractice LANGUAGES CXX)

set(BASIC_CPP_STANDARD 17 CACHE STRING "C++ standard used by the Basic practice targets")
set_property(CACHE BASIC_CPP_STANDARD PROPERTY STRINGS 17 20 23)

set(CMAKE_CXX_STANDARD ${BASIC_CPP_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /EHsc)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(basic_01_start 01-start/practice.cpp)
add_executable(basic_02_flow 02-flow/practice.cpp)
add_executable(basic_03_containers 03-containers/practice.cpp)
add_executable(basic_04_ownership 04-ownership/practice.cpp)

foreach(target
    basic_01_start
    basic_02_flow
    basic_03_containers
    basic_04_ownership
)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()
