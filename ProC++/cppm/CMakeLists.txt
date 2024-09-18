cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 설정
project(HelloProject)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Clang++를 컴파일러로 설정
set(CMAKE_CXX_COMPILER clang++)

# 공통 컴파일 옵션 설정
set(COMMON_COMPILE_OPTIONS -std=c++20)
set(CMAKE_EXE_LINKER_FLAGS -lstdc++)
# 모듈 인터페이스 파일을 컴파일하여 모듈 파일 생성
add_custom_command(
    COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_SOURCE_DIR}/Hello.cppm ${COMMON_COMPILE_OPTIONS} --precompile
    OUTPUT ${CMAKE_BINARY_DIR}/Hello.pcm
    COMMENT "Precompiling Hello.cppm to Hello.pcm"
)

# 모듈 파일을 라이브러리로 추가
add_custom_target(HelloModule DEPENDS ${CMAKE_BINARY_DIR}/Hello.pcm)

# 실행 파일 생성
add_executable(UseApp Use.cpp)
# 실행 파일이 모듈 파일에 의존하도록 설정
add_dependencies(UseApp HelloModule)
# 모듈 파일을 포함하도록 컴파일 옵션 추가
target_compile_options(UseApp PRIVATE ${COMMON_COMPILE_OPTIONS} -fprebuilt-module-path=.)


