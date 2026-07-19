# C++23 빌드 환경

## 요구 사항

- CMake 3.25 이상
- C++23을 지원하는 컴파일러
  - macOS: Apple Clang 15 이상 또는 Homebrew LLVM
  - Linux: GCC 13 이상 또는 Clang 17 이상

macOS에서는 Homebrew LLVM이 설치되어 있으면 자동으로 우선 사용하고,
없으면 시스템 기본 컴파일러를 사용합니다.

## 빌드 및 실행

```sh
cd cpp23
cmake --preset debug
cmake --build --preset debug
./build/debug/cpp23_app
```

Release 빌드는 `debug` 대신 `release` 프리셋을 사용합니다.

## 테스트

```sh
ctest --preset debug
```

VS Code에서는 권장 확장인 **C/C++**과 **CMake Tools**를 설치한 뒤,
하단 상태 표시줄에서 `C++23 Debug` 또는 `C++23 Release` 프리셋을 선택해
빌드할 수 있습니다.
