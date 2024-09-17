cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 설정
project(HelloProject)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Clang++를 컴파일러로 설정
set(CMAKE_CXX_COMPILER clang++)

# 소스 파일 추가
set(SOURCES
    Hello.cppm
    Use.cpp
)

# 모듈 인터페이스 파일을 컴파일하여 모듈 파일 생성
add_custom_command(
    OUTPUT Hello.pcm
    COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 --precompile ${CMAKE_SOURCE_DIR}/Hello.cppm -o Hello.pcm
    DEPENDS ${CMAKE_SOURCE_DIR}/Hello.cppm
)

# 모듈 파일을 라이브러리로 추가
add_library(HelloLib INTERFACE)

# 모듈 파일을 포함하도록 컴파일 옵션 추가
target_compile_options(HelloLib INTERFACE -fmodules-ts -fprebuilt-module-path=${CMAKE_BINARY_DIR})

# 실행 파일 생성
add_executable(UseApp Use.cpp)

# 라이브러리 링크
target_link_libraries(UseApp PRIVATE HelloLib)