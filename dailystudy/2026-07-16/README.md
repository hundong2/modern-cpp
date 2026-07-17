# 2026-07-16 Daily Study: Zero-Copy Serialization and Sum Window

## 오늘의 주제

7월 16일 브리핑은 구조체를 별도 버퍼에 복사하지 않고 `std::span<const std::byte>` 바이트 뷰로 노출하는 무복사 직렬화 흐름을 다룬다.

원문에는 C++26 정적 리플렉션과 pack indexing이 등장하지만, 이 실습은 현재 컴파일러에서 바로 실행되도록 `std::is_trivially_copyable`, `reinterpret_cast`, `std::span`, `std::expected`를 사용한다.

## 파일 구성

```text
dailystudy/2026-07-16/
  CMakeLists.txt
  README.md
  main.cpp      # RouterPayload 무복사 바이트 뷰 실습
  problem.cpp   # 합이 K 이하인 가장 긴 연속 부분 배열
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-16
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_16.exe
.\build\daily_problem_2026_07_16.exe
```

## 배울 문법

- `std::is_trivially_copyable_v<T>`로 바이트 뷰에 적합한 타입 제한
- `std::span<const std::byte>`로 복사 없는 직렬화 뷰 생성
- `reinterpret_cast`의 저수준 주소 재해석
- `std::expected` 기반 오류 반환
- 양의 정수 배열에서 투 포인터 슬라이딩 윈도우 적용

## 코딩 테스트 학습

`problem.cpp`는 "합이 K 이하가 되는 가장 긴 연속 부분 배열" 문제다. 입력이 양의 정수이므로 오른쪽 포인터로 합을 늘리고, 합이 `k`를 넘으면 왼쪽 포인터를 이동시키는 `O(N)` 풀이가 가능하다.
