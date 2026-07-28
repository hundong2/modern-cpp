# 2026-07-29 — `std::source_location`으로 진단 경계 만들기

오늘은 C++20의 `std::source_location`으로 **문제가 발생한 호출 위치를 자동으로 전달하는 진단 포트**를 만든다. 비즈니스 코드(`OrderService`)는 "기록하라"는 인터페이스만 알고, 콘솔 출력 같은 세부 구현은 `ConsoleDiagnosticSink` 어댑터가 담당한다. 이것이 의존성 역전의 작은 예다.

## 왜 필요한가

`__FILE__`, `__LINE__` 매크로를 직접 넘기면 호출자가 정보를 빼먹거나 함수 이름을 잘못 적기 쉽다. `std::source_location::current()`를 **기본 인자**로 두면, `report()`를 부른 곳의 파일·줄·함수 정보를 컴파일러가 채운다. 위치 객체는 값으로 전달되는 작은 진단 데이터이며, 실제 표시 형식은 어댑터가 결정한다.

## 실행

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`main.cpp`는 포트/어댑터, 다형성 호출, 값 범주와 수명을 보여 준다. `problem.cpp`은 직접 완성하는 미니 연습이다. 코드의 한국어 주석은 문법이 등장한 바로 옆에서 읽는다.

## 직접 해보기

1. `main.cpp`의 `service.place(0)`을 `service.place(3)`으로 바꾸고 성공 경로를 관찰한다.
2. `ConsoleDiagnosticSink` 대신, 메시지를 `std::vector<std::string>`에 모으는 `MemoryDiagnosticSink`를 작성해 본다.
3. `problem.cpp`의 TODO를 채워 빈 사용자 이름을 거절하는 진단을 추가한다.
4. `report()` 호출 줄을 다른 함수로 옮겨, 출력되는 줄 번호도 함께 바뀌는지 확인한다.

## 값·수명·실행 관점

`std::source_location::current()`의 결과는 prvalue이고 `report`의 값 매개변수 `where`를 직접 초기화한다. `const std::string&`는 이미 존재하는 lvalue 문자열을 복사하지 않고 빌리지만, 호출 중에만 안전하다. `std::unique_ptr`는 sink의 단독 소유자를 표현하고, 이동 뒤 원래 포인터는 비어 있다. 반환값 `PlaceResult{...}`는 prvalue로 만들어져 목적 객체에 직접 구성될 수 있어 복사/이동이 생략될 수 있다.

기계 관점에서 컴파일러는 보통 멤버 읽기(로드), 결과 저장, `if` 비교와 분기, 함수 호출을 만들며 `sink_->write()`는 가상 간접 호출일 수 있다. 단, 정확한 명령·호출 규약·인라이닝 여부는 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.
