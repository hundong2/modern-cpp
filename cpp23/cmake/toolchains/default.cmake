# Prefer Homebrew LLVM on macOS when available. This also avoids a partially
# installed Apple Command Line Tools toolchain that has no complete libc++.
if(APPLE AND NOT DEFINED CMAKE_CXX_COMPILER)
    set(_homebrew_llvm_candidates
        "/opt/homebrew/opt/llvm/bin/clang++"
        "/usr/local/opt/llvm/bin/clang++"
    )

    foreach(_compiler IN LISTS _homebrew_llvm_candidates)
        if(EXISTS "${_compiler}")
            set(CMAKE_CXX_COMPILER "${_compiler}" CACHE FILEPATH "C++ compiler")
            break()
        endif()
    endforeach()

    unset(_compiler)
    unset(_homebrew_llvm_candidates)
endif()
