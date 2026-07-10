# 2026-07-09 Daily Study: `alignas`, 바이트 뷰, 아나그램 윈도우

## 오늘의 주제

브리핑은 구조체 레이아웃과 정적 직렬화 최적화를 다룬다. 실습은 C++23에서 `alignas(8)`, `sizeof`, `alignof`, `std::span<const std::byte>`를 사용해 구조체를 바이트 뷰로 바라보는 방식을 연습한다.

## 왜 필요한가

네트워크, 파일 포맷, 하드웨어 버퍼는 결국 바이트 배열이다. 구조체의 크기와 정렬을 모르면 다른 시스템과 데이터를 주고받을 때 패딩, endian, alignment 문제로 버그가 난다.

## 빌드 및 실행

```powershell
cd D:\workspace\modern-cpp\dailystudy\2026-07-09
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\daily_modern_cpp_2026_07_09.exe
.\build\daily_problem_2026_07_09.exe
```

## 기본 문법 복습

- `alignas(8)`: 타입이나 변수를 8바이트 경계에 배치하도록 요구한다.
- `sizeof(T)`: 타입의 실제 메모리 크기다. 패딩이 포함된다.
- `alignof(T)`: 타입이 요구하는 정렬 단위다.
- `reinterpret_cast`: 객체 주소를 다른 포인터 타입으로 해석한다. 강력하지만 위험하므로 경계를 좁혀 써야 한다.
- `std::byte`: “문자가 아니라 바이트”라는 의도를 표현한다.
- `std::span`: 소유하지 않는 메모리 구간 뷰다.

## 실무 아키텍처 포인트

직렬화 코드는 도메인 로직과 분리하는 편이 좋다. 구조체를 바이트로 보는 순간부터 플랫폼 의존성이 생기기 때문이다. 실제 서비스라면 endian 변환과 명시적 필드 단위 직렬화를 추가로 고려해야 한다.

## 알고리즘 문제

문자열 `s`에서 패턴 `p`의 아나그램이 시작되는 모든 인덱스를 찾는다.

```text
s = "cbaebabacd", p = "abc"
answer = [0, 6]
```

고정 길이 윈도우와 26칸 빈도 배열을 사용한다.

복잡도:

- 시간: `O(N)`
- 공간: `O(1)`

