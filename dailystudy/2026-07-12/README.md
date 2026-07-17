# 2026-07-12 Daily Study: Modern Loops, Ranges, Prefix Sum

## 오늘의 주제

7월 12일 특별편은 구식 `for`/`while` 제어문을 현대적인 range, view, 알고리즘 호출로 바꾸는 감각을 다룬다. 원문에는 C++26 `std::generator`, `views::enumerate`, 병렬 `for_each`가 등장하지만, 이 실습은 현재 GCC 16에서 안정적으로 실행 가능한 C++23 코드로 작성했다.

## 파일 구성

```text
dailystudy/2026-07-12/
  CMakeLists.txt
  README.md
  main.cpp      # ranges 기반 패킷 처리 실습
  problem.cpp   # prefix sum + unordered_map 문제
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-12
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_12.exe
.\build\daily_problem_2026_07_12.exe
```

## 배울 문법

- range-based `for`
- `std::views::filter`
- lambda `[](const auto& value) { ... }`
- structured binding
- `std::unordered_map`
- prefix sum

## 코딩 테스트 학습

`problem.cpp`는 "합이 K가 되는 연속 부분 배열의 개수" 문제다. 음수가 포함될 수 있으므로 단순 투 포인터가 아니라 prefix sum과 hash map을 사용한다.
