"""Lesson 06: C++20 핵심 기능 (concepts, ranges, modules, coroutines 개요)."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 06 · C++20 핵심 기능

> **이 강의의 노트북 실행 방식** — xeus-cling 이 C++17 까지만 안정 지원하므로, 이 노트북부터는 **셀에서 g++ 을 직접 호출**해 빌드/실행한다. 셀에서 C++ 코드를 문자열로 작성한 뒤 system 으로 컴파일·실행하는 패턴을 자주 쓴다.

C++20 은 1998년 이후 가장 큰 규모의 변화다. 핵심 네 기둥은:

1. **Concepts** — 템플릿 매개변수에 *조건* 을 주는 방법. SFINAE 의 미래판.
2. **Ranges** — 알고리즘에 begin/end 쌍 대신 컨테이너 자체를 넘긴다. 그리고 파이프라인.
3. **Modules** — `#include` 의 헤더 모델을 대체. 컴파일 시간을 크게 줄인다.
4. **Coroutines** — 함수가 중간에 멈췄다 재개될 수 있다. 비동기 프로그래밍의 기반.

이 강의에서는 1, 2 를 충분히 다루고, 3, 4 는 개념과 작은 예제만 본다.

## 1. 개요 셀 — 빌드 헬퍼"""))

cells.append(code(r"""// xeus-cling 셀에서는 #include <cstdlib>, std::system 으로 외부 g++ 호출
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

// 코드 문자열을 받아 /tmp 에 저장하고 g++ 로 빌드·실행하는 헬퍼
void run_cpp(const std::string& code, const std::string& std_ver = "c++20",
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
}

// 컴파일러 버전 확인
std::system("g++ --version | head -1");"""))

cells.append(md(r"""## 2. Concepts — SFINAE 의 후계자

C++17 까지 "이 함수는 정수 타입에만 사용 가능" 같은 제약을 걸려면 `enable_if` 의 못생긴 형태를 써야 했다. C++20 부터:"""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <concepts>
#include <type_traits>

// 1. 표준 concept 사용
template <std::integral T>            // T 는 정수 타입이어야 함
T add_int(T a, T b) { return a + b; }

// 2. 직접 만든 concept
template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>                  // 정수 또는 부동소수점만
T square(T x) { return x * x; }

// 3. requires 절로 더 복잡한 제약
template <typename T>
concept HasSize = requires(T x) {     // 'x.size() 가 호출 가능해야 한다'
    { x.size() } -> std::convertible_to<std::size_t>;
};

template <HasSize C>
void print_size(const C& c) {
    std::cout << "size = " << c.size() << "\n";
}

#include <vector>
int main() {
    std::cout << add_int(3, 4) << "\n";
    std::cout << square(2.5) << "\n";
    print_size(std::vector<int>{1,2,3});
    // print_size(42);   // 컴파일 에러: int 는 size() 없음
}
)CPP", "c++20");"""))

cells.append(md(r"""**concept 가 SFINAE 보다 좋은 점**:

- 에러 메시지가 훨씬 명확하다 — "타입 T 가 X 를 만족하지 않음" 처럼.
- 코드가 읽기 쉽다.
- 함수 시그니처에 의도가 드러난다.

C++20 표준 라이브러리는 `<concepts>` 에 자주 쓰는 concept 을 모아두었다:

| concept | 의미 |
|---------|------|
| `std::integral<T>` | int, long, char ... |
| `std::floating_point<T>` | float, double, long double |
| `std::same_as<T, U>` | T 와 U 가 같은 타입 |
| `std::convertible_to<T, U>` | T → U 변환 가능 |
| `std::derived_from<D, B>` | D 가 B 의 자손 |
| `std::equality_comparable<T>` | == 비교 가능 |
| `std::copyable<T>` | 복사 가능 |
| `std::movable<T>` | 이동 가능 |"""))

cells.append(md(r"""## 3. Ranges — 알고리즘의 새 형태

C++17 까지 알고리즘은 begin/end 한 쌍을 받았다:

```cpp
std::sort(v.begin(), v.end());
```

C++20 의 ranges 라이브러리는 컨테이너 자체를 받는다:

```cpp
std::ranges::sort(v);
```

여기까진 단순한 편의지만 진짜는 **view** 와 **파이프라인**이다."""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>

int main() {
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 짝수만 골라서, 제곱한 뒤, 처음 3개만
    auto pipeline = v
        | std::views::filter([](int x){ return x % 2 == 0; })
        | std::views::transform([](int x){ return x * x; })
        | std::views::take(3);

    for (int x : pipeline) std::cout << x << " ";
    std::cout << "\n";

    // 표준 알고리즘과 함께
    auto sum_of_squares_of_evens = 0;
    for (int x : v | std::views::filter([](int x){ return x % 2 == 0; })
                   | std::views::transform([](int x){ return x * x; })) {
        sum_of_squares_of_evens += x;
    }
    std::cout << "sum = " << sum_of_squares_of_evens << "\n";
}
)CPP", "c++20");"""))

cells.append(md(r"""**view 의 비밀**: view 는 *데이터를 복사하지 않는다*. lazy 하게 동작한다 — 위 파이프라인을 만든 시점에는 아무 일도 일어나지 않고, for 루프에서 한 번 순회할 때마다 한 원소씩 필터·변환·자른다. 메모리도 거의 안 쓴다.

이게 처음에는 마법처럼 보이지만, 실제 동작은 그저 **이터레이터의 합성**이다.

C++20 은 view 가 적은 편이고, **C++23 이 결정타** — `std::views::zip`, `std::ranges::to`, `std::views::enumerate` 등이 추가되어 Python/Rust 에서 익숙한 패턴들을 거의 다 쓸 수 있게 된다."""))

cells.append(md(r"""## 4. Three-way comparison — `<=>` (우주선 연산자)

C++20 이전에는 사용자 정의 타입에 비교 연산자를 만들려면 `==`, `!=`, `<`, `<=`, `>`, `>=` 여섯 개를 다 적어야 했다. 이제는 *하나*만:"""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <compare>

struct Point {
    int x, y;
    auto operator<=>(const Point&) const = default;   // 6개 비교 자동 생성
};

int main() {
    Point a{1, 2}, b{1, 3}, c{1, 2};
    std::cout << (a == c) << "\n";   // 1
    std::cout << (a < b)  << "\n";   // 1 (lex 비교: x 동일하므로 y 비교)
    std::cout << (a > b)  << "\n";   // 0
}
)CPP", "c++20");"""))

cells.append(md(r"""## 5. `std::format` — printf 의 안전한 후계자"""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <format>     // C++20

int main() {
    int    n = 42;
    double d = 3.14;
    std::string name = "World";

    // type-safe, 위치 인자, 정렬·정밀도 모두 지원
    std::cout << std::format("Hello, {}! n={}, d={:.2f}\n", name, n, d);
    std::cout << std::format("{:>10} | {:<10} | {:^10}\n", "right", "left", "center");
    std::cout << std::format("hex={:#x} oct={:#o} bin={:#b}\n", 255, 8, 10);
}
)CPP", "c++20");"""))

cells.append(md(r"""C 의 `printf` 는 타입 안전하지 않다 — `printf("%d", "hello")` 는 컴파일된다 (그리고 크래시한다). `std::format` 은 *컴파일 타임에* 포맷 문자열과 인자 타입을 검사한다. C++23 부터는 `std::print` 가 표준이 되어 `std::cout << std::format(...)` 대신 `std::print("...", ...)` 로 쓸 수 있다.

## 6. Modules — `#include` 의 종말 (예고편)

`#include` 의 문제: 헤더가 텍스트 그대로 복사된다 → 같은 헤더가 여러 번 처리됨 → 컴파일 시간 폭발.

Modules 의 해결: 헤더를 한 번 컴파일해서 *바이너리 형태로 캐시*. 다음 사용에서는 그 캐시를 가져옴.

```cpp
// math.cppm  (모듈 인터페이스 단위)
export module math;
export int add(int a, int b) { return a + b; }

// main.cpp
import math;
int main() { return add(1, 2); }
```

GCC 13/Clang 16 부터 부분적으로 지원되지만 빌드 시스템 통합이 아직 거칠다. 실무 도입은 빠르지 않다 — 개념만 알아두자."""))

cells.append(md(r"""## 7. Coroutines — 비동기의 미래 (한 컷)

> 함수가 *중간에 멈췄다가 재개*될 수 있다.

```cpp
generator<int> ints_from(int start) {
    while (true) co_yield start++;     // 값을 내놓고 멈춤
}

for (int x : ints_from(1)) {
    if (x > 5) break;
    std::cout << x << " ";             // 1 2 3 4 5
}
```

C++20 은 *coroutine 의 언어 메커니즘*만 제공하고 표준 라이브러리는 거의 비어 있다. 실무에서는 보통 [cppcoro](https://github.com/lewissbaker/cppcoro) 같은 라이브러리를 가져오거나, C++23 의 `std::generator` 가 도입되기를 기다리는 편이 낫다.

핵심 키워드 셋:

- `co_await` — 어떤 일이 끝날 때까지 함수를 멈춘다
- `co_yield` — 값을 호출자에게 돌려주고 멈춘다 (제너레이터)
- `co_return` — 코루틴을 종료한다

이 메커니즘이 있으면 비동기 코드를 *동기 스타일로* 적을 수 있다 (Python 의 async/await 와 동일한 정신). 네트워크 서버에서 가장 큰 영향을 준다."""))

cells.append(md(r"""## 8. C++20 vs 17 — 한 장 요약 표

| 기능 | C++17 | C++20 |
|------|-------|-------|
| 제약 있는 템플릿 | `enable_if` (못생김) | `concept`/`requires` (깔끔) |
| 알고리즘 호출 | `std::sort(v.begin(), v.end())` | `std::ranges::sort(v)` |
| 파이프라인 | 직접 작성 | `views::filter | views::transform | ...` |
| 비교 연산자 | 6개 직접 작성 | `auto operator<=>(...) = default;` |
| 포맷팅 | `printf` 또는 sstream | `std::format` |
| 비동기 | 콜백/`std::future` | coroutines |
| 컴파일 단위 | 헤더 + .cpp | modules (점진 도입) |

## 9. 실습"""))

cells.append(code(r"""run_cpp(R"CPP(
#include <iostream>
#include <vector>
#include <ranges>
#include <concepts>
#include <numeric>

// TODO 1: '평균을 낼 수 있는 컨테이너' 의 concept 을 정의하라.
//   요구사항: begin/end 가 있고, 원소가 산술 타입이어야 한다.
//
// 힌트:
//   template <typename C>
//   concept Averageable = std::ranges::range<C>
//                      && std::is_arithmetic_v<std::ranges::range_value_t<C>>;

// TODO 2: average 함수를 그 concept 으로 제약하라.
//   double average(Averageable auto&& c) { ... }
//
// TODO 3: 1~100 중 짝수만 골라 그 평균을 ranges 파이프라인으로 구하라.
//   힌트: std::views::iota(1, 101) | std::views::filter(...)

int main() {
    // 위에 직접 작성한 뒤 호출 코드를 적어라.
    std::cout << "TODO\n";
}
)CPP", "c++20");"""))

cells.append(md(r"""## 10. 정리

- Concepts 는 SFINAE 의 가독성 문제를 해결한다. 일단 적어두면 함수의 의도가 시그니처에 보인다.
- Ranges 의 view 는 lazy. 데이터를 복사하지 않고 이터레이터로 합성된 파이프라인.
- `<=>` 한 줄로 6개 비교 연산자를 자동 생성.
- `std::format` 은 타입 안전한 포맷팅. `printf` 의 자리를 차지한다.
- Modules, Coroutines 는 도구지원이 익으면 게임을 바꿀 기능들.

다음 강의 — Lesson 07: C++23/26 의 새로움 (deducing this, 명시적 객체 파라미터, std::expected, 스택 트레이스 등).
"""))

nb = make_notebook(cells, kernel="xcpp17")
save(nb, "/home/claude/cpp_learning/notebooks/06_cpp20_features.ipynb")
