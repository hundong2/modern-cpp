# 2026-07-11 Daily Study: Static Buffer Validation, span, Character Replacement

## 오늘의 주제

7월 11일 브리핑은 정적 리플렉션을 이용한 패킷 버퍼 구조 검증을 다룬다. 현재 로컬 컴파일러에서는 C++26 리플렉션 문법을 직접 사용할 수 없으므로, 실습 코드는 C++23의 `std::span`, `std::byte`, `sizeof`, `alignof`, `std::expected`를 이용해 실행 가능한 버퍼 검증기로 구성했다.

## 파일 구성

```text
dailystudy/2026-07-11/
  CMakeLists.txt
  README.md
  main.cpp      # span 기반 보안 패킷 버퍼 검증
  problem.cpp   # character replacement 슬라이딩 윈도우 문제
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-11
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_11.exe
.\build\daily_problem_2026_07_11.exe
```

## 배울 문법

- `std::span<const std::byte>`: 복사 없이 바이트 버퍼를 안전하게 바라보는 뷰
- `reinterpret_cast`: 바이트 메모리를 구조체 포인터로 해석
- `std::expected`: 성공 값 또는 오류 메시지 반환
- `sizeof(T)`: 타입의 실제 바이트 크기
- `std::vector<int>`: 문자 빈도표

## 코딩 테스트 학습

`problem.cpp`는 "최대 K번 문자를 바꿔 만들 수 있는 가장 긴 같은 문자 부분 문자열" 문제다. 빈도 배열과 슬라이딩 윈도우를 사용하며 시간 복잡도는 `O(N)`이다.
