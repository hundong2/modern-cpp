# 소유권, 어휘 타입, 타입 특성

## `std::unique_ptr<T>`와 `std::make_unique<T>` — `<memory>`

- `unique_ptr`는 객체 하나의 독점 소유권을 표현한다. 복사는 금지되고 이동만 가능하다.
- 소유 포인터가 파괴되거나 `reset`되면 저장된 deleter로 객체를 해제한다.
- `operator*`와 `operator->`는 가리키는 객체에 접근한다. 빈 포인터에서 사용하면 안 된다.
- `get()`은 비소유 원시 포인터를 반환하고 소유권을 넘기지 않는다.
- `release()`는 포인터를 반환하며 자동 해제를 포기한다. 호출자가 새 소유자를 즉시 정하지 않으면 누수가 난다.
- `reset(next)`은 기존 객체를 해제하고 새 포인터를 소유한다.
- `make_unique<T>(args...)`는 `T`를 동적 생성하고 `unique_ptr<T>` prvalue를 반환한다. 직접 `new`보다 예외 안전성과 가독성이 좋다.
- 파생 객체를 기반 `unique_ptr<Base>`로 소유해 기반 포인터로 삭제한다면 기반 소멸자는 보통 가상이어야 한다.

## `std::shared_ptr<T>`, `std::weak_ptr<T>`, `std::make_shared<T>`

- `shared_ptr` 복사는 공유 참조 횟수를 늘리고 마지막 소유자가 사라질 때 객체를 파괴한다.
- 이동은 소유권 몫을 옮기며 원본은 보통 빈 상태가 된다.
- 참조 횟수 조작 자체는 스레드 안전하게 조정되지만 가리키는 `T`의 동시 변경까지 보호하지 않는다.
- 순환 참조는 참조 횟수가 0이 되지 않아 누수된다. 순환의 비소유 방향을 `weak_ptr`로 표현한다.
- `weak_ptr::lock()`은 객체가 살아 있으면 임시 `shared_ptr`, 만료됐으면 빈 `shared_ptr`를 반환한다. 검사와 수명 연장을 한 동작으로 묶는다.
- `expired()` 직후 다른 스레드에서 소멸할 수 있으므로 실제 사용에는 `lock()` 결과를 검사한다.
- `make_shared<T>(args...)`는 보통 객체와 제어 블록을 한 할당으로 만들며 `shared_ptr<T>`를 반환한다.
- 별도 자원 해제 정책이나 매우 큰 객체의 메모리 반환 시점 때문에 직접 생성이 필요한 경우도 있지만 기본 선택은 `make_shared`다.

### `shared_ptr` aliasing constructor와 부분 객체 수명

- 항목 종류와 헤더: `std::shared_ptr<T>`의 생성자 템플릿이며 `<memory>`에 선언된다. 대표 형태는 `template<class Y> shared_ptr(const shared_ptr<Y>& owner, element_type* stored) noexcept`다.
- 수신 객체와 인자: 아직 만들어지지 않은 새 핸들이 결과 객체다. 첫 인자 `owner`는 기존 shared owner를 `const` lvalue reference로 빌리고, 둘째 인자 `stored`는 결과 핸들이 `get()`, `operator*`, `operator->`로 관찰할 포인터 값을 복사한다. 둘째 포인터의 객체를 별도로 소유하거나 삭제하지 않는다.
- 반환과 사후 상태: 생성자는 반환값이 없다. 새 핸들의 **stored pointer**는 `stored`지만 **owned object/control block**은 `owner`와 같다. `owner`가 비어 있지 않다면 strong count가 하나 늘고, 새 핸들이 남아 있는 동안 outer owned object 전체의 수명이 유지된다. 두 상태는 독립적이라 empty owner와 non-null `stored`를 주면 `get()!=nullptr`인데 `use_count()==0`일 수 있고, 반대로 유효 owner와 null `stored`를 주면 `get()==nullptr`인데 strong count는 양수일 수 있다.
- 전제조건과 수명: 생성 자체는 서로 관련 없는 포인터도 받을 수 있지만, 나중에 역참조하려면 `stored`가 유효한 객체를 가리켜야 한다. 가장 안전한 전형은 `stored`가 owned object의 멤버·배열 원소처럼 그 수명 안에 포함되는 경우다. 지역 변수 주소나 owner보다 먼저 무효가 되는 컨테이너 원소 주소를 넣으면 control block이 있어도 댕글링을 막지 못한다.
- 복잡도·할당·예외: 제어 블록 참조 횟수와 작은 포인터 상태만 공유하므로 상수 시간이고 별도 동적 할당이 없으며 `noexcept`다. 복사된 owner와 새 alias는 서로 다른 shared pointer 객체이므로 각 핸들의 수명은 독립적이다.
- 무효화와 오류: alias를 만들었다고 outer object 내부 컨테이너의 재할당 규칙이 바뀌지 않는다. alias가 vector 원소를 가리킨다면 outer owner가 살아 있어도 vector 재할당 뒤 stored pointer는 무효다. null stored pointer에 `operator*`를 적용하면 전제조건 위반이다. `operator->()` 자체는 null stored pointer도 그대로 반환하지만, 그 결과에 내장 `->member` 접근을 이어 가면 null을 역참조해 미정의 동작이다.
- 스레드 보장: 같은 control block을 공유하는 **서로 다른** shared pointer 객체의 복사·파괴에 필요한 참조 횟수 조정은 안전하다. 그러나 같은 shared pointer 객체에 대한 동시 비const 조작이나 가리키는 `T`의 동시 읽기/쓰기는 별도 동기화가 필요하다. `use_count()`는 관찰 직후 다른 스레드가 바꿀 수 있으므로 동기화 판단 도구가 아니다.
- 현재 코드에서의 역할: 2026-09-03의 `title_handle()`과 `endpoint_handle()`은 `shared_ptr<const Outer>`의 제어 블록을 공유하면서 `const string*` 부분 객체만 API에 노출한다. 별칭 핸들이 살아 있으면 outer와 string이 함께 살아 있고, `const`로 변경 경로도 좁힌다.

관련 관찰자 계약은 다음과 같다.

- `operator->() const noexcept`는 데이터 인자 없이 stored pointer를 반환하며 그 호출 자체에는 non-null 전제조건이 없다. 반환 포인터로 멤버를 접근하려면 non-null이어야 한다. `operator*() const noexcept`는 `element_type&`를 반환하므로 stored pointer가 유효한 객체를 가리켜야 한다. 둘 다 상수 시간·무할당이고 참조 횟수를 바꾸지 않는다.
- `use_count() const noexcept`는 같은 control block의 현재 strong owner 수를 `long`으로 반환한다. 상수 시간이고 상태를 바꾸지 않는다. 빈 핸들은 0이며, 멀티스레드에서는 반환 직후 값이 달라질 수 있다.
- 이동 대입 `shared_ptr& operator=(shared_ptr&& other) noexcept`는 현재 수신자가 가진 몫을 해제하고 `other`의 stored pointer와 control-block 몫을 넘겨받는다. `other`는 빈 유효 상태가 되고 `*this`를 반환한다. 상수 시간이고 별도 할당은 없지만 기존 수신자가 마지막 owner였다면 관리 객체 소멸 비용이 이어질 수 있다.

## `std::optional<T>`와 `std::nullopt` — `<optional>`

- 값 `T`가 있거나 없는 두 상태를 표현하며 동적 할당은 필수가 아니다.
- `has_value()`와 `operator bool`은 값 존재 여부를 반환한다.
- `operator*`와 `operator->`는 값 존재를 호출자가 보장해야 한다. 빈 상태 접근은 잘못이다.
- `value()`는 값 참조를 반환하고 비어 있으면 `std::bad_optional_access`를 던진다.
- `value_or(fallback)`는 값이 있으면 복사/이동한 값, 없으면 대체값을 값으로 반환한다.
- `std::nullopt`는 빈 상태를 명시하는 태그 객체다.
- `optional<reference_wrapper<T>>`는 선택적 비소유 참조를 표현하지만 원본 수명을 연장하지 않는다.

## `std::expected<T, E>`와 `std::unexpected<E>` — `<expected>`

- 성공값 `T` 또는 오류값 `E` 중 정확히 하나를 보관하는 C++23 어휘 타입이다.
- `has_value()`와 `operator bool`은 성공 여부를 확인한다.
- `operator*`, `operator->`, `value()`는 성공값에 접근한다. `value()`는 오류 상태면 `bad_expected_access`를 던진다.
- `error()`는 오류 상태에서 `E`에 접근한다. 성공 상태에서 호출하지 않는다.
- `std::unexpected(error)`는 오류 상태를 명시적으로 구성한다.
- 예외를 던지지 않는다고 자동 보장하는 타입이 아니다. `T`/`E`의 생성·복사·이동이 예외를 던질 수 있다.
- 호출자가 성공과 실패를 처리하도록 함수 서명에 계약을 드러내는 장점이 있다.

## `std::variant<Ts...>`, `std::visit`, `std::get`, `std::get_if`

- `variant`는 후보 타입 중 하나를 같은 저장소에 보관하는 태그된 합 타입이다.
- 기본 생성은 첫 대안이 기본 생성 가능하면 그 타입을 보관한다.
- `std::holds_alternative<T>(value)`는 현재 대안이 `T`인지 반환한다.
- `std::get<T>(value)`는 `T` 참조를 반환하지만 다른 대안이면 `bad_variant_access`를 던진다.
- `std::get_if<T>(&value)`는 맞으면 포인터, 아니면 `nullptr`를 반환해 예외 없이 분기한다.
- `std::visit(visitor,value)`는 활성 대안을 방문자에 전달한다. 방문자는 가능한 모든 대안 조합에서 유효해야 한다.
- `visit`는 분기나 점프 테이블, 인라인 코드 등으로 구현될 수 있으며 구체 기계 형태는 구현과 최적화에 따라 다르다.

## `std::pair`, `std::tuple`, 구조적 바인딩

- `pair<T,U>`는 두 값을 `first`, `second`로 묶는다.
- `tuple<Ts...>`는 여러 서로 다른 타입을 위치 기반으로 묶는다.
- `std::get<I>(tuple)` 또는 `std::get<T>(tuple)`로 원소에 접근한다. 타입 기반 접근은 해당 타입이 정확히 한 번 있어야 한다.
- `auto [left,right] = pair;`는 값을 복사할 수 있고 `auto& [left,right]`는 기존 원소에 참조 바인딩한다.
- 우선순위 큐의 `(거리,정점)`처럼 작은 관계 값에 적합하지만 필드 의미가 중요하면 이름 있는 `struct`가 더 읽기 쉽다.

## `std::reference_wrapper<T>`와 `std::ref`/`std::cref`

- 일반 객체처럼 복사 가능하면서 내부에는 비소유 참조 의미를 보관한다.
- `get()`은 원본 `T&`를 반환하고 `operator T&` 변환도 제공한다.
- `std::ref(object)`는 `reference_wrapper<T>`, `std::cref(object)`는 읽기 전용 `reference_wrapper<const T>`를 만든다.
- 원본 수명을 연장하지 않으므로 컨테이너나 `optional`에 오래 저장할 때 수명을 검증한다.

## `std::function<Signature>` — `<functional>`

- 지정한 호출 서명을 만족하는 함수, 람다, 함수 객체를 타입 소거해 값으로 보관한다.
- 복사 가능한 호출 대상을 요구하며 내부 작은 객체 최적화 여부는 구현에 따라 다르다.
- 빈 `std::function`을 호출하면 `std::bad_function_call`을 던진다.
- 간접 호출·동적 할당 가능성이 있어 성능이 중요한 템플릿 경로에서는 구체 호출 타입이나 `auto` 매개변수를 검토한다.
- 서로 다른 명령을 한 컨테이너에 저장하거나 런타임 교체 가능한 콜백 경계에 유용하다.

## `std::move_only_function<Signature>` — `<functional>`

`std::move_only_function`은 C++23의 이동 전용 타입 소거 호출 래퍼다. `std::function`이 저장 호출 대상에 복사 가능성을 요구하는 것과 달리, `unique_ptr`를 값 캡처한 람다처럼 이동만 가능한 호출 대상을 소유할 수 있다.

- 대표 구성 형태 `template<class F> move_only_function(F&& f)`는 호출 가능한 객체 `f`를 전달 참조로 받고 래퍼 내부에 이동 또는 복사 구성한다. rvalue 람다는 이동하고 lvalue는 복사가 가능해야 한다. 생성자는 반환값이 없으며 내부 저장 전략에 따라 동적 할당과 `std::bad_alloc`, 호출 대상 생성자의 예외가 가능하다.
- 이동 생성·이동 대입은 저장 호출 대상의 소유권을 목적 래퍼로 옮긴다. 원본은 유효하지만 호출 대상 보유 여부는 이동 연산 계약에 따라 빈 상태로 다룬다. 복사 생성·복사 대입은 삭제되어 있다.
- `explicit operator bool() const noexcept`는 데이터 인자 없이 호출 대상 보유 여부를 `bool`로 반환한다. 수신 래퍼와 호출 대상 상태는 유지되고 할당·예외가 없다.
- `R operator()(Args... args)`는 서명의 cv/ref/noexcept 한정에 맞는 오버로드가 저장 호출 대상을 호출한다. 각 인자는 선언한 `Args` 규칙으로 전달되고 반환형은 서명의 `R`이다. `void()`이면 데이터 인자와 결과값이 모두 없다.
- 빈 `move_only_function`의 `operator()` 호출은 전제조건 위반이며 동작이 정의되지 않는다. 빈 `std::function`이 `std::bad_function_call`을 던지는 계약과 다르므로 먼저 `if (task)`로 검사하거나 프로그램 불변식으로 비어 있지 않음을 보장한다.
- 호출 대상이 던진 예외는 서명이 `noexcept`가 아니면 호출자에게 전파된다. `move_only_function<void() noexcept>`처럼 noexcept 서명을 선택했다면 호출 대상도 그 계약을 만족해야 한다.
- 호출은 타입 소거 간접 호출 비용을 가질 수 있고 작은 객체 최적화나 할당 여부는 구현에 따라 다르다. 특정 가상 함수나 어셈블리 명령으로 단정하지 않는다.
- 래퍼 자체는 저장 대상 수명을 소유하지만 참조 캡처 대상의 수명을 연장하지 않는다. 같은 래퍼나 캡처 객체를 여러 스레드에서 동시에 변경·호출하는 안전성도 자동 제공하지 않는다.
- 작업 큐, 단발 명령, 독점 자원을 넘겨받는 콜백처럼 “호출 대상 자체의 복사”가 의미적으로 잘못인 런타임 경계에 적합하다. 여러 번 호출 가능한지는 저장 호출 대상의 의미 계약을 별도로 확인한다.

## 타입 특성과 concept

- `std::is_same_v<A,B>`는 두 타입이 정확히 같은지 나타내는 컴파일 시간 `bool` 상수다.
- `std::remove_cvref_t<T>`는 최상위 `const`/`volatile`과 참조를 제거한 타입 별칭이다.
- `std::decay_t<T>`는 값 매개변수 전달과 비슷하게 배열·함수 변환과 cv/ref 제거를 적용한다.
- `std::same_as<T,U>`는 같은 타입임을 요구하는 C++20 concept다.
- `std::integral<T>`은 `bool`, 문자, 정수 등 표준 정수 타입 범주를 제약한다.
- `std::convertible_to<From,To>`는 명시·암시 변환 가능성과 의미 요구를 나타낸다.
- concept 실패는 런타임 분기가 아니라 템플릿 후보가 요구사항을 만족하지 않는 컴파일 오류다.

## `std::source_location` — `<source_location>`

- 호출 파일명, 함수명, 줄, 열 정보를 값으로 보관한다.
- 기본 인자 `std::source_location::current()`를 함수 선언에 두면 함수 본문이 아니라 호출 위치를 캡처한다.
- 문자열 포인터의 유효 기간은 구현 계약을 따르며 일반적으로 정적 저장 기간 정보로 사용한다.
- 로깅 API가 매번 `__FILE__`, `__LINE__` 매크로를 받지 않게 하지만 보안상 경로 노출 여부를 고려한다.

## `std::strong_ordering` — `<compare>`

- C++20 삼방향 비교 결과 중 강한 전체 순서 범주다.
- `less`, `equal/equivalent`, `greater` 상태를 표현하며 같음과 동등함이 일치한다.
- 사용자 정의 `operator<=>`의 반환형으로 사용하면 여러 관계 연산자가 합성될 수 있다.

## 최소 예제

```cpp
#include <expected>
#include <memory>
#include <string>

std::expected<std::unique_ptr<std::string>, std::string> make_name(bool valid) {
    if (!valid) {
        return std::unexpected(std::string{"invalid name"});
    }
    return std::make_unique<std::string>("codex");
}

int main() {
    auto result{make_name(true)};
    return result && **result == "codex" ? 0 : 1;
}
```

## 직접 검증

1. `unique_ptr`를 함수에 값으로 넘길 때 호출부에 `std::move`가 필요한 이유를 설명한다.
2. `shared_ptr`의 참조 횟수가 스레드 안전하다는 말과 `T`의 멤버가 데이터 경쟁에서 안전하다는 말을 구분한다.
3. `optional::value`, `operator*`, `value_or`의 실패·복사 계약을 비교한다.
4. `get_if`가 반환한 포인터가 variant에 새 값을 대입한 뒤 유효한지 설명한다.
5. aliasing `shared_ptr`의 stored pointer와 owned object가 다를 수 있는 예를 그리고, outer owner 소멸 뒤에도 부분 객체가 살아 있는 조건을 설명한다.
