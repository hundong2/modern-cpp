"""Lesson 07: C++23 핵심 기능."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 07 · C++23 핵심 기능

C++23 은 C++20 의 *마무리* 같은 개정판이다. 새로운 패러다임은 적지만 일상 코드를 깔끔하게 만드는 도구들이 많이 들어왔다.

> 이 노트북도 셀에서 g++ 을 직접 호출한다 — `-std=c++2b` 플래그 사용. (GCC 12 기준 부분 지원, GCC 13 부터 거의 완전)

## 1. 빌드 헬퍼"""))

cells.append(code(r"""#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

void run_cpp(const std::string& code, const std::string& std_ver = "c++2b",
             const std::string& extra_flags = "") {
    std::system("mkdir -p /tmp/cpplab");
    {
        FILE* f = std::fopen("/tmp/cpplab/snippet.cpp", "w");
        std::fputs(code.c_str(), f);
        std::fclose(f);
    }
    std::string cmd = "g++ -std=" + std_ver + " -O2 -Wall -Wextra " + extra_flags
                    + " /tmp/cpplab/snippet.cpp -o /tmp/cpplab/snippet 2>&1 "
                      "&& /tmp/cpplab/snippet";
    std::system(cmd.c_str());
}"""))

cells.append(md(r"""## 2. `std::expected<T, E>` — 예외 없는 에러 처리

> Rust 의 `Result<T, E>`, Haskell 의 `Either` 와 같은 정신.

`std::optional<T>` 가 "값이 있을 수도 없을 수도" 라면, `std::expected<T, E>` 는 "값이 있거나 *왜 실패했는지의 정보*가 있다"."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <expected>
#include <string>

enum class ParseError { Empty, NotANumber, OutOfRange };

std::expected<int, ParseError> parse_int(const std::string& s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    try {
        size_t pos;
        int v = std::stoi(s, &pos);
        if (pos != s.size()) return std::unexpected(ParseError::NotANumber);
        return v;
    } catch (const std::out_of_range&)    { return std::unexpected(ParseError::OutOfRange); }
      catch (const std::invalid_argument&) { return std::unexpected(ParseError::NotANumber); }
}

const char* err_str(ParseError e) {
    switch (e) {
        case ParseError::Empty:       return "Empty";
        case ParseError::NotANumber:  return "NotANumber";
        case ParseError::OutOfRange:  return "OutOfRange";
    }
    return "?";
}

int main() {
    for (auto s : {"42", "abc", "", "999999999999999"}) {
        auto r = parse_int(s);
        if (r.has_value()) std::cout << "ok: "  << *r << "\n";
        else               std::cout << "err: " << err_str(r.error()) << "\n";
    }

    // monadic 인터페이스 — and_then, or_else, transform 으로 체인
    auto doubled = parse_int("21").transform([](int x){ return x * 2; });
    std::cout << "doubled = " << *doubled << "\n";
}
)CPP", "c++2b");"""))

cells.append(md(r"""**왜 좋은가**:

- 예외는 *제어 흐름의 비국소적 점프* 라 추적이 어렵다. 게다가 일부 환경(임베디드, 게임)에서는 비활성화되기도 한다.
- `std::expected<T, E>` 는 함수 시그니처에 *어떤 에러가 가능한지* 가 드러난다.
- `transform`, `and_then`, `or_else` 로 깔끔한 체인이 가능 — Lesson 06 의 ranges 파이프라인과 같은 정신.

## 3. "Deducing this" (Explicit Object Parameter)

C++23 의 가장 큰 언어 기능. 멤버 함수의 첫 인자로 *명시적인 this* 를 받을 수 있다."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <string>

class Box {
public:
    std::string content;

    // 기존 방식: const, &, && 버전을 따로따로 작성해야 했다.
    //   std::string& get() &       { return content; }
    //   const std::string& get() const& { return content; }
    //   std::string&& get() &&     { return std::move(content); }
    //
    // C++23: 한 함수로!
    template <typename Self>
    auto&& get(this Self&& self) {
        return std::forward<Self>(self).content;
    }
};

int main() {
    Box b{"hello"};
    std::cout << b.get() << "\n";

    const Box cb{"const-hello"};
    std::cout << cb.get() << "\n";

    auto moved = Box{"rvalue"}.get();   // rvalue 버전이 호출되어 content 가 move 됨
    std::cout << moved << "\n";
}
)CPP", "c++2b");"""))

cells.append(md(r"""기존 방식(C++20 까지)은 lvalue, const lvalue, rvalue 마다 멤버 함수를 따로 적어야 했다 — *세 배의 코드*. C++23 은 그걸 *템플릿 한 개*로 통합한다. 라이브러리 작성자에게 큰 선물이다.

또 다른 큰 응용: **재귀 람다**.

```cpp
auto fact = [](this auto self, int n) -> int {
    return n <= 1 ? 1 : n * self(n - 1);
};
fact(5);   // 120
```

이전엔 람다가 자기 자신을 부를 방법이 없어 `std::function` 등의 우회를 썼다.

## 4. `std::print` — 마침내 표준에 들어온 출력"""))

cells.append(code(r"""run_cpp(R"CPP(
#include <print>     // C++23

int main() {
    std::println("Hello, {}!", "World");
    std::println("{:>10} {:>10}", "name", "score");
    std::println("{:>10} {:>10}", "Alice",  92);
    std::println("{:>10} {:>10}", "Bob",    87);
}
)CPP", "c++2b");"""))

cells.append(md(r"""`std::cout << std::format(...)` 의 보일러플레이트가 사라진다. C 의 printf 와 비교해도 — 타입 안전, 포맷 검사 컴파일 타임, 새 줄 자동 (`println`).

## 5. Ranges 의 결정타 — `std::ranges::to`, `views::zip`, `views::enumerate`"""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <vector>
#include <string>
#include <ranges>
#include <print>

int main() {
    std::vector<int>         ids   {1, 2, 3};
    std::vector<std::string> names {"Alice", "Bob", "Carol"};

    // zip — 두 컨테이너를 동시에 순회
    for (auto [id, name] : std::views::zip(ids, names)) {
        std::println("{}: {}", id, name);
    }

    // enumerate — Python 의 enumerate
    for (auto [i, name] : std::views::enumerate(names)) {
        std::println("[{}] {}", i, name);
    }

    // ranges::to — view 결과를 컨테이너로
    auto squares = std::views::iota(1, 6)
                 | std::views::transform([](int x){ return x*x; })
                 | std::ranges::to<std::vector<int>>();
    for (int x : squares) std::cout << x << " ";
    std::cout << "\n";
}
)CPP", "c++2b");"""))

cells.append(md(r"""C++20 ranges 가 *시작* 이라면 C++23 은 *완성*이다. 위 세 도구가 빠져 있던 게 가장 컸는데 다 들어왔다.

## 6. `std::stacktrace` — 스택 트레이스를 표준으로

```cpp
#include <stacktrace>
std::cout << std::stacktrace::current() << "\n";
```

GCC 13 부터 지원. 디버깅·로깅에서 큰 힘이다 (지금까지는 boost::stacktrace 같은 외부 라이브러리에 의존).

## 7. `if consteval` — 컴파일 타임 vs 런타임 분기"""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>

constexpr int compute(int n) {
    if consteval {        // 컴파일 타임에 평가되는 컨텍스트라면
        return n * n;     // 단순한 빠른 경로
    } else {
        // 런타임이라면 더 복잡한 로직(예: 로깅) 가능
        std::cout << "런타임 호출\n";
        return n * n;
    }
}

int main() {
    constexpr int a = compute(5);   // 컴파일 타임 — "런타임 호출" 안 찍힘
    int b = compute(7);             // 런타임 — 찍힘
    std::cout << a << ", " << b << "\n";
}
)CPP", "c++2b");"""))

cells.append(md(r"""## 8. `[[assume(expr)]]` — 컴파일러에게 힌트 주기

특정 조건이 *항상 참* 이라고 컴파일러에게 알려주면 컴파일러는 그 정보를 최적화에 활용한다.

```cpp
void f(int x) {
    [[assume(x > 0)]];     // x 가 양수임을 가정
    // 컴파일러는 이 가정 아래 분기 제거 등 최적화 수행 가능
    if (x < 0) very_slow_path();   // 제거될 가능성
}
```

**경고**: 그 가정이 거짓이면 *정의되지 않은 동작*이다. 매우 강력하지만 매우 위험. 프로파일링 후 핫 루프에서 검증된 가정에만 쓰라.

## 9. 그 외 짧은 것들

- **`std::flat_map`/`std::flat_set`** — 정렬된 vector 기반 map. 캐시 친화적이라 작은 컨테이너에서 빠르다.
- **multidimensional subscript** — `a[i, j, k]` 가 가능 (이전엔 `a[i][j][k]` 또는 `a(i,j,k)`).
- **`size_t` 리터럴 `uz`** — `for (size_t i = 0uz; i < v.size(); ++i)`.
- **`#warning`** — 표준화 (GCC 는 이전부터 지원했지만 비표준이었다).
- **`auto(x)` 연산자** — 표현식의 복사를 명시적으로.

## 10. 호환성 표"""))

cells.append(code(r"""// 우리 환경의 C++23 지원 수준 확인
std::system("g++ -std=c++2b -dM -E -x c++ /dev/null | grep __cpp_lib | head -30");
std::cout << "──\n";
std::system("g++ --version | head -1");"""))

cells.append(md(r"""## 11. 실습"""))

cells.append(code(r"""run_cpp(R"CPP(
#include <expected>
#include <string>
#include <iostream>
#include <print>

// TODO: parse_positive_int 함수를 작성하라.
//   - 음수, 0, 비정수, 빈 문자열을 모두 다른 에러로 구분.
//   - std::expected<int, std::string> 반환 (에러 메시지를 문자열로).
//   - 한 번의 transform 으로 결과를 두 배로 만든 값을 반환하는 코드도 짜본다.

std::expected<int, std::string> parse_positive_int(const std::string& s) {
    // 채워라
    return std::unexpected("not implemented");
}

int main() {
    for (auto s : {"42", "0", "-5", "abc", ""}) {
        auto r = parse_positive_int(s);
        if (r) std::println("'{}' -> {}", s, *r);
        else   std::println("'{}' -> err: {}", s, r.error());
    }
}
)CPP", "c++2b");"""))

cells.append(md(r"""## 12. 정리

- `std::expected` 는 예외 없는 에러 처리의 표준 형태.
- "Deducing this" 는 라이브러리 작성자의 보일러플레이트를 한 번에 정리한다.
- `std::print`/`std::println` 으로 출력이 깔끔해진다.
- Ranges 의 `zip`, `enumerate`, `to` 가 도착 — 모던 데이터 파이프라인이 된다.
- `std::stacktrace`, `if consteval`, `[[assume]]` 같은 작은 도구들도 알아두면 유용.

다음 강의 — Lesson 08: 동시성과 병렬성 (thread, mutex, atomic, async, future, 메모리 모델).
"""))

nb = make_notebook(cells, kernel="xcpp17")
save(nb, "/home/claude/cpp_learning/notebooks/07_cpp23_features.ipynb")
