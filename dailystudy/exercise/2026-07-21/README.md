# 2026-07-21 Modern C++ 일일 연습

오늘은 C++23의 `std::expected<T, E>`로 실패를 숨기지 않고 타입에 표현하는 방법을 배웁니다. 예제는 입력 계층이 문자열을 검사하고, 응용 서비스가 유효한 도메인 값으로 메시지를 만드는 작은 처리 파이프라인입니다.

## 학습 목표

- `std::expected<T, E>`가 성공 `T`와 오류 `E` 중 하나만 보관함을 설명한다.
- `operator bool`, `operator*`, `value()`, `error()`, `std::unexpected`를 안전하게 사용한다.
- 파싱, 도메인 값, 응용 서비스를 분리하는 아키텍처상 이유를 말한다.
- 실제 식을 lvalue, prvalue, xvalue로 분류하고 참조 바인딩·복사·이동·수명을 추적한다.

## 빌드와 실행

저장소 루트의 PowerShell에서 실행합니다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-07-21 -B dailystudy/exercise/2026-07-21/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-07-21/build
ctest --test-dir dailystudy/exercise/2026-07-21/build --output-on-failure
```

## 핵심 개념

`std::expected<Quantity, ParseError>`는 두 객체를 동시에 보관하지 않습니다. 성공이면 `Quantity`, 실패면 `ParseError`가 활성 상태입니다. `if (result)`로 먼저 상태를 확인한 뒤 성공에는 `*result`, 실패에는 `result.error()`로 접근합니다. 상태를 확인하지 않은 `value()`는 실패 시 예외를 던질 수 있으므로 초보 단계에서는 명시적 분기를 권합니다.

`main.cpp`의 역할 분리는 다음과 같습니다.

- `parse_quantity`: 외부 문자열을 검사하는 입력 경계입니다.
- `Quantity`: 유효한 수량이라는 도메인 의미를 타입으로 표현합니다.
- `make_order_message`: 입력 경계와 출력 메시지를 조합하는 응용 서비스입니다.
- `main`: 예제를 구동하고 결과를 표시하는 프레젠테이션 계층입니다.

오류를 `-1`이나 빈 문자열로 섞지 않으므로 호출자는 실패를 잊기 어렵고, 계층 사이의 계약이 반환형에 드러납니다.

## 값 범주와 수명 지도

- `parsed`처럼 이름이 있는 객체는 lvalue입니다.
- `Quantity{number}`와 함수가 돌려주는 임시 결과는 prvalue입니다. 복사 생략으로 최종 저장소에 직접 생성될 수 있습니다.
- `std::move(movable).value()`는 저장된 문자열을 가리키는 xvalue입니다. 새 문자열로 소유권을 이동할 수 있습니다.
- `const Quantity& quantity{*parsed}`는 `parsed` 내부 객체에 참조 바인딩됩니다. `parsed`보다 오래 사용하면 안 됩니다.
- `std::string_view`는 문자를 소유하지 않습니다. 함수 호출 동안 원본 문자열이 살아 있어야 합니다.

복사와 이동은 타입 구현, 컴파일러와 최적화에 따라 생략될 수 있습니다. 소스 코드의 의미는 “독립 오류를 복사한다”, “문자열 소유권을 이동할 수 있게 한다”로 이해하고, 특정 횟수를 무조건 보장한다고 생각하지 마세요.

## 기계 실행 관점

조건 검사는 보통 값을 메모리나 레지스터로 읽고 비교한 뒤 분기하는 형태가 될 수 있습니다. 함수 호출은 일반 호출 또는 인라인된 코드가 될 수 있고, `expected`의 활성 상태 표지도 읽고 저장해야 할 수 있습니다. 정확한 명령, 객체 배치, 호출 규약은 CPU·ABI·컴파일러·표준 라이브러리·최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않습니다.

## 직접 해보기

1. 먼저 수정하지 않고 빌드해 기준 동작을 확인합니다.
2. `problem.cpp`의 구현을 가리고 TODO만 보고 다시 작성합니다.
3. `safe_divide(-9, 3)` 검증을 추가합니다.
4. `parse_quantity`가 앞뒤 공백을 허용할지 정책을 정하고 구현합니다.
5. 실패 객체를 복사하는 코드와 `std::move(parsed.error())`로 이동하는 코드의 소유권 차이를 설명합니다.

정답 확인은 [CHECKPOINT.md](CHECKPOINT.md)의 초보자 검증 단계를 따르세요.
