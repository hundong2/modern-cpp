# 2026-07-16 Modern C++ Daily Exercise

Build a tiny inventory application while learning how Modern C++ types can make
an architecture easier to understand. This lesson assumes weak C++ fundamentals:
read each section, build the examples, then prove your understanding with the
checkpoint.

## Learning Goals

By the end, you should be able to:

- separate input commands, business logic, and output formatting
- model one-of-several choices with `std::variant`
- safely handle every variant choice with `std::visit`
- store key/value state in `std::map`
- understand references, lambdas, `const`, and `[[nodiscard]]`
- trace state changes and add a small feature yourself

## Files

```text
dailystudy/exercise/2026-07-16/
  README.md       # Lesson and hands-on tasks
  CMakeLists.txt  # Builds two C++20 executables
  main.cpp        # Inventory architecture example
  problem.cpp     # Smaller syntax exercise
  CHECKPOINT.md   # Beginner validation stage
```

## Build and Run

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

## 1. Follow the Architecture

The important path is:

```text
InventoryCommand -> InventoryService -> InventoryEvent -> print_event()
```

`InventoryService` owns the rules and state. It does not use `std::cout`, so it
can be tested without inspecting terminal text. `print_event()` is an adapter at
the edge: it turns a result into human-readable output.

This separation is useful in larger programs. The same service could later be
called from a GUI or web server without rewriting its business rules.

## 2. `std::variant` Means Exactly One Choice

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

## 3. Visitors and Lambdas

A lambda is a small unnamed function:

```cpp
[this](const AddStock& add) { return handle(add); }
```

`[this]` lets the lambda use the current `InventoryService` object. The
`Overloaded` helper combines several lambdas so `std::visit` can select the one
whose parameter matches the stored variant type.

Beginner safety rule: when you add a new type to a variant, add its visitor
case immediately. The compiler then helps reveal every place that must change.

## 4. Map, References, and `const`

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

## 5. Small Syntax Worth Noticing

- `{}` gives values a predictable zero/default initialization.
- `const T&` reads an existing object without copying or modifying it.
- `[[nodiscard]]` asks the compiler to warn when a result is ignored.
- `enum class` creates named choices without leaking names into the outer scope.
- `std::span<const T>` is a non-owning view over consecutive read-only elements.
- A range-based `for` loop visits every element without manual indexing.

## Hands-On Exercises

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

## Completion Criteria

You are finished when both executables pass, you have made and tested one change,
and you can explain why the service returns events instead of printing directly.
