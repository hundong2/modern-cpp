# 2026-07-15 Daily Study: Packet Slicing and Minimum Window

## 오늘의 주제

7월 15일 브리핑은 정적 리플렉션으로 구조체 멤버 오프셋을 컴파일 타임에 확정하고, 네트워크 바이트 버퍼에서 필요한 세그먼트만 무복사로 읽는 흐름을 다룬다.

현재 로컬 GCC에서 C++26 리플렉션과 pack indexing은 직접 컴파일되지 않으므로, 실습 코드는 `offsetof`, `std::span<const std::byte>`, `reinterpret_cast`, `std::expected`로 같은 구조를 실행 가능하게 재현했다.

## 파일 구성

```text
dailystudy/2026-07-15/
  CMakeLists.txt
  README.md
  main.cpp      # GigabitPacket 세그먼트 오프셋/바이트 뷰 실습
  problem.cpp   # Minimum Window Substring
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-15
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_15.exe
.\build\daily_problem_2026_07_15.exe
```

## 배울 문법

- `std::span<const std::byte>`로 raw buffer를 복사 없이 전달
- `offsetof`로 구조체 멤버 오프셋 확인
- `reinterpret_cast`로 바이트 주소를 구조체 포인터로 해석
- `std::expected`로 파싱 실패를 예외 없이 표현
- 슬라이딩 윈도우와 빈도표로 최소 포함 구간 계산

## 코딩 테스트 학습

`problem.cpp`는 "타겟 문자열의 모든 문자를 포함하는 최소 창" 문제다. 오른쪽 포인터로 창을 확장하고, 조건이 충족되면 왼쪽 포인터를 당겨 최소 길이를 갱신한다. 시간 복잡도는 `O(N)`, 공간 복잡도는 `O(1)`이다.
