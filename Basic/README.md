# Basic C++17 Practice

`Basic`은 C++17 이상 문법을 처음 실습할 때 쓰는 짧은 노트북 자료 모음입니다. 각 주제 폴더에는 `lesson.ipynb`와 같은 내용을 터미널/CMake로 확인할 수 있는 `practice.cpp`가 들어 있습니다.

## 폴더 구조

```text
Basic/
|-- 01-start/        # 출력, 변수, 타입, auto
|-- 02-flow/         # 조건문, 반복문, 함수, 참조
|-- 03-containers/   # string, vector, algorithm, lambda
|-- 04-ownership/    # struct/class, RAII, smart pointer, optional
|-- .vscode/         # Basic 폴더를 VSCode로 열었을 때 쓰는 권장 설정
|-- CMakeLists.txt   # practice.cpp 빌드용 CMake 설정
`-- README.md
```

## VSCode 준비

권장 확장:

- `ms-vscode.cpptools` 또는 `ms-vscode.cpptools-extension-pack`
- `ms-vscode.cmake-tools`
- `ms-toolsai.jupyter`

권장 컴파일러:

- Windows: MSYS2 UCRT64의 `g++`, 또는 Visual Studio Build Tools의 MSVC
- macOS: Xcode Command Line Tools의 `clang++`
- Linux/WSL: `g++` 또는 `clang++`

Windows에서 C++ 노트북까지 안정적으로 쓰려면 WSL Ubuntu 또는 conda 환경을 권장합니다.

## C++ 노트북 커널 설치

VSCode에서 `.ipynb`를 C++로 실행하려면 Jupyter C++ 커널이 필요합니다. conda-forge의 `xeus-cling`을 쓰면 `C++17` 커널을 선택할 수 있습니다.

```bash


```

VSCode에서 `lesson.ipynb`를 열고 우측 상단 커널 선택 메뉴에서 `C++17` 또는 `xcpp17`을 선택합니다. 셀을 여러 번 다시 실행하다가 재정의 오류가 나면 `Restart Kernel` 후 처음부터 실행하면 됩니다.

## CMake 빌드

`Basic` 폴더만 VSCode로 여는 방식이 가장 단순합니다.

```bash
cd Basic
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

C++20 이상으로 확인하고 싶으면 표준 값을 바꿔 구성합니다.

```bash
cmake -S . -B build -DBASIC_CPP_STANDARD=20
cmake --build build
```

VSCode에서는 `Ctrl+Shift+P` 후 다음 순서로 진행합니다.

1. `CMake: Configure`
2. `CMake: Build`
3. 하단 상태바에서 실행할 target 선택 후 `Run` 또는 `Debug`

루트 저장소를 VSCode로 열었다면 `CMake: Select Source Directory`에서 `Basic`을 선택합니다.

`nmake`를 찾을 수 없거나 `CMAKE_CXX_COMPILER not set` 오류가 나면 컴파일러가 PATH에 없다는 뜻입니다. MSVC를 쓸 때는 `Developer PowerShell for VS`에서 VSCode를 열거나 CMake Tools의 `CMake: Select a Kit`로 Visual Studio kit를 고릅니다. MSYS2/WSL/conda 환경을 쓸 때는 `g++`와 `ninja`가 설치된 터미널에서 `cmake -S . -B build -G Ninja`로 다시 구성합니다.

## 단일 파일 빠른 실행

MSYS2, WSL, Linux, macOS처럼 `g++`가 PATH에 있으면 다음처럼 직접 실행할 수 있습니다.

```bash
g++ -std=c++17 -Wall -Wextra -pedantic 01-start/practice.cpp -o basic_start
./basic_start
```

Windows PowerShell에서 `.exe`로 빌드한 경우:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic .\01-start\practice.cpp -o .\basic_start.exe
.\basic_start.exe
```

## 학습 순서

1. `01-start/lesson.ipynb`: C++ 프로그램의 최소 구성, 출력, 변수, 타입, `auto`
2. `02-flow/lesson.ipynb`: 조건문/반복문을 함수로 묶고 값 전달과 참조 전달 비교
3. `03-containers/lesson.ipynb`: `std::string`, `std::vector`, `<algorithm>`, lambda
4. `04-ownership/lesson.ipynb`: 객체 수명, RAII, `unique_ptr`, `shared_ptr`, `std::optional`

각 노트북은 설명, 실행 예제, 직접 고칠 TODO 실습 순서로 구성되어 있습니다. `practice.cpp`는 같은 주제를 독립 실행형 코드로 다시 확인하는 용도입니다.
