# 2026-07-16 Beginner Validation Checkpoint

Compilation proves the source is legal C++; it does not prove understanding.
Pass all four stages below. Write answers before checking them by running code.

## Stage 1: Explain

Answer in your own words:

1. Why does `InventoryService` return an event instead of printing?
2. What two types can an `InventoryCommand` contain?
3. What does `std::visit` do?
4. Why is `current` declared `int&` in the add handler?
5. What promise does the final `const` on `quantity_of(...) const` make?
6. How is `std::span<const Transaction>` different from a copied vector?

Pass rule: five answers are correct, and answer 1 mentions separation or
testability.

## Stage 2: Trace Without Running

For the original command array, fill in the table:

| Step | Command | Old quantity | New quantity | Event type |
| --- | --- | ---: | ---: | --- |
| 1 | add 4 keyboards | ? | ? | ? |
| 2 | remove 1 keyboard | ? | ? | ? |
| 3 | remove 10 keyboards | ? | ? | ? |

Also predict the final keyboard quantity. Now run `inventory_demo` and compare.

Pass rule: every cell and the final quantity match the program.

## Stage 3: Modify and Test

Make these changes:

1. Add `AddStock{"mouse", 6}` and `RemoveStock{"mouse", 2}` to the demo.
2. Print the final mouse quantity.
3. Add an assertion that the final mouse quantity is `4`.
4. Rebuild and run both programs.

Pass rule: compilation has no warnings, every assertion passes, and you can
point to the single map that holds both item quantities.

## Stage 4: Debugging Proof

Temporarily change this test expectation:

```cpp
assert(inventory.quantity_of("book") == 999);
```

Run the program and observe the failed assertion. Restore `3`, rebuild, and run
again. This proves that you know the tests actually execute.

## Answer Key for the Trace

Check only after writing your prediction:

| Step | Old quantity | New quantity | Event type |
| --- | ---: | ---: | --- |
| 1 | 0 | 4 | `StockChanged` |
| 2 | 4 | 3 | `StockChanged` |
| 3 | 3 | 3 (unchanged) | `CommandRejected` |

The final keyboard quantity is `3`; a rejected command does not mutate state.

## Final Pass Statement

You pass when you can truthfully say: "I can trace a command into a service,
explain how a variant selects one event type, modify the state rules, and use a
test to catch an incorrect expectation."
