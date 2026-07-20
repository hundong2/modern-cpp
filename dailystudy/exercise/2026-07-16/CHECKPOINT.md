# 2026-07-16 초보자 이해 점검

Compilation proves the source is legal C++; it does not prove understanding.
Pass all four stages below. Write answers before checking them by running code.

## 1단계: 설명하기

Answer in your own words:

1. Why does `InventoryService` return an event instead of printing?
2. What two types can an `InventoryCommand` contain?
3. What does `std::visit` do?
4. Why is `current` declared `int&` in the add handler?
5. What promise does the final `const` on `quantity_of(...) const` make?
6. How is `std::span<const Transaction>` different from a copied vector?

Pass rule: five answers are correct, and answer 1 mentions separation or
testability.

## 2단계: 실행하지 않고 추적하기

For the original command array, fill in the table:

| Step | Command | Old quantity | New quantity | Event type |
| --- | --- | ---: | ---: | --- |
| 1 | add 4 keyboards | ? | ? | ? |
| 2 | remove 1 keyboard | ? | ? | ? |
| 3 | remove 10 keyboards | ? | ? | ? |

Also predict the final keyboard quantity. Now run `inventory_demo` and compare.

Pass rule: every cell and the final quantity match the program.

## 3단계: 수정하고 테스트하기

Make these changes:

1. Add `AddStock{"mouse", 6}` and `RemoveStock{"mouse", 2}` to the demo.
2. Print the final mouse quantity.
3. Add an assertion that the final mouse quantity is `4`.
4. Rebuild and run both programs.

Pass rule: compilation has no warnings, every assertion passes, and you can
point to the single map that holds both item quantities.

## 4단계: 디버깅으로 증명하기

Temporarily change this test expectation:

```cpp
assert(inventory.quantity_of("book") == 999);
```

Run the program and observe the failed assertion. Restore `3`, rebuild, and run
again. This proves that you know the tests actually execute.

## 흐름 추적 정답

Check only after writing your prediction:

| Step | Old quantity | New quantity | Event type |
| --- | ---: | ---: | --- |
| 1 | 0 | 4 | `StockChanged` |
| 2 | 4 | 3 | `StockChanged` |
| 3 | 3 | 3 (unchanged) | `CommandRejected` |

The final keyboard quantity is `3`; a rejected command does not mutate state.

## 최종 통과 선언

추가로 `std::visit`가 어떤 variant 후보를 선택하는지, map 원소를 가리키는 `int&`가 왜 lvalue 참조인지 자기 말로 설명해야 한다.

You pass when you can truthfully say: "I can trace a command into a service,
explain how a variant selects one event type, modify the state rules, and use a
test to catch an incorrect expectation."
