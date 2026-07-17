# 2026-07-14 Daily Study: Atomic shared_ptr, Router Swap, Permutation Window

## 오늘의 주제

7월 14일 브리핑은 `std::atomic<std::shared_ptr<T>>`를 이용해 공유 라우팅 노드를 안전하게 교체하는 구조를 다룬다. mutex 없이 소유권을 안전하게 바꾸는 감각이 핵심이다.

## 파일 구성

```text
dailystudy/2026-07-14/
  CMakeLists.txt
  README.md
  main.cpp      # atomic shared_ptr 기반 라우터 노드 교체
  problem.cpp   # 문자열 순열 포함 여부 문제
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-14
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_14.exe
.\build\daily_problem_2026_07_14.exe
```

## 배울 문법

- `std::shared_ptr`: 참조 카운트 기반 공유 소유권
- `std::make_shared`: 객체와 제어 블록을 효율적으로 생성
- `std::atomic<std::shared_ptr<T>>`: shared pointer 자체를 스레드 안전하게 교체
- `exchange()`: 이전 값을 돌려주면서 새 값으로 원자적 교체
- `std::vector<int>` 빈도표와 고정 길이 슬라이딩 윈도우

## 코딩 테스트 학습

`problem.cpp`는 7/13과 같은 "Permutation in String" 문제를 한 번 더 다룬다. 같은 문제를 반복하되 함수와 출력 이름을 바꿔, 빈도표를 사용한 고정 크기 윈도우 패턴을 굳히는 데 초점을 둔다.
