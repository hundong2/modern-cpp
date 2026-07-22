# 초보자 검증 체크포인트

코드를 보지 않고 말하거나 직접 수정할 수 있어야 통과다.

## 1단계: 기초 문법

- 여섯 `#include`가 각각 어떤 이름을 제공하는지 설명한다.
- `int`, `bool`, 변수, 중괄호 초기화를 코드에서 찾아 설명한다.
- `step(int current) const`의 반환형·매개변수·함수 호출·`const` 의미를 말한다.
- 포인터를 소유하는 `unique_ptr`와 기존 객체에 바인딩되는 `IJob&` 참조의 차이를 말한다.
- `while`, `if`, `?:`, `<`, `&&`, `!`, `++`, `<<`의 실행 위치를 찾는다.

## 2단계: 객체 문법

- `struct`의 기본 접근은 public, `class`의 기본 접근은 private임을 설명한다.
- `public`/`private`, 멤버 변수 `limit_`, 생성자 매개변수 `limit`를 찾는다.
- 생성자에는 반환형이 없으며 `: limit_{limit}`가 멤버 초기화 목록임을 설명한다.
- `explicit`이 막는 `CountingJob job = 5;`와 올바른 `CountingJob job{5};`를 비교한다.
- `using JobOwner`의 별칭 대상과 `make_unique<CountingJob>`의 템플릿 인자를 설명한다.

## 3단계: 값 범주와 수명

- `runner`가 lvalue, `make_runner()`가 prvalue, `std::move(job)`이 xvalue인 이유를 말한다.
- xvalue 변환과 실제 `unique_ptr` 이동 생성이 서로 다른 단계임을 설명한다.
- `const IJob& job`의 참조 바인딩 대상과 유효 수명을 말한다.
- 람다의 `&result`가 worker보다 먼저 파괴되면 왜 위험한지 설명한다.
- `make_runner()` 반환에서 복사 생략이 가능한 지점과 복사·이동 차이를 말한다.

## 4단계: 아키텍처와 실행

- `IJob`, `CountingJob`, `JobRunner`의 역할을 한 문장씩 말한다.
- `request_stop()`이 강제 종료가 아닌 이유를 설명한다.
- 중지 상태 로드·비교·조건 분기, 멤버 저장, 함수 호출, 가상 간접 호출 위치를 짚는다.
- 실제 명령은 CPU·ABI·컴파일러·표준 라이브러리·최적화 옵션에 따라 달라진다고 설명한다.

## 5단계: 직접 증명

- `CountingJob{3}`으로 직접 초기화하고 결과를 예측한다.
- `problem.cpp`에 중지 요청을 먼저 한 토큰을 전달하는 테스트를 추가한다.
- `JobRunner` 복사를 시도해 독점 소유권 때문에 실패하는 컴파일 오류를 확인한 뒤 되돌린다.
- 두 실행 파일과 CTest가 모두 통과하면 오늘 학습을 완료한다.
