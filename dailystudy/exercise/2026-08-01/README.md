# 2026-08-01 — `std::string_view`와 명령 처리 경계

오늘은 C++20의 `std::string_view`로 문자열을 **복사하지 않고 읽는 방법**과, 명령 해석 규칙을 출력 장치에서 분리하는 포트/어댑터 구조를 배웁니다.

`Command`(도메인 값) → `parse_command`(해석 함수) → `CommandService`(유스케이스) → `OutputPort`(출력 포트) → `ConsoleOutput`(콘솔 어댑터)

`std::string_view`는 문자 데이터를 소유하지 않고 주소와 길이만 보관합니다. 따라서 원본 문자열보다 오래 살아서는 안 됩니다. 이 예제에서는 `main`의 `std::string input`이 파싱과 실행이 끝날 때까지 살아 있으므로 안전합니다. 서비스는 출력 구현을 `std::unique_ptr`로 단독 소유하고, 가상 함수 호출로 실제 어댑터를 선택합니다.

## 빌드와 실행

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

- `main.cpp`: 완성 예제입니다. 비소유 문자열 뷰, 파서, 포트/어댑터, 소유권 이동을 함께 봅니다.
- `problem.cpp`: `TODO` 두 곳을 바꾸어 직접 결과를 예측하고 확인합니다.
- `CHECKPOINT.md`: 말하기, 코드 찾기, 수정·실행의 세 단계로 이해를 증명합니다.

## 값 범주와 객체 수명

이름이 있는 `input`, `service`, `command`는 lvalue입니다. `std::string{"echo hello"}`와 `Command{...}`는 prvalue입니다. `std::move(output)`은 이름 있는 lvalue를 xvalue로 표현하지만, 실제 이동은 `unique_ptr`의 이동 생성자가 실행할 때 일어납니다. 이동 후 `output`은 비어 있고 소유권은 `service`가 가집니다.

`parse_command(input)`에서 `input`은 `const std::string&`에 바인딩되고, 함수 안에서 만든 `std::string_view text{input}`은 문자 버퍼를 빌리지 소유하지 않습니다. `Command`는 필요한 인수를 `std::string`으로 복사해 소유하므로 함수 반환 뒤에도 안전합니다. 반환하는 `Command{...}` prvalue는 C++17 이후 보장되는 복사 생략으로 호출자의 결과 객체 자리에 직접 만들어질 수 있습니다. 이름 있는 객체를 값으로 전달하면 복사가, `std::move`로 전달하면 이동이 선택될 수 있습니다.

## 기계 실행 관점

개념적으로 문자열 뷰는 주소·길이 로드, 길이 비교, 문자 비교, 조건 분기를 거칩니다. `output_->write(...)`는 동적 타입에 따라 구현을 고르는 가상 간접 호출일 수 있고, 문자열 생성·이동은 함수 호출과 메모리 저장을 동반할 수 있습니다. 다만 실제 레지스터, 메모리 접근, 호출 제거 여부와 명령 선택은 CPU, ABI, 컴파일러, 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않습니다.

## 직접 해보기

1. `input`을 `"quit"`, `"echo modern cpp"`, `"unknown"`으로 바꾸고 분기를 예측한 뒤 실행합니다.
2. `parse_command`가 `Command` 안에 `string_view`를 저장하도록 바꾸면 어떤 수명 문제가 생기는지 말해 봅니다.
3. `ConsoleOutput` 대신 문자열을 모으는 테스트 어댑터를 설계해 봅니다.
4. `problem.cpp`의 두 `TODO`를 수정하고 출력과 종료 코드를 확인합니다.
