cmake_minimum_required(VERSION 3.16)
project(BoostPracticalCurriculum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

set(BOOST_HEADER_STYLE_EXAMPLES
    01_algorithm_string
    02_lexical_cast_uuid
    03_multi_index
    04_signals2_observer
    05_asio_timer
    06_beast_http_message
    07_json_value
)

foreach(example IN LISTS BOOST_HEADER_STYLE_EXAMPLES)
    add_executable(${example} ${example}.cpp)
    target_include_directories(${example} PRIVATE ${Boost_INCLUDE_DIRS})

    if (MSVC)
        target_compile_options(${example} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${example} PRIVATE -Wall -Wextra -pedantic)
    endif()
endforeach()

# boost::program_options는 일반적으로 별도 라이브러리 링크가 필요합니다.
# 설치 환경에 따라 아래 블록을 켜서 사용하세요.
#
# find_package(Boost REQUIRED COMPONENTS program_options)
# add_executable(08_program_options_shape 08_program_options_shape.cpp)
# target_include_directories(08_program_options_shape PRIVATE ${Boost_INCLUDE_DIRS})
# target_link_libraries(08_program_options_shape PRIVATE Boost::program_options)

