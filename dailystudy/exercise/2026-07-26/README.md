# 2026-07-26 — C++20 concept로 정책 계약 만들기

## 오늘의 목표

오늘은 할인 계산 정책을 C++20 `concept`와 `requires`로 검사한다. `CheckoutService`는 계산 순서를 담당하고, 정책 객체는 할인 규칙을 담당하며, `IReceiptSink`는 출력 목적지를 추상화한다. 이렇게 역할을 나누면 정책은 컴파일 시간에 조합하고 출력 장치는 실행 시간에 교체할 수 있다.

## 먼저 알아둘 기초 문법

- `int count{3};`은 기본 정수 타입 `int` 변수에 중괄호 초기화를 사용한다. 중괄호는 위험한 축소 변환을 막는 데 도움이 된다.
- 함수 선언의 앞부분은 반환형, 괄호 안은 매개변수다. `total(items)`처럼 이름 뒤에 괄호를 붙여 호출한다.
- `const T&`는 기존 객체를 복사하지 않고 읽기만 하는 참조다. 포인터는 `nullptr`일 수 있지만 참조는 살아 있는 객체에 바인딩되어야 한다.
- `struct`의 기본 접근은 `public`, `class`의 기본 접근은 `private`다. 예제는 경계를 분명히 하려고 `public:`과 `private:`를 직접 쓴다.
- 생성자는 클래스와 이름이 같고 반환형이 없다. `explicit`은 인자 하나짜리 생성자를 통한 뜻밖의 암시적 변환을 막는다.
- `using ItemList = std::vector<Item>;`은 긴 타입에 별칭을 붙인다. `CheckoutService<RatePolicy>`의 `RatePolicy`는 템플릿 인자다.

## 오늘의 Modern C++ 문법

`DiscountPolicy` concept는 “`policy.discount_for(price)`를 호출할 수 있고 결과를 `int`로 바꿀 수 있어야 한다”는 문법 계약이다. 계약을 만족하지 않는 타입을 `CheckoutService<Policy>`에 넣으면 실행 전, 컴파일 단계에서 오류가 난다. concept는 비즈니스 의미 전체를 증명하지는 않는다. 예를 들어 음수 할인을 반환하는 정책도 문법만 맞으면 통과하므로 실행 중 값 검사는 별도로 둔다.

템플릿 정책은 보통 정적 호출로 최적화하기 쉽고, `IReceiptSink`의 가상 함수는 실행 중 출력 대상을 바꾸기 쉽다. 이것이 오늘의 아키텍처 선택이다.

## 값 범주·수명·소유권

- 이름 있는 `items`, `item`, `service`는 다시 가리킬 수 있는 **lvalue**다.
- `Item{"책", 20000}`과 `make_items()`의 결과는 이름 없는 **prvalue**다. 반환 prvalue로 목적 객체를 직접 만들 때 C++17 이후 보장되는 복사 생략이 적용될 수 있다.
- `std::move(items)`는 이동 자체가 아니라 lvalue를 자원을 넘길 수 있는 **xvalue**로 바꾸는 캐스트다. 이동 뒤 객체는 유효하지만 값은 정해져 있지 않을 수 있다.
- `const Item& item`은 lvalue에 바인딩되어 복사를 피한다. `Item&& temporary = Item{"펜", 1000};`처럼 rvalue 참조에 임시 객체를 바인딩하면 참조의 수명까지 임시 수명이 연장된다.
- `ItemList`가 요소를 소유한다. 서비스가 받는 `const ItemList&`와 싱크 포인터는 소유하지 않으므로 원본과 싱크가 호출 동안 살아 있어야 한다.

## 기계 실행 관점

합계 계산에는 보통 멤버 값 로드, 덧셈, 정책 함수 호출, 할인 범위 비교, 조건 분기, 결과 저장이 대응된다. `sink->write(...)`는 가상 함수 테이블을 통한 간접 호출이 될 수 있다. 템플릿 정책 호출은 인라인될 수도 있다. 그러나 실제 명령과 호출 제거 여부는 CPU, ABI, 컴파일러, 표준 라이브러리 구현, 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 빌드와 실행

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 직접 해보기

1. `RatePolicy{10}`을 `RatePolicy{25}`로 바꾸고 결제 금액을 예상한 뒤 실행한다.
2. `FixedPolicy`를 만들어 `discount_for(int)`가 항상 1000을 반환하게 한다.
3. `discount_for`의 이름을 일부러 바꿔 concept 진단을 관찰한 뒤 되돌린다.
4. `problem.cpp`의 TODO를 가리고 직접 구현한다.
5. [CHECKPOINT.md](./CHECKPOINT.md)에서 초보자 검증 단계를 통과한다.

