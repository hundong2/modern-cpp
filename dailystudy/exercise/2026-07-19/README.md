# 2026-07-19 Modern C++ 일일 연습

Today you will build a tiny welcome-message service while learning one of C++'s
most important design questions: **who owns this object?** The examples are
small, compilable, and written for a learner whose C++ fundamentals are weak.

## 학습 목표

By the end, you should be able to:

- explain ownership and lifetime in plain language
- use `std::unique_ptr` for exactly one owner
- create an object with `std::make_unique`
- transfer ownership with `std::move`
- distinguish an owning smart pointer from a non-owning raw pointer or reference
- inject an interface into a service without manual `new` or `delete`
- use assertions and tests to prove both behavior and ownership transfer

## 아키텍처 한눈에 보기

```text
main (composition root: creates concrete objects)
              |
              | transfers unique ownership
              v
WelcomeService (application rule) ---> MessageSink (interface)
                                          ^
                                          |
                                   RecordingSink (adapter)
```

`WelcomeService` knows what message to create, but not whether messages go to
email, a file, or a test recorder. `main` chooses the concrete adapter and joins
the pieces. This final assembly point is often called the **composition root**.

## 빌드와 실행

From the repository root in PowerShell:

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-07-19 -B dailystudy/exercise/2026-07-19/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-07-19/build
ctest --test-dir dailystudy/exercise/2026-07-19/build --output-on-failure
./dailystudy/exercise/2026-07-19/build/ownership_demo.exe
./dailystudy/exercise/2026-07-19/build/ownership_problem.exe
```

Expected final lines include:

```text
[TESTS] ownership architecture example passed
[TESTS] ownership syntax problem passed
```

## 1. 소유권과 객체 수명

An object's **lifetime** is the time between its construction and destruction.
Its **owner** is responsible for keeping it alive and eventually destroying it.
Unclear ownership can cause leaks (nothing destroys an object) or dangling
pointers (code uses an object after destruction).

Local values are simplest:

```cpp
std::string name{"Mina"};
```

`name` owns its characters and is destroyed automatically at the closing brace.
Prefer values when practical.

## 2. 하나의 소유자: `std::unique_ptr`

Sometimes an object must live inside another object, and its concrete type is
chosen at runtime. `std::unique_ptr<T>` expresses that exactly one pointer owns
the `T` object:

```cpp
auto sink = std::make_unique<RecordingSink>();
```

`auto` lets the compiler infer `std::unique_ptr<RecordingSink>`. `make_unique`
creates the object safely. Do not write a matching `delete`; the smart pointer
destroys the object automatically (RAII).

Unique ownership cannot be copied. It can be transferred:

```cpp
WelcomeService service{std::move(sink)};
```

`std::move` does not move bytes by itself. It permits the destination to take
resources from `sink`. Afterward, `sink` is empty, which the example proves with
`assert(sink == nullptr)`.

## 3. 소유와 관찰 구분하기

In `main.cpp`, `WelcomeService` owns the sink. The temporary `observer` pointer
does not own it; it only lets the test inspect the recorder. Calling `.get()`
never transfers ownership. The observer is valid only while `service` (the real
owner) remains alive.

Use these beginner rules:

- plain value: prefer this when no indirection is needed
- `std::unique_ptr<T>`: one owner, runtime-selected object, automatic cleanup
- `T&` or `T*`: normally observes an object owned elsewhere
- `std::shared_ptr<T>`: only when the design truly requires multiple owners;
  do not use it merely to avoid deciding who owns an object

## 4. 인터페이스 문법

`virtual` enables calls through the `MessageSink` interface to reach the
concrete `RecordingSink`. A polymorphic base class needs a `virtual` destructor
so deletion through `std::unique_ptr<MessageSink>` destroys the entire concrete
object. `override` asks the compiler to verify the derived function matches the
interface. `final` says this concrete adapter is not intended as another base.

The constructor is `explicit`, preventing accidental conversions. Its member
initializer list constructs `sink_` directly. The `const` after `messages()`
promises the function will not modify the recorder. `[[nodiscard]]` asks callers
not to silently ignore its returned value.

## 직접 해보기

1. Build and run both programs unchanged. Read each assertion as a claim.
2. In `problem.cpp`, add `UppercaseFormatter` and make it transform ASCII
   lowercase letters to uppercase. Inject it into a second `Printer`.
3. In `main.cpp`, add a second adapter named `ConsoleSink`. Its `send` function
   should print the recipient and body. Change only the composition root to use
   it; do not change `WelcomeService`.
4. Add a `static int destroyed` counter to `RecordingSink`. Increment it in the
   destructor, put the service in a nested scope, and assert the count becomes
   one after the scope ends. This directly proves RAII cleanup.

Avoid committing the `build/` directory; it contains generated files.

## 값 범주와 기계 실행 관점

이름 있는 `recorder`는 lvalue다. `std::move(recorder)`는 이를 xvalue로 변환할 뿐 실제 이동은 `unique_ptr` 이동 생성자가 수행한다. 이동 뒤 원본은 유효하지만 빈 상태다. `MessageSink` 호출은 동적 타입에 따른 간접 호출이 일반적이고, 소멸 시 `unique_ptr`가 자동으로 delete를 수행한다.

## 초보자 검증 단계

Complete [CHECKPOINT.md](CHECKPOINT.md) without looking at its answer key. Then:

1. predict the output before running the programs;
2. explain aloud why `formatter == nullptr` after construction of `Printer`;
3. complete at least exercise 2 or 3;
4. rebuild and make every assertion and CTest pass.

You understood today's material if you can earn at least 6/8 checkpoint points,
explain which object owns the dependency, and make a new adapter work without
editing the application service.
