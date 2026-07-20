# 2026-07-15 Modern C++ 일일 연습

Today you will build a tiny command router for a server-like program. The goal is
to practice both architecture and syntax:

- Keep parsing, business rules, and output adapters separate.
- Use values and small types instead of passing raw strings everywhere.
- Use `std::string_view`, `std::span`, `std::optional`, and `std::expected`.
- Use RAII so cleanup happens automatically.
- Use a C++20 concept to describe what a logger adapter must provide.

This material is written for a developer whose C++ fundamentals are still weak.
Read slowly, build the code, run it, then change it.

## 파일 구성

```text
dailystudy/exercise/2026-07-15/
  CMakeLists.txt   # Builds the two examples.
  README.md        # Concepts, build steps, and exercises.
  main.cpp         # Architecture example: command routing.
  problem.cpp      # Syntax drill: optional lookup and span-based averaging.
  CHECKPOINT.md    # Beginner validation stage.
```

## 빌드와 실행

From PowerShell:

```powershell
cd D:\workspace\modern-cpp\dailystudy\exercise\2026-07-15
$env:PATH='D:\workspace\modern-cpp\tools\w64devkit\bin;' + $env:PATH
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
.\build\exercise_router.exe
.\build\exercise_problem.exe
```

If you already have another compiler, the same CMake project should still work.

## 개념 1: 작은 타입이 아키텍처를 단순하게 만든다

Beginner C++ code often passes plain strings through the whole program. That
works at first, but every function then has to remember the format.

This lesson uses these small types:

```cpp
enum class CommandKind { start, stop, status };

struct Command {
    CommandKind kind;
    std::string target;
};
```

After parsing succeeds, the rest of the program receives a `Command`, not a raw
line such as `"start api"`. That means business logic can focus on decisions,
not text splitting.

## 개념 2: `std::string_view`는 문자열을 빌려 본다

`std::string_view` is a non-owning view of characters. It does not copy the
characters and it does not keep them alive.

Use it for function parameters when the function only needs to read text:

```cpp
std::expected<Command, std::string> parse_command(std::string_view line);
```

Beginner rule: a view is safe when the original string or string literal is
still alive. Do not store a `std::string_view` for later unless you are sure the
original text will outlive it.

## 개념 3: `std::expected`는 실패를 타입에 드러낸다

Parsing can fail. Instead of guessing with a partially filled object, the
function returns either:

- a valid `Command`
- or a clear error message

```cpp
auto parsed = parse_command("restart api");
if (!parsed) {
    std::cout << parsed.error() << '\n';
}
```

This is easier to test than printing from inside the parser. The parser returns
facts; the caller decides how to display them.

## 개념 4: RAII는 정리 책임을 객체에 둔다

`RouteScope` increments an active counter in its constructor and decrements it in
its destructor. The destructor runs when the object leaves scope, even if the
function returns early because of an error.

This is RAII: Resource Acquisition Is Initialization.

Beginner translation: put "open/start/increment" in the constructor and
"close/stop/decrement" in the destructor when the two operations must always be
paired.

## 개념 5: concept는 요구 동작을 선언한다

The router does not care whether logs go to memory, a file, or the console. It
only requires a logger with this shape:

```cpp
template <typename Logger>
concept CommandLogger = requires(Logger logger, std::string_view message) {
    { logger.write(message) } -> std::same_as<void>;
};
```

This gives a clear compile-time error if you pass the wrong kind of object.

## 직접 해보기

1. Build and run both executables.
2. In `main.cpp`, add the command `"status worker"` to the batch and predict the
   new success count before running it.
3. Change the allowed targets so `"worker"` is accepted, then rebuild.
4. Add a bad command such as `"pause api"` and read the error message.
5. In `problem.cpp`, add a new server load value and update the tests.
6. Complete [CHECKPOINT.md](./CHECKPOINT.md) without looking at the answer first.

## 값 범주와 기계 실행 관점

- 이름 있는 `logger`, `report`, `parsed`는 lvalue이며 저장 위치가 있다.
- 파싱 함수가 반환하는 `expected` 임시값은 prvalue이며 지역 객체를 직접 초기화할 수 있다.
- 반복문은 일반적으로 비교·조건 분기·증가로 구현되지만 정확한 명령은 최적화와 CPU에 따라 달라진다.
- 템플릿으로 Logger 타입이 확정되므로 가상 간접 호출 없이 인라인될 가능성이 있다.

## 완료 기준

You are done when:

- both executables build
- both executables run and print passing test messages
- you can explain `std::string_view`, `std::span`, `std::optional`,
  `std::expected`, RAII, and concepts in your own words
- you can change one rule, rebuild, and explain why the output changed
