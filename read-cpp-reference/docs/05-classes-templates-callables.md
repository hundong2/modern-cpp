# 05. 클래스, 템플릿, 호출 가능 객체

## class와 struct

둘 다 클래스 타입을 정의합니다. 차이는 기본 접근 권한과 기본 상속 권한입니다.

- `struct`: 기본 `public`
- `class`: 기본 `private`

```cpp
class Counter {
public:
    explicit Counter(int start) : value_(start) {}
    int value() const noexcept { return value_; }
    void increment() noexcept { ++value_; }
private:
    int value_;
};
```

- 생성자는 반환 타입이 없고 객체를 초기화합니다.
- member initializer list `: value_(start)`는 멤버에 대입하는 것이 아니라 초기화합니다.
- 멤버는 선언된 순서로 초기화됩니다. 목록에 쓴 순서가 아닙니다.
- `this`는 현재 객체를 가리키며 `*this`는 현재 객체입니다.

## 접근 제어와 캡슐화

`public`은 사용자 인터페이스, `private`은 구현 세부입니다. `protected`는 파생 클래스에도
보입니다. 캡슐화는 단순히 숨기는 것이 아니라 “유효한 상태만 만들고 유지”하게 합니다.

## 상속과 가상 함수

```cpp
struct Shape {
    virtual ~Shape() = default;
    virtual double area() const = 0;
};

struct Circle final : Shape {
    double area() const override;
};
```

- `virtual`: base 포인터/참조를 통한 호출이 동적 타입을 따름
- `= 0`: pure virtual, 클래스를 추상 클래스로 만듦
- `override`: base 가상 함수를 정확히 재정의하는지 검사
- `final`: 더 이상의 재정의/상속 금지
- 다형적으로 삭제할 base는 보통 virtual destructor가 필요

상속이 “is-a”가 아니면 멤버로 포함하는 composition을 우선 검토합니다.

## overload, override, hiding

- overload: 같은 scope의 같은 이름, 다른 매개변수 목록
- override: 파생 클래스가 base virtual 함수를 재정의
- name hiding: 파생 클래스의 같은 이름이 base overload 집합을 가릴 수 있음

```cpp
struct Derived : Base {
    using Base::print; // base의 overload도 현재 scope로 가져옴
    void print(int);
};
```

## 연산자 overload

`operator==`, `operator()`, `operator<<` 같은 함수 이름입니다. 연산자의 우선순위/피연산자
개수/short-circuit 성질을 새로 만들 수는 없습니다. 타입의 자연스러운 의미를 유지하세요.

## 템플릿의 핵심

템플릿은 타입/값을 매개변수화한 코드 생성 규칙입니다.

```cpp
template<class T>
T maximum(const T& a, const T& b) {
    return a < b ? b : a;
}
```

호출 시 `T`를 추론하거나 `maximum<int>(1, 2)`처럼 명시합니다. 컴파일러는 사용한 구체
인자로 specialization을 인스턴스화합니다. 템플릿 정의가 보통 헤더에 있어야 하는 이유입니다.

## dependent name과 `typename`/`template`

템플릿 인자에 따라 의미가 달라지는 이름이 dependent name입니다.

```cpp
template<class Container>
void use() {
    typename Container::value_type value{};
    // value_type이 타입임을 typename으로 알림
}
```

dependent 객체 뒤에서 멤버 템플릿을 지칭할 때는 `object.template call<T>()` 형태가 필요할
수 있습니다. C++20에서 일부 `typename` 요구가 완화됐어도 이 개념은 문서 해독에 필요합니다.

## specialization, SFINAE, type traits, concepts

- full/partial specialization: 특정 템플릿 인자에 별도 정의 제공
- SFINAE: 템플릿 치환 실패를 즉시 오류로 만들지 않고 후보에서 제거
- type trait: `<type_traits>`의 컴파일 타임 타입 질문/변환
- concept(C++20): 템플릿 인자 요구 조건에 이름을 붙이고 제약

```cpp
template<class T>
requires std::integral<T>
T twice(T value) {
    return value + value;
}
```

cppreference의 `T must meet the requirements of ...`는 구버전 이름 있는 요구 조건일 수 있고,
`template<std::input_iterator I>`는 실제 concept 제약일 수 있습니다.

## callable

`f(args...)` 또는 `std::invoke(f, args...)`로 호출할 수 있는 것을 묶어 callable이라 합니다.

- 일반 함수/함수 포인터
- 함수 객체: `operator()`를 가진 객체
- lambda
- 멤버 함수 포인터/데이터 멤버 포인터
- `std::function`에 저장된 대상

```cpp
auto lambda = [](int x) { return x * 2; };

struct Doubler {
    int operator()(int x) const { return x * 2; }
};
```

`Callable`, `Predicate`, `Compare`, `Hash` 같은 문서 요구 조건은 “어떤 형태로 호출되고 어떤
결과/법칙을 만족해야 하는가”를 함께 봅니다. `Compare`는 단지 bool을 반환하는 것뿐 아니라
strict weak ordering을 만족해야 합니다.

## lambda 문법

```cpp
[capture](parameters) mutable noexcept -> ReturnType { body }
```

- `[]`: capture 없음
- `[x]`: x를 값으로 복사
- `[&x]`: x를 참조로 capture
- `[=]`, `[&]`: 기본 capture
- `[this]`: 현재 객체 포인터 capture
- `[*this]`(C++17): 현재 객체를 값으로 capture
- `mutable`: 값 capture를 lambda의 const 호출 연산자 안에서 수정 허용
- generic lambda: `[](auto x) {...}`

참조 capture의 대상이 lambda보다 먼저 죽으면 dangling입니다.

## parameter pack과 fold expression

```cpp
template<class... Ts>
auto sum(Ts... values) {
    return (values + ...); // C++17 unary right fold
}
```

- `Ts...`: 타입 parameter pack
- `values...`: 함수 parameter pack
- pack expansion: pack의 각 원소에 패턴을 반복
- fold expression: 이항 연산자로 pack을 접음

빈 pack에 허용되는 연산/초깃값 규칙을 확인해야 합니다.

다음: [표준 라이브러리 어휘](06-standard-library-vocabulary.md)
