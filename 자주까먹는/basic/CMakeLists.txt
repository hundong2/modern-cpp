cmake_minimum_required(VERSION 3.20)

project(historical_example LANGUAGES CXX)

file(GLOB example_sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(source IN LISTS example_sources)
    get_filename_component(target "${source}" NAME_WE)

    add_executable("${target}" "${source}")
    target_compile_features("${target}" PRIVATE cxx_std_23)
    set_target_properties("${target}" PROPERTIES CXX_EXTENSIONS NO)

    if(MSVC)
        target_compile_options("${target}" PRIVATE /W4 /permissive-)
    else()
        target_compile_options("${target}" PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
