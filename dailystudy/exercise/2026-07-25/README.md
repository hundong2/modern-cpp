# 2026-07-25 — C++20 ranges/views로 읽기 파이프라인 만들기

## 오늘의 목표

오늘은 `std::ranges`와 `std::views`로 “데이터 저장”과 “조회 규칙”을 분리한다. `OrderRepository`는 주문을 소유하고, `ExpensiveOrderQuery`는 조건에 맞는 주문만 지연 평가하는 조회 파이프라인을 만든다. 뷰는 원소를 새 컨테이너에 복사하지 않는 비소유 관찰자이므로 원본의 수명이 더 길어야 한다.

## 먼저 알아둘 기초 문법

- `int count{3};`은 기본 정수 타입 `int` 변수를 중괄호로 초기화한다. 중괄호 초기화는 위험한 축소 변환을 잘 잡아 준다.
- 함수 선언의 앞 `int`는 반환형, 괄호 안은 매개변수다. `total_price(orders)`처럼 이름 뒤 괄호로 호출한다.
- `const T&`는 기존 객체를 복사하지 않고 읽기만 하는 참조다. 포인터는 주소 또는 `nullptr`를 저장하지만 참조는 살아 있는 객체에 바인딩되어야 한다.
- `struct`의 기본 접근은 `public`, `class`의 기본 접근은 `private`다. 예제는 경계를 분명히 하려고 `public:`과 `private:`를 직접 쓴다.
- 생성자는 클래스와 이름이 같고 반환형이 없다. 생성자 매개변수는 입력이고, `: member_{value}`는 생성자 본문 전에 멤버를 만드는 멤버 초기화 목록이다.
- `explicit`은 인자 하나짜리 생성자를 통한 뜻밖의 암시적 변환을 막는다. `Money price{1200};` 같은 직접 초기화는 올바르지만 `Money price = 1200;`은 금지된다.
- `using OrderList = std::vector<Order>;`는 긴 타입에 별칭을 붙인다. `std::vector<Order>`의 `<Order>`는 템플릿 인자다.

## ranges와 아키텍처

`orders | std::views::filter(predicate) | std::views::transform(projection)`에서 `|`는 파이프 연산자다. `filter`는 조건을 통과한 원소만 보이고, `transform`은 보이는 값을 다른 형태로 투영한다. 이들은 보통 즉시 전체 결과를 저장하지 않고 반복할 때 원소별로 계산한다. 따라서 조회 정책을 조합하기 쉽고 불필요한 중간 컨테이너를 줄일 수 있다.

`OrderRepository`는 소유권과 변경을 담당한다. `ExpensiveOrderQuery`는 조회 정책만 담당하고 저장소를 `const` 참조로 관찰한다. 인터페이스 `IOrderSink`는 출력 목적지를 바꿀 수 있는 포트이며, 호출부는 구체 출력 클래스 대신 추상 계약에 의존한다.

## 값 범주·수명·소유권

- 이름이 있는 `repository`, `order`, `view`는 다시 식별할 수 있는 **lvalue**다.
- `Order{1, Money{1500}}`와 `make_sample_orders()`의 결과는 이름 없는 **prvalue**다. 반환 prvalue로 목적 객체를 직접 만들 때 C++17 이후 보장되는 복사 생략이 적용될 수 있다.
- `std::move(order)`는 이동 자체가 아니라 이름 있는 객체를 **xvalue**로 바꾸는 캐스트다. 이동 생성자가 선택되면 자원을 넘길 수 있다.
- `const Order& order`는 기존 lvalue에 바인딩된다. `Order&& temporary = Order{...};`는 prvalue로 물질화된 임시 객체에 rvalue 참조를 바인딩하며, 그 지역 참조의 수명까지 임시 수명이 연장된다.
- 뷰가 참조하는 저장소가 먼저 파괴되면 뷰는 매달린 참조가 된다. 예제의 조회 객체와 뷰는 저장소보다 짧게 산다.

## 기계 실행 관점

반복 중에는 일반적으로 반복자 상태 로드, 가격 로드, 임계값 비교, 조건 분기, 누적값 저장이 일어난다. `sink.write(...)`는 가상 함수 테이블을 통한 간접 호출이 될 수 있다. 다만 인라이닝·벡터화·가상 호출 제거 여부와 실제 명령 모양은 CPU, ABI, 컴파일러, 표준 라이브러리 구현, 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 빌드와 실행

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 직접 해보기

1. `minimum{1000}`을 `minimum{1500}`으로 바꾸고 비교 연산자 `>=`의 경계가 어떻게 달라지는지 확인한다.
2. 파이프에 `std::views::take(1)`을 덧붙여 첫 결과만 소비한다.
3. `problem.cpp`의 TODO를 먼저 가리고 직접 구현한다.
4. 뷰를 저장소보다 오래 보관하는 함수를 일부러 작성해 왜 수명 오류인지 설명한다.

완료 검증은 [CHECKPOINT.md](./CHECKPOINT.md)를 따른다.
