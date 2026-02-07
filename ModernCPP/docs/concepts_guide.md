# C++20 Concepts 상세 학습 가이드

## Concepts란?

Concepts는 C++20에서 도입된 기능으로, 템플릿 인자에 대한 제약(constraints)을 표현하는 방법입니다.

## 왜 필요한가?

### 기존 템플릿의 문제점

```cpp
// C++17 이전
template<typename T>
T add(T a, T b) {
    return a + b;
}

// 문제: 컴파일 에러 메시지가 복잡함
add("hello", "world");  // 에러 메시지가 수십 줄...
```

### Concepts를 사용한 해결

```cpp
// C++20
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<Addable T>
T add(T a, T b) {
    return a + b;
}

// 명확한 에러 메시지
add("hello", "world");  // 에러: const char* is not Addable
```

## 기본 문법

### 1. Concept 정의

```cpp
template<typename T>
concept MyConstraint = <boolean expression>;
```

### 2. requires 표현식

```cpp
template<typename T>
concept HasSize = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};
```

## 표준 라이브러리 Concepts

### 1. 타입 카테고리

```cpp
#include <concepts>

template<std::integral T>
void func(T value) {
    // T는 정수 타입만 가능
}

template<std::floating_point T>
void process(T value) {
    // T는 부동소수점 타입만 가능
}
```

### 2. 비교 가능성

```cpp
template<typename T>
    requires std::equality_comparable<T>
bool are_equal(T a, T b) {
    return a == b;
}
```

## 실전 예제

### 예제 1: 컨테이너 제약

```cpp
template<typename C>
concept Container = requires(C c) {
    typename C::value_type;
    typename C::iterator;
    { c.begin() } -> std::same_as<typename C::iterator>;
    { c.end() } -> std::same_as<typename C::iterator>;
};

template<Container C>
void print_all(const C& container) {
    for (const auto& item : container) {
        std::cout << item << " ";
    }
}
```

### 예제 2: 여러 제약 조합

```cpp
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Printable = requires(T t) {
    { std::cout << t } -> std::same_as<std::ostream&>;
};

template<Numeric T>
    requires Printable<T>
void print_number(T value) {
    std::cout << "숫자: " << value << std::endl;
}
```

## 학습 체크리스트

- [ ] `requires` 표현식 이해
- [ ] 간단한 concept 작성
- [ ] 표준 라이브러리 concepts 사용
- [ ] 여러 제약 조합
- [ ] 실제 프로젝트에 적용

## 연습 문제

1. `Sortable` concept 정의하기 (< 연산자 필요)
2. `Serializable` concept 정의하기 (serialize() 메서드 필요)
3. 기존 템플릿 함수를 concept로 개선하기
