# 2026-07-18 Modern C++ Daily Exercise

Today you will build a tiny checkout application. It teaches a useful design
rule: **make invalid outcomes visible in the type system**. The material assumes
weak C++ fundamentals, so every new piece of syntax is explained before the
exercises.

## Learning goals

By the end, you should be able to:

- separate domain data, business rules, application service, and output
- use a small struct as a strong type instead of passing every number as `int`
- use `enum class` for a fixed set of named errors
- read and return `std::expected<Value, Error>`
- understand `const`, references, range-based `for`, and `[[nodiscard]]`
- prove success and failure behavior with executable assertions

## Architecture at a glance

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

## Build and run

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

## Syntax tour

### 1. Strong types

`struct Cents { int value{}; };` creates a type distinct from plain `int`.
`Cents{450}` clearly means money. A function asking for `Cents` cannot silently
receive an item quantity. The `{}` initializes the number to zero by default.

### 2. Scoped errors with `enum class`

`CheckoutError::invalid_quantity` is a named value. Unlike an old-style `enum`,
its names stay inside `CheckoutError`, and it does not silently become an
integer. That prevents accidental comparisons such as an error code against a
price.

### 3. A value or an error

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

### 4. References and `const`

`const Cart& cart` means “borrow this cart, do not copy it, and do not modify
it.” In the loop, `const LineItem& item` borrows each element for the same
reason. The referenced objects must remain alive while these references are
used.

### 5. Small safety signals

- `[[nodiscard]]` asks the compiler to warn if a result is ignored.
- `override` is not needed today because there is no inheritance.
- `assert(condition)` stops a debug run when a promised fact is false.
- `switch` lists how every named error becomes a user-facing message.

## Guided code reading

Before running `main.cpp`, predict the two printed lines. Then trace the valid
cart with paper and write the running total after each item. Notice that
`validate` returns immediately on the first invalid item. This is called
fail-fast behavior: later calculations never use bad data.

## Hands-on exercises

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

## Beginner validation stage

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
