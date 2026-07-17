# 2026-07-17 Daily Study: Thread Affinity and K-Distinct Window

## 오늘의 주제

7월 17일 브리핑은 정적 리플렉션 기반 하드웨어 스레드 선호도 매핑과 캐시 라인 토폴로지 최적화를 다룬다.

실제 `pthread_setaffinity_np` 같은 OS별 스레드 고정 API는 플랫폼 의존성이 크므로, 실습은 이식 가능한 C++23 코드로 선호도 매핑 결정을 시뮬레이션한다. `alignas(64)`로 캐시 라인 정렬을 확인하고, `std::thread::hardware_concurrency()`로 요청 코어가 유효한지 검사하며, `std::jthread`로 비동기 단계 실행을 흉내 낸다.

## 파일 구성

```text
dailystudy/2026-07-17/
  CMakeLists.txt
  README.md
  main.cpp      # ExchangeChannel 선호도 매핑 시뮬레이션
  problem.cpp   # 최대 K개 고유 문자를 포함하는 최장 부분 문자열
```

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-17
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_17.exe
.\build\daily_problem_2026_07_17.exe
```

## 배울 문법

- `alignas(64)`로 캐시 라인 경계 정렬 요청
- `std::thread::hardware_concurrency()`로 하드웨어 스레드 수 조회
- `std::jthread`로 join 누락 위험 없는 스레드 실행
- lambda 캡처와 forwarding reference `Handler&&`
- structured binding `auto [name, value]`
- 고유 문자 개수를 제한하는 가변 슬라이딩 윈도우

## 코딩 테스트 학습

`problem.cpp`는 "최대 K개의 서로 다른 문자를 포함하는 가장 긴 부분 문자열" 문제다. 오른쪽 포인터로 윈도우를 확장하고, 고유 문자 수가 `k`를 초과하면 왼쪽 포인터를 이동한다. 시간 복잡도는 `O(N)`, 공간 복잡도는 알파벳 크기에 고정되는 `O(1)`이다.
