# Boost 설치 확인 및 C++23 빌드 방법

이 문서는 `g++`에서 Boost 설치 여부를 확인하고, Boost를 사용하는 프로그램을 C++23 표준으로 빌드하는 방법을 설명합니다.

## 가장 간단한 사용 방법

현재 폴더처럼 `CMakeLists.txt`가 이미 있다면 최초 한 번만 구성합니다.

```bash
cmake -S . -B build
```

그다음부터는 소스 코드를 수정한 후 다음 한 줄로 빌드하고 실행합니다.

```bash
cmake --build build && ./build/sliding_window_k_distinct
```

프로그램 인자를 전달할 수도 있습니다.

```bash
cmake --build build && ./build/sliding_window_k_distinct eceba 2
```

`main.cpp` 하나만 빠르게 빌드하고 싶다면 CMake 없이 다음 명령을 사용할 수 있습니다.

```bash
g++ -std=c++23 main.cpp -o app && ./app
```

## 기존 CMake 프로젝트에서 Boost 사용하기

### 1. Boost 설치

macOS에서 Homebrew를 사용한다면 다음 명령만 실행합니다.

```bash
brew install boost
```

Ubuntu 또는 Debian에서는 다음 명령을 사용합니다.

```bash
sudo apt update
sudo apt install libboost-all-dev
```

설치 여부는 다음과 같이 확인할 수 있습니다.

```bash
# macOS
brew list boost

# Ubuntu/Debian
dpkg -s libboost-all-dev
```

### 2. 헤더 전용 Boost 사용

많은 Boost 라이브러리는 헤더만 포함하면 사용할 수 있습니다. 예를 들어 `Boost.Algorithm`은 다음과 같이 사용합니다.

```cpp
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>

int main()
{
    std::string text = "hello boost";
    boost::algorithm::to_upper(text);
    std::cout << text << '\n';
}
```

이 경우 `CMakeLists.txt`는 다음처럼 작성하면 됩니다.

```cmake
cmake_minimum_required(VERSION 3.16)
project(boost_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

add_executable(boost_example main.cpp)
target_link_libraries(boost_example PRIVATE Boost::headers)
```

빌드와 실행은 평소와 같습니다.

```bash
cmake -S . -B build
cmake --build build && ./build/boost_example
```

### 3. 링크가 필요한 Boost 사용

`Boost.Filesystem`, `Boost.ProgramOptions`, `Boost.Thread`처럼 컴파일된 라이브러리를 링크해야 하는 구성 요소도 있습니다.

예를 들어 `Boost.ProgramOptions`를 사용하려면 `CMakeLists.txt`에 구성 요소를 지정합니다.

```cmake
cmake_minimum_required(VERSION 3.16)
project(boost_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(boost_example main.cpp)
target_link_libraries(boost_example PRIVATE Boost::program_options)
```

`main.cpp` 예제입니다.

```cpp
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
    po::options_description options{"Options"};
    options.add_options()
        ("help,h", "도움말 출력")
        ("name,n", po::value<std::string>()->default_value("C++"), "이름");

    po::variables_map values;
    po::store(po::parse_command_line(argc, argv, options), values);
    po::notify(values);

    if (values.contains("help")) {
        std::cout << options << '\n';
        return 0;
    }

    std::cout << "Hello, " << values["name"].as<std::string>() << "!\n";
}
```

빌드하고 실행합니다.

```bash
cmake -S . -B build
cmake --build build
./build/boost_example --name Boost
```

핵심은 다음 두 줄입니다.

```cmake
find_package(Boost REQUIRED COMPONENTS program_options)
target_link_libraries(boost_example PRIVATE Boost::program_options)
```

다른 Boost 라이브러리를 사용할 때는 `program_options` 부분을 필요한 구성 요소 이름으로 바꾸면 됩니다.

## 1. Boost 설치 확인

### Linux

Boost 헤더가 설치되어 있는지 확인합니다.

```bash
ls /usr/include/boost/version.hpp
```

설치된 Boost 버전을 확인합니다.

```bash
grep 'BOOST_LIB_VERSION' /usr/include/boost/version.hpp
```

### macOS(Homebrew)

```bash
brew list boost
brew --prefix boost
```

설치된 Boost 버전을 확인합니다.

```bash
grep 'BOOST_LIB_VERSION' "$(brew --prefix boost)/include/boost/version.hpp"
```

### 컴파일로 확인

다음과 같이 `boost_check.cpp`를 작성합니다.

```cpp
#include <boost/version.hpp>
#include <iostream>

int main()
{
    std::cout << "Boost version: "
              << BOOST_VERSION / 100000 << '.'
              << BOOST_VERSION / 100 % 1000 << '.'
              << BOOST_VERSION % 100 << '\n';
}
```

Linux에서 빌드하고 실행합니다.

```bash
g++ -std=c++23 boost_check.cpp -o boost_check
./boost_check
```

macOS에서 Homebrew Boost를 사용한다면 다음과 같이 빌드합니다.

```bash
BOOST_PREFIX="$(brew --prefix boost)"

g++ -std=c++23 \
    -I"$BOOST_PREFIX/include" \
    boost_check.cpp \
    -o boost_check

./boost_check
```

## 2. Boost 설치

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install libboost-all-dev
```

### macOS(Homebrew)

```bash
brew install boost
```

### Fedora

```bash
sudo dnf install boost-devel
```

### Arch Linux

```bash
sudo pacman -S boost
```

## 3. 헤더 전용 Boost 라이브러리 빌드

`Boost.Algorithm`, `Boost.Container`, `Boost.Optional` 등 많은 Boost 구성 요소는 별도 라이브러리 링크 없이 사용할 수 있습니다.

```bash
g++ -std=c++23 -Wall -Wextra -pedantic main.cpp -o app
```

macOS에서 Homebrew 설치 경로를 명시해야 한다면 다음 명령을 사용합니다.

```bash
BOOST_PREFIX="$(brew --prefix boost)"

g++ -std=c++23 \
    -Wall -Wextra -pedantic \
    -I"$BOOST_PREFIX/include" \
    main.cpp \
    -o app
```

## 4. 링크가 필요한 Boost 라이브러리 빌드

예를 들어 `Boost.Filesystem`과 `Boost.System`을 사용한다면 다음과 같이 링크합니다.

### Linux

```bash
g++ -std=c++23 main.cpp \
    -lboost_filesystem \
    -lboost_system \
    -o app
```

### macOS(Homebrew)

```bash
BOOST_PREFIX="$(brew --prefix boost)"

g++ -std=c++23 \
    -I"$BOOST_PREFIX/include" \
    -L"$BOOST_PREFIX/lib" \
    main.cpp \
    -lboost_filesystem \
    -lboost_system \
    -o app
```

스레드 라이브러리를 사용한다면 다음과 같이 빌드합니다.

```bash
g++ -std=c++23 main.cpp \
    -pthread \
    -lboost_thread \
    -lboost_system \
    -o app
```

`-lboost_*` 옵션은 일반적으로 소스 또는 오브젝트 파일 뒤에 두는 것이 안전합니다.

## 5. CMake 사용 방법

링크가 필요한 `Boost.Filesystem`과 `Boost.System`을 사용하는 예제입니다.

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyBoostApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS filesystem system)

add_executable(app main.cpp)

target_link_libraries(app PRIVATE
    Boost::filesystem
    Boost::system
)
```

다음 명령으로 구성하고 빌드합니다.

```bash
cmake -S . -B build
cmake --build build
./build/app
```

Boost 헤더만 사용하는 프로젝트라면 다음과 같이 설정할 수 있습니다.

```cmake
find_package(Boost REQUIRED)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE Boost::headers)
```

## 6. GCC의 C++23 지원 확인

먼저 GCC 버전을 확인합니다.

```bash
g++ --version
```

C++23 모드로 간단히 빌드해 지원 여부를 확인할 수 있습니다.

```bash
g++ -std=c++23 main.cpp -o app
```

컴파일러가 `-std=c++23` 옵션을 인식하지 못한다면 `-std=c++2b`를 사용하거나 GCC를 업그레이드해야 합니다.
