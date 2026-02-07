# Copilot Instructions for modern-cpp

This is a C++ learning repository focused on modern C++ features (C++17, C++20) and Boost library examples.

## Build and Run

### Quick compile and run (most examples)
```bash
./exec.sh <filename.cpp>
```
This compiles with `-std=c++20 -fcoroutines` and runs the output.

### CMake builds

**ProC++ directory:**
```bash
cd ProC++/build
cmake ..
make

# Run specific executables
./ExampleMain
./ExampleArray2
./ExampleOptional
```

**Boost examples:**
```bash
cd Boost-Cpp-Application-Development-Cookbook-Second-Edition/SETUP
cmake -DCMAKE_BUILD_TYPE=debug .
make
```

### CI Build
The GitHub Actions workflow builds the ProC++ project:
```bash
cmake -S ProC++ -B build
cmake --build build
```

## Repository Structure

### Main Learning Modules

- **ProC++/**: Examples from "Professional C++, 5th Edition" book
  - Covers fundamentals: pointers, references, const, casting, strings, arrays
  - Uses C++20 standard with coroutines enabled
  - Main executable: `ExampleMain` (links multiple example libraries)

- **CPP20/**: Organized C++20 feature examples by topic
  - ALGORITHM: `erase_if`, `construct_at`, `shift_left/right`, safe comparisons
  - CONCEPT: Concepts and requires clauses
  - COROUTINE: Coroutine examples
  - RANGE: Range library and views
  - LAMBDA: Lambda expression features
  - CONSTEXPR, SPAN, UTILITY: Other C++20 features

- **example/**: Standalone C++20 feature examples
  - Lambda, functor, perfect forwarding
  - Concept, coroutine, range examples
  - Each has a corresponding `.md` documentation file

- **CPPTHREAD/**: Multithreading examples

- **Boost-Cpp-Application-Development-Cookbook-Second-Edition/**: Boost library examples

## Key Conventions

### Compiler Requirements
- **C++20 standard required**: All modern code uses `-std=c++20`
- **Coroutine support**: Add `-fcoroutines` flag (required for coroutine examples)
- **GCC preferred**: Repository uses g++ (installed via Homebrew on macOS)

### File Organization
- Subdirectories use uppercase names for topic categories (e.g., `ALGORITHM`, `CONCEPT`)
- Examples include both `.cpp` files and corresponding documentation (`.md` files or comments in README.md)
- Output binary is typically named `c.out`
- Build artifacts go in `build/` subdirectories

### CMake Structure
- ProC++ uses modular CMakeLists.txt with separate libraries per example domain:
  - `ExampleAllocateArrayLib`, `ExampleConstLib`, `ExampleRefLib`, etc.
  - These link into `ExampleMain` executable
- Boost examples use subdirectory structure with `add_subdirectory()`

### String and Type Conventions
- Use string literals with `s` suffix for `std::string`: `"Hello"s`
- Use `sv` suffix for `std::string_view`: `"My string_view"sv`
- Prefer `std::string_view` for read-only string parameters
- Use `auto` for type inference where appropriate
- Type aliases use `using` keyword: `using Integer = int;`

### Memory Management
- Modern examples use smart pointers, but educational pointer examples exist
- Array examples demonstrate both C-style and `std::array`
- Raw pointer examples always show proper `delete` and `nullptr` assignment

### Code Examples Reference
The ProC++ README contains extensive inline code documentation:
- Pointer usage patterns and best practices
- `const` and `constexpr` distinctions
- String manipulation with C++20 features (`std::format`, `to_chars`, `from_chars`)
- Type inference and type aliases
- Designated initializers
- String literals (raw, regular, with suffixes)

## Environment Setup

### macOS Requirements
```bash
brew install boost
brew install cmake
```

### Docker Support
ProC++ includes Docker configuration:
```bash
docker build -t my_cpp_project .
docker run --rm my_cpp_project
docker run -it my_cpp_project /bin/bash
```

## Testing
No formal test framework is configured. Examples are run individually to verify behavior.
