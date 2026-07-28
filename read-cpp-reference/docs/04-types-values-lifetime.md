# 04. 타입, 값, 객체 수명

## 이 구분이 필요한 이유

cppreference의 “the lifetime of the referred object must exceed…”, “prvalue is materialized”,
“the behavior is undefined” 같은 문장을 읽으려면 타입 문법만으로는 부족합니다. 객체가
언제 생기고 없어지며 표현식이 그 객체를 어떻게 가리키는지 알아야 합니다.

## 정적 타입과 동적 타입

- static type: 컴파일 시 표현식에 알려진 타입
- dynamic type: 다형적 base 참조/포인터가 실행 중 실제로 가리키는 가장 파생된 타입

```cpp
Base& ref = derived;
// ref의 정적 타입: Base
// ref가 가리키는 객체의 동적 타입: Derived
```

가상 함수는 동적 타입을 따라 호출됩니다. 비가상 함수와 overload 선택은 주로 정적 타입에
따릅니다.

## value category

간단한 실전 구분:

- lvalue: 정체성과 위치가 지속되는 객체를 나타냄. 이름 붙은 변수 표현식은 대개 lvalue.
- xvalue: 수명이 끝나 가며 자원을 이동해도 되는 객체를 나타냄. `std::move(x)`.
- prvalue: 값을 계산하거나 임시 객체를 초기화하는 순수 rvalue. `42`, `Widget{}`.
- glvalue = lvalue + xvalue, rvalue = prvalue + xvalue.

중요한 함정: 변수의 선언 타입이 `T&&`여도 그 **이름을 쓴 표현식**은 lvalue입니다.

```cpp
Widget&& w = Widget{};
consume(w);            // w 표현식은 lvalue
consume(std::move(w)); // xvalue로 변환
```

`std::move`는 실제로 옮기지 않고 xvalue로 캐스트할 뿐입니다. 이동 생성자/대입이 선택되어야
자원이 이동합니다.

## 저장 기간과 수명

- automatic storage duration: 보통 블록 지역 객체, scope를 나갈 때 저장 해제
- static storage duration: 프로그램 시작부터 종료까지
- thread storage duration: 각 스레드 시작부터 종료까지
- dynamic storage duration: `new`로 얻고 `delete`할 때까지

storage duration은 저장 공간의 존속, lifetime은 그 공간 안의 특정 객체가 유효한 기간입니다.
placement new 같은 고급 기능에서는 둘이 특히 다릅니다.

## RAII

Resource Acquisition Is Initialization은 자원의 획득/해제를 객체의 생성/소멸에 묶습니다.

```cpp
{
    std::lock_guard<std::mutex> lock(mutex); // 생성하며 lock
    // 보호된 작업
} // scope를 벗어나 소멸하며 unlock
```

파일, mutex, 메모리, 소켓 같은 자원에 RAII를 적용하면 `return`이나 예외에서도 정리가
됩니다. `std::vector`, `std::string`, `std::unique_ptr`를 우선 사용하고 직접 `new/delete`는
소유권 구현 코드가 아니면 피합니다.

## 특별 멤버 함수와 Rule of 0/5

클래스에는 기본 생성자, 소멸자, 복사 생성/대입, 이동 생성/대입이 관련됩니다.

- Rule of 0: 자원을 표준 RAII 멤버에 맡겨 다섯 함수를 직접 쓰지 않기
- Rule of 5: 하나를 직접 관리해야 한다면 복사/이동/소멸 정책 전체를 검토하기

```cpp
class Owner {
    std::unique_ptr<int> value_;
public:
    explicit Owner(int v) : value_(std::make_unique<int>(v)) {}
    // unique_ptr 덕분에 소멸/이동은 안전, 복사는 자동 금지
};
```

## 참조·포인터와 소유권

- `T&`: null이 아닌 기존 객체를 빌려 쓴다는 관례
- `T*`: null 가능 또는 배열 첫 요소 등; 자체로 소유 여부를 말하지 않음
- `std::unique_ptr<T>`: 단독 소유
- `std::shared_ptr<T>`: 참조 횟수 기반 공동 소유
- `std::weak_ptr<T>`: shared 소유에 참여하지 않는 관찰자
- `std::span<T>`, `std::string_view`: 비소유 view; 원본 수명이 더 길어야 함

타입만으로 불분명한 raw pointer 소유권은 API 문서에 명시해야 합니다.

## temporary와 lifetime extension

```cpp
const std::string& ok = std::string("temporary");
// 지역 const 참조에 직접 바인딩된 임시 객체 수명이 ok의 수명까지 연장
```

모든 전달에서 연장되는 것은 아닙니다. 함수가 임시 객체에 대한 참조를 반환하거나,
`std::string_view`에 임시 `std::string`을 연결하면 dangling이 될 수 있습니다.

```cpp
std::string_view bad() {
    std::string local = "gone";
    return local; // view는 함수 종료 후 사라진 문자열을 가리킴
}
```

## iterator/reference invalidation

컨테이너를 수정하면 기존 iterator, pointer, reference가 무효화될 수 있습니다.
예를 들어 `std::vector` 재할당은 모든 요소 주소를 바꿉니다. API 페이지에서
“iterator invalidation” 문장을 반드시 확인하세요.

```cpp
std::vector<int> v{1, 2};
int& first = v[0];
v.push_back(3); // 재할당되었다면 first는 dangling
```

## 정의되지 않은 동작 (UB)

UB가 발생하면 표준은 어떤 결과도 보장하지 않습니다. 크래시만을 뜻하지 않으며 겉보기
정상 동작도 가능합니다.

대표 원인:

- 배열 범위 밖 접근
- dangling pointer/reference 역참조
- 초기화되지 않은 값의 부적절한 사용
- signed integer overflow
- data race
- null pointer 역참조
- 이미 파괴된 객체 사용
- API precondition 위반

“내 컴퓨터에서 됐다”는 UB의 안전 근거가 아닙니다. 경고를 켜고, 가능하면 AddressSanitizer와
UndefinedBehaviorSanitizer를 사용합니다.

## 예외 안전성

- no-throw guarantee: 실패해도 예외를 던지지 않음
- strong guarantee: 실패 시 관찰 가능한 상태가 호출 전과 같음
- basic guarantee: 불변식은 유지되고 누수는 없지만 상태는 바뀔 수 있음
- no guarantee: 더 약한 상태

`noexcept`는 “오류가 절대 없다”가 아니라 예외가 함수 밖으로 나가지 않는다는 계약입니다.
위반하여 예외가 빠져나가면 `std::terminate`가 호출됩니다.

다음: [클래스·템플릿·호출 가능 객체](05-classes-templates-callables.md)
