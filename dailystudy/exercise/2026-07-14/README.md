# 2026-07-14 Modern C++ Daily Exercise

오늘의 목표는 "작게 나눈 아키텍처"와 Modern C++ 문법을 한 번에 복습하는 것이다. 기초가 부족한 개발자도 따라올 수 있도록, 예제는 컴파일 가능한 코드와 검증 질문을 함께 제공한다.

## 파일 구성

```text
dailystudy/exercise/2026-07-14/
  CMakeLists.txt   # 두 실습 파일을 빌드하는 CMake 설정
  README.md        # 학습 순서와 핵심 개념
  main.cpp         # 아키텍처 실습: 입력 파싱, 규칙, 출력 어댑터 분리
  problem.cpp      # 문법 반복: span, optional, fixed-size sliding window
  CHECKPOINT.md    # 초급자 이해도 검증 단계와 통과 기준
```

## 빌드와 실행

Windows PowerShell 기준:

```powershell
cd D:\workspace\modern-cpp\dailystudy\exercise\2026-07-14
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\exercise_architecture.exe
.\build\exercise_window.exe
```

## 오늘의 핵심 개념

### 1. 아키텍처는 "바뀌는 것"을 분리하는 일이다

`main.cpp`는 센서 문자열을 읽어 위험 온도를 알리는 작은 시스템이다. 이 시스템은 세 부분으로 나뉜다.

- 입력 파싱: `"101:84"` 같은 문자열을 `TemperatureReading`으로 바꾼다.
- 도메인 규칙: 온도가 임계값 이상이면 알림을 만든다.
- 출력 어댑터: 알림을 어디에 저장하거나 보낼지 결정한다.

중요한 점은 도메인 규칙이 콘솔, 파일, 네트워크 같은 구체적인 출력 방식에 묶이지 않는다는 것이다. 이 분리가 커지면 테스트하기 쉬운 구조가 된다.

### 2. `std::string_view`와 `std::span`은 소유하지 않는다

`std::string_view`는 문자열을 복사하지 않고 바라본다. `std::span`도 배열이나 `std::vector`의 일부를 복사하지 않고 바라본다. 빠르고 편하지만, 원본 데이터가 먼저 사라지면 view도 안전하지 않다.

초급자 기준으로는 이렇게 기억하면 된다.

- `std::string`과 `std::vector`는 데이터를 가진다.
- `std::string_view`와 `std::span`은 데이터를 빌려 본다.
- 빌려 보는 타입은 함수 인자로 쓰기 좋지만, 오래 저장할 때는 조심한다.

### 3. `std::expected`는 실패를 값으로 돌려준다

`parse_reading()`은 실패할 수 있다. 예외를 던지는 대신 `std::expected<성공값, 오류문자열>`을 반환한다. 호출자는 `if (!result)`로 실패를 확인하고, `result.error()`로 이유를 읽는다.

이 방식은 오류가 정상 흐름의 일부일 때 유용하다. 특히 입력 검증, 파싱, 네트워크 응답 처리처럼 "실패 가능성"이 자연스러운 코드에서 흐름이 명확해진다.

### 4. RAII는 일찍 반환해도 정리를 보장한다

`ProcessingSession`은 생성될 때 활성 세션 수를 올리고, 소멸될 때 다시 내린다. 중간에 오류가 나서 함수가 일찍 끝나도 지역 객체의 소멸자는 호출된다. 이것이 RAII의 핵심이다.

초급자에게는 "열었으면 닫기"를 객체의 생성자와 소멸자에 맡기는 습관이라고 설명할 수 있다.

### 5. `concept`는 템플릿의 요구사항을 코드로 적는다

`AlertPublisher` concept는 `publish(Alert)` 함수를 가진 타입만 `ReadingService`에 들어올 수 있게 한다. 덕분에 잘못된 타입을 넣으면 실행 중이 아니라 컴파일 중에 더 빠르게 알 수 있다.

## 실습 순서

1. `main.cpp`를 빌드하고 실행한다.
2. `MemoryAlertPublisher`가 왜 테스트용 출력 어댑터인지 설명한다.
3. `high_temperature_celsius` 값을 80에서 90으로 바꾸고 알림 개수가 어떻게 바뀌는지 확인한다.
4. `"broken-line"` 입력을 추가하고 `std::expected` 오류 메시지를 확인한다.
5. `problem.cpp`의 `first_window_at_least()`를 손으로 추적한다.
6. [CHECKPOINT.md](./CHECKPOINT.md)의 검증 질문을 답한다.

## 값 범주와 기계 실행 관점

- 이름 있는 `value`, `report`, `publisher`는 lvalue이며 수정 가능한 저장 위치를 나타낸다.
- `TemperatureReading{...}`와 `unexpected(...)` 결과는 prvalue로 반환 객체를 직접 초기화할 수 있다.
- `string_view`와 `span`은 보통 주소와 길이를 복사할 뿐 원본 데이터를 소유하지 않는다.
- 파싱과 반복은 대체로 load·비교·조건 분기로 실행되지만 정확한 어셈블리는 CPU와 최적화 옵션에 따라 달라진다.

## 오늘의 완료 기준

- 두 실행 파일이 모두 빌드된다.
- `main.cpp`의 테스트가 모두 통과한다.
- `problem.cpp`의 테스트가 모두 통과한다.
- `std::string_view`, `std::span`, `std::expected`, RAII, concept를 각각 한 문장으로 설명할 수 있다.
- `CHECKPOINT.md`의 실습 변경 과제 중 2개 이상을 직접 수정하고 다시 실행할 수 있다.
