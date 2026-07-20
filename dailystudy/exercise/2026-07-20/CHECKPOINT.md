# 초보자 이해 점검: `std::variant` 상태 모델

먼저 README와 코드를 닫고 답하세요. 총 8점이며 문항당 1점입니다.

## 문제

1. `std::variant<A, B>`에는 A와 B가 동시에 저장되는가?
2. `using Result = std::variant<Success, Failure>;`에서 `using`의 역할은 무엇인가?
3. 현재 값이 `Failure`인지 예외 없이 확인하는 함수 한 가지를 쓰세요.
4. `std::get<Success>(result)`를 호출했는데 실제 값이 `Failure`라면 안전한가?
5. `std::get_if<Success>(&result)`에서 Success가 아니면 무엇을 반환하는가?
6. `std::visit`는 어떤 값을 방문자에게 전달하는가?
7. `if constexpr`와 일반 `if`의 중요한 차이는 무엇인가?
8. 상태별 구조체를 나누는 것이 여러 `bool` 필드를 두는 것보다 좋은 이유를 한 문장으로 쓰세요.

## 실기 검증

다음 코드를 작성한 뒤 빌드와 CTest를 다시 실행하세요.

- `main.cpp`에 `Cancelled { std::string reason; }`를 추가합니다.
- `OrderState`의 후보에 `Cancelled`를 넣습니다.
- `describe(OrderState{Cancelled{"재고 없음"}})`가 `"취소: 재고 없음"`인지 `assert`로 확인합니다.

컴파일 오류가 생기면 오류가 가리키는 타입과 `std::visit`의 각 분기를 먼저 확인하세요.

## 정답과 자기 설명

1. 아니다. 후보 중 정확히 하나만 저장된다.
2. 긴 variant 타입에 `Result`라는 별명을 붙인다.
3. `std::holds_alternative<Failure>(result)` 또는 `std::get_if<Failure>(&result)`.
4. 안전하지 않다. 실제 타입이 다르면 `std::bad_variant_access` 예외가 발생한다.
5. `nullptr`.
6. variant에 현재 저장된 실제 값을 전달한다.
7. `if constexpr`는 컴파일 시 조건을 결정하여 선택되지 않은 가지를 인스턴스화하지 않는다.
8. 예: 각 상태에 유효한 데이터만 둘 수 있어 모순된 필드 조합을 만들기 어렵다.

## 통과 기준

- 6점 이상: 기본 개념 통과
- 실기까지 성공: 새 상태를 모델과 동작에 연결할 수 있음
- 5점 이하: README의 1~4절을 다시 읽고 `problem.cpp`를 손으로 한 번 다시 작성하기

마지막으로 다음 문장을 자신의 예제로 완성해 말해 보세요.

> 내 variant의 후보는 ___이고, 현재 값은 ___이며, 안전하게 확인하려면 ___를 사용한다.
