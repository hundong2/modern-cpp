"""Lesson 02: 메모리 모델 — 스택/힙, 포인터, 참조."""
import sys, os
sys.path.insert(0, os.path.dirname(__file__))
from nb_helper import md, code, make_notebook, save

cells = []

cells.append(md(r"""# Lesson 02 · 메모리 모델, 포인터, 참조

> **핵심 질문** — 변수 `int x = 5;` 를 적었을 때, 그 5는 *어디에* 있는가? 함수가 끝나면 그 5는 어떻게 되나? 왜 누군가는 `int*` 를 쓰고 누군가는 `int&` 를 쓰는가?

C++ 학습에서 가장 큰 첫 산은 **메모리가 보인다는 점**이다. 이걸 이해하면 그다음 RAII, 스마트 포인터, move 같은 개념들이 전부 자연스러운 결론처럼 보인다.

## 1. 프로세스 메모리 레이아웃

리눅스에서 프로세스가 시작되면 OS가 가상 메모리 공간을 이렇게 잡아준다 (높은 주소 → 낮은 주소):

```
┌─────────────────────────┐  높은 주소 (예: 0x7fff...)
│ Stack                   │  ← 함수 호출시 자동 할당, 위에서 아래로 자란다
│   ↓                     │
│                         │
│   ↑                     │
│ Heap                    │  ← new / malloc, 아래에서 위로 자란다
├─────────────────────────┤
│ BSS (초기화 안 된 전역) │  int g;
│ Data (초기화된 전역)    │  int g = 42;
│ Text (코드/명령어)      │  함수 본문 기계어
└─────────────────────────┘  낮은 주소 (예: 0x0040...)
```

각 영역의 특징:

| 영역  | 누가 관리 | 수명 | 빠르기 |
|-------|----------|------|--------|
| Stack | 컴파일러 (자동) | 함수 진입~종료 | 매우 빠름 (단순 포인터 이동) |
| Heap  | 프로그래머 (`new`/`delete` 또는 스마트 포인터) | 직접 해제할 때까지 | 상대적으로 느림 (할당자 호출, 단편화) |
| Data/BSS | 컴파일러+OS | 프로그램 수명 전체 | — |
| Text  | OS | 프로그램 수명 전체 | — |

**경험칙**: 작은 객체, 짧은 수명 → 스택. 크기를 런타임에 결정해야 하거나 함수 바깥까지 살아야 함 → 힙.

## 2. 스택 변수의 수명을 눈으로 확인하기"""))

cells.append(code(r"""#include <iostream>

void show_addr() {
    int local = 42;
    std::cout << "local 의 주소: " << &local << ", 값: " << local << "\n";
}

show_addr();
show_addr();   // 같은 주소? 다른 주소? 환경마다 다를 수 있다."""))

cells.append(md(r"""두 번 호출했을 때 같은 주소가 찍힐 가능성이 높다 — 함수가 끝나면서 그 스택 프레임이 회수되고, 다음 호출이 같은 자리를 재사용했기 때문이다. **즉, 함수가 반환된 뒤의 지역 변수 주소는 의미가 없다**. 다음과 같은 코드는 절대로 쓰면 안 된다:

```cpp
int* dangerous() {
    int x = 42;
    return &x;     // 함수가 끝나는 순간 x는 사라짐. 댕글링 포인터.
}
```

이 코드는 컴파일은 되지만 호출자가 그 포인터를 역참조하는 순간 **정의되지 않은 동작 (UB)**이다. 운이 좋으면 크래시, 운이 나쁘면 가짜 값이 들어가서 한참 뒤에 알아챈다.

## 3. 포인터 — 그냥 주소를 담는 변수"""))

cells.append(code(r"""#include <iostream>

int x = 100;
int* p = &x;     // p 는 int 를 가리키는 포인터. & 는 '주소 가져오기' 연산자.

std::cout << "x  = " << x  << "\n";
std::cout << "p  = " << p  << "  (x 의 주소)\n";
std::cout << "*p = " << *p << "  (* 는 역참조 — 그 주소의 값)\n";

*p = 200;        // 포인터를 통해 x 를 수정
std::cout << "x  = " << x  << "  (200 으로 바뀜)\n";"""))

cells.append(md(r"""포인터에서 헷갈리기 쉬운 것 두 가지:

### (a) `int*` vs `const int*` vs `int* const` vs `const int* const`

```cpp
int x = 1, y = 2;

int*       p1 = &x;   // p1 도 *p1 도 둘 다 바뀔 수 있다
const int* p2 = &x;   // *p2 는 못 바꿈 (가리키는 값은 const). p2 자체는 다른 주소를 가리킬 수 있다
int* const p3 = &x;   // p3 는 못 바꿈 (포인터 자체 const). *p3 는 바꿀 수 있다
const int* const p4 = &x;  // 둘 다 못 바꿈
```

읽는 요령: **`*` 를 기준으로 좌측은 가리키는 값, 우측은 포인터 자체에 대한 자격**.

### (b) `nullptr`

```cpp
int* p = nullptr;     // C++11 이후의 권장 표기. NULL/0 보다 안전 (타입이 분명).
if (p) { /* p 가 유효한 주소면 */ }
```"""))

cells.append(md(r"""## 4. 참조 — 별명

참조는 "이미 존재하는 변수에 다른 이름을 붙이는 것"이다."""))

cells.append(code(r"""#include <iostream>

int x = 100;
int& r = x;       // r 은 x 의 별명. 선언과 동시에 반드시 초기화되어야 한다.

std::cout << "x = " << x << ", r = " << r << "\n";
r = 200;          // 이건 r 이 가리키는 곳을 바꾸는 게 아니라 x 를 바꾸는 것.
std::cout << "x = " << x << ", r = " << r << "\n";

std::cout << "&x = " << &x << "\n";
std::cout << "&r = " << &r << "  (같은 주소!)\n";"""))

cells.append(md(r"""포인터 vs 참조 — 무엇이 다른가:

| 항목 | 포인터 (`T*`) | 참조 (`T&`) |
|------|--------------|-------------|
| 초기화 안 한 채 선언 | 가능 (위험) | 불가능 |
| `nullptr` 가능 | 가능 | 불가능 (항상 유효한 객체를 참조) |
| 다른 객체로 재바인딩 | 가능 (`p = &y`) | 불가능. `r = y` 는 *값 대입* |
| 산술 연산 | `p++` 등 가능 | 없음 |
| 문법 | `*p`, `p->mem` | `r`, `r.mem` (그냥 변수처럼) |

**언제 무엇을 쓰는가?** — 함수 인자에서:

- 인자를 수정하지 않고, 작은 타입 (int, double): 그냥 값으로 받기
- 인자를 수정하지 않고, 큰 타입 (std::string, struct, vector): `const T&`
- 인자를 수정해야 함, 항상 유효한 객체: `T&`
- 인자를 수정해야 함, '없음(null)' 도 의미가 있음: `T*`

이 규칙을 정확히 따르면 코드 리뷰의 절반은 줄어든다.

## 5. 힙 — `new` 와 `delete` (그리고 왜 직접 쓰면 안 되는가)"""))

cells.append(code(r"""#include <iostream>

int* p = new int(42);   // 힙에 int 한 개 할당, 42로 초기화. p 는 그 주소.
std::cout << *p << "\n";
delete p;               // 반드시 짝이 맞아야 한다.
p = nullptr;            // 댕글링 포인터를 막는 좋은 습관

int* arr = new int[5]{1,2,3,4,5};   // 배열 할당
delete[] arr;           // 배열은 delete[] — delete 와 짝이 안 맞으면 UB"""))

cells.append(md(r"""문제는 `new` 가 한 줄, `delete` 가 다른 줄에 있다는 점이다. 함수 중간에 예외가 던져지거나 early return 이 끼면 `delete` 가 호출되지 않고, 그 메모리는 **누수**된다.

```cpp
void leaky() {
    int* p = new int(42);
    if (some_check()) return;    // 누수!
    do_something(p);
    delete p;
}
```

이 문제를 해결하기 위해 C++은 **RAII** (Resource Acquisition Is Initialization) 라는 패턴을 표준 라이브러리에서 강제한다. **객체의 수명이 자원의 수명을 지배한다** 가 한 줄 정의다. 스택 객체는 함수가 끝날 때 *반드시* 소멸자가 호출되므로, 자원 해제 코드를 소멸자에 넣어두면 누수가 원천적으로 불가능하다.

스마트 포인터 (`std::unique_ptr`, `std::shared_ptr`)는 이 패턴을 포인터에 적용한 것이다 — Lesson 03에서 본격적으로 다룬다.

**현대 C++의 대원칙**: 학습 이외의 목적으로는 `new`/`delete` 를 직접 쓰지 마라. `std::make_unique`, `std::make_shared`, 또는 `std::vector` 같은 컨테이너를 써라."""))

cells.append(md(r"""## 6. 메모리 레벨에서 본 std::vector

`std::vector<int> v` 는 스택에 작은 객체 (보통 24바이트 정도: 데이터 포인터 + 크기 + 용량) 를 만들고, 실제 원소들은 *힙*에 둔다. 그래서 `v` 자체는 빠르게 복사·전달되지 않지만 — 잠깐, 원소가 1억 개면? 복사 비용이 폭발한다. 이게 Lesson 03 에서 다룰 **move 시맨틱**이 등장한 이유다.

직접 보자:"""))

cells.append(code(r"""#include <iostream>
#include <vector>

std::vector<int> v;
std::cout << "sizeof(vector<int>) = " << sizeof(v) << " bytes (스택)\n";
std::cout << "초기 capacity: " << v.capacity() << "\n";

for (int i = 0; i < 20; ++i) {
    v.push_back(i);
    std::cout << "push " << i << " → size=" << v.size()
              << ", capacity=" << v.capacity()
              << ", data=" << static_cast<const void*>(v.data()) << "\n";
}"""))

cells.append(md(r"""실행 결과를 잘 보면:

- `capacity` 는 1, 2, 4, 8, 16, 32 ... 처럼 *지수적으로* 늘어난다 (구현에 따라 1.5배인 경우도 있음). 이게 amortized O(1) push_back 의 비밀.
- `data()` 의 주소가 capacity 가 늘어나는 시점에 *바뀐다*. 즉 vector 가 재할당되면 기존 원소들의 주소는 무효화된다 — 만약 원소를 가리키는 포인터/참조를 보관하고 있었다면 댕글링이 된다.

이걸 알면 다음 코드가 왜 위험한지 바로 보인다:

```cpp
std::vector<int> v = {1,2,3};
int& r = v[0];        // 첫 원소 참조
v.push_back(4);       // 재할당이 일어나면 r 은 댕글링!
std::cout << r;       // UB
```"""))

cells.append(md(r"""## 7. 디버깅 도구 — Valgrind 와 AddressSanitizer 한 번 보기

이 도구들이 어떻게 메모리 버그를 잡는지 한 번 본 사람과 안 본 사람의 디버깅 능력은 두 배 이상 차이가 난다.

다음 셀에서는 일부러 잘못된 코드를 만들어 g++ 의 **AddressSanitizer (ASan)** 로 검출해본다 — Jupyter 의 system 셸 호출을 사용한다.

> 💡 셸 명령은 `!` 로 시작하지만 xeus-cling 은 `!` 를 지원하지 않는다. 대신 `system(...)` 함수를 써서 프로세스를 띄운다."""))

cells.append(code(r"""#include <cstdlib>
#include <iostream>

// 일부러 버그가 있는 프로그램을 파일로 쓴다
const char* buggy = R"CPP(
#include <iostream>
int main() {
    int* p = new int[10];
    p[15] = 42;          // 범위 밖 쓰기 (heap-buffer-overflow)
    delete[] p;
    p[0] = 1;            // 해제 후 사용 (use-after-free)
    return 0;
}
)CPP";

std::system("mkdir -p /tmp/cpplab");
{
    FILE* f = std::fopen("/tmp/cpplab/buggy.cpp", "w");
    std::fputs(buggy, f);
    std::fclose(f);
}

std::cout << "── 빌드 (AddressSanitizer 켜고) ──\n";
std::system("g++ -std=c++17 -O1 -g -fsanitize=address -fno-omit-frame-pointer "
            "/tmp/cpplab/buggy.cpp -o /tmp/cpplab/buggy 2>&1");

std::cout << "\n── 실행 ──\n";
std::system("/tmp/cpplab/buggy 2>&1 | head -40");"""))

cells.append(md(r"""ASan 출력을 천천히 읽어보면 이런 정보가 들어 있다:

- 어떤 종류의 에러인지 (heap-buffer-overflow, use-after-free, ...)
- 잘못 접근한 주소와, 그 주소가 *어떤 할당의 어디에 위치한 메모리*인지 (예: "8 bytes to the right of 40-byte region")
- 호출 스택 (어느 줄에서 일어났는지)

실제 프로젝트에서는 디버그 빌드에 항상 `-fsanitize=address,undefined` 를 켜둔다. 거의 모든 메모리 버그가 첫 실행에서 정확히 잡힌다."""))

cells.append(md(r"""## 8. 실습"""))

cells.append(code(r"""#include <iostream>
#include <vector>

// TODO 1: swap 함수를 두 가지 버전으로 작성하라.
//   (a) 포인터 버전:  void swap_ptr(int* a, int* b)
//   (b) 참조 버전:    void swap_ref(int& a, int& b)
//   호출 예와 비교해 어느 쪽이 더 읽기 쉬운지 생각해본다.

void swap_ptr(int* a, int* b) {
    // 채워라
}

void swap_ref(int& a, int& b) {
    // 채워라
}

int x = 1, y = 2;
swap_ptr(&x, &y);
std::cout << "after swap_ptr: x=" << x << ", y=" << y << "\n";

int p = 10, q = 20;
swap_ref(p, q);
std::cout << "after swap_ref: p=" << p << ", q=" << q << "\n";"""))

cells.append(md(r"""## 9. 정리

- 스택은 자동, 힙은 수동. 함수 지역 변수의 주소를 함수 밖으로 내보내지 마라.
- 포인터는 변수, 참조는 별명. nullable 이거나 재바인딩이 필요하면 포인터, 아니면 참조.
- `new`/`delete` 는 직접 쓰지 마라 — 다음 강의에서 스마트 포인터로 대체한다.
- vector 는 스택에 살지만 원소는 힙에 있다. 재할당이 일어나면 원소 포인터는 무효화된다.
- AddressSanitizer 는 거의 공짜로 메모리 버그를 잡아준다. 디버그 빌드에서는 그냥 켜라.

다음 강의 — Lesson 03: 클래스, RAII, 스마트 포인터의 모든 것.
"""))

nb = make_notebook(cells, kernel="xcpp17")
save(nb, "/home/claude/cpp_learning/notebooks/02_memory_model.ipynb")
