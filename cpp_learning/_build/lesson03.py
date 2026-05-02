"""Lesson 03: 클래스, RAII, 스마트 포인터."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 03 · 클래스, RAII, 스마트 포인터

> **이 강의의 한 줄 요약** — C++ 객체는 *생성될 때*와 *소멸될 때* 정해진 코드를 실행할 수 있다. 이 사실 하나로 메모리·파일·소켓·뮤텍스 같은 모든 자원을 안전하게 다룰 수 있게 된다.

## 1. 클래스 — 데이터 + 그 데이터를 다루는 함수의 묶음"""))

cells.append(code(r"""#include <iostream>
#include <string>

class Person {
public:                              // public: 외부에서 보임
    Person(std::string n, int a)     // 생성자 (이름이 클래스명과 같음, 반환타입 없음)
        : name(std::move(n)), age(a) // ← 멤버 초기화 리스트
    {
        std::cout << "Person 생성: " << name << "\n";
    }

    ~Person() {                      // 소멸자 (~클래스명, 인자 없음, 반환 없음)
        std::cout << "Person 소멸: " << name << "\n";
    }

    void greet() const {             // const 멤버 함수: this 객체를 변경하지 않는다는 보증
        std::cout << "Hi, I'm " << name << " (" << age << ")\n";
    }

private:                             // private: 외부에서 안 보임 (기본값)
    std::string name;
    int age;
};

{
    Person alice{"Alice", 30};       // 스택에 객체 생성 → 생성자 호출
    alice.greet();
}                                    // 이 중괄호가 끝나는 순간 alice 의 소멸자 자동 호출"""))

cells.append(md(r"""읽어볼 핵심 5가지:

1. **생성자 (constructor)** — 객체가 만들어질 때 자동으로 호출된다. 이름은 반드시 클래스명. 반환 타입 없음.
2. **소멸자 (destructor)** — 객체가 사라질 때 자동으로 호출된다. `~클래스명`. 인자도 반환도 없음. 객체당 정확히 한 번 호출됨이 보장된다.
3. **멤버 초기화 리스트** — `: name(...), age(...)` 부분. 본문 `{ ... }` 안에서 `name = n;` 으로 대입하는 것과 다르다 — 이쪽은 *초기화*고, 본문은 *기본 생성된 다음 대입*이다. const 멤버나 참조 멤버는 초기화 리스트가 아니면 못 만든다.
4. **`const` 멤버 함수** — 함수 시그니처 끝의 `const`. "이 함수는 객체의 멤버를 수정하지 않겠다" 는 보증이다. const 객체에 대해서는 const 멤버 함수만 호출 가능.
5. **public/private** — `class` 의 기본 접근 제어는 `private`, `struct` 의 기본은 `public`. 그게 둘의 유일한 차이다.

## 2. RAII — Resource Acquisition Is Initialization

위 예제에서 봤다시피 객체 수명이 끝나면 소멸자가 *자동으로* 호출된다 (예외가 던져져도 호출됨, 이게 결정적이다). 이걸 이용해 자원 해제 코드를 소멸자에 넣어두면 — 자원 누수가 원천적으로 불가능해진다. 이 패턴이 **RAII** 다.

직접 작은 예를 보자: 파일을 열고 자동으로 닫는 클래스."""))

cells.append(code(r"""#include <cstdio>
#include <iostream>
#include <string>

class FileGuard {
public:
    FileGuard(const std::string& path, const char* mode) {
        f_ = std::fopen(path.c_str(), mode);
        if (!f_) throw std::runtime_error("파일 열기 실패: " + path);
        std::cout << "파일 열림\n";
    }
    ~FileGuard() {
        if (f_) {
            std::fclose(f_);
            std::cout << "파일 닫힘 (자동)\n";
        }
    }

    // 복사 금지 — 같은 파일 핸들을 두 객체가 들고 있으면 위험
    FileGuard(const FileGuard&) = delete;
    FileGuard& operator=(const FileGuard&) = delete;

    void write(const std::string& s) { std::fputs(s.c_str(), f_); }

private:
    std::FILE* f_ = nullptr;
};

{
    FileGuard fg("/tmp/cpplab_test.txt", "w");
    fg.write("hello RAII\n");
    // 여기서 예외가 나도, return 이 끼어도, 함수 끝에서 ~FileGuard() 가 반드시 호출됨
}
std::cout << "스코프 빠져나옴\n";"""))

cells.append(md(r"""주목: `FileGuard(const FileGuard&) = delete;` — 복사 생성자를 *삭제*했다. 이 클래스의 객체는 복사할 수 없다. 왜? 복사를 허용하면 두 객체가 같은 `FILE*` 를 들고 있다가 둘 다 `fclose` 를 호출해 *이중 해제*가 일어난다. 복사를 막아두면 이 버그 자체가 컴파일 에러가 된다.

이 "**자원을 가지는 클래스는 기본적으로 복사 금지**" 가 모던 C++의 큰 지침이다.

## 3. The Rule of Five (와 그 0)

자원을 직접 들고 있는 클래스는 다섯 개의 *특수 멤버 함수* 를 어떻게 처리할지 명시적으로 결정해야 한다:

```
1. 소멸자             ~T()
2. 복사 생성자        T(const T&)
3. 복사 대입 연산자   T& operator=(const T&)
4. 이동 생성자        T(T&&)
5. 이동 대입 연산자   T& operator=(T&&)
```

규칙: **이 중 하나라도 직접 정의하면, 나머지 네 개도 명시적으로 정의(또는 `= default` / `= delete`) 하라.** 컴파일러가 알아서 만들어주는 디폴트가 *틀린* 동작일 수 있기 때문이다 (예: 두 객체가 같은 포인터를 공유 → 이중 해제).

더 좋은 규칙은 **The Rule of Zero** — 자원을 직접 들고 있지 마라. 자원은 `std::unique_ptr`, `std::vector`, `std::string` 같은 RAII 클래스에 위임하라. 그러면 다섯 개를 다 컴파일러에게 맡길 수 있다."""))

cells.append(md(r"""## 4. Move 시맨틱 — 왜 필요한가

`std::vector<int>` 가 1억 개의 원소를 가지고 있다고 하자. 이걸 함수에서 반환하면? C++03 까지는 *복사*가 일어났다 (1억 번의 메모리 복사). 끔찍하다.

C++11 부터 도입된 **move 시맨틱**의 아이디어: "곧 죽을 객체에서는 데이터를 *훔쳐와도* 된다". vector 의 경우 — 새 vector 를 만들고 원래 vector 의 *내부 포인터를 가져오고* 원래 vector 의 포인터는 nullptr 로 둔다. O(N) 복사가 O(1) 포인터 교환이 된다.

`std::move` 는 마법이 아니라 그냥 *캐스트* 다 — 객체를 "rvalue 참조 (`T&&`)" 로 다루겠다고 표시할 뿐이다. 실제로 데이터를 옮기는 일은 그 객체의 **이동 생성자** / **이동 대입 연산자** 가 한다."""))

cells.append(code(r"""#include <iostream>
#include <vector>
#include <string>

std::vector<int> make_big() {
    std::vector<int> v(1'000'000, 7);     // 1백만 개
    return v;                             // C++17 이후 보장된 RVO + move
}

std::vector<int> a = make_big();
std::cout << "a.size = " << a.size() << ", a.front = " << a.front() << "\n";

std::vector<int> b = std::move(a);        // a 에서 b 로 *이동* (O(1))
std::cout << "이동 후 b.size = " << b.size() << "\n";
std::cout << "이동 후 a.size = " << a.size() << "  (보통 0)\n";
// 이동 후 a 는 '유효하지만 미정의 상태(valid but unspecified)' — destroy/assign 만 해도 안전"""))

cells.append(md(r"""중요한 함정 — **이동 후의 객체에 접근하지 말라**. 표준은 "valid but unspecified state" 만 보장한다. 다시 새 값을 *대입하거나* 객체가 소멸되는 것까지만 안전하다고 가정하라. 읽으려고 하면 의미 없는 값이 나올 수 있다.

### lvalue vs rvalue (간단하게)

- **lvalue** — 이름이 있고 주소를 가질 수 있는 것. `int x;` 의 `x`.
- **rvalue** — 임시값, 곧 사라질 것. `x + 1`, `make_big()` 의 반환값.
- `T&` — lvalue 참조 (보통의 참조)
- `T&&` — rvalue 참조. "곧 사라질 것을 받겠다" 는 의도 표시.

`std::move(x)` 는 x 의 값을 옮기는 게 아니라 x 를 rvalue 처럼 보이게 해서 — 오버로드 결정 시 이동 생성자가 호출되도록 만든다. *그 호출이 실제로 데이터를 옮긴다*."""))

cells.append(md(r"""## 5. 스마트 포인터

### `std::unique_ptr<T>` — 단독 소유

한 시점에 정확히 한 객체만 그 자원을 소유한다. 복사 불가, 이동만 가능. `new`/`delete` 의 99% 케이스를 대체한다."""))

cells.append(code(r"""#include <iostream>
#include <memory>
#include <vector>

class Widget {
public:
    Widget(int id) : id_(id) { std::cout << "Widget(" << id_ << ")\n"; }
    ~Widget()                { std::cout << "~Widget(" << id_ << ")\n"; }
    int id() const { return id_; }
private:
    int id_;
};

{
    std::unique_ptr<Widget> w = std::make_unique<Widget>(1);
    std::cout << "id = " << w->id() << "\n";

    // std::unique_ptr<Widget> w2 = w;       // ← 컴파일 에러: 복사 금지
    std::unique_ptr<Widget> w2 = std::move(w);  // 이동은 OK. w 는 비어버림.
    std::cout << "이동 후 w 가 가리키는 곳: "
              << (w ? "있음" : "비었음") << "\n";
} // 여기서 w2 가 소멸 → ~Widget 자동 호출"""))

cells.append(md(r"""### `std::shared_ptr<T>` — 공유 소유 (참조 카운팅)

여러 곳에서 같은 객체를 가리킬 수 있다. 마지막 shared_ptr 가 사라질 때 객체가 해제된다. 내부적으로 **참조 카운트**를 가진 control block 을 둔다."""))

cells.append(code(r"""#include <iostream>
#include <memory>

class Connection {
public:
    Connection()  { std::cout << "Connection 생성\n"; }
    ~Connection() { std::cout << "Connection 소멸\n"; }
};

{
    auto c1 = std::make_shared<Connection>();
    std::cout << "use_count = " << c1.use_count() << "\n";   // 1

    {
        auto c2 = c1;                                         // 복사 OK (카운트 증가)
        std::cout << "use_count = " << c1.use_count() << "\n";  // 2
    }                                                         // c2 소멸 (카운트 감소)
    std::cout << "use_count = " << c1.use_count() << "\n";   // 1
}
std::cout << "다 끝남\n";"""))

cells.append(md(r"""shared_ptr 는 만능처럼 보이지만 **두 가지 비용**이 있다:

1. **메모리** — control block 이 추가로 잡힌다 (보통 16~32 바이트).
2. **시간** — 참조 카운트 증감이 *원자적 연산* (atomic)이다. 멀티스레드 안전성을 위함이지만 빈번한 복사는 캐시 일관성 트래픽을 만든다.

그래서 **기본은 unique_ptr 을 써라. 정말 공유가 필요할 때만 shared_ptr**.

### `std::weak_ptr<T>` — 순환 참조 끊기

shared_ptr 두 객체가 서로를 shared_ptr 로 가지고 있으면 카운트가 절대 0 이 안 되어 — 메모리 누수가 난다. 이 순환을 끊으려고 한쪽을 `weak_ptr` 로 둔다."""))

cells.append(code(r"""#include <iostream>
#include <memory>

struct Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node>   prev;     // ← weak_ptr! shared 였으면 순환 발생
    int value;
    Node(int v) : value(v) {}
    ~Node() { std::cout << "~Node(" << value << ")\n"; }
};

{
    auto a = std::make_shared<Node>(1);
    auto b = std::make_shared<Node>(2);
    a->next = b;
    b->prev = a;        // weak — 카운트 안 올림

    if (auto p = b->prev.lock()) {   // weak_ptr → shared_ptr 로 변환 (lock)
        std::cout << "b->prev->value = " << p->value << "\n";
    }
}  // 여기서 a, b 둘 다 정리됨"""))

cells.append(md(r"""## 6. 직접 만들어보는 RAII — 작은 mutex 잠금

표준의 `std::lock_guard` 가 정확히 이 패턴이다. 30줄짜리 RAII 가 멀티스레드 안전성을 어떻게 보장하는지 보자."""))

cells.append(code(r"""#include <iostream>
#include <mutex>
#include <thread>

template <typename Mutex>
class MyLockGuard {
public:
    explicit MyLockGuard(Mutex& m) : m_(m) { m_.lock(); }
    ~MyLockGuard() { m_.unlock(); }

    MyLockGuard(const MyLockGuard&) = delete;
    MyLockGuard& operator=(const MyLockGuard&) = delete;

private:
    Mutex& m_;
};

std::mutex g_mtx;
int counter = 0;

auto worker = [] {
    for (int i = 0; i < 1000; ++i) {
        MyLockGuard<std::mutex> lg(g_mtx);  // 잠금
        ++counter;
        // 여기서 무슨 예외가 나도 lg 의 소멸자에서 unlock 보장됨
    }
};

std::thread t1(worker), t2(worker), t3(worker);
t1.join(); t2.join(); t3.join();
std::cout << "counter = " << counter << " (3000 이어야 함)\n";"""))

cells.append(md(r"""## 7. 실습"""))

cells.append(code(r"""#include <iostream>
#include <memory>
#include <string>
#include <vector>

// TODO 1: 다음 코드는 메모리 누수가 있다. unique_ptr 로 고쳐라.
//   힌트: std::vector<std::unique_ptr<Resource>> 를 쓰면 자연스럽다.

class Resource {
public:
    Resource(std::string n) : name_(std::move(n)) {
        std::cout << "Resource " << name_ << " 생성\n";
    }
    ~Resource() {
        std::cout << "Resource " << name_ << " 소멸\n";
    }
    void use() { std::cout << "use " << name_ << "\n"; }
private:
    std::string name_;
};

void buggy_version() {
    std::vector<Resource*> pool;
    pool.push_back(new Resource("A"));
    pool.push_back(new Resource("B"));
    for (auto* r : pool) r->use();
    // 누수: delete 가 어디에도 없음
}

// 위 함수를 unique_ptr 기반으로 다시 짜서 fixed_version 이라 부르자.
void fixed_version() {
    // TODO: 채워라
}

buggy_version();
std::cout << "──────\n";
fixed_version();    // 정상이라면 Resource 가 만들어진 만큼 정확히 소멸되어야 한다."""))

cells.append(md(r"""## 8. 정리

- 객체 수명 = 자원 수명. 이게 RAII.
- Rule of Zero: 자원을 직접 들지 마라. RAII 클래스(스마트 포인터, vector, string ...)에 맡겨라.
- `std::unique_ptr` 가 기본. `std::shared_ptr` 는 정말 공유 소유가 필요할 때.
- `std::move` 는 캐스트일 뿐, 옮기는 일은 이동 생성자/대입 연산자가 한다. 이동된 객체에 다시 접근하지 말라.
- 순환은 `weak_ptr` 로 끊는다.

다음 강의 — Lesson 04: 헷갈리기 쉬운 문법 정리 (const correctness, 초기화의 다섯 가지 얼굴, 오버로딩 결정 규칙).
"""))

nb = make_notebook(cells, kernel="xcpp17")
save(nb, "/home/claude/cpp_learning/notebooks/03_classes_raii_smartptr.ipynb")
