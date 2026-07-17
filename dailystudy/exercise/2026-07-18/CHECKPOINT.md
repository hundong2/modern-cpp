# Beginner Checkpoint and Answer Key

Complete the questions before reading the answer key. Give yourself one point
for each correct answer or demonstrated behavior.

## Questions (10 points)

1. Why is `Cents` safer than using `int` for both money and quantity?
2. What two alternatives can `CheckoutResult` contain?
3. How do you test whether an `std::expected` contains success?
4. How do you read its error?
5. What does `const Cart&` avoid, and what does it forbid?
6. Why does `CheckoutService` not print terminal messages itself?
7. What happens after validation finds quantity zero?
8. Predict the valid cart total without running the program.
9. Add an assertion that proves `validate_celsius(-273.15)` succeeds.
10. Make a cart fail with `blank_name` and show the matching assertion passes.

## Answer key

1. `Cents` is a distinct type, so a function cannot accidentally accept an
   unrelated integer quantity where money is required.
2. A successful `Cents` value or a `CheckoutError`.
3. Use it as a Boolean: `if (result)` or `result.has_value()`.
4. Call `result.error()` only when it is in the failure state.
5. It avoids copying the cart and forbids modifying it through that reference.
6. Calculation and presentation are separate responsibilities. The service can
   later be reused by a GUI, web API, or test without terminal output.
7. Validation returns `invalid_quantity`; the service immediately returns that
   error, so no total is produced.
8. `1260` cents.
9. One valid proof is:

   ```cpp
   const auto absolute_zero = validate_celsius(-273.15);
   assert(absolute_zero && *absolute_zero == -273.15);
   ```

10. One valid proof is:

    ```cpp
    const Cart blank{{"", Cents{100}, 1}};
    const auto result = checkout.total(blank);
    assert(!result && result.error() == CheckoutError::blank_name);
    ```

## Score interpretation

- **9-10:** Ready to use these ideas in a small feature.
- **7-8:** Good start; repeat any missed coding proof.
- **5-6:** Re-read the syntax tour and trace both success and failure by hand.
- **0-4:** Start with `problem.cpp`, change one rule at a time, and rerun it.

The goal is not memorizing punctuation. You understand the lesson when you can
predict behavior, make a safe change, and explain why the types prevent a bug.
