# 2026-07-16 Modern C++ 일일 연습

Build a tiny inventory application while learning how Modern C++ types can make
an architecture easier to understand. This lesson assumes weak C++ fundamentals:
read each section, build the examples, then prove your understanding with the
checkpoint.

## 학습 목표

By the end, you should be able to:

- separate input commands, business logic, and output formatting
- model one-of-several choices with `std::variant`
- safely handle every variant choice with `std::visit`
- store key/value state in `std::map`
- understand references, lambdas, `const`, and `[[nodiscard]]`
- trace state changes and add a small feature yourself

## 파일 구성

```text
dailystudy/exercise/2026-07-16/
  README.md       # Lesson and hands-on tasks
  CMakeLists.txt  # Builds two C++20 executables
  main.cpp        # Inventory architecture example
  problem.cpp     # Smaller syntax exercise
  CHECKPOINT.md   # Beginner validation stage
```

## 빌드와 실행

From PowerShell at the repository root:

```powershell
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S dailystudy/exercise/2026-07-16 -B dailystudy/exercise/2026-07-16/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-07-16/build
.\dailystudy\exercise\2026-07-16\build\inventory_demo.exe
.\dailystudy\exercise\2026-07-16\build\syntax_problem.exe
```

Expected highlights:

```text
[CHANGED] keyboard: 0 -> 4
[CHANGED] keyboard: 4 -> 3
[REJECTED] not enough stock for keyboard
final keyboard quantity: 3
[TESTS] inventory exercise passed
```

## 1. 아키텍처 흐름 따라가기

The important path is:

```text
InventoryCommand -> InventoryService -> InventoryEvent -> print_event()
```

`InventoryService` owns the rules and state. It does not use `std::cout`, so it
can be tested without inspecting terminal text. `print_event()` is an adapter at
the edge: it turns a result into human-readable output.

This separation is useful in larger programs. The same service could later be
called from a GUI or web server without rewriting its business rules.

## 2. `std::variant`는 정확히 하나의 선택이다

An inventory command is either `AddStock` or `RemoveStock`:

```cpp
using InventoryCommand = std::variant<AddStock, RemoveStock>;
```

Unlike a class containing many optional fields, a variant holds exactly one of
its listed types. `InventoryEvent` uses the same idea: it is either a successful
`StockChanged` or a `CommandRejected`.

Useful operations:

- `std::holds_alternative<T>(value)` asks which choice is stored.
- `std::get<T>(value)` retrieves a known choice; it throws if the choice is wrong.
- `std::visit(callable, value)` runs matching code for the stored choice.

## 3. 방문자와 람다

A lambda is a small unnamed function:

```cpp
[this](const AddStock& add) { return handle(add); }
```

`[this]` lets the lambda use the current `InventoryService` object. The
`Overloaded` helper combines several lambdas so `std::visit` can select the one
whose parameter matches the stored variant type.

Beginner safety rule: when you add a new type to a variant, add its visitor
case immediately. The compiler then helps reveal every place that must change.

## 4. map, 참조와 `const`

`std::map<std::string, int>` stores an item name and its quantity. This line:

```cpp
int& current = quantities_[command.item];
```

binds `current` as a reference to the value inside the map. Changing `current`
changes the stored quantity. The subscript operator creates a missing value as
zero, which is convenient when adding new stock.

`quantity_of(...) const` promises not to change the service. Its string view
borrows text only for the duration of the call. `std::less<>` enables lookup
with that view without first constructing a temporary `std::string`.

## 5. 놓치기 쉬운 문법

- `{}` gives values a predictable zero/default initialization.
- `const T&` reads an existing object without copying or modifying it.
- `[[nodiscard]]` asks the compiler to warn when a result is ignored.
- `enum class` creates named choices without leaking names into the outer scope.
- `std::span<const T>` is a non-owning view over consecutive read-only elements.
- A range-based `for` loop visits every element without manual indexing.

## 직접 해보기

1. Build and run both executables before changing anything.
2. Predict the three inventory events by hand, then compare with output.
3. Add `AddStock{"keyboard", 2}` to the command array and update your predicted
   final quantity.
4. Add a test proving that removing the exact available quantity leaves zero.
5. In `problem.cpp`, add one incoming and one outgoing transaction, then update
   all three expected totals.
6. Stretch task: define `RenameItem`, add it to `InventoryCommand`, and let the
   compiler show which visitor needs another case.
7. Complete [CHECKPOINT.md](./CHECKPOINT.md) without copying README phrases.

## 값 범주와 기계 실행 관점

`command`는 이름 있는 const 참조이므로 lvalue다. `StockChanged{...}`는 prvalue이며 반환 객체를 직접 구성할 수 있다. `int& current`는 map 내부 저장 위치의 별명이라 수정 시 보통 해당 메모리의 load·연산·store가 일어난다. `std::visit`는 variant의 상태 태그를 확인해 알맞은 방문자를 선택하는 분기가 필요할 수 있다.

## 완료 기준

You are finished when both executables pass, you have made and tested one change,
and you can explain why the service returns events instead of printing directly.
