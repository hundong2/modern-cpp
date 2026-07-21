# 초보자 검증 체크포인트

코드를 보지 않고 말하거나 직접 수정할 수 있어야 통과다.

## 1단계: 문법 읽기

- 각 `#include`가 제공하는 이름을 하나씩 말한다.
- `int`, `double`, `std::size_t`의 용도를 설명한다.
- `ScoreStore store{make_store()};`에서 타입, 변수 이름, 중괄호 초기화를 찾는다.
- `Summary summarize(ScoreView values) const`에서 반환형, 함수 이름, 매개변수, 끝의 `const`를 설명한다.
- `struct`의 기본 접근은 `public`, `class`의 기본 접근은 `private`임을 말한다.
- 생성자에는 반환형이 없고, 생성자 매개변수와 멤버 초기화 목록이 각각 무엇인지 찾는다.
- `explicit` 때문에 막히는 암시적 변환과 `ScoreStore{vector}` 직접 초기화가 옳은 이유를 말한다.
- `using ScoreView = ...`와 `ScoreService<PositiveOnly>`의 `<PositiveOnly>` 역할을 설명한다.

## 2단계: 실행 추적

- 반복문 각 회차의 `value`, `total`, `count`를 표로 적는다.
- `policy_.accept(value) && value <= 100`의 단락 평가와 조건 분기를 설명한다.
- `view.data()`, `*pointer`, `view.front()`의 값과 포인터/참조 차이를 설명한다.
- 출력 전에 `result.average`가 어떤 표준 라이브러리 함수와 연산자를 거쳐 계산되는지 말한다.

## 3단계: 값 범주와 수명

- `store`가 lvalue, `store.scores()`가 prvalue인 이유를 말한다.
- `std::move(initial)`이 xvalue이며 그 자체로 이동을 수행하지 않는 이유를 말한다.
- `const int& first`가 무엇에 바인딩되고 언제까지 유효한지 말한다.
- `ScoreStore`가 파괴된 뒤 `view`를 읽으면 안 되는 이유를 소유권과 객체 수명으로 설명한다.
- `make_store()` 반환에서 복사 생략이 가능한 지점을 찾고, 복사와 이동의 차이를 말한다.

## 4단계: 직접 변경

- `PositiveOnly`가 짝수만 받도록 `%` 연산자를 사용해 수정하고 테스트한다.
- `problem.cpp`에 `count_non_negative` 함수를 추가한다.
- 빈 벡터 테스트를 추가해 0 나눗셈과 빈 포인터 역참조가 없는지 확인한다.
- `ScoreService`를 가상 함수 기반 정책으로 바꾼다면 호출과 소유권 설계가 어떻게 달라질지 한 문단으로 쓴다.

## 5단계: 기계 관점 설명

- 순회 중 로드·저장·비교·조건 분기·함수 호출이 어디에서 일어날 수 있는지 코드 줄과 연결한다.
- 가상 인터페이스라면 간접 호출 가능성이 있음을 설명한다.
- 실제 명령은 CPU, ABI, 컴파일러, 최적화 옵션에 따라 달라져 특정 명령으로 단정할 수 없음을 덧붙인다.

모든 항목을 설명하고 두 실행 파일 및 CTest가 통과하면 오늘 학습을 완료한다.
