# 초보자 검증 체크포인트

## 1단계: 읽고 말하기

- [ ] `#include <source_location>`이 제공하는 타입과 `current()`의 역할을 설명할 수 있다.
- [ ] `struct`는 기본이 `public`, `class`는 기본이 `private`라는 차이를 말할 수 있다.
- [ ] 생성자는 반환형이 없고, 멤버 초기화 목록이 본문 전에 멤버를 초기화한다는 것을 말할 수 있다.
- [ ] `explicit ConsoleDiagnosticSink(std::string)`이 암시적 변환을 막고 `ConsoleDiagnosticSink{"app"}` 직접 초기화는 허용함을 설명할 수 있다.

## 2단계: 손으로 고치기

- [ ] `main.cpp`의 주문 수량을 0과 양수로 각각 실행해 조건 분기를 확인했다.
- [ ] `using DiagnosticPtr = std::unique_ptr<DiagnosticSink>;`의 템플릿 인자와 소유권 의미를 설명했다.
- [ ] `problem.cpp`의 TODO를 구현하고 빌드·테스트했다.

## 3단계: 이해 증명

1. `const std::string&`를 값 매개변수로 바꾸면 어떤 복사가 생길 수 있는가?
2. `std::move(sink)` 뒤의 `sink`를 다시 사용하면 왜 안 되는가?
3. `PlaceResult{false, 0}`은 lvalue/prvalue/xvalue 중 무엇이며, 왜 반환 시 복사 생략이 가능한가?
4. `sink_->write(...)`가 가상 호출일 때, 실제 기계 명령을 하나로 단정할 수 없는 이유는 무엇인가?

모두 자신의 말로 답하고, 테스트가 두 개 모두 통과하면 완료다.
