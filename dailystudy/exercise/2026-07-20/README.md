# 2026-07-20 Modern C++ 일일 연습

오늘은 주문 처리 예제로 **여러 상태 중 정확히 하나만 보관하는 `std::variant`**를 배웁니다. C++ 기초가 약해도 따라올 수 있도록 구조체부터 한 단계씩 설명합니다.

## 오늘의 목표

- `struct`로 관련 데이터를 하나의 타입으로 묶는다.
- `std::variant<A, B, C>`가 A, B, C 중 한 값만 저장한다는 뜻을 설명한다.
- `std::holds_alternative`, `std::get`, `std::get_if`의 차이를 안다.
- `std::visit`와 제네릭 람다로 모든 상태를 처리한다.
- 상태별 필드를 분리하여 잘못된 조합을 줄이는 아키텍처를 이해한다.

## 빌드와 실행

저장소 루트에서 PowerShell로 실행합니다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-07-20 -B dailystudy/exercise/2026-07-20/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-07-20/build
ctest --test-dir dailystudy/exercise/2026-07-20/build --output-on-failure
./dailystudy/exercise/2026-07-20/build/variant_demo.exe
./dailystudy/exercise/2026-07-20/build/variant_problem.exe
```

마지막에 CTest 2개가 모두 통과하고 `[검증 완료]`가 출력되어야 합니다. `build/`는 생성물이므로 커밋하지 않습니다.

## 1. 왜 상태를 타입으로 나눌까?

처음에는 주문을 다음처럼 만들고 싶을 수 있습니다.

```cpp
struct Order {
    bool paid;
    bool shipped;
    int paid_won;
    std::string tracking_number;
};
```

하지만 `paid == false`인데 `paid_won == 25000`인 모순된 객체도 만들 수 있습니다. 배송 전인데 운송장 번호가 들어갈 수도 있습니다.

오늘 예제는 상태별 데이터를 별도 타입으로 나눕니다.

```cpp
struct Pending { int item_count; };
struct Paid { int item_count; int paid_won; };
struct Shipped { std::string tracking_number; };

using OrderState = std::variant<Pending, Paid, Shipped>;
```

`using`은 긴 타입에 읽기 좋은 별명을 붙입니다. 이제 `OrderState`는 세 상태 중 **동시에 하나만** 가집니다. 결제 대기 상태에 결제 금액이나 운송장 번호를 억지로 넣을 자리가 없습니다.

## 2. 값을 만들고 바꾸기

```cpp
OrderState state{Pending{2}};
state = Paid{2, 25'000};
```

중괄호 `{}`는 객체를 초기화합니다. `25'000`의 작은따옴표는 사람이 숫자를 읽기 쉽게 하는 C++ 자릿수 구분자이며 값은 `25000`과 같습니다. 두 번째 줄에서 기존 `Pending` 값은 사라지고 `Paid` 값이 들어갑니다.

## 3. 현재 상태 확인하기

- `std::holds_alternative<Shipped>(state)`: 현재 타입이 맞는지 `bool`로 묻습니다.
- `std::get<Pending>(state)`: 맞는 타입이라고 확신할 때 값을 얻습니다. 타입이 틀리면 예외가 발생합니다.
- `std::get_if<Paid>(&state)`: 맞으면 포인터, 틀리면 `nullptr`를 줍니다. 안전한 조건 분기에 편리합니다.

초보 단계에서는 먼저 `holds_alternative`로 확인하거나 `get_if`의 `nullptr`를 검사하세요.

## 4. 모든 상태 처리: `std::visit`

`std::visit`는 현재 들어 있는 값에 방문자를 호출합니다. `main.cpp`의 람다에서 `auto` 매개변수는 Pending, Paid, Shipped 중 실제 타입을 받습니다.

```cpp
[](const auto& current) -> std::string { /* ... */ }
```

- `[]`: 람다가 바깥 변수를 캡처하지 않습니다.
- `const auto&`: 복사하지 않고 읽기 전용 참조로 받습니다.
- `-> std::string`: 반환 타입을 명시합니다.
- `decltype(current)`: 현재 매개변수의 정확한 타입을 구합니다.
- `std::decay_t`: `const`와 참조를 제거해 원래 구조체 타입과 비교하게 합니다.
- `if constexpr`: 컴파일 시점에 맞는 타입의 가지 하나만 유효하게 만듭니다.

새 상태를 variant에 추가하면 방문 로직도 함께 검토해야 합니다. 상태별 동작이 한곳에 모이므로 누락을 찾기 쉽습니다.

## 5. 아키텍처 관점

```text
입력/이벤트 → OrderState(도메인 상태) → describe 방문자 → 출력 문자열
```

상태 데이터는 화면 출력과 분리되어 있습니다. 나중에 콘솔 대신 GUI나 웹 API를 사용해도 `OrderState`는 그대로 재사용할 수 있습니다. 이처럼 핵심 상태와 출력 방식을 분리하면 변경 범위가 작아집니다.

## 직접 해보기

1. 프로그램을 실행하기 전에 각 `assert`의 결과를 예측합니다.
2. `problem.cpp`의 `message` 본문을 주석만 보고 처음부터 다시 작성합니다.
3. `Cancelled { std::string reason; }` 상태를 `OrderState`에 추가하고 `describe`가 `취소: 고객 요청`을 반환하게 합니다.
4. `Paid` 상태에서만 금액을 반환하는 `paid_amount(const OrderState&) -> int`를 만듭니다. 나머지는 0을 반환합니다.
5. 일부러 `std::get<Paid>`를 Pending 상태에 호출해 어떤 오류가 나는지 관찰한 뒤 되돌립니다.

## 값 범주와 기계 실행 관점

- 이름 있는 `state`는 lvalue, `Pending{2}`와 `Paid{...}`는 prvalue다.
- variant 대입은 기존 활성 객체의 수명을 끝내고 새 후보 객체의 수명을 시작한다.
- 방문자는 상태 태그를 확인해 분기하는 형태가 일반적이지만 실제 어셈블리는 구현과 최적화에 따라 다르다.
- `const auto& current`는 활성 객체를 복사하지 않고 lvalue 참조로 읽는다.

## 초보자 이해 검증

[CHECKPOINT.md](CHECKPOINT.md)의 8점 문제를 자료를 보지 않고 먼저 풉니다. 그다음 직접 해보기 3번 또는 4번을 구현하고 다시 빌드하세요.

다음을 모두 만족하면 오늘 내용을 이해한 것입니다.

- 체크포인트에서 6점 이상을 받았다.
- “variant에는 후보 중 한 값만 존재한다”를 자기 말로 설명할 수 있다.
- `get_if`가 `nullptr`를 반환하는 경우를 설명할 수 있다.
- 새 상태를 추가하고 방문자를 수정하여 CTest 2개를 다시 통과시켰다.
