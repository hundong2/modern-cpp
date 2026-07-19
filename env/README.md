# 공통 C++ 빌드 환경

이 폴더는 특정 소스 트리에 의존하지 않는 CMake 기반 C++17/20/23 빌드 도구입니다.
기본값은 **C++23 + Debug**이며, 빌드 결과는 대상 프로젝트의 `build/<표준>-<설정>`에 생성됩니다.

전체 명령과 파라미터는 언제든 다음 명령으로 확인할 수 있습니다.

```bash
./env/cpp-env help
```

바로 실행해 볼 수 있는 예제는 [`test/README.md`](test/README.md)에 있습니다.

## 1. 도구 설치

```bash
# macOS (Homebrew가 없으면 먼저 설치 안내를 출력합니다)
./env/setup-macos.sh

# Ubuntu/Debian, Fedora/RHEL, Arch
./env/setup-linux.sh
```

Windows PowerShell(관리자 권한은 패키지 설치 시 필요할 수 있음):

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\env\setup-windows.ps1
```

설치 대상은 CMake, Ninja, 최신 컴파일러, GDB/LLDB(플랫폼에서 제공되는 경우)입니다.

## 2. 프로젝트에 설정 한 번에 추가

대상 폴더에는 `CMakeLists.txt`가 있어야 합니다.

```bash
./env/cpp-env init /path/to/project
```

```powershell
.\env\cpp-env.ps1 init C:\path\to\project
```

다음 파일을 **없는 경우에만** 추가합니다.

- `CMakeUserPresets.json`: C++17/20/23 × Debug/Release 프리셋
- `.vscode/tasks.json`: configure/build/test 작업
- `.vscode/launch.json`: 현재 선택된 프리셋의 실행 파일 디버그
- `.vscode/settings.json`: CMake Tools 기본 설정

VS Code에서는 CMake Tools와 C/C++ 확장을 설치하고, 상태 표시줄에서 프리셋을 선택한 뒤
`CMake: Debug` 또는 `F5`를 실행합니다. `launch.json`의 `program`은 기본적으로
`${command:cmake.launchTargetPath}`를 사용하므로 CMake Tools에서 실행 대상을 먼저 선택합니다.

## 3. CLI 빌드

```bash
./env/cpp-env configure . 23 Debug
./env/cpp-env build . 23 Debug
./env/cpp-env test . 23 Debug
./env/cpp-env clean . 23 Debug
```

```powershell
.\env\cpp-env.ps1 build . 20 Release
```

표준은 `17`, `20`, `23`, 설정은 `Debug`, `Release`, `RelWithDebInfo`, `MinSizeRel`을 지원합니다.
`build`와 `test`는 configure를 자동 수행합니다. 추가 CMake 인자는 환경 변수로 넘길 수 있습니다.

```bash
CPP_ENV_CMAKE_ARGS='-DBUILD_TESTING=ON' ./env/cpp-env build . 23 Debug
```

Windows:

```powershell
$env:CPP_ENV_CMAKE_ARGS='-DBUILD_TESTING=ON'; .\env\cpp-env.ps1 build . 23 Debug
```

## 컴파일러 선택

기본 컴파일러는 운영체제 기본값입니다. 필요하면 configure 전에 지정합니다.

```bash
CC=clang CXX=clang++ ./env/cpp-env build . 23 Debug
CC=gcc CXX=g++ ./env/cpp-env build . 17 Release
```

```powershell
$env:CC='clang'; $env:CXX='clang++'; .\env\cpp-env.ps1 build . 23 Debug
```

macOS 디버깅은 LLDB, Linux는 GDB 또는 LLDB, Windows MSVC는 Visual Studio Debugger를 권장합니다.

## 공통 경고·Sanitizer 설정(선택)

각 프로젝트에서 이 저장소의 helper를 include한 뒤 대상별로 적용할 수 있습니다.

```cmake
include(/path/to/env/cmake/CppEnvironment.cmake)
cpp_env_apply(my_target)
```

`-DCPP_ENV_WARNINGS_AS_ERRORS=ON`으로 경고를 오류로 처리하고,
`-DCPP_ENV_SANITIZERS=ON`으로 GCC/Clang Debug 빌드에 ASan/UBSan을 추가합니다.
