# 12. 실전 선언 해독 문제

먼저 “반환 타입 → 이름 → 명시적 인자 → 멤버 cv/ref → 예외/기타 조건”을 종이에 적고,
각 문제 바로 아래의 해설을 나중에 펼쳐 읽는 방식으로 사용하세요. 실제 라이브러리에는
allocator와 표준 버전에 따라 선언이 더 복잡한 overload가 있을 수 있습니다.

## 1. 읽기 전용 원소 접근

```cpp
const_reference at(size_type pos) const;
```

<details>
<summary>해설</summary>

`size_type` 위치를 받아 `const_reference`를 반환하는 const 멤버 함수입니다. `at`은
일반적으로 범위를 검사하고 잘못된 위치에는 `std::out_of_range`를 던집니다.
`const_reference`와 `size_type`은 해당 컨테이너가 제공하는 member type alias입니다.
</details>

## 2. const/non-const overload

```cpp
T* data() noexcept;
const T* data() const noexcept;
```

<details>
<summary>해설</summary>

비-const 객체에서는 수정 가능한 요소 포인터, const 객체에서는 const 요소 포인터를
반환합니다. 두 번째 `const`는 반환 타입의 `const T*`와 별개로 `*this`를 한정합니다.
</details>

## 3. 조건부 `noexcept`

```cpp
void swap(vector& other)
    noexcept(allocator_traits<Allocator>::propagate_on_container_swap::value
             || allocator_traits<Allocator>::is_always_equal::value);
```

<details>
<summary>해설</summary>

같은 `vector` 타입의 수정 가능한 참조를 받아 교환합니다. 괄호 속 compile-time bool이
참일 때만 비투척 명세입니다. `noexcept(expression)`은 함수를 호출하는 것이 아니라
예외 명세 조건입니다.
</details>

## 4. variadic forwarding reference

```cpp
template<class... Args>
std::pair<iterator, bool> emplace(Args&&... args);
```

<details>
<summary>해설</summary>

0개 이상의 타입/함수 parameter pack을 받아 요소를 제자리 생성합니다. `Args`가 추론되므로
각 `Args&&`는 forwarding reference입니다. 반환 pair는 보통 iterator와 새 삽입 성공 여부를
담습니다. 생성 가능 요구 조건, 중복 key 동작, iterator 무효화를 추가로 봐야 합니다.
</details>

## 5. 변환 함수

```cpp
explicit operator bool() const noexcept;
```

<details>
<summary>해설</summary>

함수 이름이 `operator bool`인 사용자 정의 변환 함수라 별도 반환 타입 표기가 없습니다.
`explicit`이므로 일반 암시적 산술 변환은 막되 `if (object)` 같은 contextual conversion to
bool에는 사용할 수 있습니다.
</details>

## 6. 삭제한 overload

```cpp
basic_string(std::nullptr_t) = delete;
```

<details>
<summary>해설</summary>

`nullptr`로 문자열을 만들려는 호출이 다른 pointer overload로 잘못 들어가지 않도록 해당
생성자를 후보로 두되 선택 시 컴파일 오류로 만듭니다.
</details>

## 7. iterator를 받는 erase

```cpp
iterator erase(const_iterator pos);
```

<details>
<summary>해설</summary>

읽기 전용 위치 iterator로 지울 대상을 지정하고, 보통 지운 요소 다음 위치의 iterator를
반환합니다. 반환값을 무시해도 되는지, 어떤 iterator/reference가 무효화되는지는 컨테이너마다
다릅니다.
</details>

## 8. trailing return type

```cpp
template<class F, class... Args>
auto invoke(F&& f, Args&&... args)
    -> std::invoke_result_t<F, Args...>;
```

<details>
<summary>해설</summary>

`auto` 뒤 `->`에 실제 반환 타입을 쓴 함수 템플릿입니다. 호출 가능 객체와 여러 인자를
forwarding reference로 받고, 그 호출 결과 타입을 type trait으로 계산합니다.
</details>

## 9. ref-qualified 접근자

```cpp
const std::string& name() const & noexcept;
std::string name() && noexcept;
```

<details>
<summary>해설</summary>

수명이 지속되는 const lvalue 객체에서는 내부 문자열을 참조로 빌리고, 곧 사라질 rvalue
객체에서는 dangling을 피하도록 문자열 값을 반환할 수 있는 설계입니다.
</details>

## 10. 가상 함수

```cpp
virtual std::unique_ptr<Base> clone() const = 0;
```

<details>
<summary>해설</summary>

const 객체를 복제해 단독 소유 base pointer를 반환하는 pure virtual 함수입니다. 이 함수가
있는 클래스는 추상 클래스입니다. 파생 구현에는 `override`를 붙입니다.
</details>

## 11. compile-time 멤버 상수

```cpp
static constexpr size_type npos = size_type(-1);
```

<details>
<summary>해설</summary>

객체마다 있지 않고 타입에 속하는 compile-time 상수입니다. unsigned `size_type`으로 변환한
`-1`은 보통 그 타입의 최댓값이 되어 “찾지 못함” sentinel로 쓰입니다.
</details>

## 12. C++20 제약

```cpp
template<std::input_iterator I, std::sentinel_for<I> S, class O>
requires std::indirectly_copyable<I, O>
constexpr ranges::copy_result<I, O> operator()(I first, S last, O result) const;
```

<details>
<summary>해설</summary>

입력 iterator `I`, 그 끝 sentinel `S`, 출력 `O`에 concept 제약이 붙은 호출 연산자입니다.
`requires`는 간접 복사가 가능해야 함을 추가합니다. 결과는 최종 input/output 위치를 묶은
타입입니다. 실제 선언의 overload와 projection 여부도 함께 확인합니다.
</details>

## 13. compare-exchange

```cpp
bool compare_exchange_weak(
    T& expected,
    T desired,
    std::memory_order success,
    std::memory_order failure) noexcept;
```

<details>
<summary>해설</summary>

현재 값이 `expected`와 같으면 `desired`를 저장합니다. 실패하면 실제 현재 값을
`expected`에 씁니다. 따라서 첫 인자는 input/output 참조입니다. 성공/실패 memory order의
허용 조합을 별도로 확인해야 하며 `weak`은 spurious failure가 가능해 보통 loop에서 씁니다.
</details>

## 14. 처음의 선언

```cpp
bool is_lock_free() const volatile noexcept;
```

<details>
<summary>해설</summary>

인자가 없고 bool을 반환하는 `std::atomic<T>` 멤버 함수입니다. `const volatile *this`에서
호출 가능하고 예외를 전파하지 않습니다. `volatile`이 atomicity를 제공하는 것은 아닙니다.
[동시성 문서](07-concurrency-volatile-atomic.md#예시-선언을-다시-읽기)에서 의미를 다시
확인하세요.
</details>

## 합격 기준

각 선언마다 다음을 말할 수 있으면 됩니다.

- 어떤 이름이 타입이고 어떤 이름이 객체/함수인가?
- 소유/비소유, 값/참조/포인터 중 무엇을 주고받는가?
- const가 가리키는 대상은 반환값, pointee, pointer, `*this` 중 무엇인가?
- overload를 가르는 부분은 무엇인가?
- 수명, 예외, 무효화, 복잡도, 동시성 중 추가 확인할 항목은 무엇인가?
- 어느 C++ 표준 버전부터 유효한가?
