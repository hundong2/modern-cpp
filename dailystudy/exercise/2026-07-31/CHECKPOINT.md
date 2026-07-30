# 초보자 이해 검증

체크만 하지 말고 각 항목의 근거가 되는 코드 줄을 가리키며 소리 내어 답합니다.

## 1단계 — 기초 문법 읽기

- [ ] `<iostream>`, `<memory>`, `<numbers>`, `<utility>`가 각각 무엇을 제공하는지 말한다.
- [ ] `int`, `bool`, `double`이 저장하는 값의 종류를 구분한다.
- [ ] `Scalar r{radius.value()};`가 중괄호 초기화인 이유와 축소 변환 방지 장점을 말한다.
- [ ] `AreaResult calculate(const Radius& radius) const`의 반환형, 함수 이름, 매개변수, 마지막 `const`를 짚는다.
- [ ] 포인터 `pointer`, 참조 `reference`, `unique_ptr`의 비소유/비소유/단독 소유 차이를 말한다.
- [ ] `if`와 `for`, `!`, `>`, `&&`, `*`, `->`, `<<`, `?:`, `++`의 역할을 실제 식에서 설명한다.

## 2단계 — 클래스와 Modern C++

- [ ] `struct`의 기본 접근은 public, `class`의 기본 접근은 private임을 말한다.
- [ ] `public:`과 `private:`가 외부 접근 가능 범위를 어떻게 나누는지 설명한다.
- [ ] 생성자에 반환형이 없고, 생성자 매개변수와 멤버 초기화 목록의 역할이 다름을 말한다.
- [ ] `explicit`이 `AreaService service = std::move(policy);` 같은 암시적 변환을 막고, 직접 초기화 `AreaService service{...};`는 허용함을 말한다.
- [ ] `using Scalar = double;`이 새 타입이 아닌 별칭임을 설명한다.
- [ ] `pi_v<Scalar>`와 `unique_ptr<AreaPolicy>`에서 꺾쇠 안의 템플릿 인자가 무엇을 고르는지 말한다.
- [ ] `std::numbers`가 매크로 상수보다 타입 안전한 이유를 말한다.

## 3단계 — 아키텍처, 값 범주와 수명

- [ ] `AreaPolicy`가 포트, `CircleAreaPolicy`가 어댑터, `AreaService`가 유스케이스인 이유를 말한다.
- [ ] 이름 있는 `policy`는 lvalue, `make_unique` 결과는 prvalue, `std::move(policy)` 결과는 xvalue임을 식에서 찾는다.
- [ ] `std::move` 자체가 이동하지 않고 `unique_ptr` 이동 생성자가 소유권을 옮긴다는 점을 설명한다.
- [ ] `const Radius&`가 `Radius{2.0}`에 바인딩될 때 임시 객체 수명이 언제까지인지 말한다.
- [ ] `AreaResult{...}` 반환에서 복사 생략이 어떻게 불필요한 복사·이동을 없앨 수 있는지 말한다.
- [ ] 가상 간접 호출과 로드·저장·비교·조건 분기의 실제 구현은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라짐을 말한다.

## 4단계 — 직접 증명

- [ ] 반지름을 3.0으로 바꾸고 예상 넓이를 적은 뒤 빌드·실행해 비교한다.
- [ ] `problem.cpp`의 TODO 1을 수행하고 반지름 0의 결과를 확인한다.
- [ ] TODO 2를 수행하고 출력 횟수가 세 번임을 확인한다.
- [ ] `CircleAreaPolicy` 대신 항상 42를 반환하는 테스트 정책을 만들어 서비스에 주입한다.
- [ ] 변경을 원복한 뒤 두 실행 파일과 CTest가 모두 성공하는지 확인한다.

합격 기준: 1~3단계를 근거 코드와 함께 설명하고, 4단계 중 최소 3개를 직접 수정·빌드·실행한 뒤 원복합니다.
