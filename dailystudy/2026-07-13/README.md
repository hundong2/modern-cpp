# 2026-07-13 Daily Study: Error Recovery, Static DI, Sliding Windows

## 오늘의 주제

7월 13일에는 같은 날짜의 브리핑이 두 가지 들어왔다. 하나는 정적 리플렉션과 현대 루프 문법을 이용한 오류 복구 파이프라인이고, 다른 하나는 정적 DI와 보안 검증 파이프라인이다.

현재 로컬 컴파일러에서 C++26 리플렉션과 pack indexing은 직접 사용할 수 없으므로, 실습 코드는 C++23의 `std::span`, `std::expected`, `std::views::single`, `std::tuple`, 템플릿 DI로 같은 설계 의도를 실행 가능하게 구성했다.

## 파일 구성

```text
dailystudy/2026-07-13/
  CMakeLists.txt
  README.md
  main.cpp      # 오류 플래그가 있는 패킷 복구 실습
  problem.cpp   # 문자열 순열 포함 여부 문제
  integrated_gateway.cpp       # 정적 DI + 보안 검증 보너스 실습
  character_replacement.cpp    # 문자 교체 슬라이딩 윈도우 보너스 문제
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-13
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_13.exe
.\build\daily_problem_2026_07_13.exe
.\build\daily_integrated_gateway_2026_07_13.exe
.\build\daily_character_replacement_2026_07_13.exe
```

## 배울 문법

- `std::span<const std::byte>`로 raw buffer를 복사 없이 전달
- `reinterpret_cast`로 바이트 주소를 구조체 포인터로 해석
- `std::expected`로 복구 가능/불가능 상태 표현
- `std::views::single`로 단일 객체를 range처럼 처리
- 템플릿 기반 정적 DI로 검증 컴포넌트 결합
- `std::vector<int>` 빈도표로 고정 크기 슬라이딩 윈도우 구현

## 코딩 테스트 학습

`problem.cpp`는 LeetCode 스타일의 "Permutation in String" 문제다. `s2` 안에 `s1`의 순열이 포함되어 있는지 고정 길이 윈도우와 알파벳 빈도표로 검사한다.

`character_replacement.cpp`는 "문자를 최대 K번 변경해 만들 수 있는 가장 긴 동일 문자 부분 문자열" 문제다. 둘 다 시간 복잡도는 `O(N)`, 공간 복잡도는 `O(1)`이다.
