cmake_minimum_required(VERSION 3.20) # target별 표준 속성과 CTest를 안정적으로 사용할 최소 CMake 버전이다.

project(concurrency_deep_dive LANGUAGES C CXX) # C ABI client는 C, 구현과 예제는 C++ compiler로 빌드한다.

find_package(Threads REQUIRED) # 플랫폼의 표준 thread link 옵션/라이브러리를 Threads::Threads target으로 찾는다.

function(enable_strict_warnings target_name) # target 이름 하나를 받아 compiler별 엄격 경고를 추가하는 함수다.
    if(MSVC) # Microsoft Visual C++ compiler 계열인지 구성 시점에 확인한다.
        target_compile_options(${target_name} PRIVATE /W4 $<$<COMPILE_LANGUAGE:CXX>:/permissive->) # 높은 경고는 C/C++에, 표준 준수 옵션은 C++ source에 적용한다.
    else() # GCC, Clang 등 slash option을 사용하지 않는 compiler 경로다.
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic) # 일반·추가·표준 준수 경고를 활성화한다.
    endif() # compiler별 분기를 닫는다.
endfunction() # strict warning helper 함수 정의를 끝낸다.

add_executable(deep_dive_cpp17 example.cpp) # C++17 mutex/guard/thread 설명 예제 실행 target을 만든다.
set_target_properties(deep_dive_cpp17 PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO) # ISO C++17을 강제한다.
target_link_libraries(deep_dive_cpp17 PRIVATE Threads::Threads) # 플랫폼 thread runtime을 private 의존성으로 연결한다.
enable_strict_warnings(deep_dive_cpp17) # 이 설명 예제에 엄격 경고를 적용한다.

add_executable(deep_dive_cxx20 cxx20_example.cpp) # semaphore/jthread/atomic wait를 쓰는 C++20 실행 target을 만든다.
set_target_properties(deep_dive_cxx20 PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO) # ISO C++20을 강제한다.
target_link_libraries(deep_dive_cxx20 PRIVATE Threads::Threads) # C++20 worker가 사용할 platform thread runtime을 연결한다.
enable_strict_warnings(deep_dive_cxx20) # C++20 설명 예제에도 엄격 경고를 적용한다.

add_executable(deep_dive_exercise exercise.cpp) # 학습자가 수정할 C++17 실습 target을 만든다.
set_target_properties(deep_dive_exercise PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO) # 실습의 최소 표준을 C++17로 고정한다.
target_link_libraries(deep_dive_exercise PRIVATE Threads::Threads) # 실습의 std::thread에 필요한 runtime을 연결한다.
enable_strict_warnings(deep_dive_exercise) # 실습 코드의 잠재 문제를 compiler 경고로 드러낸다.

add_executable(deep_dive_solution solution.cpp) # 실습 완료 뒤 비교할 C++17 정답 target을 만든다.
set_target_properties(deep_dive_solution PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO) # 정답도 같은 C++17 조건으로 비교한다.
target_link_libraries(deep_dive_solution PRIVATE Threads::Threads) # 정답의 std::thread runtime을 연결한다.
enable_strict_warnings(deep_dive_solution) # 정답 코드에도 동일한 경고 기준을 적용한다.

add_library(deep_dive_ffi STATIC ffi_api.cpp) # C ABI를 제공하지만 내부는 C++인 정적 library target을 만든다.
set_target_properties(deep_dive_ffi PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO) # FFI 구현은 C++17로 고정한다.
target_include_directories(deep_dive_ffi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}) # C caller가 공개 header를 include할 경로를 전파한다.
target_link_libraries(deep_dive_ffi PRIVATE Threads::Threads) # opaque 객체 내부 mutex 구현에 필요한 runtime을 연결한다.
enable_strict_warnings(deep_dive_ffi) # ABI 구현에 엄격 경고를 적용한다.

add_executable(deep_dive_c_client c_client.c) # header를 실제 C compiler로 처리하는 client 실행 target을 만든다.
target_link_libraries(deep_dive_c_client PRIVATE deep_dive_ffi) # C client를 C++로 구현된 ABI library와 연결한다.
set_target_properties(deep_dive_c_client PROPERTIES LINKER_LANGUAGE CXX) # C++ runtime이 포함되도록 최종 link driver를 C++로 선택한다.
enable_strict_warnings(deep_dive_c_client) # C source에도 compiler의 엄격 경고를 적용한다.

include(CTest) # BUILD_TESTING option과 add_test 기반 실행 검증을 활성화한다.
if(BUILD_TESTING) # 사용자가 test 생성을 끄지 않았을 때만 실행 test를 등록한다.
    add_test(NAME deep_dive_cpp17_runs COMMAND deep_dive_cpp17) # C++17 불변식 exit code가 0인지 검증한다.
    add_test(NAME deep_dive_cxx20_runs COMMAND deep_dive_cxx20) # C++20 semaphore/publish 불변식을 검증한다.
    add_test(NAME deep_dive_exercise_runs COMMAND deep_dive_exercise) # 기본 실습 상태도 안전하게 실행되는지 검증한다.
    add_test(NAME deep_dive_solution_runs COMMAND deep_dive_solution) # 정답의 최종 불변식을 검증한다.
    add_test(NAME deep_dive_c_client_runs COMMAND deep_dive_c_client) # C에서 C++ ABI를 호출해 42를 얻는지 검증한다.
endif() # test 등록 조건을 닫는다.
