# 초보자 검증 체크포인트

아래 답을 코드의 실제 식을 가리키며 말할 수 있어야 완료다.

## 1단계 — 기초 문법

- [ ] 세 `#include`가 각각 어떤 표준 기능을 가져오는지 설명한다.
- [ ] `int current{start}`에서 타입, 변수, 초기값, 중괄호 초기화의 장점을 말한다.
- [ ] `make_countdown`의 반환형, 매개변수, 호출식 하나씩을 찾는다.
- [ ] `const T& value() const`의 두 `const`가 각각 무엇을 제한하는지 말한다.
- [ ] 포인터 `printer_ptr`과 참조 `value_ref`의 차이, `&`와 `*`, `->`의 역할을 말한다.
- [ ] `if`, `while`, `for`의 조건식과 다음 반복으로 가는 연산을 찾는다.

## 2단계 — 타입과 클래스

- [ ] `struct`와 `class`의 기본 접근 차이를 말한다.
- [ ] `public` API와 `private handle_`이 나뉜 이유를 설명한다.
- [ ] 생성자에는 반환형이 없음을 코드에서 확인한다.
- [ ] 생성자 매개변수와 `: handle_{handle}` 멤버 초기화 목록을 찾는다.
- [ ] `explicit`이 막는 `Sequence<int> s = handle;`과 허용하는 `Sequence<int> s{handle};`를 비교한다.
- [ ] `using Handle`의 원래 타입과 `<promise_type>`, `Sequence<int>`의 템플릿 인자를 말한다.

## 3단계 — 수명과 값 범주

- [ ] `make_countdown(3)`이 prvalue이고 `sequence`가 lvalue인 이유를 말한다.
- [ ] `std::move(other.handle_)`가 xvalue 식인 점, `other`는 이름이 있어 lvalue 식인 점, 이동 생성자의 `Sequence&&` 바인딩을 구분한다.
- [ ] 복사를 금지하고 이동을 허용한 소유권 이유를 설명한다.
- [ ] 코루틴 프레임이 언제 생성되고 `destroy()`로 언제 수명이 끝나는지 설명한다.
- [ ] prvalue 결과로 `sequence`를 초기화할 때 복사 생략이 어떻게 적용되는지 말한다.
- [ ] 다시 `next()`를 호출한 뒤 이전 `value_ref`를 장기간 보관하면 안 되는 이유를 말한다.

## 4단계 — 아키텍처와 실행 관점

- [ ] `promise_type`, `Sequence`, `ISequencePrinter`의 책임을 한 문장씩 말한다.
- [ ] `resume`, `done`, 가상 `print` 호출이 일반적으로 어떤 상태 로드·비교·분기·간접 호출을 유발할 수 있는지 말한다.
- [ ] 실제 명령은 CPU, ABI, 컴파일러, 최적화 옵션에 따라 달라진다고 함께 설명한다.

## 5단계 — 직접 증명

- [ ] `problem.cpp`에서 1~5의 제곱과 합계 55가 출력됨을 확인한다.
- [ ] `limit`을 3으로 바꾸고 기대 출력과 합계 14를 실행 전에 예측한다.
- [ ] `co_yield i * i`를 `co_yield i * i * i`로 바꾸어 세제곱 수열을 만든다.
- [ ] 변경 후 다시 빌드하고 두 테스트가 통과하는지 확인한다.
