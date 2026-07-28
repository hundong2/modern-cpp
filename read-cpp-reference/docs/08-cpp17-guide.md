# 08. C++17 문서 읽기 가이드

원문: [cppreference C++17](https://cppreference.com/cpp/17)

## 읽기 전에

C++17 페이지는 입문 목차가 아니라 변경 목록입니다. 다음 선행 개념이 필요합니다.

- 선언/cv/`noexcept`: [선언 해독](03-declarations-and-signatures.md)
- 값·수명·이동: [타입·값·수명](04-types-values-lifetime.md)
- 템플릿/lambda/pack: [클래스·템플릿](05-classes-templates-callables.md)
- 컨테이너/iterator/algorithm: [라이브러리 어휘](06-standard-library-vocabulary.md)

## 추천 읽기 순서

### 1. 매일 쓰는 언어 기능

#### 구조적 바인딩

```cpp
auto [key, value] = pair;
auto& [x, y] = point;       // 원본을 참조
const auto& [name, score] = record;
```

배열, tuple-like 타입, public 데이터 멤버 aggregate를 분해합니다. `auto`/`auto&`에 따라
복사인지 참조인지 달라집니다.

#### `if`/`switch` initializer

```cpp
if (auto it = map.find(key); it != map.end()) {
    // it의 scope가 if/else로 제한됨
}
```

세미콜론 앞은 초기화, 뒤는 조건입니다.

#### `if constexpr`

```cpp
template<class T>
void print(T value) {
    if constexpr (std::is_integral_v<T>) {
        // 선택되지 않은 branch는 해당 specialization에서 인스턴스화되지 않음
    } else {
        // ...
    }
}
```

일반 `if`와 달리 compile-time branch 선택입니다. 조건은 constant expression이어야 합니다.

#### class template argument deduction(CTAD)

```cpp
std::pair p{1, 2.0}; // std::pair<int, double>
```

생성자 인자와 deduction guide로 클래스 템플릿 인자를 추론합니다. 함수 템플릿 추론과
동일하지 않으며, 모호하면 템플릿 인자를 명시합니다.

#### fold expression

```cpp
template<class... Ts>
auto sum(Ts... xs) {
    return (xs + ...);
}
```

unary/binary, left/right fold 네 형태와 빈 pack의 규칙을 원문 표에서 확인합니다.

#### lambda `[*this]`

`this` 포인터가 아니라 현재 객체를 값으로 capture합니다. 비동기 실행에서 원래 객체의
수명과 분리할 때 유용하지만, 복사 비용과 멤버의 복사 가능성을 확인합니다.

### 2. 안전하고 표현력 좋은 라이브러리 타입

#### `std::optional<T>` — 값이 있을 수도 없음

```cpp
std::optional<int> parse(std::string_view text);
```

확인은 `if (result)`, 접근은 `*result`, `result.value()`, 기본값은 `value_or`.
빈 `optional`의 `operator*` 사용은 잘못이며 `value()`는 `bad_optional_access`를 던질 수
있습니다.

#### `std::variant<Ts...>` — 여러 타입 중 정확히 하나

`std::get<T>`, `std::get_if<T>`, `std::visit`를 사용합니다. C union보다 활성 대안을
추적하고 수명을 관리합니다. 중복 타입과 `valueless_by_exception`을 확인하세요.

#### `std::any` — 실행 시간 타입 삭제 저장소

`std::any_cast<T>`로 정확한 타입을 꺼냅니다. 닫힌 대안 집합이면 `variant`가 보통 더
명시적이고 효율적입니다.

#### `std::string_view` — 비소유 문자열 view

복사 없이 문자 구간을 봅니다. null 종료를 보장하지 않고 원본보다 오래 살면 dangling입니다.
리터럴/장수 문자열을 보는 데 안전하며 임시 `std::string` 저장은 주의합니다.

#### `std::filesystem`

`path`, `directory_iterator`, `exists`, `file_size`, `copy` 등을 제공합니다. 운영체제별 경로
표현, 권한/경쟁 조건, 예외 overload와 `error_code` overload를 확인합니다.

### 3. 동시성과 호출 도구

- `std::scoped_lock`: 여러 mutex를 deadlock 회피 방식으로 함께 잠글 수 있는 RAII wrapper
- `std::atomic<T>::is_always_lock_free`: 구현에서 항상 lock-free인지 compile-time 질의
- `std::apply`: tuple 원소를 함수 인자로 펼침
- `std::invoke`: 일반 함수/멤버 포인터/함수 객체를 통일해 호출
- `std::is_invocable`: 해당 호출이 유효한지 compile-time 확인
- 병렬 알고리즘/execution policy: 실행 순서, 예외 처리, thread-safety 요구를 먼저 공부

`is_always_lock_free`는 [atomic 문서](07-concurrency-volatile-atomic.md#lock-free의-단계)로
연결됩니다.

### 4. 성능/인프라 기능

- `<charconv>`: locale 비의존, 비할당 숫자↔문자 변환
- `<memory_resource>`: polymorphic allocator와 memory resource
- node handle `extract`/`merge`: map/set node를 재할당 없이 이동
- `try_emplace`, `insert_or_assign`: map 삽입 의도를 분명히 함
- `std::byte`: 숫자가 아닌 raw byte 표현
- `std::clamp`, `gcd`, `lcm`, scan/reduce
- `std::size`, `std::empty`, `std::data`
- uninitialized memory algorithms: 객체 수명 관리의 고급 영역
- over-aligned allocation, `std::launder`: 저수준 수명/정렬 심화 영역
- hardware interference size: cache line 간섭 최적화 힌트

## 새 언어 기능 전체 점검표

원문 목록을 놓치지 않기 위한 색인입니다.

- [ ] `u8` character literal
- [ ] `noexcept`가 함수 타입 시스템의 일부
- [ ] evaluation order 규칙 강화
- [ ] lambda의 `*this` capture
- [ ] `if constexpr`, constexpr lambda
- [ ] inline variable
- [ ] structured binding
- [ ] `if`/`switch` initializer
- [ ] guaranteed copy elision, temporary materialization
- [ ] fold expression
- [ ] CTAD와 deduction guide
- [ ] non-type template parameter에 `auto`
- [ ] 간소화한 nested namespace `namespace a::b {}`
- [ ] 여러 이름의 using-declaration
- [ ] attribute namespace 반복 생략
- [ ] `[[fallthrough]]`, `[[maybe_unused]]`, `[[nodiscard]]`
- [ ] `__has_include`

## 새 헤더 전체

`<any>`, `<charconv>`, `<execution>`, `<filesystem>`, `<memory_resource>`,
`<optional>`, `<string_view>`, `<variant>`.

## 주요 제거·폐기

제거:

- `std::auto_ptr` → `std::unique_ptr`
- `std::random_shuffle` → `std::shuffle`
- 오래된 함수 객체 adapter, obsolete iostream alias
- trigraph
- `register` 키워드의 저장 지정자 용도
- `bool`에 대한 `++`
- dynamic exception specification `throw(T)` → `noexcept`/일반 예외 모델

deprecated:

- `std::iterator`, `std::raw_storage_iterator`, `std::get_temporary_buffer`
- `std::is_literal_type`, `std::result_of`
- `<codecvt>` 전체

오래된 코드 읽기에는 제거된 이름도 필요하지만 새 코드에 복원해 쓰지는 않습니다.

## 추천 실습 질문

[`07_cpp17.cpp`](../examples/07_cpp17.cpp)를 실행한 뒤:

1. 구조적 바인딩에서 `auto`를 `auto&`로 바꾸면 원본이 바뀌는가?
2. `optional.value()`를 빈 상태에서 호출하면 무엇이 일어나는가?
3. 지역 `std::string`으로 만든 `string_view`를 반환하면 왜 위험한가?
4. `variant`에 같은 타입을 두 번 넣으면 `get<T>`는 어떻게 되는가?
5. `if constexpr`를 일반 `if`로 바꾸면 선택되지 않은 코드도 컴파일되는가?
