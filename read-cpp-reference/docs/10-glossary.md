# 10. 용어·기호 색인

문서에서 낯선 항목을 만났을 때 Ctrl+F로 찾는 첫 장소입니다. 한 항목을 읽고도 이해되지
않으면 연결된 장 전체를 읽으세요.

## 기호

| 기호 | 의미 | 더 읽기 |
|---|---|---|
| `::` | namespace/class scope의 이름을 지정 | [이름 읽기](01-how-to-read-cppreference.md#이름을-읽는-법) |
| `.` / `->` | 객체 / 포인터를 통한 멤버 접근 | [포인터·참조](03-declarations-and-signatures.md#포인터참조const) |
| `*` | 곱셈, 포인터 선언, 역참조 | 문맥으로 구분 |
| `&` | AND, 주소 얻기, lvalue 참조, ref-qualifier | [선언](03-declarations-and-signatures.md) |
| `&&` | 논리 AND, rvalue 참조, forwarding reference | [value category](04-types-values-lifetime.md#value-category) |
| `<...>` | 템플릿 인자 목록(또는 비교 연산 문맥) | [템플릿](05-classes-templates-callables.md#템플릿의-핵심) |
| `(...)` | 그룹, 함수 선언/호출, 초기화, cast | [선언](03-declarations-and-signatures.md) |
| `{...}` | block, 목록 초기화, 함수/클래스 몸체 | [초기화](02-core-syntax.md#초기화는-대입과-다르다) |
| `[...]` | 배열 첨자, attribute `[[...]]`, lambda capture | [lambda](05-classes-templates-callables.md#lambda-문법) |
| `...` | parameter pack/fold expansion | [fold](05-classes-templates-callables.md#parameter-pack과-fold-expression) |
| `->` | 멤버 접근 또는 trailing return type | [선언](03-declarations-and-signatures.md) |
| `<=>` | three-way comparison(C++20) | [C++20](09-cpp20-guide.md#defaulted-comparison과-) |
| `#` / `##` | 전처리 stringize / token paste | 매크로 심화 |

## A–D

- **ABI**: 컴파일된 코드 사이의 호출/배치/이름 규약. API와 달리 binary 호환성 문제.
- **abstract class**: pure virtual 함수를 가져 직접 객체를 만들 수 없는 클래스.
- **aggregate**: 특별 규칙을 만족해 멤버 목록 초기화 가능한 클래스/배열.
- **alignment**: 객체 주소가 만족해야 하는 배수 조건.
- **allocator**: 저장 공간 획득/해제 정책 객체.
- **argument**: 호출할 때 전달하는 실제 표현식. parameter와 구분.
- **atomic operation**: 다른 thread가 중간 상태를 관찰하지 않는 연산.
- **attribute**: `[[nodiscard]]`처럼 선언/문에 부가 의미를 주는 표준 문법.
- **automatic storage duration**: 보통 블록 진입/이탈에 연결된 저장 기간.
- **bit/byte**: bit는 이진 자리, byte는 주소 지정 가능한 최소 단위. `std::byte`는 raw byte 타입.
- **borrowed range**: 임시 range가 사라진 뒤 iterator 유효성 모델을 나타내는 C++20 개념.
- **callable**: 함수 호출 문법/`std::invoke`로 호출 가능한 대상.
- **class invariant**: public 연산 전후 유지해야 하는 객체 유효 조건.
- **compile time / run time**: 번역 중 결정 / 실행 중 결정.
- **concept**: 템플릿 인자의 요구 조건에 이름을 붙인 C++20 언어 기능.
- **constant expression**: 컴파일 때 평가가 허용되는 규칙을 만족한 표현식.
- **constructor/destructor**: 객체 수명 시작 시 초기화 / 수명 종료 시 정리하는 특별 멤버.
- **conversion**: 한 타입의 값을 다른 타입 표현으로 바꾸는 과정.
- **copy elision**: 조건에 따라 복사/이동 객체 생성을 생략하거나 C++17부터 직접 구성.
- **coroutine**: 실행을 suspend/resume할 수 있는 함수 메커니즘(C++20).
- **cv-qualified**: const/volatile 또는 둘 다로 한정된 타입.
- **dangling**: 가리키던 객체/범위 수명이 끝나 유효하지 않은 pointer/reference/iterator/view.
- **data race**: 비동기 충돌 접근이 동기화되지 않은 상태. C++에서는 UB.
- **declaration/definition**: 이름/타입을 알림 / 프로그램 실체를 제공.
- **deduction guide**: CTAD가 클래스 템플릿 인자를 추론할 규칙.
- **dependent name**: 템플릿 인자에 따라 의미가 달라지는 이름.
- **deprecated**: 향후 제거 가능하여 사용 중단이 권고되는 기능.

## E–L

- **exception**: `throw`로 전달되고 handler가 잡는 실행 중 오류 전달 객체/메커니즘.
- **exception safety**: 예외 발생 후 상태/누수에 대한 보장.
- **exposition-only**: 표준 설명을 위한 이름/도우미로 실제 public API가 아님.
- **expression/statement**: 값을 계산·side effect를 냄 / 실행 문법 단위.
- **feature-test macro**: 아래 별도 항목 참고.
- **forwarding reference**: 타입 추론되는 `T&&`가 value category 보존 전달에 쓰이는 경우.
- **function object(functor)**: `operator()`를 정의한 클래스 객체.
- **glvalue/lvalue/xvalue/prvalue/rvalue**: [value category](04-types-values-lifetime.md#value-category).
- **happens-before**: 한 평가의 효과가 다른 평가에 보이도록 정하는 메모리 모델 관계.
- **header**: 선언/정의를 소스에 포함시키는 표준/사용자 인터페이스 파일.
- **ill-formed**: C++ 문법/의미 규칙을 만족하지 않는 프로그램.
- **implementation-defined**: 구현이 선택하고 문서화해야 하는 동작.
- **incomplete type**: 선언됐지만 크기/멤버 정의가 아직 완전하지 않은 타입.
- **instantiation**: 구체 템플릿 인자로 실제 specialization을 만드는 과정.
- **iterator**: range 안의 위치를 표현하는 일반화된 포인터.
- **lambda**: 익명 함수 객체를 만드는 표현식.
- **linkage**: 같은/다른 번역 단위의 이름이 같은 실체를 가리키는 규칙.
- **literal**: 소스에 직접 쓴 값 표기.
- **lock-free/wait-free**: [lock-free 단계](07-concurrency-volatile-atomic.md#lock-free의-단계).

## M–R

- **memory order**: atomic 연산과 주변 메모리 효과의 순서 규칙.
- **module**: import 가능한 번역 단위/인터페이스 체계(C++20).
- **move semantics**: 수명이 끝나 가는 객체의 자원을 복사 대신 이전하는 규칙.
- **mutex**: 상호 배제로 임계 구역을 보호하는 동기화 객체.
- **name hiding**: inner/derived scope의 이름이 바깥/base의 같은 이름 탐색을 가리는 현상.
- **namespace**: 관련 이름을 묶고 충돌을 줄이는 scope.
- **narrowing**: 범위/정밀도 손실 가능 변환.
- **noexcept**: 예외 비투척 명세. 밖으로 예외가 나오면 terminate.
- **ODR**: One Definition Rule. 정의 개수와 동일성에 관한 프로그램 규칙.
- **overload/override**: 같은 이름의 여러 함수 / base virtual 재정의.
- **parameter**: 함수/템플릿 선언의 입력 자리. argument와 구분.
- **parameter pack**: 0개 이상의 템플릿/함수 매개변수 묶음.
- **polymorphism**: 같은 인터페이스로 여러 실제 타입의 동작을 사용하는 성질.
- **precondition/postcondition**: 호출 전 만족 조건 / 정상 반환 후 보장.
- **RAII**: 자원 수명을 객체 수명에 묶는 설계.
- **range**: `begin`부터 `end`까지 순회 가능한 요소 구간.
- **reference collapsing**: 템플릿 추론 등에서 `&`/`&&` 조합을 단일 참조로 정규화하는 규칙.
- **requires-expression/clause**: 표현식 유효성 검사 / 템플릿 제약 부착 문법.

## S–Z

- **scope**: 이름의 가시성과 의미가 적용되는 영역.
- **SFINAE**: 템플릿 치환 실패 시 후보 제거 규칙.
- **side effect**: 객체 변경, I/O, volatile 접근 같은 상태 변화.
- **specialization**: 구체 템플릿 인자에 대한 템플릿의 결과/별도 정의.
- **static storage duration**: 프로그램 전체 실행 동안 유지되는 저장 공간.
- **strict weak ordering**: 정렬 comparator가 만족해야 하는 순서 관계 법칙.
- **template**: 타입/값/템플릿을 매개변수화한 선언.
- **thread safety**: 여러 thread 접근 조합에 대해 문서가 제공하는 보장.
- **translation unit**: 전처리 후 하나의 소스 파일이 이루는 컴파일 단위.
- **type alias**: `using Name = Type`으로 만든 같은 타입의 다른 이름.
- **type trait**: 타입 성질 질문/변환용 compile-time 템플릿.
- **undefined behavior(UB)**: 표준이 아무 결과도 요구하지 않는 동작.
- **unspecified behavior**: 허용된 동작 중 하나지만 무엇을 택할지 지정하지 않음.
- **value category**: 표현식이 객체 정체성/이동 가능성을 나타내는 분류.
- **view**: 일반적으로 원본을 소유하지 않고 변환/부분 구간을 보는 가벼운 range.
- **volatile**: 특수한 관찰 가능한 접근을 위한 cv 한정자. thread 동기화가 아님.

## 표준 문서 약어

- **CWG**: Core Working Group, 언어 핵심 결함
- **LWG**: Library Working Group, 표준 라이브러리 결함
- **DR**: Defect Report, 이미 발표된 표준에 소급 적용될 수 있는 수정
- **PxxxxRn**: 제안 paper 번호와 revision
- **Nxxxx**: WG21 문서 번호
- **FTM**: feature-test macro
- **TS**: Technical Specification, 본 표준 합류 전 별도 규격

## 기능 테스트 매크로

```cpp
#if defined(__cpp_structured_bindings) && __cpp_structured_bindings >= 201606L
// 언어 기능
#endif

#include <version>
#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L
// 라이브러리 기능
#endif
```

날짜처럼 보이는 수치는 대체로 승인 시점을 반영한 개정 수준이지 C++ 버전 번호 자체가
아닙니다. cppreference의 feature-test 표에서 요구하는 최솟값을 확인합니다.
