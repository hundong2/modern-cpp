cmake_minimum_required(VERSION 3.16)
project(CppConSystemArchitectureCourse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(architecture_vs_design
    01_architecture_vs_design/architecture_vs_design.cpp)

add_executable(strong_types_and_contracts
    02_types_contracts_invariants/strong_types_and_contracts.cpp)

add_executable(object_lifecycle_raii
    03_lifecycle_memory/object_lifecycle_raii.cpp)

add_executable(adapter_boundary
    04_adapters_boundaries/adapter_boundary.cpp)

add_executable(thread_safe_queue
    05_concurrency_constraints/thread_safe_queue.cpp)

if (MSVC)
    target_compile_options(architecture_vs_design PRIVATE /W4)
    target_compile_options(strong_types_and_contracts PRIVATE /W4)
    target_compile_options(object_lifecycle_raii PRIVATE /W4)
    target_compile_options(adapter_boundary PRIVATE /W4)
    target_compile_options(thread_safe_queue PRIVATE /W4)
else()
    target_compile_options(architecture_vs_design PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(strong_types_and_contracts PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(object_lifecycle_raii PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(adapter_boundary PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(thread_safe_queue PRIVATE -Wall -Wextra -pedantic)
endif()

