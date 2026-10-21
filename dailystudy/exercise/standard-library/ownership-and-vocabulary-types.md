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

## `std::out_ptr`와 `std::out_ptr_t` — `<memory>`의 C++23 출력 포인터 어댑터

`std::out_ptr`는 `T**` 또는 `void**` 출력 매개변수에 새 자원의 원시 포인터를 써 주는 C API와 C++ 스마트 포인터 사이를 잇는 C++23 함수 템플릿이다. 반환되는 `std::out_ptr_t` 임시 객체가 출력 슬롯을 제공하고, 자신의 수명이 끝날 때 그 슬롯의 non-null 포인터를 스마트 포인터에 다시 채택시킨다. 수동 `release()`/임시 raw pointer/`reset()` 사이의 누수 경로를 없애는 RAII 경계지만, C API의 상태 코드나 포인터 계약 자체를 검증하지는 않는다.

### `std::out_ptr(s, args...)` 호출 계약

- **항목 종류·대표 선언:** 자유 함수 템플릿 `template<class Pointer = void, class Smart, class... Args> constexpr auto out_ptr(Smart& s, Args&&... args);`와 non-copyable class template `out_ptr_t<Smart, Pointer, Args...>`이며 `<memory>`에 선언된다. 명시한 `Pointer`가 `void`가 아니면 그것을 쓰고, 생략하면 `Smart::pointer`, `Smart::element_type*`, `pointer_traits<Smart>::element_type*` 순서의 표준 선택 규칙으로 실제 포인터 타입 `P`를 정한다.
- **수신 객체·인자:** 자유 함수라 별도 수신자는 없다. 첫 인자 `s`는 수명이 유효한 수정 가능한 `Smart` lvalue를 non-const reference로 빌리며 복사하거나 소유하지 않는다. `args...`는 `Smart::reset(p,args...)` 또는 `Smart(p,args...)`에 넘길 deleter·allocator 같은 추가 정책을 전달한다. `P`는 null 상태를 표현하고 비교할 수 있는 NullablePointer 요구사항을 만족해야 한다.
- **반환형·사용:** 반환형은 `std::out_ptr_t<Smart, P, Args&&...>` prvalue다. 보통 `legacy_create(std::out_ptr(owner))`처럼 같은 full-expression에서 C 함수의 `P*` 출력 인자로 즉시 변환해 사용한다. 어댑터는 복사할 수 없으며, 반환값을 오래 이름 붙여 보관하기보다 호출 한 번의 범위에 가두는 편이 수명 순서를 명확하게 한다.
- **생성 직후 상태:** 어댑터는 `s`와 추가 인자를 보관하고 내부 `P` 슬롯을 null로 값 초기화한다. 이어 가능한 경우 `s.reset()`, 아니면 `s = Smart()`로 기존 스마트 포인터를 비운다. 따라서 기존 자원이 있었다면 이 시점에 기존 deleter로 해제될 수 있고, 그 자원을 보던 raw pointer·reference는 무효가 된다. 기존 값을 C 함수가 입력으로도 사용하거나 직접 해제하는 API에는 `out_ptr`가 아니라 `inout_ptr` 계약을 검토해야 한다.
- **출력 포인터 변환:** `operator P*() const noexcept`는 내부 슬롯의 주소를 반환한다. C 함수는 호출 동안 그 주소가 가리키는 슬롯에 새 포인터 또는 null을 쓸 수 있지만, `P*` 자체를 저장해 어댑터 수명 뒤 접근하면 안 된다. `operator void**()`도 조건부로 제공되며, 같은 어댑터에서 `P*` 변환과 `void**` 변환을 둘 다 평가하는 것은 각각의 전제조건을 어긴다. 특히 반환된 `void**`가 가리키는 슬롯을 어댑터 수명 밖에서 접근하는 것은 미정의 동작이다.
- **파괴와 사후 상태:** C 호출을 포함한 full-expression 끝에서 어댑터가 파괴된다. 내부 슬롯 `p`가 non-null이면 `s.reset(static_cast<SP>(p), forwarded_args...)`, 또는 지원되는 `Smart` 직접 구성·대입과 동등한 동작으로 새 자원을 채택한다. `p`가 null이면 `s`는 앞서 비운 상태로 남는다. 상태 코드가 실패여도 C API가 non-null을 썼다면 어댑터는 그 포인터를 채택하므로, 성공/실패 시 출력 슬롯을 어떻게 다루는지는 반드시 그 API 문서로 따로 확인한다.
- **복잡도·할당:** 표준은 모든 `Smart`와 deleter에 공통인 점근 상한을 정하지 않는다. 빈 `unique_ptr`와 단순 deleter를 쓰는 오늘 예제에서 어댑터의 슬롯 준비와 소유권 채택은 상수 시간이고 별도 할당이 필요 없지만, 기존 자원 해제 비용과 사용자 정의 `Smart`/deleter 비용은 각각의 계약을 따른다. `shared_ptr` 적응은 제어 블록을 준비하기 위해 어댑터 생성 중 할당할 수 있다.
- **예외·컴파일 오류:** 어댑터 생성자는 일반적으로 `noexcept`가 아니므로 전달 인자 보관, 기존 소유자 비우기, `shared_ptr` 제어 블록 준비의 예외가 C 함수 호출 전에 전파될 수 있다. 파괴 시 채택 경로에서 예외가 안전하게 빠져나올 것이라 기대하면 안 되며, `noexcept`인 스마트 포인터 동작이나 deleter가 던지면 종료될 수 있다. `Smart`가 `shared_ptr` 특수화인데 추가 인자가 하나도 없으면 프로그램은 ill-formed이므로 새 raw pointer에 맞는 deleter를 추가 인자로 전달해야 한다. 반면 `unique_ptr<T,D>`는 저장된 `D`를 유지한 채 새 pointer를 reset하므로 보통 별도 deleter 인자가 필요 없다. `Smart`를 비우거나 새 포인터와 인자로 다시 구성할 수 없는 조합도 ill-formed다.
- **수명·소유권·무효화:** C 함수가 쓴 새 자원은 어댑터 파괴 전까지 임시 슬롯에 있고, 파괴가 끝난 뒤 `s`가 소유한다. 그 뒤 raw observer의 유효 기간은 `s`의 다음 reset/이동/소멸과 deleter 계약에 묶인다. 어댑터와 `s`, 전달한 참조 인자는 적어도 어댑터 파괴가 끝날 때까지 살아 있어야 한다. C API가 출력한 포인터를 별도로 해제하거나 다른 owner에도 채택하면 이중 해제 위험이 있다.
- **스레드 보장:** 어댑터는 같은 `Smart` 객체를 비우고 다시 쓴다. 같은 스마트 포인터 객체나 같은 어댑터 변환을 다른 실행 흐름과 동기화 없이 함께 평가하면 데이터 경쟁 또는 충돌 연산이 될 수 있다. 스마트 포인터가 가리키는 대상의 동시 접근 안전성도 별도 문제이며 `out_ptr`가 동기화를 추가하지 않는다.
- **오늘 코드에서의 역할:** [`2026-09-10 main.cpp`](../2026-09-10/main.cpp)의 `legacy_connect(endpoint, std::out_ptr(handle))`는 빈 `ConnectionPtr`를 `LegacyConnection**` 출력 슬롯으로 잠시 적응한다. C 함수가 성공해 기록한 포인터는 full-expression 끝에 custom deleter를 보존한 `unique_ptr`가 채택하고, 실패해 슬롯이 null이면 owner는 빈 상태로 남는다.

### 선택 판단과 흔한 실수

- API가 **새 값만 출력**하고 기존 포인터를 읽지 않는다면 `out_ptr`가 맞다. 기존 포인터를 입력으로 받아 재할당·교체하거나 직접 해제한다면 `inout_ptr` 또는 그 API 전용 래퍼가 필요하다.
- `if (legacy_create(std::out_ptr(owner)) == 0 && owner)`처럼 같은 full-expression 안에서 owner를 이어 검사하면 오른쪽 피연산자를 평가할 때 어댑터 임시가 아직 파괴되지 않아 owner가 여전히 비어 있다. 상태 코드를 먼저 받은 뒤 다음 문장에서 owner를 관찰한다.
- `P**` 출력 주소를 C 라이브러리가 호출 뒤에도 비동기로 보관하는 API에는 임시 `out_ptr_t`를 넘길 수 없다. 그 주소의 유효 기간은 어댑터 수명뿐이다.
- `shared_ptr`에는 자원에 맞는 deleter를 추가 인자로 넘긴다. `unique_ptr<T,D>`는 기존 `D` 객체를 유지한 채 새 `pointer`를 reset하므로 보통 별도 deleter 인자가 필요 없다.

## `std::any`, `std::any_cast`, `std::bad_any_cast` — `<any>`

`std::any`는 C++17부터 제공되는 **소유형 단일 값 타입 소거(type erasure) 컨테이너**다. 한 `any` 객체는 현재 정확히 한 타입의 값을 소유하거나 비어 있다. 저장 타입 후보를 컴파일 시간에 열거하는 `variant`와 달리 호출 시점에 CopyConstructible인 여러 타입을 받을 수 있지만, 읽을 때는 저장 타입과 `typeid`가 일치하는 타입을 다시 알아야 한다. 이 비교는 최상위 cv 한정을 별도 런타임 타입으로 구별하지 않는다. `int{5}`는 `int`로 저장되며 `double`이나 문자열로 암시 변환해 꺼내 주지 않는다.

실무에서는 플러그인 속성, 추적 문맥, 프레임워크 확장 슬롯처럼 핵심 계층이 모든 확장 타입을 미리 알 수 없는 좁은 경계에 적합하다. 도메인 상태가 닫힌 집합이면 `variant`, 값이 없을 수 있을 뿐이면 `optional`, 공통 동작이 중요하면 가상 인터페이스가 보통 더 명시적이다. `any`를 비즈니스 모델 전체에 퍼뜨리면 타입 오류가 컴파일 시점에서 실행 시점으로 늦어지므로 키와 타입의 대응을 한 래퍼 안에 가둔다.

### 값 생성·복사·이동 계약

- **항목 종류·대표 선언:** `<any>`의 비템플릿 클래스 `std::any`다. 주요 생성자는 `constexpr any() noexcept`, `any(const any&)`, `any(any&&) noexcept`, `template<class T> any(T&& value)`다. 템플릿 생성자는 `VT = decay_t<T>`를 실제 저장 타입으로 삼고, `VT`가 `any`나 `in_place_type_t` 특수화가 아니며 CopyConstructible일 때 참여한다.
- **값 생성자 인자와 결과:** `any(T&& value)`의 `value`는 전달 참조다. lvalue는 보통 복사되고 rvalue는 이동될 수 있지만, 완성된 `any`는 `decay_t<T>` 객체를 독립적으로 소유한다. 생성자는 반환값이 없고 성공 뒤 `has_value()==true`다. move-only 타입은 rvalue여도 이 생성자 요구를 만족하지 않으므로 `unique_ptr`를 직접 저장할 수 없다. 필요하다면 복사 가능한 공유 소유 핸들이나 별도 타입 소거 설계를 검토한다.
- **복사 생성:** 원본이 비어 있으면 결과도 비고, 값이 있으면 원본 contained value를 `const` lvalue로 보아 같은 타입의 새 값을 구성한다. 따라서 문자열·컨테이너 같은 소유 값은 논리적으로 독립된 복사본을 갖는다. contained 타입의 복사 생성이 던진 예외가 전파될 수 있다.
- **이동 생성:** `any(any&& other) noexcept`는 원본이 비면 빈 결과를 만들고, 그렇지 않으면 contained value 자체를 이전하거나 그 값을 rvalue로 보아 같은 타입 객체를 구성한다. 목적지는 원래 타입과 논리 값을 이어받지만 **표준은 이동 뒤 `other.has_value()==false`를 보장하지 않는다**. 원본이 비었다고 검사하거나 원래 값에 의존하지 말고, 확실히 비워야 하면 별도로 `reset()`한다.
- **할당·복잡도:** 표준은 모든 contained 타입과 구현에 공통인 점근 복잡도나 “항상 무할당”을 보장하지 않는다. 작은 값은 내부 버퍼에 두도록 권장하지만 그러한 small-object optimization은 nothrow-move-constructible 타입에만 적용할 수 있고 버퍼 크기·적용 여부는 구현 세부다. 큰 값은 동적 할당과 `bad_alloc` 가능성이 있다. 복사 비용은 적어도 contained 타입의 복사 비용을 포함하고, 파괴는 contained 타입 소멸 비용을 포함한다.
- **소유권·수명:** contained value의 수명은 `any` 안에서 시작하고 그 `any`가 파괴되거나 값이 reset/대입/emplace로 교체될 때 끝난다. `any_cast`가 돌려준 포인터·참조는 값을 소유하지 않고 이 수명을 연장하지 않는다. owner가 이동됐을 때 과거 observer가 목적지 값을 가리킨다고 가정할 수도 없다.

### 포인터형 `std::any_cast<T>(&operand)` 호출 계약

- **항목 종류·signature:** 자유 함수 템플릿이다. 읽기 전용 overload는 `template<class T> const T* any_cast(const any* operand) noexcept`, 수정 가능 overload는 `template<class T> T* any_cast(any* operand) noexcept`다. `T`는 `void`가 아닌 객체 타입이어야 하며, 저장 타입과 `typeid(T)`가 일치해야 성공한다. `typeid` 비교는 `T`의 최상위 `const`/`volatile`을 구별하지 않는다.
- **수신 객체·인자:** 멤버 함수가 아니므로 별도 수신자는 없다. 인자는 살아 있는 `any`를 가리키거나 null일 수 있는 비소유 포인터 prvalue다. `&context_value`처럼 주소를 넘겨도 `any`나 contained value의 소유권은 이전되지 않는다.
- **반환형·사용:** `operand != nullptr`이고 `operand->type() == typeid(T)`일 때 contained object를 가리키는 `T*` 또는 `const T*`를 반환한다. null operand, 빈 `any`, 타입 불일치에서는 `nullptr`를 반환한다. 호출자는 결과 포인터를 `if (pointer != nullptr)`로 검사한 뒤 역참조한다.
- **호출 뒤 상태:** cast는 `any`, contained value, 인자 포인터를 바꾸지 않는다. 반환 포인터를 통해 non-const `T`를 변경할 수는 있지만 그것은 cast 호출 뒤의 별도 접근이다. `int`가 든 객체에 `long`이나 사용자 변환 가능한 타입을 요청해도 “비슷한 타입” 변환을 하지 않는다. 반면 `any_cast<const int>(&value)`는 최상위 cv를 제외한 `typeid`가 일치하므로 성공해 읽기 전용 포인터를 반환할 수 있다. 코드에서는 저장 타입과 요청 타입의 이 규칙을 한 API에서 맞춘다.
- **복잡도·할당·오류:** 이 overload들은 `noexcept`이며 실패를 예외가 아닌 null로 표현하고 새 저장소를 소유하지 않는다. 표준은 별도의 모든 구현 공통 점근 상한을 명시하지 않으므로 특정 RTTI 테이블 조회 명령 수로 단정하지 않는다. null 검사 없이 결과를 역참조하면 null pointer UB이고, 성공 포인터도 owner의 값 교체·reset·파괴 뒤 사용하면 dangling이다.
- **스레드 보장:** cast가 동기화를 추가하지 않는다. 같은 `any`와 contained value를 여러 실행 흐름이 읽기만 하는 경우에도 contained 타입의 계약을 따르고, 한쪽이 reset/대입/이동/수정하는 동안 다른 쪽이 cast 결과를 읽으면 별도 mutex 같은 동기화가 필요하다.

### 값·참조형 `std::any_cast<T>(operand)`와 `bad_any_cast`

- `any_cast<T>(const any&)`, `any_cast<T>(any&)`, `any_cast<T>(any&&)`는 `T`가 요구하는 cv/ref와 생성 가능성에 맞는 overload를 고른다. 저장 타입이 `remove_cvref_t<T>`와 다르면 `std::bad_any_cast`를 던진다.
- `T`를 값 타입으로 요청하면 contained value를 복사하거나 rvalue overload에서 이동해 새 값을 반환할 수 있다. `T&`/`const T&`는 같은 contained object의 참조를 반환하므로 owner 수명과 교체에 묶인다. 예외 기반 제어가 불필요한 조회 경계에서는 포인터형이 실패를 값으로 드러내 더 간단하다.
- `bad_any_cast`는 `std::bad_cast`에서 파생한 예외 타입이고 `what()`은 구현 정의 null-terminated 문자열 포인터를 반환한다. 오늘 코드는 예상 가능한 키 누락·타입 불일치를 정상 분기로 처리하므로 이 예외 경로를 사용하지 않는다.

### `std::any::has_value()`와 `std::any::reset()` 계약

| 대표 형태 | 수신 객체·인자 | 반환값·사후 상태 | 비용·수명·오류 |
|---|---|---|---|
| `bool has_value() const noexcept` | 살아 있는 const 또는 non-const `any` lvalue, 데이터 인자 없음 | contained object가 있으면 `true`, 비면 `false`; 수신 상태는 바뀌지 않는다. | `noexcept`; 새 할당·observer 무효화 없음. 표준의 별도 점근 상한은 없고 동시 변경과 함께 쓰지 않는다. |
| `void reset() noexcept` | 값이 있거나 비어 있는 수정 가능한 `any` lvalue, 데이터 인자 없음 | 값이 있으면 contained object를 파괴하고, 반환값 없이 반드시 빈 상태로 만든다. | contained 소멸이 실행되고 과거 포인터·참조가 모두 무효가 된다. `noexcept` 경계이므로 소멸자가 던져 빠져나오게 설계하면 종료될 수 있다. |

### 컨테이너 안에서 사용할 때

`unordered_map<string, any>`에 저장하면 map node가 `any`를 소유하고 그 `any`가 실제 값을 소유한다. 새 key 삽입이나 rehash는 map 반복자를 무효화할 수 있지만 원소 포인터·참조는 일반적으로 살아 있는 node를 계속 가리킨다. 그러나 같은 key의 mapped `any`를 `insert_or_assign`로 교체하면 과거 contained value는 파괴되므로 그 값에 대한 `any_cast` 포인터·참조는 즉시 무효다. key erase와 map 파괴도 같은 효과를 낸다. 따라서 조회 포인터는 짧은 표현식 안에서만 사용하고 mutation 경계를 넘어 보관하지 않는다.

### 최소 예제

```cpp
#include <any>
#include <cassert>
#include <string>
#include <utility>

int main() {
    std::any value{std::string{"trace"}};
    const std::string* text{std::any_cast<std::string>(&value)};
    assert(text != nullptr && *text == "trace");
    assert(std::any_cast<int>(&value) == nullptr);

    std::any destination{std::move(value)};
    // 이동만으로 value가 빈다고 가정하지 않는다. 확실한 빈 상태는 reset이 만든다.
    value.reset();
    return destination.has_value() && !value.has_value() ? 0 : 1;
}
```

## `std::shared_ptr<T>`, `std::weak_ptr<T>`, `std::enable_shared_from_this<T>`, `std::make_shared<T>` — `<memory>`

`shared_ptr`는 관리 객체와 control block의 공유 소유권을 나타낸다. control block에는 strong/weak 소유 상태와
deleter·allocator 정보가 들어갈 수 있다. 마지막 strong owner가 사라지면 관리 객체의 수명은 끝나지만,
`weak_ptr`가 남아 있으면 만료 상태를 표현하기 위해 control block 저장소는 더 오래 남을 수 있다. `weak_ptr`는
객체를 살려 두지 않으며, 순환 소유권의 비소유 방향이나 원래 객체보다 오래 보관될 수 있는 콜백에 적합하다.

### `std::make_shared<T>(args...)` 생성 계약

- **항목 종류·대표 선언:** `<memory>`의 자유 함수 템플릿이다. 오늘 선택하는 비배열 overload의 대표 형태는 `template<class T, class... Args> shared_ptr<T> make_shared(Args&&... args);`다. C++20 이후 배열용 overload도 있으므로 인자 모양만 보고 모두 같은 계약이라고 일반화하지 않는다.
- **수신 객체·인자:** 자유 함수라 수신 객체는 없다. 각 `args`는 forwarding reference이며 `T`의 생성자로 값 범주와 cv 한정을 전달한다. lvalue 인자는 복사·참조 바인딩 대상, rvalue 인자는 이동 대상이 될 수 있지만 실제 자원 이전은 선택된 `T` 생성자가 수행한다. 모든 인자 식은 호출 전에 평가되고 참조 인자의 대상은 생성이 끝날 때까지 유효해야 한다.
- **반환형·사후 상태:** 완성한 `T`와 그 control block을 공동 소유하는 `shared_ptr<T>` prvalue를 반환한다. 반환값은 호출자의 핸들을 직접 초기화하며, 별도 owner 복사 없이 결과 객체에 구성될 수 있다. 성공 직후 다른 owner를 아직 만들지 않았다면 strong owner는 반환 핸들 하나다.
- **복잡도·할당·예외:** wrapper 작업은 상수 크기지만 전체 비용은 동적 저장소 획득과 `T` 생성 비용을 포함한다. 구현은 보통 객체와 control block을 한 번의 할당으로 배치하지만 그 물리적 배치와 정확한 할당 횟수에 의존하지 않는다. 저장소 획득은 `bad_alloc`, `T` 생성은 해당 생성자의 예외를 전파할 수 있으며, 실패하면 이미 획득한 자원을 정리해 완성되지 않은 owner를 공개하지 않는다.
- **수명·무효화:** 반환 핸들의 마지막 strong 몫이 해제될 때 `T`를 파괴한다. 객체와 control block을 같은 할당에 둔 구현에서는 남은 weak owner 때문에 `T`가 파괴된 뒤에도 그 결합 저장소 반환이 늦어질 수 있다. 객체 파괴 뒤 과거 raw pointer·reference는 무효지만 weak handle 자체는 만료 상태로 검사할 수 있다.
- **스레드 보장·현재 역할:** factory 자체가 `T`의 상태 접근을 동기화하지 않는다. 같은 control block을 공유하는 서로 다른 스마트 포인터 핸들의 참조 횟수 조정은 안전하지만 `T`의 동시 읽기·쓰기는 별도 계약이 필요하다. 오늘은 callback을 만들 객체가 처음부터 올바른 shared control block에 연결되도록 raw `new` 대신 factory 경계에서 사용한다.

### `std::enable_shared_from_this<T>`와 `weak_from_this()`

- **항목 종류·상속 조건:** `<memory>`의 class template이며, `T`가 `public std::enable_shared_from_this<T>`를 접근 가능하고 모호하지 않은 기반으로 정확히 한 번 갖는 패턴을 사용한다. `shared_ptr`가 새 control block으로 `T`를 최초 소유할 때 이 기반을 발견하면 내부 weak 관찰자를 같은 control block에 연결한다. 기반 subobject와 내부 weak 관찰자는 `T`를 strong ownership하지 않는다.
- **대표 시그니처·수신 객체:** `weak_ptr<T> weak_from_this() noexcept;`와 `weak_ptr<const T> weak_from_this() const noexcept;` 두 overload가 있다. non-const `T` lvalue 수신자에는 전자, `const T` lvalue 수신자에는 후자가 선택된다. 명시적 데이터 인자는 없으며, 멤버를 호출하려면 그 순간 `*this` 자체의 수명이 이미 보장돼 있어야 한다.
- **반환형·사후 상태:** 연결된 내부 관찰자를 복사한 `weak_ptr` prvalue를 반환하고 수신 `T`와 기존 owner를 바꾸지 않는다. 반환 weak handle을 lambda에 값 캡처하면 closure가 control block 관찰 상태를 소유하지만 `T`의 strong count는 늘지 않는다. 따라서 외부 queue가 callback을 오래 보관해도 그 사실만으로 `T`를 살려 두지 않는다.
- **너무 이른 호출과 오류:** `T`가 아직 shared owner에 연결되지 않은 stack/raw 객체이거나 `make_shared`가 객체 생성자를 실행 중인 시점에는 `weak_from_this()`가 빈 weak handle을 반환하며 예외를 던지지 않는다. 같은 상태의 `shared_from_this()`는 `bad_weak_ptr`를 던질 수 있다. 생성자에서 자기 callback을 완성하지 말고 factory가 owner를 만든 뒤 별도 멤버 함수에서 생성한다.
- **전제조건 위반과 UB:** private/protected 상속이나 모호한 다중 기반은 shared owner가 기반을 정상 연결하는 공개·비모호 패턴을 만족하지 않는다. 이를 우회해 `shared_ptr<T>(this)`로 별도 control block을 만들면 같은 객체를 독립 owner들이 중복 삭제할 수 있어 미정의 동작으로 이어진다. `enable_shared_from_this`는 이미 만들어진 독립 control block들을 합치지 않는다.
- **복잡도·할당·수명:** `weak_from_this()`는 상수 시간에 control block 관찰 몫을 복사하고 별도 동적 할당 없이 `noexcept`로 끝난다. 마지막 strong owner가 사라지면 `T`는 파괴되며 반환 weak handle은 만료된다. 마지막 weak handle까지 사라질 때 control block도 해제될 수 있다. weak handle 또는 callback을 파괴하면 그 handle만 무효가 되고 다른 핸들은 유지된다.
- **스레드 보장·현재 역할:** control block 참조 횟수의 안전성은 `T` 멤버 상태를 자동으로 보호하지 않는다. 같은 callback에서 얻은 strong handle로 객체 수명은 지킬 수 있지만 mutable 멤버의 병행 접근에는 mutex·불변 객체 같은 별도 정책이 필요하다. 이미 수명이 끝날 수 있는 raw `this`로 `weak_from_this()` 자체를 호출하는 것은 안전 장치가 아니다. 오늘 패턴은 살아 있는 owner를 통해 callback factory를 호출하고, callback 실행 때마다 weak handle을 승격한다.

### `std::weak_ptr<T>::lock()` 승격 계약

- **수신 객체·시그니처:** 수신자는 살아 있는 `const` 또는 non-const `weak_ptr<T>` lvalue이며 연결된 control block이 있거나 빈 상태일 수 있다. 선택되는 멤버는 `shared_ptr<T> lock() const noexcept;`이고 데이터 인자는 없다.
- **반환형·사후 상태:** 관리 객체가 아직 살아 있으면 같은 control block의 strong 몫 하나를 얻은 `shared_ptr<T>` prvalue, 이미 만료됐거나 처음부터 비어 있으면 빈 `shared_ptr<T>`를 반환한다. 수신 weak handle은 변하지 않는다. 성공 반환값을 지역 변수에 보관하면 그 지역 핸들이 사는 동안 객체 수명이 유지된다.
- **원자적 검사·경쟁:** 만료 확인과 strong count 증가는 control block에 대해 하나의 원자적 승격 동작으로 수행된다. `expired()`를 먼저 검사한 뒤 raw pointer를 사용하는 두 단계는 사이에 마지막 owner가 사라질 수 있으므로 대신 `lock()` 반환값 자체를 검사한다.
- **복잡도·할당·오류:** control block 상태 확인과 참조 횟수 조정은 상수 시간이고 새 control block이나 관리 객체를 할당하지 않으며 `noexcept`다. 만료는 오류 예외가 아니라 빈 반환값이다. 빈 결과에 `operator*` 또는 `operator->`를 적용하면 전제조건을 어겨 미정의 동작이므로 먼저 bool 상태를 확인한다.
- **수명·스레드:** 성공한 strong handle 파괴는 strong count를 줄이고 마지막 owner라면 객체를 파괴할 수 있다. 서로 다른 shared/weak handle의 control-block 조정은 안전하지만 같은 handle 객체에 대한 `reset`/대입과 읽기를 동기화 없이 겹치면 안 되고, 가리키는 `T`의 데이터 경쟁도 별도 문제다.

### `std::shared_ptr<T>::reset()` 해제 계약

- **수신 객체·시그니처:** 수신자는 비어 있거나 strong 몫 하나를 가진 수정 가능한 `shared_ptr<T>` lvalue다. 오늘의 무인자 호출은 `void reset() noexcept;`를 선택하며 새 pointer·deleter·allocator 인자는 없다. pointer를 받는 template overload들은 별도 control block 생성과 할당 실패 가능성이 있어 이 계약과 다르다.
- **반환형·사후 상태:** 반환값은 `void`이고 호출 뒤 수신 핸들은 빈 상태다. 기존 control block의 strong count를 하나 줄이며, 이것이 마지막 strong 몫이면 관리 객체의 소멸자와 저장된 deleter를 실행한다. 다른 strong owner가 있으면 객체 수명은 계속된다.
- **복잡도·할당·오류:** 핸들 교체와 참조 횟수 조정은 상수 시간이지만 마지막 owner에서는 `T` 소멸과 deleter 비용이 더해진다. 무인자 overload는 새 저장소를 할당하지 않고 `noexcept`다. 소멸자나 deleter가 예외를 밖으로 내보내도록 설계하면 `noexcept` 파괴 경계에서 종료될 수 있으므로 던지지 않아야 한다.
- **무효화·수명:** 마지막 owner였다면 객체를 가리키던 raw pointer·reference는 호출 과정에서 무효가 된다. weak handle과 이를 값 캡처한 callback은 control block을 관찰하는 유효한 만료 객체로 남고, 이후 `lock()`은 빈 값을 반환한다. 빈 수신자에 다시 호출하는 것은 허용되며 계속 빈 상태다.
- **스레드 보장:** 같은 control block의 다른 스마트 포인터 핸들은 독립적으로 사용할 수 있지만 동일한 `shared_ptr` 객체에 대한 `reset`과 다른 접근을 동기화 없이 겹치면 데이터 경쟁이다. reset이 가리키던 `T`의 임의 동시 접근을 기다려 주는 것은 아니므로 각 사용자는 자신의 strong owner나 상위 동기화로 수명을 보장해야 한다.

요약하면 `shared_ptr` 복사는 strong count를 늘리고 이동은 소유 몫을 옮긴다. 강한 자기 캡처는 callback이 객체를
불필요하게 오래 살리거나 객체가 callback을 소유할 때 순환을 만들 수 있다. 반대로 weak 자기 캡처는 callback
실행 때 `lock()`에 성공한 짧은 범위에서만 strong 수명을 확보한다. 별도 자원 해제 정책이나 매우 큰 객체의
메모리 반환 시점 때문에 직접 생성이 필요한 경우도 있지만 일반 객체와 control block의 공동 생성을 의도하면
`make_shared`가 기본 선택이다.

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
- C++23 `transform(f)`는 값이 있을 때만 그 값을 `f`로 변환해 새 `optional`에 소유하고, 비어 있으면 `f`를 실행하지 않은 채 빈 결과를 반환한다.
- `std::nullopt_t`는 빈 상태를 선택하는 태그 타입이고 `std::nullopt`는 그 타입의 표준 상수 객체다. `optional(nullopt_t)`는 인자를 값으로 받아 상수 시간·무할당·`noexcept`로 disengaged 객체를 만들며 contained `T`를 생성하지 않는다.
- `optional<reference_wrapper<T>>`는 선택적 비소유 참조를 표현하지만 원본 수명을 연장하지 않는다.

### 기본 생성·`emplace`·`value` 호출 계약

| 대표 형태 | 수신 객체·각 입력 | 반환값 | 호출 뒤 상태·계약 |
|---|---|---|---|
| `std::optional()` (`constexpr optional() noexcept`) | 아직 구성되지 않은 `optional<T>` 목적 객체. 명시적 데이터 인자와 외부 소유권은 없다. | 생성자는 별도 반환값이 없다. | 빈 optional의 수명이 시작된다. contained `T`는 아직 존재하지 않으며 optional 자체가 동적 할당을 요구하지 않는다. |
| `template<class... Args> constexpr T& emplace(Args&&... args)` | 살아 있는 optional 수신 lvalue와 `T`를 구성할 수 있는 전달 인자 팩. 각 참조 인자 대상은 구성 중 살아 있어야 한다. | 새 contained `T`의 lvalue 참조를 반환한다. 호출자는 즉시 사용하거나 의도적으로 버릴 수 있다. | 기존 값이 있으면 먼저 파괴하고 전달 인자로 새 T를 직접 구성한다. 성공 뒤 값이 있다. T 생성이 던지면 optional은 비어 있고 과거 contained 참조·포인터는 무효다. |
| `constexpr T& value() &` / `constexpr const T& value() const &` | 값을 가질 수도 있는 non-const/const optional lvalue. 데이터 인자는 없다. | contained T의 cv가 맞는 lvalue 참조를 반환한다. | 수신 상태와 소유권은 바뀌지 않는다. 비어 있으면 `std::bad_optional_access`를 던진다. 반환 참조는 optional 수명과 다음 reset/emplace/대입 중 먼저 오는 때까지만 유효하다. |

- 기본 생성과 `value()`의 성공 경로는 `O(1)`·무할당이다. `emplace` 비용과 예외는 선택된 T 생성자 비용에 따르고 optional 자체가 별도 heap allocation을 요구하지는 않지만 T가 내부에서 할당할 수 있다.
- `emplace`는 기존 값을 먼저 파괴하므로 새 T 생성 실패 시 옛 값을 보존하는 강한 보장을 주지 않는다. “옛 값 또는 새 값”이 필요하면 먼저 별도 candidate를 완성한 뒤 예외 없는 교체 전략을 설계한다.
- `value()`는 빈 상태를 정의된 예외로 보고한다. 반면 `operator*`와 `operator->`는 값 존재 전제조건을 호출자가 지켜야 하므로 검사 없이 빈 optional에 적용하면 안 된다.
- optional 객체 자체를 다른 스레드가 변경하는 동안 `value()`로 읽는 것은 안전하지 않다. [`../2026-09-08/main.cpp`](../2026-09-08/main.cpp)은 `std::call_once`의 returning→passive 동기화 뒤 cache를 다시 변경하지 않는 불변식으로 const reference 공유를 안전하게 만든다.
- 오늘 `cache_.emplace(source_.load())`는 load 결과 prvalue를 contained Config/Report로 소유하고 반환 `T&`는 버린다. 이어 `const optional<T>&`로 바인딩한 `published_cache.value()`가 반환한 `const T&`는 Provider/Cache가 파괴되기 전까지만 유효하다.

### C++23 `std::optional::transform` 호출 계약

`std::optional::transform`은 engaged/disengaged 상태를 보존하면서 성공 값만 다른 타입으로 투영하는 C++23 멤버 함수 템플릿이다. 수신 객체의 cv/ref 한정에 따라 다음 네 overload 중 정확히 하나가 선택된다.

```cpp
template<class F> constexpr auto transform(F&& f) &;
template<class F> constexpr auto transform(F&& f) const &;
template<class F> constexpr auto transform(F&& f) &&;
template<class F> constexpr auto transform(F&& f) const &&;
```

| 수신 식과 호출 전 상태 | callable에 전달되는 contained 식 | 선택 의미 |
|---|---|---|
| 수정 가능한 `optional<T>` lvalue | `T&` lvalue | 값을 빌려 읽거나 명시적으로 수정할 수 있다. |
| `const optional<T>` lvalue | `const T&` lvalue | 값을 읽기 전용으로 빌린다. |
| 수정 가능한 `optional<T>` xvalue | `T&&` xvalue | callable이 선택하면 contained 값에서 자원을 이동할 수 있다. |
| `const optional<T>` xvalue | `const T&&` xvalue | const 때문에 일반적인 이동 생성자는 선택되지 않을 수 있다. |

- **시그니처·인자:** `F&&`는 forwarding reference다. `f`의 lvalue/rvalue 성질을 보존해 즉시 한 번 이하 호출하며 `optional` 안에 callable을 저장하지 않는다. 함수 인자 식과 callable 객체 구성은 일반 호출 규칙에 따라 먼저 평가되지만, disengaged 수신자에서는 callable 본문을 호출하지 않는다. engaged 수신자에서는 위 표의 정확한 값 범주로 contained 값을 전달한다.
- **반환형·빈 상태:** 각 overload는 `U = remove_cv_t<invoke_result_t<F, 전달되는-contained-식>>`에 해당하는 새 `optional<U>` prvalue를 반환한다. engaged이면 callable 결과로 `U`를 결과 저장소에 직접 생성하고, disengaged이면 빈 `optional<U>`를 반환한다. 빈 상태는 정상 분기이므로 `bad_optional_access`를 던지지 않는다. `U`는 C++23 `optional`의 유효한 contained 타입인 완전한 비배열 객체 타입이어야 하고 `in_place_t`나 `nullopt_t`여서는 안 되며, callable 결과에서 직접 초기화 가능해야 한다. `void`, 참조, 배열 같은 부적합한 결과는 미정의 동작이 아니라 컴파일 오류다. 직접 생성하므로 `U` 자체의 이동 가능성은 별도 필수 조건이 아니다.
- **호출 뒤 상태·소유권:** lvalue overload는 callable이 받은 `T&`로 직접 수정하지 않는 한 수신자의 engaged 여부와 contained 값·소유권을 유지한다. rvalue overload도 수신자를 자동으로 disengaged 상태로 만들지 않는다. 다만 callable이 `T&&`에서 이동하면 수신자는 여전히 engaged이면서 contained `T`가 유효하지만 이동 후 값은 타입 계약에 따라 미지정일 수 있다. 결과 `optional<U>`는 `U`를 독립 소유하지만, `U` 자체가 pointer·`string_view`·`reference_wrapper` 같은 비소유 타입이면 원본 수명을 연장하지 않는다.
- **복잡도·할당:** 상태 검사는 `O(1)`이고 callable은 0회 또는 1회 실행된다. 전체 시간은 `O(1) + f 실행 + U 직접 생성` 비용이다. `optional` 포장 자체는 별도 동적 할당을 요구하지 않지만 `f`나 `U`의 생성자가 문자열·컨테이너 저장소를 할당할 수 있다.
- **무효화·수명:** `transform` 자체는 수신자의 contained 객체를 교체하거나 파괴하지 않으므로 기존 포인터·참조를 자동 무효화하지 않는다. 그러나 callable이 `T&`를 변경하거나 `T&&`에서 이동하는 동작의 무효화 규칙은 `T`가 정한다. 결과가 원본 내부를 빌리는 비소유 값이면 원본의 reset·대입·파괴 또는 내부 재할당 뒤 댕글링될 수 있으며, 임시 optional 내부를 가리키는 결과를 전체 식 밖에서 쓰면 안 된다.
- **예외·오류·UB:** `f` 호출 또는 `U` 직접 생성이 던진 예외는 그대로 전파되며 이 함수는 일반적으로 `noexcept`가 아니다. 예외 전까지 callable이 수신자의 `T&`를 수정했거나 `T&&`에서 이동했다면 그 상태는 자동 rollback되지 않고 해당 연산의 예외 보장을 따른다. 빈 상태 자체는 오류도 UB도 아니다. 반면 callable이 받은 참조의 수명·자체 전제조건을 어기거나, 댕글링 결과를 역참조하거나, 다른 실행 흐름의 무동기 변경과 겹치면 해당 연산의 미정의 동작 또는 데이터 경쟁이 될 수 있다.
- **스레드 보장:** `transform`은 잠금이나 원자적 snapshot을 제공하지 않는다. 적절히 게시된 서로 다른 optional 또는 변경되지 않는 같은 const optional의 동시 읽기는 `T`와 callable도 동시 읽기에 안전할 때 가능하다. 같은 optional의 contained 값을 한 실행 흐름이 수정·이동하는 동안 다른 실행 흐름이 읽거나 쓰면 외부 동기화가 필요하다.

`transform`과 `and_then`은 빈 상태를 건너뛴다는 점은 같지만 callable 반환 계약이 다르다.

| 연산 | callable의 정상 반환 | 최종 반환 | 선택 기준 |
|---|---|---|---|
| `transform(f)` | 보통 값 `U` | `optional<U>` | 성공 값을 순수하게 다른 값으로 투영한다. `f`가 `optional<U>`를 반환하면 결과는 중첩 `optional<optional<U>>`가 된다. |
| `and_then(f)` | `optional<U>` | 그 `optional<U>` | 다음 단계도 실패·부재를 만들 수 있을 때 평탄하게 연결한다. |

최소 실행 예제는 engaged 값에서 문자열을 만들고, 빈 값에서는 부수 효과 없이 빈 결과가 전파되는지 검증한다.

```cpp
#include <cassert>
#include <optional>
#include <string>

int main() {
    const std::optional<int> port{443};
    int calls{0};

    const auto label = port.transform([&calls](int value) {
        ++calls;
        return std::string{"tcp:"} + std::to_string(value);
    });

    const std::optional<int> missing{};
    const auto skipped = missing.transform([&calls](int value) {
        ++calls;
        return value * 2;
    });

    assert(label.has_value());
    assert(*label == "tcp:443");
    assert(!skipped.has_value());
    assert(calls == 1);
}
```

오늘 [`../2026-10-22/main.cpp`](../2026-10-22/main.cpp)의 첫 `transform` 호출에서는 실제 수신자의 cv/ref와 lambda 식의 값 범주를 먼저 확인하고, 반환된 `optional<U>`를 저장하는지 즉시 소비하는지까지 호출 계약에 기록한다. 기계 실행 관점에서는 보통 engaged 상태 로드·조건 분기·callable 본문·결과 객체 구성이 후보지만, 실제 인라인·로드·분기·복사 생략은 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라진다.

## `std::expected<T, E>`와 `std::unexpected<E>` — `<expected>`

`expected<T,E>`는 성공 `T` 또는 실패 `E`를 **자기 저장소 안에 정확히 하나** 소유하는 C++23 클래스 템플릿이다. `unexpected<E>`는 실패 값을 명시적으로 싣는 별도 클래스 템플릿이다. `optional<T>`의 단순 부재와 달리 실패 이유가 호출 경계에 남는다. 오늘 [`../2026-09-17/main.cpp`](../2026-09-17/main.cpp)는 파싱 결과를 `expected<Config,ParseError>`로 반환하고, 성공 객체를 완성한 뒤에만 현재 설정을 교체한다.

`std::in_place`는 `<utility>`에 선언된 `const std::in_place_t` 태그 객체다. 값을 보관하지 않고 `expected`의 성공값 직접 생성 오버로드를 선택한다. 인자 자체를 넘기는 데 할당·상태 변경·참조 무효화가 없으며, 실제 객체 구성의 비용과 예외는 뒤따르는 생성자 인자가 결정한다. 태그의 정적 저장 기간과 결과 객체의 수명은 별개다.

- **생성·입력:** `expected<T,E>{std::in_place,args...}`는 `<utility>`의 `std::in_place_t` 태그 객체와 `Args&&...`를 받아 저장소 안에서 `T`를 직접 생성한다. 생성자에는 별도 반환값이 없고 성공 상태의 목적 객체가 생긴다. `T` 생성 비용·예외를 그대로 따르며 `expected` 자체가 별도 heap 할당을 요구하지는 않지만 `T`가 내부에서 할당할 수 있다. 오늘 `parsed`는 `Config::Port` 값으로 복사되고 임시 `std::string`은 `Config` 안으로 이동된다. 반환되는 `expected` prvalue는 호출자 결과 객체를 직접 초기화할 수 있어 같은 타입의 중간 `expected` 복사·이동을 요구하지 않는다.
- **실패 생성:** `unexpected(E&&)` 또는 `unexpected(const E&)`는 인자의 값 범주에 맞게 `E`를 소유한다. `std::unexpected{ParseError::Invalid}`는 CTAD로 `E=ParseError`를 추론하고, `expected<T,E>`의 `unexpected<G>` 생성 경로가 오류 대안을 만든다. 새 성공 `T`는 생성되지 않는다. 열거형 오류는 상수 시간·무할당·무예외지만 일반 `E`의 복사·이동·할당은 예외가 될 수 있다.
- **상태 관찰:** `bool has_value() const noexcept`와 `explicit operator bool() const noexcept`는 인자 없이 성공 여부를 `bool` 값으로 돌려준다. 수신 객체와 저장 값은 그대로이고 `O(1)`·무할당·무효화 없음이다. 오늘 `if (candidate)` 검사가 다음 역참조의 전제조건을 세운다.
- **성공 접근:** lvalue 수신자의 `T& operator*() & noexcept`, const lvalue의 `const T& operator*() const & noexcept` 및 대응 `operator->`는 인자 없이 소유 `T`를 빌려준다. 반드시 성공 상태여야 하며 실패 상태 역참조는 미정의 동작이다. `value()`도 cv/ref 수신자에 따라 `T&`, `const T&`, `T&&`, `const T&&`를 돌려주되 실패 상태에서는 `bad_expected_access<E>`를 던진다. 참조·포인터는 수신 `expected`의 값 대안 교체 또는 파괴 뒤 무효다. 접근 자체는 `O(1)`·무할당이지만 `value()`의 오류 예외 구성에는 `E` 복사 등이 관여할 수 있다.
- **오류 접근:** lvalue의 `E& error() & noexcept`, const lvalue의 `const E& error() const & noexcept` 등은 실패 대안을 빌려준다. 인자는 없고 반환 참조를 읽거나 수정할 수 있지만, 반드시 오류 상태여야 한다. 성공 상태에서 `error()` 호출은 미정의 동작이다. `O(1)`·무할당이며 참조는 수신 객체의 오류 대안 수명에 묶인다. 오늘 `if (!rejected)`가 이 전제조건을 세운다. 코드 주석의 `std::expected::error`는 이 멤버를 가리킨다.
- **성공값 변환 `transform`:** C++23 `template<class F> constexpr auto transform(F&& f) &&`는 rvalue 수신 `expected<T,E>`가 성공이면 `T&&`를 `f`에 전달해 결과 `U`를 보관한 `expected<U,E>`를 반환하고, 실패이면 `f`를 실행하지 않고 오류 `E`를 새 결과로 이동시킨다. 반환값은 독립 소유 객체이며 호출자가 사용한다. 오늘 [`../2026-09-17/problem.cpp`](../2026-09-17/problem.cpp)의 `expected<Quota::Units,ValidationError>` prvalue에서 `F`는 람다 타입, 입력 `unsigned` 값은 람다 값 매개변수로 복사되고 성공 결과 `Quota`가 직접 소유된다. 수신 임시 객체는 전체 식 끝에서 파괴되며 외부 `requested`는 바뀌지 않는다. 분기 자체는 상수 시간이고 실제 비용은 함수 객체 실행·`U/E` 생성 비용이다. 저장 객체 생성·할당 예외는 전파될 수 있으며 참조·포인터를 임시 내부에 저장해 반환하면 수명 종료 뒤 댕글링된다.
- **결과 연결 `and_then`:** C++23 `template<class F> constexpr auto and_then(F&& f) &&`는 성공 수신자의 `T&&`를 `f`에 넘기고 `f`가 돌려준 `expected<U,E>`를 반환한다. 실패면 `f`를 부르지 않고 같은 오류 타입 `E`를 새 `expected<U,E>`에 이동해 반환한다. `F`의 반환형이 호환되는 `expected`여야 하며 `transform`과 달리 함수 결과를 다시 `expected`로 감싸지 않는다. 오늘 성공 `int`를 `validate(int)`에 값으로 넘겨 `expected<Units,ValidationError>`를 받고, 실패면 검증 함수를 건너뛴다. 반환 prvalue는 바로 다음 `transform` 수신자가 되고, 원래 임시의 수명은 전체 식 끝까지다. 분기 자체는 상수 시간, 전체는 `f` 및 `T/E` 이동·생성 비용을 따르며 그 예외가 전파될 수 있다. 같은 수신 객체에 동시 변경을 허용하지 않는다.
- **소유권과 변경:** `std::move(*candidate)`는 성공 `Config&`를 `Config&&` xvalue로 표시할 뿐 즉시 이동하지 않는다. 뒤따르는 `Config` 이동 대입이 `active`를 바꾸며, `candidate`는 여전히 성공 상태이고 담긴 `Config`도 살아 있지만 이동된 문자열 값은 미지정이다. 파싱 실패에서는 이 대입 자체를 건너뛰므로 기존 설정을 보존한다. 다만 성공 후 실제 `Config` 대입에서 할당 또는 사용자 타입 연산이 예외를 던지는 경우까지 무조건 강한 예외 보장으로 확대하지 않는다. 필요하면 완성 후보와 예외 없는 `swap` 등을 별도로 설계한다.
- **무효화·스레드:** 상태 읽기는 수신 객체를 바꾸지 않고 별도 외부 관찰자를 무효화하지 않는다. 상태 전환·대입·파괴는 이전 활성 대안의 참조·포인터를 무효화할 수 있다. 같은 `expected` 객체의 무동기 읽기/쓰기는 데이터 경쟁이며, 한 실행 흐름에서 완성한 불변 결과를 적절히 게시한 뒤 여러 흐름이 읽는 형태로 사용한다.
- **실수 방지:** `expected`가 있다는 사실만으로 함수 전체가 `noexcept`가 되는 것은 아니다. `T`/`E` 생성·복사·이동과 내부 소유 객체 할당이 예외를 던질 수 있다. 성공 접근과 오류 접근은 반드시 상태 검사와 짝지으며, `error()`는 `value()`처럼 잘못된 상태를 예외로 바꾸는 검사형 접근자가 아니다.

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

### `std::get<I>(tuple)` — tuple 위치 접근 함수 템플릿

- **항목 종류·헤더·현재 역할:** `<tuple>`의 함수 템플릿 오버로드 집합이다. 2026-09-23의 `views::zip` 반복자는 세 원소 참조를 담은 `std::tuple<const std::string&, const int&, const int&>` proxy를 값으로 돌려준다. `auto&& [name, value, limit]` 구조적 바인딩은 tuple-like 규칙에 따라 일반 이름 조회를 하지 않고 `get<0/1/2>(proxy)`를 ADL로 찾아, `std` 연관 네임스페이스의 `std::get`으로 세 참조를 꺼낸다.
- **선택 오버로드·템플릿 인자:** 대표 위치 오버로드는 `template<std::size_t I, class... Types> constexpr std::tuple_element_t<I, std::tuple<Types...>>&& get(std::tuple<Types...>&&) noexcept`이며, 오늘 `I`는 각각 0, 1, 2이고 tuple 객체는 구조적 바인딩의 숨은 변수에서 xvalue로 전달된다. lvalue/const lvalue/const rvalue tuple에는 각각 `T&`, `const T&`, `const T&&` 계열 오버로드가 별도로 있다. 타입 기반 `get<T>`는 해당 `T`가 원소 목록에 정확히 한 번만 있을 때만 성립한다.
- **매개변수·값 범주·소유권:** 함수 인자는 살아 있는 tuple xvalue를 참조로 빌리며 tuple이나 그 원소의 소유권을 넘기지 않는다. 오늘 tuple의 원소 타입 자체가 `const U&`이므로 `T&&` 반환형과 참조 축약한 최종 결과는 `const U&`다. 즉 proxy가 xvalue여도 기반 vector의 `string`/`int`를 이동하거나 복사하지 않는다.
- **반환·사용:** 위치 `I`의 원소에 대한 cv/ref 보존 참조를 반환하고 구조적 바인딩 초기화가 그 결과를 사용한다. 오늘 세 이름은 각각 원본 이름·관측값·기준값을 가리키며 값 복사본이 아니다. 값 원소를 가진 일반 rvalue tuple에서는 `T&&`가 나와 뒤 호출이 이동을 선택할 수 있지만 `get` 자체가 이동 생성자를 실행하지는 않는다.
- **호출 뒤 상태·무효화·수명:** 접근은 tuple과 기반 원소를 변경하지 않고 참조·포인터·반복자를 무효화하지 않는다. 오늘 proxy tuple은 한 range-for 반복의 구조적 바인딩 수명 동안만 살지만, 안의 참조가 가리키는 원소 수명은 owner table이 정한다. proxy 파괴는 원본 원소를 파괴하지 않는다. 반환 참조는 owner 파괴, 기반 vector 재할당 또는 해당 원소 제거 뒤 댕글링된다.
- **전제조건·복잡도·할당·예외:** `I < sizeof...(Types)`가 컴파일 시간에 성립해야 하며 잘못된 위치는 런타임 오류가 아니라 ill-formed다. 위치 접근은 `O(1)`, 무할당, `noexcept`이고 오류값을 반환하지 않는다. 타입 기반 접근은 요청 타입이 정확히 한 번 나타나야 한다. 유효한 tuple과 살아 있는 참조 대상이 전제이며, 댕글링 결과의 역참조나 동기화 없는 공유 읽기/쓰기는 미정의 동작이다.
- **예외·스레드·기계 관점:** `get` 자체는 예외를 던지지 않고 잠금이나 원자적 snapshot을 제공하지 않는다. 별도 tuple/owner 또는 적절히 게시된 const owner의 동시 읽기는 원소 타입 계약을 따르지만, 같은 원소를 한 실행 흐름이 쓰는 동안 다른 흐름이 읽으면 데이터 경쟁이다. 구현은 보통 컴파일 시간 offset의 참조 투영으로 인라인할 수 있으나 실제 load, 주소 계산, 복사 생략 형태는 ABI·표준 라이브러리·컴파일러·최적화 옵션에 따라 달라진다.

### `std::apply` — tuple-like 호출 어댑터

- 항목 종류와 헤더: C++17부터 제공되는 `<tuple>`의 함수 템플릿이다. 오늘의 C++20 표준 선언은 `template<class F, class Tuple> constexpr decltype(auto) apply(F&& f, Tuple&& tuple)`처럼 제약과 `noexcept` 명세가 없다. C++23 선언은 `Tuple`을 표준 tuple-like 요구로 제약하고 펼친 호출의 예외 명세에 대응하는 조건부 `noexcept`를 추가한다. 템플릿 인자 `F`는 호출 가능 객체 타입, `Tuple`은 원소를 펼칠 tuple-like 타입이며 보통 호출식에서 추론하므로 직접 적지 않는다.
- 현재 코드에서의 역할: 2026-09-07의 tuple 어댑터는 `RawDeploymentRow`가 보관한 `std::string`, `int`, `bool`을 위치 순서대로 펼쳐 `DeploymentPlan`을 만드는 람다의 세 값 매개변수로 전달한다. `std::get<0>`, `std::get<1>`, `std::get<2>`를 수동 나열하지 않아 원시 행과 도메인 객체 생성 경계를 분리하며, tuple xvalue의 문자열 소유권도 결과 객체 쪽으로 이동할 수 있게 보존한다.
- 수신 객체와 인자: 자유 함수라 수신 객체는 없다. 첫 인자 식 `f`는 추론되는 `F&&` 전달 참조이며 함수, 함수 객체, 람다, 멤버 포인터처럼 펼친 인자들로 호출 가능한 대상이어야 한다. 둘째 인자 식 `tuple`은 추론되는 `Tuple&&` 전달 참조이며, 컴파일 시간 원소 수와 각 위치의 `std::get<I>` 접근을 제공하는 지원 tuple-like 객체여야 한다. 두 인자는 참조로 전달되므로 `apply`가 소유권을 새로 취하지 않는다.
- 선택된 호출과 값 범주: 원소 수를 `N`이라 하면 의미상 `std::invoke(std::forward<F>(f), std::get<0>(std::forward<Tuple>(tuple)), ..., std::get<N - 1>(std::forward<Tuple>(tuple)))` 한 번을 수행한다. non-const lvalue tuple의 값 원소는 lvalue로, const lvalue tuple의 값 원소는 const lvalue로, rvalue tuple의 값 원소는 보통 xvalue로 전달된다. 다만 원소 타입 자체가 참조이면 참조 축약 결과를 따른다. rvalue tuple을 넘긴 사실만으로 원소가 자동 이동되는 것은 아니며, 피호출자가 그 xvalue로 이동 생성·대입할 때 실제 자원 이전이 일어난다.
- 반환: 반환형 `decltype(auto)`는 피호출 결과의 정확한 타입과 값 범주를 보존한다. 피호출자가 `void`를 반환하면 `apply`도 `void`, 값을 반환하면 값, `T&` 또는 `T&&`를 반환하면 같은 참조를 반환한다. 호출자가 결과를 사용하거나 버릴지는 호출식이 정한다.
- 전제조건과 사후 상태: 펼친 모든 원소 식으로 `f`를 호출하는 식이 유효해야 하며, 원소 수·순서·cv/ref 한정이 callable의 매개변수 계약과 맞지 않으면 런타임 오류가 아니라 컴파일 실패다. 호출 전 tuple 및 그 안의 참조가 유효해야 한다. 호출 뒤 `apply` 자체는 tuple의 크기나 구조를 바꾸지 않지만, callable이 non-const 참조 원소를 변경하거나 xvalue 원소를 소비하면 해당 원소 상태가 바뀔 수 있고 callable 자체도 `operator()` 한정과 구현에 따라 변경될 수 있다.
- 복잡도·할당·무효화: `N`개 원소 접근과 callable 호출 한 번이므로 tuple 원소 수에 대해 `O(N)`이다. `apply`는 별도 소유 저장소나 동적 할당을 요구하지 않으며 tuple의 참조·포인터·반복자를 자체적으로 무효화하지 않는다. 다만 피호출 함수가 수행하는 복사·이동·할당과 컨테이너 변경 비용 및 무효화 규칙은 그대로 적용된다.
- 수명: `apply`는 callable, tuple, 원소 또는 원소가 가리키는 대상의 수명을 연장하지 않는다. 반환 참조는 tuple 원소나 외부 객체를 가리킬 수 있다. 임시 tuple의 원소를 참조로 반환하면 그 임시는 전체 표현식 끝에 파괴되므로 이후 참조 사용은 댕글링이 될 수 있다. 이동된 원소의 구체적 사후 값도 해당 원소 타입의 이동 계약을 따른다.
- 예외·오류·미정의 동작: 오늘 사용하는 C++20의 이식 가능한 계약에서는 `apply` 자체가 `noexcept`로 선언되지 않아 callable이 실제로 던지지 않더라도 `noexcept(std::apply(...))`를 참이라고 기대할 수 없다. C++23 조건부 `noexcept`는 전달된 callable을 전달된 원소 식들로 `std::invoke`하는 호출이 던지지 않는지에 대응한다. 어느 버전이든 실제 callable이나 원소 전달 과정에서 난 예외는 호출자에게 전파될 수 있다. 일부 구현이 새 명세를 이전 언어 모드에 확장 제공할 수 있지만 이에 의존하지 않는다. 호출 불가능한 타입 조합은 ill-formed이며, 잘못된 인덱스를 런타임에 선택하는 API가 아니다. 댕글링 참조, 데이터 경쟁, 또는 피호출 함수의 전제조건 위반으로 생기는 미정의 동작을 `apply`가 방지하지는 않는다.
- 스레드 보장: 자체 동기화를 제공하지 않는다. 서로 독립된 callable과 tuple을 호출하는 것은 각 객체의 계약에 따르지만, 같은 callable 또는 같은 원소를 한 스레드가 변경하는 동안 다른 스레드가 동기화 없이 접근하면 데이터 경쟁이 될 수 있다. 모든 공유 접근이 읽기뿐인지, 아니면 mutex·atomic 등 별도 동기화가 있는지 호출자가 보장한다.
- 기계 실행 관점: 구현은 보통 컴파일 시간 인덱스 팩을 펼쳐 원소 로드와 한 번의 호출로 낮추고, 구체 callable이면 인라인될 수도 있다. 그러나 타입 소거 래퍼·함수 포인터라면 간접 호출이 남을 수 있고, 실제 로드·저장·분기 및 복사 생략 형태는 컴파일러, ABI, 최적화 옵션에 따라 달라진다.

```cpp
#include <cassert>
#include <string>
#include <tuple>
#include <utility>

int main() {
    auto arguments = std::tuple{std::string{"camera"}, 7};

    // rvalue tuple의 string 원소는 xvalue로 펼쳐지고, 값 매개변수가 그 자원을 소비할 수 있다.
    const auto label = std::apply(
        [](std::string name, int id) { return name + "-" + std::to_string(id); },
        std::move(arguments));

    assert(label == "camera-7");
}
```

## `std::reference_wrapper<T>`와 `std::ref`/`std::cref`

- 일반 객체처럼 복사 가능하면서 내부에는 비소유 참조 의미를 보관한다.
- `get()`은 원본 `T&`를 반환하고 `operator T&` 변환도 제공한다.
- `std::ref(object)`는 `reference_wrapper<T>`, `std::cref(object)`는 읽기 전용 `reference_wrapper<const T>`를 만든다.
- 원본 수명을 연장하지 않으므로 컨테이너나 `optional`에 오래 저장할 때 수명을 검증한다.

## `std::bind_back` — 뒤쪽 인자를 소유하는 C++23 호출 어댑터

- **항목 종류·헤더**: `<functional>`이 선언하는 C++23 함수 템플릿이다. 대표 선언은 `template<class F, class... Args> constexpr unspecified bind_back(F&& f, Args&&... args);`다. 반환 타입은 이름을 직접 쓸 수 없는 구체 perfect-forwarding call wrapper이므로 보통 `auto`로 받는다. `std::function`처럼 런타임 타입 소거를 수행하는 타입은 아니다.
- **저장 타입과 현재 역할**: 함수 대상은 `FD = decay_t<F>`, 각 bound 인자는 `Bound_i = decay_t<Args_i>` 값으로 wrapper 안에 저장된다. 2026-09-21의 가격 예제는 함수 포인터와 이동된 `PricingPolicy` 값을 소유하고, 예약 예제는 임시 `ReservationLedger`의 값에서 별도 저장 subobject를 직접 구성해 호출 사이 변경 상태를 유지한다. 원본 임시는 전체 표현식 끝에 파괴되며, 참조 수명이 연장되는 것이 아니라 별개의 wrapper subobject가 wrapper와 함께 살아 있는 것이다.
- **생성 인자**: 자유 함수이므로 수신 객체는 없다. `f`와 각 `args`는 전달 참조이며 각각 `std::forward<F>(f)`, `std::forward<Args_i>(args_i)`로 대응 저장 상태를 직접 초기화한다. 정확한 요구는 각 `decay_t<Arg>`가 전달된 그 `Arg` 식에서 구성 가능하고 요구되는 이동 구성을 만족하는 것이다. 보통 같은 타입 lvalue는 복사되고 rvalue는 이동되지만 이것을 무조건 `CopyConstructible` 요구로 바꾸어 말하면 안 된다. 함수 overload set은 그 자체로 타입을 추론할 수 없으므로 함수 포인터 cast나 이름 있는 정확한 함수 포인터로 먼저 해소한다.
- **생성 결과·사후 상태**: 반환 wrapper prvalue가 함수 대상과 bound 인자의 수명을 소유한다. 생성자 같은 별도 `void` 반환이 아니라 wrapper 값 자체가 결과다. xvalue로 넘긴 원본은 실제 저장 subobject 생성이 이동을 선택하면 유효하지만 값이 미지정인 상태가 될 수 있다. wrapper의 복사·이동 가능 여부와 그 뒤 원본 상태는 모든 저장 타입의 복사·이동 계약에 의존한다.
- **호출 위치와 순서**: wrapper의 `operator()(call_args...)`는 표준 INVOKE 의미 규칙으로 저장 함수 대상에 **호출 시 인자를 먼저**, 저장 bound 인자를 그 뒤에 붙여 정확히 한 번 호출한다. 즉 의미상 `fd(call_args..., bound_args...)`다. 반환형과 값 범주는 대상 호출의 결과를 그대로 보존하며 대상이 `void`면 wrapper 호출도 `void`다.
- **cv/ref 전달 규칙**: non-const lvalue wrapper는 각 저장 상태를 `T&`, const lvalue는 `const T&`, non-const rvalue는 `T&&`, const rvalue는 `const T&&`로 대상에 전달한다. 호출 시 인자는 원래 값 범주를 보존한다. volatile 또는 const-volatile wrapper 호출은 지원되지 않는다. 따라서 변경 가능한 `Ledger&`를 요구하는 대상은 non-const lvalue wrapper로 호출할 수 있지만 const wrapper로는 호출할 수 없다.
- **전제조건·컴파일 오류**: decay 저장 타입은 전달된 입력에서 구성 가능하고 표준이 요구하는 이동 구성을 만족해야 한다. 선택한 wrapper cv/ref와 call-time 인자, 저장 인자를 이어 붙였을 때 대상이 호출 가능해야 해당 `operator()`가 성립한다. 인자 순서·cv/ref·arity가 맞지 않으면 런타임 상태 코드가 아니라 컴파일 오류다.
- **복잡도·할당**: 생성은 함수 대상 하나와 각 bound 상태 하나를 초기화하고, 호출은 대상 호출 한 번을 중계한다. 표준은 `bind_back` 전체에 대한 일반적인 시간 복잡도나 무할당 보장을 따로 주지 않으므로 저장 타입의 복사·이동·할당 비용과 구현 계약을 확인한다. “항상 O(1)” 또는 “항상 allocation-free”라고 단정하지 않는다.
- **무효화·수명**: `bind_back` 자체가 별도의 범용 반복자 무효화 규칙을 추가하지 않으며, `string`·`vector` 같은 각 저장 타입의 연산이 그 타입 고유 규칙을 따른다. 저장 상태 내부를 가리키는 참조·포인터·반복자는 wrapper의 이동·파괴 또는 대상이 수행한 상태 변경 뒤 댕글링되거나 무효화될 수 있다. raw pointer, `string_view`, iterator, `reference_wrapper`처럼 비소유 값을 bind해도 pointee·원본·범위의 수명은 연장되지 않는다. rvalue wrapper 호출은 저장 상태를 xvalue로 전달할 수 있으므로 대상이 이를 이동 소비한 뒤 같은 wrapper를 다시 호출할 수 있는지도 대상 계약으로 증명해야 한다.
- **예외·오류 보장**: 생성 중 저장 상태의 복사·이동·변환이 던진 예외는 호출자에게 전파되고 완성 wrapper가 남지 않는다. 호출 중에는 인자 변환과 실제 대상 함수가 던진 예외가 전파된다. 호출 불가능한 조합은 컴파일 오류이고, 댕글링 bound 상태 역참조·null 함수 포인터 호출·동기화 없는 데이터 경쟁처럼 원래 대상 계약이 금지한 실행은 미정의 동작이다. `bind_back`은 이를 런타임에 검사하지 않는다.
- **스레드 보장**: 자체 동기화를 제공하지 않는다. 읽기 전용 저장 상태와 thread-safe 대상만 공유 호출할 수 있다. 같은 wrapper가 소유한 변경 상태를 여러 스레드가 동시에 접근하면 mutex·atomic 같은 별도 동기화가 필요하다.
- **기계 실행 관점**: 반환 래퍼 타입과 저장 대상이 컴파일 시간에 보이므로 함수 객체 호출은 인라인될 수 있다. 함수 포인터를 저장하면 간접 호출이 남을 수도 있다. 실제 load/store, 분기, 인라인, 저장 layout은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.

```cpp
#include <functional>

struct Policy { int add{}; };

int price(int value, const Policy& policy) {
    return value + policy.add;
}

int main() {
    // 원본 임시는 식 끝에 파괴되고, 그 값에서 만든 별도 decay 저장 객체를 wrapper가 소유한다.
    auto add_tax = std::bind_back(&price, Policy{10});
    return add_tax(90) == 100 ? 0 : 1;
}
```

## `std::function<Signature>` — `<functional>`

`std::function<R(Args...)>`은 지정한 서명으로 호출할 수 있는 함수, lambda, 함수 객체를 타입 소거해 **값으로
소유**하는 class template이다. 서로 다른 구체 callback을 한 저장소나 scheduler 경계에 넣을 수 있지만, 저장
대상은 복사 가능해야 하고 타입 소거 호출·저장 비용을 고려해야 한다.

### 호출 대상 생성과 소유 계약

- **항목 종류·대표 형태:** `<functional>`의 class template이며, 호출 대상을 받는 대표 생성자는 `template<class F> function(F&& f);`다. 실제 참여 조건은 `decay_t<F>`가 CopyConstructible이고 선언한 `R(Args...)` 서명으로 호출 가능하며 결과가 `R` 요구를 만족하는지 등을 검사한다.
- **수신 객체·인자:** 아직 구성되지 않은 `function<R(Args...)>` 목적 객체가 결과다. `f`는 forwarding reference다. lvalue 호출 대상은 내부 저장 객체로 복사되고 rvalue는 이동될 수 있으며, 람다의 값 캡처도 closure의 subobject로 함께 복사·이동된다. 참조 캡처는 참조 대상의 수명을 연장하지 않는다.
- **반환형·사후 상태:** 생성자는 별도 반환값이 없다. 성공하면 wrapper가 decay된 호출 대상의 독립 수명을 소유하고 bool 검사에서 참이 된다. 오늘 weak-self lambda의 closure는 `weak_ptr` 값만 소유하므로 callback이 살아 있어도 원래 서비스 객체의 strong count는 늘지 않는다.
- **복잡도·할당·예외:** 저장과 복사 비용은 호출 대상 크기·연산에 따르며 작은 객체 최적화의 크기와 적용 여부는 구현 세부다. 함수 pointer와 `reference_wrapper`를 저장하는 경우 동적 할당을 하지 않아야 하지만 일반 lambda는 동적 할당과 `bad_alloc` 가능성을 배제할 수 없다. 호출 대상 복사·이동 생성자의 예외도 전파되며 실패하면 목적 wrapper는 완성되지 않는다.
- **수명·무효화:** wrapper 파괴·대입·reset 성격의 빈 값 교체는 보관한 closure를 파괴하고 그 closure가 가진 weak count도 해제한다. closure가 빌린 reference/pointer는 별도 owner가 먼저 파괴하면 댕글링할 수 있다. 값 캡처한 weak handle은 객체 파괴 뒤에도 만료 상태로 검사 가능하며 마지막 weak 몫이면 control block 해제로 이어질 수 있다.

### `std::function::operator()` 호출 계약

- **수신 객체·시그니처:** 수신자는 호출 대상을 가진 `const` 또는 non-const `function<R(Args...)>` lvalue다. 대표 멤버는 `R operator()(Args... args) const;`이며 오늘의 `function<bool(int)>`에는 `int` 하나를 값으로 받아 `bool`을 반환하는 specialization이 대응한다.
- **매개변수·반환값:** 일반 서명에서는 선언된 `Args...`에 맞게 각 호출 인자를 받아 저장 대상에 전달한다. 참조 매개변수는 대상을 빌리고 값 매개변수는 별도 매개변수 객체를 만들 수 있다. 오늘의 `int` prvalue는 event/job code 매개변수 값이 되고 소유권을 전달하지 않는다. 저장 대상의 호출 결과를 `R`로 반환하며 `void` 서명은 값 결과가 없다. 오늘 반환 `bool`은 weak 승격 성공 뒤 조건까지 만족했는지를 뜻하고 호출자가 검사한다.
- **사후 상태:** `function` wrapper 자체는 계속 같은 호출 대상을 소유한다. 다만 mutable lambda나 상태 있는 함수 객체는 호출 과정에서 내부 상태를 바꿀 수 있으므로 `operator() const` 표기만 보고 논리적 불변이라고 단정하지 않는다. weak-self callback은 매 호출마다 `lock()`의 지역 strong handle을 만들었다가 호출 끝에 해제한다.
- **오류·전제조건:** 빈 wrapper 호출은 미정의 동작이 아니라 `bad_function_call` 예외를 던진다. 저장 대상이 던진 예외도 호출자에게 전파된다. 수명 보장이 없는 참조 캡처나 빈 `shared_ptr` 역참조에서 생기는 UB를 wrapper가 막아 주지는 않는다.
- **복잡도·할당·기계 실행:** 호출 자체는 일반적으로 새 저장소를 할당하지 않지만 대상 함수의 동작은 할당할 수 있다. 타입 소거 dispatch는 간접 호출이 될 수 있으나 정확한 인라이닝·분기 형태는 compiler, ABI, 최적화 옵션에 따라 다르다. 성능이 중요한 닫힌 template 경로에서는 구체 lambda 타입이나 `auto` 매개변수를 검토한다.
- **스레드 보장:** 서로 다른 wrapper 사본의 저장 상태는 독립적이지만 캡처한 대상이나 control block은 공유될 수 있다. 같은 wrapper 또는 mutable closure 상태를 동시 변경·호출하는 안전성은 자동 제공되지 않는다. weak/shared control block 조정이 안전해도 원래 서비스 객체의 mutable 데이터에는 별도 동기화가 필요하다.

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
