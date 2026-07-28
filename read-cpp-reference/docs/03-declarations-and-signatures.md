# 03. 선언과 call signature 해독

## “signature”라는 말의 주의점

일상적으로 call signature는 함수 이름, 매개변수 타입, 반환 타입, 한정자를 포함한 “호출
모양”을 뜻합니다. 그러나 C++ 표준에서 **function signature**는 overload 구분 등에 쓰는
더 엄밀한 용어이며 반환 타입은 일반 함수 signature에 포함되지 않습니다. 문맥을 확인하세요.

## 선언을 읽는 고정 순서

복잡한 선언은 다음 순서로 분해합니다.

1. 이름을 찾는다.
2. 이름 바로 오른쪽의 `()`, `[]`를 본다.
3. 이름 왼쪽으로 가며 포인터/참조와 기본 타입을 본다.
4. 함수라면 괄호 뒤의 cv/ref 한정자, `noexcept`, `override` 등을 본다.
5. 템플릿 머리와 attribute를 마지막에 결합한다.

```cpp
[[nodiscard]] constexpr const T& get(std::size_t i) const & noexcept;
```

| 조각 | 의미 |
|---|---|
| `[[nodiscard]]` | 반환값을 무시하면 경고를 권고 |
| `constexpr` | 조건이 맞으면 상수 평가 가능 |
| `const T&` | const T에 대한 lvalue 참조 반환 |
| `get` | 함수 이름 |
| `std::size_t i` | 인덱스 매개변수 |
| 뒤의 `const` | `*this`를 변경하지 않는 멤버 함수 |
| `&` | lvalue 객체에서만 호출 가능 |
| `noexcept` | 예외를 전파하지 않음 |

## 포인터·참조·const

이름에서 바깥으로 읽습니다.

```cpp
const int* p1;       // const int를 가리키는 포인터: *p1 수정 불가, p1 변경 가능
int const* p2;       // p1과 같음
int* const p3 = ...; // 고정된 포인터: *p3 수정 가능, p3 변경 불가
const int* const p4 = ...; // 둘 다 const
int& ref = value;    // lvalue 참조
const int& cref = 3; // const lvalue 참조는 임시 값에도 바인딩 가능
int&& rref = 3;      // rvalue 참조
```

`const`는 기본적으로 바로 왼쪽을 꾸미고, 왼쪽이 없으면 오른쪽을 꾸민다고 읽으면 도움이
됩니다. 참조 자체는 한 번 바인딩되면 다른 객체를 가리키도록 바뀌지 않습니다.

## 함수 포인터와 “가장 안쪽부터”

```cpp
int (*operation)(double);
```

`operation` → `*operation`은 포인터 → `(*operation)(double)`은 `double`을 받는 함수를
가리킴 → 그 함수가 `int`를 반환합니다.

```cpp
using Operation = int (*)(double);
Operation op = nullptr;
```

실전에서는 `using` 별칭으로 난도를 낮추세요. `int* function(double)`은 함수 포인터가
아니라 `int*`를 반환하는 함수입니다.

## 멤버 함수 뒤에 붙는 것

```cpp
struct Box {
    int read() const;             // const 객체에서 호출 가능
    int read_volatile() volatile; // volatile 객체에서 호출 가능
    void use() &;                 // lvalue Box에서만
    void consume() &&;            // rvalue Box에서만
    void safe() noexcept;         // 예외가 밖으로 나가면 terminate
    virtual void draw();
    void reset() = delete;        // 호출 금지
};
```

- 뒤의 `const`/`volatile`은 암시적 객체 매개변수 `*this`의 cv 한정입니다.
- `&`/`&&`는 `*this`의 value category를 제한하는 ref-qualifier입니다.
- `noexcept(expr)`는 `expr`이 참일 때 비투척 명세입니다.
- `virtual`, `override`, `final`은 동적 다형성 관련입니다.
- `= default`는 컴파일러가 특별 멤버 함수를 생성하게 합니다.
- `= delete`는 해당 overload가 선택되면 컴파일 오류가 되게 합니다.

## 완전 해부: `bool is_lock_free() const volatile noexcept;`

이 선언은 [`std::atomic<T>::is_lock_free`](https://cppreference.com/cpp/atomic/atomic/is_lock_free)의
두 번째 overload입니다.

```text
bool  is_lock_free  ( )  const volatile  noexcept  ;
│     │             │    │               │         └ 선언 끝
│     │             │    │               └ 예외를 전파하지 않음
│     │             │    └ *this가 const volatile
│     │             └ 명시적인 매개변수 없음
│     └ 멤버 함수 이름
└ bool 반환
```

“`volatile` bool을 반환한다”가 아닙니다. `volatile`이 매개변수에 붙은 것도 아닙니다.
멤버 함수 뒤에 있으므로 이 함수를 호출하는 객체에 붙습니다.

대략적인 암시적 객체 매개변수를 상상하면 이해하기 쉽습니다(실제 소스 문법은 아님).

```cpp
bool is_lock_free(/* const volatile std::atomic<T>* this */) noexcept;
```

같은 페이지의 첫 overload는 다음과 같습니다.

```cpp
bool is_lock_free() const noexcept;
```

일반 `const` 객체에는 첫 번째가, `volatile`까지 붙은 객체에는 두 번째가 대응합니다.
`volatile`의 실제 목적과 한계는 [동시성 문서](07-concurrency-volatile-atomic.md#volatile)에
있습니다.

## overload와 overload resolution

같은 scope에서 이름은 같고 매개변수 타입/개수나 멤버 한정자가 다른 함수를 둘 수 있습니다.
컴파일러는 인자 변환의 품질, 템플릿 규칙 등을 비교해 가장 적합한 하나를 선택합니다.

```cpp
void print(int);
void print(double);
void print(std::string_view);
```

반환 타입만 다르게 overload할 수는 없습니다. 기본 인자는 overload가 아니라 호출 시
생략한 인자를 채우는 규칙입니다.

## `auto`와 `decltype`

```cpp
const int x = 1;
auto a = x;        // int: top-level const 제거
const auto b = x;  // const int
auto& c = x;       // const int&
decltype(x) d = 2; // const int
decltype((x)) e=x; // const int&: 괄호 친 이름 표현식의 category 반영
```

`decltype(auto)`는 `decltype` 규칙을 유지하는 placeholder입니다. 참조를 뜻하지 않을 수도
있으므로 반환 수명에 특히 주의합니다.

## 템플릿 선언

```cpp
template<class T, std::size_t N>
const T& first(const T (&array)[N]) noexcept;
```

- `T`: 타입 템플릿 매개변수
- `N`: non-type 템플릿 매개변수
- `const T (&array)[N]`: 크기 `N`인 T 배열에 대한 참조
- 인자에서 `T`, `N`을 추론할 수 있음

`typename`과 `class`는 타입 템플릿 매개변수 자리에서 거의 같은 뜻입니다. dependent name,
specialization, SFINAE, concepts는 [클래스·템플릿 문서](05-classes-templates-callables.md)를
참고하세요.

## 자주 보이는 선언 조각

| 조각 | 읽기 |
|---|---|
| `static` 멤버 | 객체 없이 타입에 속하는 멤버 |
| `inline` | ODR을 만족하며 여러 번 정의 가능; 최적화 명령이 아님 |
| `friend` | 지정된 함수/타입에 private 접근 허용 |
| `explicit` | 원치 않는 암시적 생성 변환 억제 |
| `mutable` | const 객체에서도 해당 데이터 멤버 수정 허용 |
| `extern` | 정의가 다른 곳에 있음을 알리는 데 사용 |
| `thread_local` | 스레드마다 별도 객체 |
| `[[nodiscard]]` | 결과 무시 진단 권고 |
| `[[deprecated]]` | 사용 중단 진단 권고 |
| `requires` | 템플릿 제약(C++20) |
| `consteval` | 호출이 반드시 상수 평가(C++20) |
| `constinit` | 정적/스레드 저장 객체의 정적 초기화 요구(C++20) |

다음: [타입·값·수명](04-types-values-lifetime.md)
