# 2026-07-15 초보자 이해 점검

This checkpoint proves whether you understood the material. Do not only ask
"did it compile?" A beginner passes today only when they can explain, trace,
modify, and rebuild.

## 1단계: 쉬운 말로 설명하기

Answer each question in 1 to 3 sentences.

- Why does `parse_command()` return `std::expected<Command, std::string>` instead
  of printing an error directly?
- What is the difference between `std::string` and `std::string_view`?
- Why does `CommandRouter::route()` accept `std::span<const std::string_view>`?
- What does `std::optional<std::size_t>` mean in `first_index_at_least()`?
- How does `RouteScope` prove that RAII cleanup happened after an early return?
- What mistake would the `CommandLogger` concept catch at compile time?

Pass rule: at least five answers connect the concept to a specific line or
function in the code.

## 2단계: 손으로 실행 흐름 추적하기

Trace this batch from `main.cpp`:

```cpp
const std::vector<std::string_view> batch = {
    "start api",
    "status database",
    "stop cache",
};
```

Write down:

- how many commands parse successfully
- how many log messages are stored
- what `active_routes` is after `route()` returns
- why `active_routes` has that final value

Pass rule: your hand-written answer matches the program output.

## 3단계: 작은 변경 직접 만들기

Make at least two of these changes, rebuild, and run again:

- Add `"status worker"` to the demo batch.
- Add `"worker"` to the allowed target list.
- Add `"pause api"` and explain why routing stops early.
- Change `warning_threshold` in `problem.cpp` from `80` to `90`.
- Add one more load sample in `problem.cpp` and update one assertion.

Pass rule: `cmake --build build` succeeds and both executables still run.

## 4단계: 초보자 자기 검증

You pass today if you can say all of this without reading the README:

- A parser should turn text into a useful type or return a clear error.
- `std::string_view` and `std::span` borrow data; they do not own it.
- `std::optional` represents "there may be no value."
- `std::expected` represents "there may be either a value or an error."
- RAII keeps paired cleanup reliable.
- A concept states what operations a template argument must support.

## 검증 기록

## 통과 기준

- 개념 문제 70% 이상을 자료 없이 설명한다.
- 실행 전 결과를 예측하고 실제 출력과 비교한다.
- 한 가지 기능을 직접 추가한 뒤 빌드와 CTest 2개를 모두 통과시킨다.
- lvalue와 prvalue의 예를 코드에서 각각 하나씩 찾는다.

The provided code includes assertions for:

- valid command parsing
- invalid command parsing
- successful routing
- early return on invalid routing
- RAII active route cleanup
- first threshold index lookup
- missing threshold lookup
- average calculation
- empty average input

The learner should still change the code and rebuild. Reading is not enough.
