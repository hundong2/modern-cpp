# 초보자 이해 점검

Do this stage without looking back at the README. Write your answers first,
then run the validation task, and only afterward compare with the answer key.

## A단계: 자기 말로 설명하기

1. Which class owns the rule that an already-borrowed book cannot be borrowed?
2. Why does `find_by_id` return `std::optional<Book>` instead of a plain `Book`?
3. What promise does `override` ask the compiler to check?
4. What lifetime rule must be true for `LendingService::repository_`?
5. Why is terminal printing kept outside `LendingService`?

## B단계: 실행 전에 예측하기

Given an available book with id `cpp-101`, predict the statuses from these
calls in order:

```cpp
service.borrow("cpp-101");
service.borrow("cpp-101");
service.return_book("cpp-101");
service.return_book("cpp-101");
```

Also predict these parsing results:

```text
parse_positive_count("7")
parse_positive_count("-2")
parse_positive_count("7x")
```

## C단계: 코드로 증명하기

Add the two missing `return_book("cpp-101")` calls from Part B to `main.cpp`.
Add `require` checks for their exact statuses. Rebuild and run both programs.

You pass the beginner validation stage only if:

- you predicted all four loan statuses before running the code
- your new checks compile and print no `[FAILED]` line
- both programs still end with their `[TESTS]` line
- you can explain one answer from Part A without reading it

## 자기 채점

- **5/5 Part A and proof passes:** ready for the stretch task
- **3-4/5 and proof passes:** understood the core; reread only missed topics
- **0-2/5 or proof fails:** trace one call through service, optional, and save,
  then retry the checkpoint

## 정답표: 먼저 시도한 뒤 열기

## 추가 통과 질문

`*found`를 반환할 때는 왜 복사가 일어나고 `std::move(book)`에서는 왜 이동 생성자가 선택될 수 있는지 lvalue와 xvalue 용어로 설명한다.

1. `LendingService` owns the borrowing rule.
2. A search may find no matching id; an empty optional represents that normal
   outcome without inventing a fake `Book` or special id.
3. It verifies that the derived function matches a virtual base function.
4. The referenced repository must outlive the service.
5. Keeping output separate makes rules reusable and directly testable.

Part B loan statuses, in order: `borrowed`, `already_borrowed`, `returned`,
`already_available`. Parsing `"7"` contains `7`; `"-2"` and `"7x"` are empty.
