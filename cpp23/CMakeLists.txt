cmake_minimum_required(VERSION 3.25)

project(
    cpp23_study
    VERSION 0.1.0
    DESCRIPTION "C++23 study project"
    LANGUAGES CXX
)

add_executable(cpp23_app
    src/main.cpp
)

target_compile_features(cpp23_app PRIVATE cxx_std_23)
set_target_properties(cpp23_app PROPERTIES
    CXX_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(cpp23_app PRIVATE /W4 /permissive-)
else()
    target_compile_options(cpp23_app PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
    )
endif()

include(CTest)
if(BUILD_TESTING)
    add_test(NAME cpp23_app_runs COMMAND cpp23_app)
endif()
