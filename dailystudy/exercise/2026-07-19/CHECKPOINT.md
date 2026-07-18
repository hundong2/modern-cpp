# Beginner Checkpoint: Ownership and Dependency Injection

Answer before reading the key. Score one point per question.

## Questions

1. In one sentence, what does an object's owner do?
2. Who owns the `RecordingSink` after `WelcomeService service{std::move(recorder)}`?
3. What value should `recorder` hold after its ownership was transferred?
4. Why does `MessageSink` have a `virtual` destructor?
5. Does `observer = recorder.get()` make `observer` an owner?
6. What does RAII give us when `WelcomeService` is destroyed?
7. Why is `std::unique_ptr` a better fit than `std::shared_ptr` here?
8. Where can `RecordingSink` be replaced by `ConsoleSink` without changing the
   business workflow?

## Practical proof

Without consulting the README, draw this ownership statement and fill the gaps:

```text
________ owns WelcomeService as a local value.
WelcomeService uniquely owns ________ through a ________ pointer.
The observer pointer owns ________ objects.
```

Next, implement one hands-on exercise, rebuild, and run CTest. A verbal answer
alone is not enough: successful compilation and assertions are part of the
validation.

## Answer key

1. The owner keeps an object alive and is responsible for its destruction.
2. The `WelcomeService` object, through its `sink_` member.
3. It is empty, so comparison with `nullptr` is true.
4. Deleting through the base pointer must also run the derived destructor.
5. No. `.get()` returns a non-owning pointer and transfers nothing.
6. Its `unique_ptr` automatically destroys the sink; no manual `delete` is used.
7. There is exactly one intended owner, so shared reference counting would hide
   the simpler lifetime design and add unnecessary overhead.
8. In `main`, the composition root where concrete objects are constructed.

Practical blanks: `main`; `MessageSink`; `std::unique_ptr<MessageSink>`; zero.

## Result

- **8/8 plus practical proof:** ready to use unique ownership in small designs.
- **6-7/8:** understood; revisit the missed syntax and rerun the example.
- **0-5/8:** reread README sections 1-3, trace the owner after each statement,
  and retry before moving on.
