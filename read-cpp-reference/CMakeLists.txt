cmake_minimum_required(VERSION 3.16)
project(read_cpp_reference LANGUAGES CXX)

enable_testing()

function(add_lesson target source standard)
    add_executable(${target} ${source})
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD ${standard}
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )

    if(MSVC)
        # 주석과 출력 문자열은 UTF-8이다. /utf-8은 한국어 소스의 C4819 경고를 막는다.
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_lesson(lesson01_declarations examples/01_declarations.cpp 17)
add_lesson(lesson02_control_flow examples/02_control_flow.cpp 17)
add_lesson(lesson03_pointers_refs_const examples/03_pointers_refs_const.cpp 17)
add_lesson(lesson04_classes_raii examples/04_classes_raii.cpp 17)
add_lesson(lesson05_templates_callables examples/05_templates_callables.cpp 17)
add_lesson(lesson06_iterators_algorithms examples/06_iterators_algorithms.cpp 17)
add_lesson(lesson07_cpp17 examples/07_cpp17.cpp 17)
add_lesson(lesson08_atomic examples/08_atomic.cpp 17)
add_lesson(lesson09_cpp20 examples/09_cpp20.cpp 20)
add_lesson(lesson10_lifetime_move examples/10_lifetime_move.cpp 17)
