"""Lesson 04: 헷갈리기 쉬운 문법 (const correctness, 초기화의 다섯 얼굴, 오버로딩)."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 04 · 헷갈리기 쉬운 C++ 문법

> 다른 언어를 하다가 C++로 오면 *동작은 같아 보이는데 결과가 다른* 코드를 자주 만난다. 이 강의에서 그런 함정 다섯 가지를 한꺼번에 정리한다.

## 1. const correctness

`const` 의 의미는 한 줄로: **이 값을 통해서는 수정하지 않겠다는 약속**. 컴파일러는 이 약속을 어기는 코드를 거부한다.

### const 가 붙는 다섯 위치"""))

cells.append(code(r"""#include <string>

class Buffer {
public:
    Buffer(std::string s) : data_(std::move(s)) {}

    // 1. 멤버 함수 자체에 붙는 const — '나는 *this 를 안 바꾼다'
    std::size_t size() const { return data_.size(); }

    // 2. 반환 타입에 붙는 const — 호출자에게 수정을 못 하게 한다
    const std::string& get() const { return data_; }

    // 3. 인자에 붙는 const — 함수 안에서 그 인자를 수정 안 한다
    void append(const std::string& s) { data_ += s; }

private:
    // 4. 멤버 자체에 const — 한 번 정해지면 못 바꿈 (생성자 초기화 리스트로만 설정 가능)
    // const int id_ = 42;

    std::string data_;
};

const Buffer b{"hello"};
// b.append(" world");   // ← 컴파일 에러: const 객체에는 const 멤버 함수만 호출 가능
b.size();                // OK
b.get();                 // OK"""))

cells.append(md(r"""5번째 위치 — **포인터의 const 위치 두 가지** (Lesson 02 에서 본 것):

```cpp
const int*       p1;   // 가리키는 값 const
int* const       p2;   // 포인터 자체 const
const int* const p3;   // 둘 다
```

읽는 요령: `*` 의 위치를 기준으로 좌측은 가리키는 값, 우측은 포인터 자체.

### const 멤버 함수와 const 오버로딩

흔하게 보는 패턴:

```cpp
class Vector {
    int* data_;
public:
    int& operator[](size_t i)         { return data_[i]; }   // 비-const 객체용
    const int& operator[](size_t i) const { return data_[i]; }   // const 객체용
};
```

같은 이름의 두 함수 — `const` 차이로 오버로드된다. const 객체에 호출하면 두 번째가, 아니면 첫 번째가 선택된다."""))

cells.append(md(r"""## 2. 초기화의 다섯 가지 얼굴

C++ 초기화는 *문법이 너무 많다*. 다섯 가지를 다 알아둬야 다른 사람의 코드를 읽을 수 있다."""))

cells.append(code(r"""#include <iostream>
#include <vector>
#include <string>

// (1) Default initialization — int, double 같은 내장 타입은 *초기화 안 됨!*
int x1;                          // 쓰레기 값
std::cout << "(1) x1 = " << x1 << "  ← 쓰레기 값일 수 있음\n";

// (2) Value initialization — () 또는 = T()
int x2 = int();                  // 0
int x3{};                        // 0  (C++11 brace init)
std::cout << "(2) x2=" << x2 << ", x3=" << x3 << "\n";

// (3) Direct initialization — ()
std::string s1("hello");
std::cout << "(3) s1 = " << s1 << "\n";

// (4) Copy initialization — =
std::string s2 = "world";
std::cout << "(4) s2 = " << s2 << "\n";

// (5) Brace (uniform) initialization — {}
//     C++11 이후 권장. narrowing conversion 을 막아준다.
int x4{42};
std::vector<int> v{1, 2, 3, 4};
// int x5 {3.14};   // ← 컴파일 에러: double → int 좁힘 변환 금지
std::cout << "(5) x4=" << x4 << ", v.size=" << v.size() << "\n";"""))

cells.append(md(r"""**권장**: C++11 이후로는 `T x{...}` 스타일을 기본으로 써라. 이유:

1. 좁힘 변환을 컴파일 에러로 잡아준다.
2. **most vexing parse** 를 피할 수 있다 — 다음 코드 보자."""))

cells.append(code(r"""#include <iostream>

struct Point {
    Point() { std::cout << "Point() 호출\n"; }
};

// 다음 줄은 함수 선언으로 해석된다 — Point() 객체 생성이 *아니다*.
// "Point 를 반환하고 인자가 (Point()) 인 함수 p"
// 이걸 most vexing parse 라 부른다.
//   Point p(Point());

// {} 를 쓰면 명확히 객체 생성:
Point p{Point{}};      // 임시 Point 로 p 를 초기화"""))

cells.append(md(r"""### `auto` 와 brace init 의 함정

```cpp
auto x{42};        // C++17 부터: int (그 전엔 std::initializer_list<int>)
auto y = {1, 2};   // std::initializer_list<int>  (이건 변하지 않음)
```

이 차이는 C++17 이전과 이후가 다르므로 컴파일러 버전을 확인하라. 안전을 위해 `auto x = 42;` 를 쓰는 사람도 많다."""))

cells.append(md(r"""## 3. 오버로딩 결정 규칙 — 컴파일러는 어느 함수를 부를까

오버로드된 함수 호출에서 컴파일러는 다음 순서로 결정한다:

1. **이름 조회** — 그 이름을 가진 후보들을 모두 모은다.
2. **오버로드 결정** — 후보 중에서 인자 타입에 가장 잘 맞는 것을 고른다.
3. **접근 검사** — 그 함수가 호출자에게 보이는지 확인.

이때 인자 타입을 인자에 정확히 맞추는 *변환의 비용 순서*가 있다:

```
1. 정확한 일치 (T → T)
2. 사소한 변환 (T → const T,  T[] → T*,  T → T&)
3. 표준 변환 (int → double,  파생 → 기반,  Derived* → Base*)
4. 사용자 정의 변환 (생성자, 변환 연산자)
5. 가변 인자 (...)  ← 가장 약함
```

여러 후보가 *동일한 비용*이면 ambiguity 에러. 정확히 한 후보가 가장 비용이 낮으면 그 후보 선택."""))

cells.append(code(r"""#include <iostream>

void f(int)    { std::cout << "f(int)\n"; }
void f(double) { std::cout << "f(double)\n"; }
void f(const char*) { std::cout << "f(const char*)\n"; }

f(1);          // f(int)         — 정확한 일치
f(1.5);        // f(double)      — 정확한 일치
f('a');        // f(int)         — char → int 표준 변환
f("hi");       // f(const char*) — 정확한 일치 (string literal)
// f(true);    // f(int)         — bool → int (어떤 컴파일러는 경고)"""))

cells.append(md(r"""### 임시 객체와 const T&

이 패턴 자주 본다:

```cpp
void process(const std::string& s);

process("hello");  // const char* → std::string 임시 객체 생성 → const T& 가 임시를 받음
```

`const T&` 는 *임시 객체* 를 받을 수 있다 (수명을 임시 객체의 수명까지 연장한다). 그래서 함수 인자에서는 `const T&` 가 매우 유연하다.

`T&` (비-const 참조)는 임시를 못 받는다 — 의도적으로. 임시를 수정해봤자 바로 사라지는데 의미가 있나? 컴파일러가 이런 무의미를 막아준다.

```cpp
void modify(std::string& s);
modify("hello");   // ← 컴파일 에러
```

## 4. 캐스팅 — C 스타일 vs C++ 의 네 가지

```cpp
double d = 3.7;
int i = (int)d;          // C 스타일 캐스트 — 무엇이든 강제로 시도. 위험.

int i2 = static_cast<int>(d);          // 1. static_cast: 컴파일 타임에 합리적인 변환
                                       //    (int↔double, Base*↔Derived* 등)

const int& r = i;
int& r2 = const_cast<int&>(r);         // 2. const_cast: const/volatile 만 떼고 붙임.
                                       //    원래 const 였던 객체를 수정하면 UB.

// reinterpret_cast: 비트 패턴을 그대로 다른 타입으로 본다. 가장 위험.
int n = 42;
char* p = reinterpret_cast<char*>(&n);  // 3. reinterpret_cast: 거의 항상 잘못 사용됨

// dynamic_cast: 런타임 타입 검사가 들어간 다운캐스트. 가상 함수가 있는 클래스에만.
//   Base* → Derived* 가 안 되면 nullptr (포인터) 또는 std::bad_cast (참조) 반환.
```

**규칙**: C 스타일 `(T)x` 는 코드에서 추방하라. 의도가 분명한 `static_cast`/`const_cast`/`dynamic_cast`/`reinterpret_cast` 중 하나로 적어라. 문법이 길고 못생긴 건 의도다 — 캐스트는 자주 쓰지 말라는 표준의 메시지."""))

cells.append(md(r"""## 5. `nullptr` vs `NULL` vs `0`

```cpp
void f(int);
void f(char*);

f(NULL);     // C 시대 매크로: (void*)0 또는 0. 컴파일러에 따라 f(int) 가 호출될 수 있다 (!).
f(nullptr);  // 항상 f(char*) — nullptr 의 타입은 std::nullptr_t.
```

**규칙**: 포인터의 null 표현은 `nullptr` 만 쓰라. `NULL` 과 `0` 은 잊어라.

## 6. `using` 의 세 가지 용도"""))

cells.append(code(r"""#include <iostream>
#include <vector>

// (1) using 선언 — 특정 이름을 현재 스코프에 가져옴
using std::cout;
cout << "hello\n";

// (2) using 지시 — 네임스페이스 통째로 (헤더에선 절대 금지!)
// using namespace std;

// (3) 타입 별칭 (typedef 대체. 현대적이고 템플릿과 잘 맞음)
using IntVec = std::vector<int>;
IntVec v{1, 2, 3};
cout << v.size() << "\n";"""))

cells.append(md(r"""`typedef std::vector<int> IntVec;` 와 동일하지만 `using` 이 템플릿과 함께 쓸 때 훨씬 자연스럽다:

```cpp
template <typename T>
using Vec = std::vector<T>;     // typedef 로는 이렇게 못 씀
```

## 7. 함수 객체와 람다 — 한 컷"""))

cells.append(code(r"""#include <algorithm>
#include <iostream>
#include <vector>

std::vector<int> v{5, 1, 4, 2, 3};

// 람다 — 익명 함수 객체. C++11 이후.
//   [캡처](인자) { 본문 }
std::sort(v.begin(), v.end(), [](int a, int b) {
    return a > b;          // 내림차순
});

for (int x : v) std::cout << x << " ";
std::cout << "\n";

// 캡처 — 외부 변수를 가져오는 방법
int threshold = 3;
auto count = std::count_if(v.begin(), v.end(),
    [threshold](int x) { return x > threshold; }   // 값으로 캡처
);
std::cout << "threshold(" << threshold << ") 보다 큰 개수: " << count << "\n";"""))

cells.append(md(r"""캡처 종류:

- `[x]`     — x 를 값으로 캡처 (복사)
- `[&x]`    — x 를 참조로 캡처
- `[=]`     — 사용된 모든 외부 변수를 값으로 캡처
- `[&]`     — 사용된 모든 외부 변수를 참조로 캡처
- `[this]`  — 멤버 함수 안에서 클래스의 this 포인터 캡처

`[&]` 와 `[=]` 는 편하지만 람다가 *원래 스코프보다 오래 살아남으면* 문제다 — 참조 캡처는 댕글링 위험. 명시적으로 `[name1, &name2]` 로 적는 게 보통 더 안전하다."""))

cells.append(md(r"""## 8. 실습"""))

cells.append(code(r"""#include <iostream>
#include <vector>
#include <string>

// TODO 1: 다음 함수의 시그니처를 const-correct 하게 고쳐라.
//   - 인자를 수정하지 않고 큰 객체 → const T&
//   - 객체 상태를 변경하지 않는 멤버 함수 → 끝에 const
//   - 반환 타입도 const 가 적절한지 검토

class Catalog {
public:
    void add(std::string item) { items_.push_back(std::move(item)); }

    // TODO: 이 두 함수에 const 를 적절히 추가하라.
    std::size_t size()           { return items_.size(); }
    std::string& at(std::size_t i) { return items_.at(i); }

    // TODO: 이 함수의 인자에 const 를 추가하라 (수정하지 않으므로).
    bool contains(std::string& target) {
        for (auto& s : items_) if (s == target) return true;
        return false;
    }

private:
    std::vector<std::string> items_;
};

const Catalog cat = [] {
    Catalog c;
    c.add("apple"); c.add("banana");
    return c;
}();
// 다음 줄들이 컴파일되어야 한다 (const 객체에 호출 가능해야 함):
std::cout << "size = " << cat.size() << "\n";
std::cout << "[0]  = " << cat.at(0)  << "\n";
std::string target = "banana";
std::cout << "has banana? " << cat.contains(target) << "\n";"""))

cells.append(md(r"""## 9. 정리

- `const` 가 붙을 수 있는 다섯 위치를 다 알아두라. const 객체에는 const 멤버 함수만 호출 가능.
- 초기화는 가능한 한 `T x{...}` brace init 을 써라 — 좁힘 변환을 막고 most vexing parse 를 피한다.
- 오버로드 결정 규칙: 정확한 일치 → 사소한 변환 → 표준 변환 → 사용자 정의 변환 → 가변 인자.
- C 스타일 캐스트는 코드에서 추방. `static_cast` 가 99% 정답.
- `nullptr`. `NULL` 도 `0` 도 쓰지 말라.
- 람다 캡처는 `[&]`, `[=]` 보다 명시적 캡처가 안전.

다음 강의 — Lesson 05: 템플릿과 제네릭 프로그래밍.
"""))

nb = make_notebook(cells, kernel="xcpp17")
save(nb, "/home/claude/cpp_learning/notebooks/04_tricky_syntax.ipynb")
