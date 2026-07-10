cmake_minimum_required(VERSION 3.16)
project(FrequentModernCppPatterns LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(EXAMPLES
    01_range_for_and_references
    02_parameter_passing
    03_vector_memory_reserve
    04_string_view_span
    05_move_semantics
    06_smart_pointers_raii
    07_algorithms_lambdas
    08_templates_policy
    09_perfect_forwarding_factory
    10_concepts_type_traits
    11_containers_lookup
    12_status_result_no_exception
    13_pointer_shared_ptr_memory
    14_preprocessor_macros
)

foreach(example IN LISTS EXAMPLES)
    add_executable(${example} ${example}.cpp)

    if (MSVC)
        target_compile_options(${example} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${example} PRIVATE -Wall -Wextra -pedantic)
    endif()
endforeach()
