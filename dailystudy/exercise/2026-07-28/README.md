# 2026-07-28 — `shared_ptr`/`weak_ptr`로 안전한 이벤트 구독 만들기

## 오늘의 목표

오늘은 여러 객체가 함께 사용하는 구독자를 `std::shared_ptr`로 표현하고, 이벤트 발행자는 소유하지 않는 `std::weak_ptr`만 보관한다. 이 구조는 발행자와 구독자가 서로를 강하게 소유해 영원히 파괴되지 않는 순환 소유를 피한다.

- `IEventListener`: 애플리케이션 계층이 요구하는 이벤트 수신 포트
- `EventBus`: 약한 참조만 저장하고 살아 있는 구독자에게 이벤트를 전달하는 조정자
- `ConsoleListener`: 포트를 구현하는 외부 출력 어댑터
- `std::shared_ptr<T>`: 제어 블록의 강한 참조 수를 통해 객체 수명을 공유
- `std::weak_ptr<T>`: 강한 참조 수를 늘리지 않는 관찰자. `lock()` 성공 시 임시 `shared_ptr` 획득

## 기초 문법을 코드에서 읽는 법

`int count{0};`은 기본 정수 타입 변수의 중괄호 초기화다. 중괄호는 위험한 축소 변환을 막는 데 도움을 준다. 함수는 `반환형 이름(매개변수)` 모양이며 `bus.publish("ready")`처럼 호출한다. `void`는 반환값이 없음을 뜻한다.

`struct`의 기본 접근은 `public`, `class`의 기본 접근은 `private`다. 예제는 경계를 분명히 보이려고 `public:`과 `private:`를 직접 쓴다. 생성자는 클래스와 이름이 같고 반환형이 없다. `explicit ConsoleListener(std::string name)`의 `name`은 생성자 매개변수이며, `: name_{std::move(name)}`은 본문 실행 전에 멤버를 만드는 멤버 초기화 목록이다. `explicit` 때문에 `ConsoleListener listener = "ui";` 같은 암시적 변환은 막히며 `ConsoleListener listener{"ui"};` 같은 직접 초기화가 올바르다.

`using ListenerPtr = std::shared_ptr<IEventListener>;`는 긴 타입에 붙인 별칭이다. `std::shared_ptr<IEventListener>`의 `<IEventListener>`는 템플릿 인자다. `++count`, `!=`, `==`, `<<`, `->`는 각각 증가·비교·비교·스트림 삽입·포인터 멤버 접근 연산자다. `std::make_shared`, `std::move`, `weak.lock()`, `expired()`는 표준 라이브러리 기능이며 등장 위치의 주석을 함께 읽는다.

## 값 범주, 수명, 복사·이동

- 이름이 있는 `listener`, `bus`, 함수 매개변수 `message`는 lvalue다.
- `std::make_shared<ConsoleListener>("ui")`의 결과는 prvalue다. 반환 prvalue는 목적 객체를 직접 만드는 복사 생략 대상이다.
- `std::move(name)`은 이름 있는 lvalue를 xvalue로 바꾸는 캐스트다. 실제 이동 여부는 선택된 생성자에 달려 있다.
- `const std::string& message`는 기존 lvalue에 읽기 전용으로 바인딩되어 복사를 피한다. 반면 rvalue 참조는 임시값에 바인딩할 수 있다.
- `weak.lock()`이 성공하면 `shared_ptr` prvalue가 만들어져 지역 변수에 이동 또는 생략되어 들어가고, 그 지역 변수가 살아 있는 동안 대상 객체도 살아 있다.
- 마지막 강한 `shared_ptr`가 파괴되면 객체 수명이 끝난다. `weak_ptr`는 객체를 살려 두지 않으며 이후 `lock()`은 빈 포인터를 반환한다.

## 기계 실행 관점

발행 과정에는 보통 약한 참조 상태 로드, 만료 여부 비교, 조건 분기, 참조 수 갱신, 함수 호출이 포함될 수 있다. `listener->on_event(...)`는 인터페이스 포인터를 통한 가상 간접 호출이 될 수 있지만 컴파일러가 실제 타입을 증명하면 직접 호출로 최적화할 수도 있다. 실제 명령과 메모리 접근은 CPU, ABI, 컴파일러, 표준 라이브러리 구현, 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 빌드와 실행

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 직접 해보기

1. `main.cpp`에서 두 번째 구독자를 추가하고 두 객체가 모두 호출되는지 확인한다.
2. 첫 번째 구독자의 강한 포인터를 `reset()`한 뒤 다시 발행해 만료 구독자가 제거되는지 확인한다.
3. `problem.cpp`의 `TODO` 구현을 먼저 가리고 직접 작성한다.
4. `EventBus`가 `shared_ptr`를 저장하면 어떤 순환 소유 위험이 생길지 객체 수명 그림으로 설명한다.

마지막으로 [CHECKPOINT.md](./CHECKPOINT.md)의 검증 단계를 통과해 이해를 증명한다.
