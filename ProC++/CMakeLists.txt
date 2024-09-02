cmake_minimum_required(VERSION 3.10)

# 프로젝트 이름 설정
project(ProC++)

# C++ 표준 설정
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 소스 파일 추가
file(GLOB_RECURSE SOURCES "*.cpp")

add_library(ExampleAllocateArrayLib ExampleAllocateArray.cpp)
add_library(ExampleConstLib ExampleConst.cpp)
add_library(ExampleRefLib ExampleReference.cpp)
add_library(ExampleCastLib ExampleCasting.cpp)
#add_library(EmployeeLib Employee.cpp)
# 실행 파일 생성
#add_executable(ProC++ ${SOURCES})
add_executable(ExampleArray2 ExampleArray2.cpp)
add_executable(ExampleOptional ExampleOptional.cpp)
add_executable(ExampleFor ExampleFor.cpp)
add_executable(ExampleInitList ExampleInitList.cpp)
add_executable(ExampleInit ExampleInit.cpp)
add_executable(ExampleMain ExampleMain.cpp)


target_link_libraries(ExampleMain PRIVATE ExampleAllocateArrayLib)
target_link_libraries(ExampleMain PRIVATE ExampleConstLib)
target_link_libraries(ExampleMain PRIVATE ExampleRefLib)
target_link_libraries(ExampleMain PRIVATE ExampleCastLib)
# g++ 컴파일러 사용 설정
set(CMAKE_CXX_COMPILER "g++")

# 컴파일러 옵션 추가
#target_compile_options(ProC++ PRIVATE -std=c++20 -fcoroutines)
target_compile_options(ExampleArray2 PRIVATE -std=c++20 -fcoroutines)
target_compile_options(ExampleInit PRIVATE -std=c++20 -fcoroutines)
target_compile_options(ExampleMain PRIVATE -std=c++20 -fcoroutines)
# 필요한 경우 추가 라이브러리 링크
# target_link_libraries(ProC++ <라이브러리 이름>)