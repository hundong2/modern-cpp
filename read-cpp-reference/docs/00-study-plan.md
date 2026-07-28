# 00. 학습 순서

## 왜 이 순서인가

cppreference는 교과서가 아니라 “이미 문법을 아는 사람이 빠르게 사실을 확인하는 사전”에
가깝습니다. `std::atomic` 페이지 하나에도 클래스 템플릿, 멤버 함수, cv 한정자, 예외 명세,
메모리 모델이 한꺼번에 등장합니다. 아래 과정은 그 의존성을 앞에서부터 풀어냅니다.

## 8주 과정

속도보다 “선언을 소리 내어 자기 말로 읽을 수 있는가”를 기준으로 진도를 정하세요.

### 1주: 프로그램과 기본 문법

- [ ] 번역 단위, 헤더, `#include`, `main`, 문(statement), 표현식(expression)을 구분한다.
- [ ] 기본 타입, 변수, 초기화, 연산자, `if`/`for`/함수를 설명한다.
- [ ] `01_declarations.cpp`, `02_control_flow.cpp`의 모든 결과를 먼저 예상한다.
- 읽기: [핵심 문법](02-core-syntax.md)

### 2주: 선언과 타입 읽기

- [ ] `const int*`, `int* const`, `const int&`를 구분한다.
- [ ] 반환 타입, 함수명, 매개변수, 후행 한정자를 분리한다.
- [ ] 오버로드와 기본 인자가 서로 다름을 설명한다.
- 읽기: [선언·signature](03-declarations-and-signatures.md)
- 실습: `03_pointers_refs_const.cpp`

### 3주: 객체, 수명, 자원

- [ ] 객체·값·타입·저장 기간·수명을 구분한다.
- [ ] lvalue/rvalue, 복사/이동, RAII, 스마트 포인터의 목적을 설명한다.
- [ ] dangling과 undefined behavior의 위험을 안다.
- 읽기: [타입·값·수명](04-types-values-lifetime.md)
- 실습: `04_classes_raii.cpp`, `10_lifetime_move.cpp`

### 4주: 클래스, 템플릿, 호출

- [ ] 생성자/소멸자, 접근 제어, `this`, 가상 함수를 읽는다.
- [ ] `template<class T>`, 타입 추론, specialization의 역할을 안다.
- [ ] 함수 포인터, 함수 객체, 람다, `std::invoke`를 “호출 가능 객체”로 묶어 본다.
- 읽기: [클래스·템플릿·호출](05-classes-templates-callables.md)
- 실습: `05_templates_callables.cpp`

### 5주: 표준 라이브러리 문법

- [ ] 컨테이너/반복자/알고리즘 관계를 이해한다.
- [ ] Requirements, Complexity, Exceptions, Notes를 빠뜨리지 않는다.
- [ ] iterator category와 invalidation을 확인하는 습관을 들인다.
- 읽기: [표준 라이브러리 어휘](06-standard-library-vocabulary.md)
- 실습: `06_iterators_algorithms.cpp`

### 6주: 동시성, `volatile`, `atomic`

- [ ] data race와 race condition을 구분한다.
- [ ] `volatile`은 스레드 동기화 수단이 아님을 설명한다.
- [ ] atomicity, ordering, lock-free를 구분한다.
- [ ] 예시 선언을 왼쪽부터 정확히 읽는다.
- 읽기: [동시성](07-concurrency-volatile-atomic.md)
- 실습: `08_atomic.cpp`

### 7주: C++17

- [ ] 언어 기능과 라이브러리 기능을 구분해 읽는다.
- [ ] `optional`, `variant`, `string_view`, `filesystem`의 소유권/실패 모델을 설명한다.
- [ ] 구조적 바인딩, `if constexpr`, fold expression을 실행해 본다.
- 읽기: [C++17](08-cpp17-guide.md)
- 실습: `07_cpp17.cpp`

### 8주: C++20

- [ ] concepts/ranges가 템플릿 오류와 반복자 코드를 어떻게 바꾸는지 본다.
- [ ] `span`의 비소유 수명 규칙을 설명한다.
- [ ] `jthread`, stop token, semaphore/barrier/latch의 역할을 구분한다.
- [ ] coroutine/module은 도구 체인 지원을 확인하고 별도 심화 주제로 둔다.
- 읽기: [C++20](09-cpp20-guide.md)
- 실습: `09_cpp20.cpp`

과정을 마친 뒤 [실전 선언 해독 문제](12-signature-workbook.md)를 답을 가린 채 풀어,
개별 API 선언으로 지식을 합칩니다.

## 매 API 페이지마다 쓰는 10문장 템플릿

1. 이 이름은 어느 헤더와 namespace에 있는가?
2. 함수/클래스/변수/별칭/개념 중 무엇인가?
3. 언제 도입·변경·폐기되었는가?
4. 선언을 토큰별로 어떻게 읽는가?
5. 입력 타입과 각 인자의 의미는 무엇인가?
6. 반환 타입과 반환값의 의미는 무엇인가?
7. 선행 조건과 유효한 타입 요구 조건은 무엇인가?
8. 시간·공간 복잡도는 무엇인가?
9. 예외, 무효화, 수명, 스레드 안전성 주의점은 무엇인가?
10. Example을 한 줄 바꾸면 무엇이 달라지는가?

답하지 못한 용어는 [용어·기호 색인](10-glossary.md)에 표시하고,
[모르는 항목 처리 절차](11-unknown-term-protocol.md)를 따릅니다.
