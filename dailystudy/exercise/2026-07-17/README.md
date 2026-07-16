# 2026-07-17 Modern C++ Daily Exercise

Today you will build a tiny library-lending program. The goal is not a large
application: it is to see how C++ syntax supports a clean architecture. This
lesson starts from first principles and includes executable checks, so you can
prove what you understood instead of only reading about it.

## Learning goals

By the end, you should be able to:

- separate business rules from data storage and terminal output
- explain an interface, an implementation, and dependency injection
- use `std::optional<T>` when a value may be absent
- read basic class syntax: `public`, `private`, constructors, and members
- understand `virtual`, `override`, `final`, references, and object lifetimes
- use `std::move` only when an object is being handed to a new owner

## Files

```text
dailystudy/exercise/2026-07-17/
  README.md       # Concepts, build steps, and hands-on tasks
  CMakeLists.txt  # Builds two warning-enabled C++20 programs
  main.cpp        # Layered library architecture and executable checks
  problem.cpp     # Smaller std::optional and parsing exercise
  CHECKPOINT.md   # Beginner validation and answer key
```

## Build and run

From PowerShell at the repository root:

```powershell
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S dailystudy/exercise/2026-07-17 -B dailystudy/exercise/2026-07-17/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-07-17/build
.\dailystudy\exercise\2026-07-17\build\library_demo.exe
.\dailystudy\exercise\2026-07-17\build\optional_problem.exe
```

The last lines should be:

```text
[TESTS] library architecture example passed
[TESTS] optional syntax exercise passed
```

## 1. Follow the architecture

The program has four responsibilities:

```text
main / print_result
        |
        v
LendingService  --->  BookRepository (interface)
                            ^
                            |
                 InMemoryBookRepository
```

- `Book` is a domain value: it describes the data the program cares about.
- `LendingService` owns borrowing and returning rules.
- `BookRepository` says which storage operations the service needs.
- `InMemoryBookRepository` performs those operations with a `std::vector`.
- `main` assembles the objects and displays results.

The service does not know whether books live in a vector, file, or database.
Depending on the small `BookRepository` interface keeps the business rules
independent from that storage decision. This is dependency inversion in a very
small form.

## 2. Read the class syntax

```cpp
class LendingService {
public:
    explicit LendingService(BookRepository& repository)
        : repository_{repository} {}

private:
    BookRepository& repository_;
};
```

`public` names operations callers may use. `private` hides implementation
details. The constructor initializer list after `:` initializes the member.
`explicit` prevents an accidental implicit conversion.

`repository_` is a reference: it is another name for an existing repository,
not a copy and not an owned object. Therefore the repository must remain alive
longer than the service. In `main`, construction order makes that true.

## 3. Understand the interface

`BookRepository` has `virtual` functions, which allow a call through the base
type to run the chosen implementation. `= 0` makes a function pure virtual, so
the interface itself cannot be constructed. The derived class uses `override`
to ask the compiler to verify that its function really matches the interface.
`final` says this small implementation is not intended as another base class.

The virtual destructor matters when a derived object is ever destroyed through
a base pointer. `= default` asks the compiler to generate its normal behavior.

## 4. Model absence with `std::optional`

Searching can succeed or fail:

```cpp
std::optional<Book> book = repository_.find_by_id(id);
if (!book) {
    return {LoanStatus::not_found, "book was not found"};
}
book->is_borrowed = true;
```

An optional either contains one `Book` or is empty. The `if` checks for a
value, `book->member` accesses the contained object's member, and `*book`
accesses the object itself. Never use `*` or `->` before proving a value exists.

`problem.cpp` also demonstrates `value_or(default)` and `std::nullopt`. It uses
`std::from_chars`, a non-throwing conversion well suited to validating input.

## 5. Copies, moves, and `const`

The repository returns a `Book` copy. The service changes that local copy and
calls `save`, so state changes are explicit. `std::move(*book)` says the local
book may transfer its string storage into `save`; do not read that moved-from
book afterward.

`find_by_id(...) const` promises not to change the repository. Parameters such
as `std::string_view` cheaply borrow text for the duration of a call. A
`const auto` local cannot be reassigned after initialization.

## Hands-on exercises

1. Build and run both programs before editing them.
2. Predict all four `[RESULT]` lines, then compare your answer with the output.
3. Add a test that returning `cpp-101` succeeds after it has been borrowed.
4. Add a `total_books() const` operation to the interface and implementation.
5. In `problem.cpp`, reject counts larger than 100 and add checks for `"101"`
   and `"100"`.
6. Stretch task: create a second repository implementation that always returns
   `std::nullopt`; prove that `LendingService` needs no changes to use it.
7. Complete [CHECKPOINT.md](./CHECKPOINT.md) before viewing its answer key.

## Completion criteria

You are done when both executables pass, your new checks also pass, and you can
explain why the service stores a repository reference instead of constructing
`InMemoryBookRepository` internally.
