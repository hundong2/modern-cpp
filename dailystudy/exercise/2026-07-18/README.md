# 2026-07-18 Modern C++ 일일 연습

Today you will build a tiny checkout application. It teaches a useful design
rule: **make invalid outcomes visible in the type system**. The material assumes
weak C++ fundamentals, so every new piece of syntax is explained before the
exercises.

## 학습 목표

By the end, you should be able to:

- separate domain data, business rules, application service, and output
- use a small struct as a strong type instead of passing every number as `int`
- use `enum class` for a fixed set of named errors
- read and return `std::expected<Value, Error>`
- understand `const`, references, range-based `for`, and `[[nodiscard]]`
- prove success and failure behavior with executable assertions

## 아키텍처 한눈에 보기

```text
Cart + LineItem (domain data)
          |
          v
validate(LineItem) (one-item business rules)
          |
          v
CheckoutService::total (application workflow)
          |
          v
print_receipt (terminal adapter)
```

Keeping these responsibilities separate makes each rule easier to find and
test. The checkout service calculates totals; it does not decide how a terminal
message should look.

## 빌드와 실행

From this directory in PowerShell:

```powershell
$kit = (Resolve-Path ../../../tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
& "$kit/cmake.exe" -S . -B build -G "MinGW Makefiles"
& "$kit/cmake.exe" --build build
& "$kit/ctest.exe" --test-dir build --output-on-failure
./build/checkout_demo.exe
./build/checkout_problem.exe
```

If CMake is unavailable, compile directly from the repository root:

```powershell
./tools/w64devkit/bin/g++.exe -std=c++23 -Wall -Wextra -Wpedantic `
  dailystudy/exercise/2026-07-18/main.cpp -o checkout_demo.exe
```

## 문법 둘러보기

### 1. 강한 타입

`struct Cents { int value{}; };` creates a type distinct from plain `int`.
`Cents{450}` clearly means money. A function asking for `Cents` cannot silently
receive an item quantity. The `{}` initializes the number to zero by default.

### 2. `enum class`로 오류 범위 제한하기

`CheckoutError::invalid_quantity` is a named value. Unlike an old-style `enum`,
its names stay inside `CheckoutError`, and it does not silently become an
integer. That prevents accidental comparisons such as an error code against a
price.

### 3. 값 또는 오류

```cpp
using CheckoutResult = std::expected<Cents, CheckoutError>;
```

An `expected` holds exactly one of two things: the successful `Cents` value or a
`CheckoutError`. Test it with `if (result)`. Read success with `*result` or
`result->value`; read failure with `result.error()`. `std::unexpected(error)`
constructs the failure case. This is C++23.

Unlike an exception, the error is part of the return type and callers cannot
miss that failure is possible. Unlike a magic value such as `-1`, every valid
integer price remains available.

### 4. 참조와 `const`

`const Cart& cart` means “borrow this cart, do not copy it, and do not modify
it.” In the loop, `const LineItem& item` borrows each element for the same
reason. The referenced objects must remain alive while these references are
used.

### 5. 작은 안전 신호

- `[[nodiscard]]` asks the compiler to warn if a result is ignored.
- `override` is not needed today because there is no inheritance.
- `assert(condition)` stops a debug run when a promised fact is false.
- `switch` lists how every named error becomes a user-facing message.

## 안내에 따라 코드 읽기

Before running `main.cpp`, predict the two printed lines. Then trace the valid
cart with paper and write the running total after each item. Notice that
`validate` returns immediately on the first invalid item. This is called
fail-fast behavior: later calculations never use bad data.

## 직접 해보기

1. Change the pen quantity to `4`. Predict the new total, run, and update the
   assertion so it passes.
2. Add `CheckoutError::total_too_large`. Reject totals above 10,000 cents inside
   `CheckoutService::total`, and add its message.
3. Add `struct Percentage { int value{}; };`. Write a function that applies a
   discount only after validation. Decide which invalid percentages to reject.
4. In `problem.cpp`, first read the completed temperature example. Then hide
   `validate_celsius`, retype it from the rules, and add a boundary assertion
   proving that exactly `-273.15` is accepted.
5. Replace one `const LineItem&` with `LineItem` in the loop. Explain what gets
   copied and why the program still produces the same result.

## 값 범주와 기계 실행 관점

`Cents{...}`는 prvalue이고 C++17 이후 반환 객체를 직접 구성할 수 있다. 이름 있는 지역 `item`은 lvalue지만 반환 문맥에서는 암시적 이동 후보가 된다. `expected`는 성공/오류 태그와 저장 공간을 가지는 형태가 일반적이며, 조건 검사는 태그 load와 분기로 구현될 수 있다.

## 초보자 검증 단계

Do not look at `CHECKPOINT.md` until completing these steps:

1. **Prediction:** write both lines from `checkout_demo` before running it.
2. **Trace:** calculate `450 * 2 + 120 * 3` and mark where validation occurs.
3. **Change:** implement exercise 2 and demonstrate one success plus one failure.
4. **Explain aloud:** define strong type, `enum class`, `expected`, and
   `const T&` without reading this page.
5. **Proof:** run both executables and all CTest tests. Passing tests prove the
   code behaves as claimed; your explanations prove you understand why.

Use the rubric and answer key in `CHECKPOINT.md` to score yourself. If you score
below 7/10, repeat the trace and the temperature exercise before moving on.
