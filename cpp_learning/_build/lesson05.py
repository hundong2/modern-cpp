"""Lesson 05: 템플릿과 제네릭 프로그래밍."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 05 · 템플릿 — C++의 제네릭

> **한 줄 요약** — 템플릿은 *코드를 생성하는 코드*다. 컴파일러가 사용된 타입마다 그에 맞는 함수/클래스를 자동으로 만들어낸다. C++ 표준 라이브러리의 거의 모든 컨테이너·알고리즘이 템플릿이다. 이걸 모르고 표준 라이브러리 문서를 보면 알아볼 수 없다.

## 1. 함수 템플릿 — 가장 단순한 형태"""))

cells.append(code(r"""#include <iostream>

template <typename T>          // T 는 타입 매개변수. 'class T' 라 적어도 같다.
T max_of(T a, T b) {
    return (a > b) ? a : b;
}

std::cout << max_of(3, 5)          << "\n";   // T = int
std::cout << max_of(3.14, 2.71)    << "\n";   // T = double
std::cout << max_of(std::string("apple"), std::string("banana")) << "\n";

// 명시적으로 T 를 지정할 수도 있다:
std::cout << max_of<double>(3, 5.5) << "\n";  // 3 → 3.0 으로 변환"""))

cells.append(md(r"""**핵심 사실**: 컴파일러는 `max_of(3, 5)` 를 보고 `T = int` 인 버전을 *생성*한다. `max_of(3.14, 2.71)` 을 보면 `T = double` 인 버전을 또 생성한다. 즉, **템플릿은 사용된 만큼 인스턴스화된다**. 이게 빠른 이유 (런타임 타입 체크 없음) 이자 컴파일이 느린 이유 (코드 양이 늘어남).

## 2. 클래스 템플릿"""))

cells.append(code(r"""#include <iostream>

template <typename T>
class Box {
public:
    Box(T v) : value_(std::move(v)) {}
    const T& get() const { return value_; }
    void set(T v) { value_ = std::move(v); }
private:
    T value_;
};

Box<int> bi{42};
Box<std::string> bs{"hello"};

std::cout << bi.get() << ", " << bs.get() << "\n";"""))

cells.append(md(r"""클래스 템플릿은 *반드시* 사용 시 타입을 지정해야 한다 (`Box<int>`). C++17 부터는 생성자 인자로부터 추론하는 **CTAD** 가 추가되었다:

```cpp
Box bi{42};        // C++17: Box<int> 로 추론
Box bs{"hello"};   // Box<const char*> ← 의도한 것과 다를 수 있음. 주의.
```

## 3. 템플릿 매개변수의 종류

### (a) 타입 매개변수 — 위에서 본 것
### (b) 비타입 매개변수 (NTTP) — 컴파일 타임 *값*"""))

cells.append(code(r"""#include <array>
#include <iostream>

template <typename T, std::size_t N>     // N 은 *값*(컴파일 타임)
class FixedArray {
public:
    T& operator[](std::size_t i) { return data_[i]; }
    constexpr std::size_t size() const { return N; }
private:
    T data_[N];
};

FixedArray<int, 5> arr;
for (std::size_t i = 0; i < arr.size(); ++i) arr[i] = static_cast<int>(i*i);
for (std::size_t i = 0; i < arr.size(); ++i) std::cout << arr[i] << " ";
std::cout << "\n";"""))

cells.append(md(r"""표준의 `std::array<T, N>` 가 정확히 이 패턴이다 — 크기를 *컴파일 타임에* 박아두는 배열. 따라서 `std::array<int, 5>` 와 `std::array<int, 6>` 은 *서로 다른 타입*이다.

### (c) 템플릿 템플릿 매개변수 — 템플릿 그 자체를 매개변수로

지금 단계에서는 "그런 게 있다" 만 알면 된다. 표준 라이브러리 내부에서 가끔 본다."""))

cells.append(md(r"""## 4. 특수화 — 특정 타입에 대해서만 다르게 동작시키기"""))

cells.append(code(r"""#include <iostream>
#include <string>

template <typename T>
struct Printer {
    static void print(const T& x) { std::cout << x << "\n"; }
};

// bool 에 대해서만 다른 출력을 원한다 — 명시적 특수화
template <>
struct Printer<bool> {
    static void print(const bool& b) { std::cout << (b ? "yes" : "no") << "\n"; }
};

Printer<int>::print(42);        // 42
Printer<std::string>::print("hi");
Printer<bool>::print(true);     // yes  ← 특수화 버전이 호출됨"""))

cells.append(md(r"""## 5. SFINAE 와 `enable_if` — 복잡한 곳

> "Substitution Failure Is Not An Error" — 템플릿 인자 치환에 실패해도 그 후보를 *조용히 빼버릴 뿐* 컴파일 에러로 만들지 않는다.

이 메커니즘으로 "이 함수는 정수 타입에 대해서만 활성화" 같은 조건부 오버로드를 만들 수 있다."""))

cells.append(code(r"""#include <iostream>
#include <type_traits>

template <typename T,
          typename = std::enable_if_t<std::is_integral_v<T>>>
void only_for_int(T x) {
    std::cout << "정수 버전: " << x << "\n";
}

only_for_int(42);          // OK
// only_for_int(3.14);     // ← 컴파일 에러: double 은 is_integral 아님"""))

cells.append(md(r"""C++20 부터는 같은 일을 훨씬 깔끔한 **concept** 으로 적을 수 있다 — Lesson 06 에서 본격적으로.

## 6. 가변 템플릿 (Variadic Templates) — `printf` 를 안전하게 다시 만들기"""))

cells.append(code(r"""#include <iostream>

// 종료 조건 — 인자 0개일 때
void print() { std::cout << "\n"; }

// 한 개 + 나머지(0개 이상)
template <typename T, typename... Rest>
void print(const T& first, const Rest&... rest) {
    std::cout << first;
    if constexpr (sizeof...(rest) > 0) std::cout << ", ";
    print(rest...);     // 재귀
}

print(1, 2.5, "hello", 'A', true);"""))

cells.append(md(r"""문법 해부:

- `typename... Rest` — *0개 이상의 타입* 매개변수 팩.
- `const Rest&... rest` — *0개 이상의 인자* 팩.
- `rest...` — 팩을 푸는 (expansion) 연산.
- `sizeof...(rest)` — 팩의 크기 (컴파일 타임 상수).
- `if constexpr` — C++17. 조건이 컴파일 타임에 결정되면 *그 분기만* 컴파일에 포함된다. 일반 if 와 달리 분기 둘 다 유효해야 할 필요가 없다.

C++17 부터는 **fold expression** 으로 더 짧게:"""))

cells.append(code(r"""#include <iostream>

template <typename... Args>
void print2(const Args&... args) {
    ((std::cout << args << " "), ...);   // fold expression: args 각각에 대해 (cout << args << " ") 실행
    std::cout << "\n";
}

print2(1, 2.5, "hello", true);"""))

cells.append(md(r"""## 7. 표준 라이브러리 코드 읽는 법 — 한 번 같이 읽어보기

`std::vector` 의 `push_back` 시그니처를 보면:

```cpp
template <class T, class Allocator = std::allocator<T>>
class vector {
public:
    void push_back(const T& value);   // (1) lvalue 버전 — 복사
    void push_back(T&& value);        // (2) rvalue 버전 — 이동
};
```

이 두 줄을 읽을 수 있다는 게 모던 C++을 익혔다는 뜻이다:

- 클래스 템플릿이고, 두 번째 매개변수 `Allocator` 는 디폴트 값이 있다 (기본은 `std::allocator<T>`).
- `push_back` 이 두 개로 오버로드되어 있다 — lvalue 가 들어오면 (1) 이 호출되어 *복사*, rvalue 가 들어오면 (2) 가 호출되어 *이동*. 그래서 `v.push_back(make_widget())` 같은 코드는 자동으로 이동된다.

### `std::move` 의 시그니처

```cpp
template <class T>
constexpr std::remove_reference_t<T>&& move(T&& t) noexcept;
```

겉보기엔 무서워 보이지만 풀어보면:

- T 가 무엇이든 받기 위해 universal reference (`T&&`) 를 사용.
- 반환 타입은 `T` 에서 참조를 떼고 `&&` 를 붙인 타입 — 즉 *rvalue 참조* 로 캐스트.
- `noexcept` — 절대 예외를 던지지 않음. 컴파일러는 이걸 보고 더 공격적인 최적화를 한다.

이 한 줄을 한 번에 알아본다면 표준 라이브러리 문서가 이제부터 읽힌다."""))

cells.append(md(r"""## 8. 템플릿 디버깅 — 에러 메시지 읽는 요령

템플릿 에러 메시지는 길고 무섭다. 처음 보면 패닉. 하지만 패턴이 있다:

1. **첫 줄을 읽지 말고 마지막 의미 있는 줄을 찾아라**. 보통 "candidate template ignored: requirement '...' not satisfied" 같은 줄이 진짜 원인.
2. **타입 이름을 정신없이 길게 보지 말고** 핵심 단어만 본다. `std::__1::vector<std::__1::basic_string<char, ...>>` 는 그냥 `vector<string>` 이다.
3. **`-fdiagnostics-color` 와 `-Werror=template-arg-list-after-template-keyword`** 같은 옵션을 켜라.

가장 중요한 것: **컴파일러는 템플릿이 *사용된 시점에* 검사를 시작한다**. 정의만 있고 사용이 없으면 에러가 안 난다. 이게 헤더에 템플릿을 두는 이유 (분리 컴파일이 어렵다)."""))

cells.append(md(r"""## 9. 실습"""))

cells.append(code(r"""#include <iostream>
#include <vector>
#include <string>

// TODO 1: 컨테이너의 모든 원소를 출력하는 함수 print_all 을 템플릿으로 작성하라.
//   - 어떤 컨테이너든 받을 수 있어야 한다 (vector, list, array, set, ...).
//   - 인자는 const 참조로.
//   - 출력 사이엔 공백, 끝에 줄바꿈.
//
// 힌트: 범위 기반 for 로 순회. const auto& 사용.

template <typename Container>
void print_all(const Container& c) {
    // 채워라
}

// 테스트
std::vector<int> vi{1, 2, 3, 4};
std::vector<std::string> vs{"a", "b", "c"};
print_all(vi);
print_all(vs);


// TODO 2: 두 컨테이너의 모든 쌍의 합을 출력하는 cross_sum 을 템플릿으로 작성하라.
//   - 두 컨테이너의 원소 타입이 다를 수 있다 (예: vector<int> 와 vector<double>).
//   - 두 개의 템플릿 매개변수가 필요하다.

template <typename C1, typename C2>
void cross_sum(const C1& c1, const C2& c2) {
    // 채워라
}

std::vector<int>    a{1, 2};
std::vector<double> b{0.5, 1.5};
cross_sum(a, b);    // 1+0.5, 1+1.5, 2+0.5, 2+1.5"""))

cells.append(md(r"""## 10. 정리

- 템플릿은 코드 생성기. 사용된 타입마다 컴파일 타임에 인스턴스화된다.
- 함수 템플릿은 인자 타입 추론, 클래스 템플릿은 명시 (C++17 이후 CTAD).
- 비타입 매개변수로 *값* 을 컴파일 타임 매개변수로 받을 수 있다 (`std::array<T, N>`).
- 가변 템플릿 + fold expression 으로 안전한 가변 인자 함수.
- SFINAE/enable_if 는 강력하지만 못생겼다 — C++20 의 concept 으로 다음 강의에서 정리.

다음 강의 — Lesson 06: C++20 핵심 기능 (concepts, ranges, modules, coroutines).
"""))

nb = make_notebook(cells, kernel="xcpp17")
save(nb, "/home/claude/cpp_learning/notebooks/05_templates.ipynb")
