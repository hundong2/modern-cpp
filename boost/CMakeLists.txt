cmake_minimum_required(VERSION 3.16)
project(BoostPracticalCurriculum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS program_options)

# 이 폴더의 각 .cpp 파일을 같은 이름의 독립 실행 파일로 만듭니다.
# CONFIGURE_DEPENDS 덕분에 .cpp 파일을 추가하거나 제거하면 CMake가 목록을 갱신합니다.
file(GLOB BOOST_EXAMPLE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(source IN LISTS BOOST_EXAMPLE_SOURCES)
    get_filename_component(example "${source}" NAME_WE)

    add_executable(${example} "${source}")
    target_include_directories(${example} PRIVATE ${Boost_INCLUDE_DIRS})

    if(example STREQUAL "08_program_options_shape")
        target_link_libraries(${example} PRIVATE Boost::program_options)
    endif()

    if (MSVC)
        target_compile_options(${example} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${example} PRIVATE -Wall -Wextra -pedantic)
    endif()
endforeach()
