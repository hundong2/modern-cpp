# 2026-07-14 초보자 이해 점검

이 문서는 "읽었다"가 아니라 "설명하고 고칠 수 있다"를 확인하기 위한 검증 단계다. C++ 기초가 약한 개발자도 아래 항목을 순서대로 통과하면 오늘 자료를 제대로 학습한 것으로 본다.

## 1단계: 말로 설명하기

아래 질문에 각각 1~3문장으로 답한다.

- `std::string_view`는 왜 빠를 수 있고, 왜 위험할 수 있는가?
- `std::span<const T>`를 함수 인자로 받으면 어떤 장점이 있는가?
- `std::expected<T, E>`는 예외 처리와 무엇이 다른가?
- `ProcessingSession`은 함수가 중간에 실패해도 왜 세션 수를 원래대로 돌리는가?
- `AlertPublisher` concept가 없으면 어떤 실수를 더 늦게 발견할 수 있는가?

통과 기준: 다섯 질문 중 네 개 이상을 코드의 실제 줄과 연결해서 설명한다.

## 2단계: 코드 추적하기

`main.cpp`의 입력이 다음과 같다고 가정한다.

```cpp
const std::vector<std::string_view> batch = {
    "201:73",
    "202:88",
    "203:91",
    "204:64",
};
```

다음을 손으로 계산한다.

- 파싱에 성공한 줄 수
- 알림이 발생하는 센서 ID
- `active_sessions`가 마지막에 0이 되는 이유

통과 기준: 실행 결과와 손 계산 결과가 일치한다.

## 3단계: 직접 바꿔보기

아래 변경 과제 중 2개 이상을 수행한 뒤 다시 빌드한다.

- `ReadingService service{publisher, 80, active_sessions};`의 80을 90으로 바꾸고 알림 개수를 확인한다.
- `batch`에 `"bad-input"`을 추가하고 오류 메시지가 몇 번째 줄을 가리키는지 확인한다.
- `MemoryAlertPublisher`에 `clear()` 함수를 추가하고 테스트에서 사용한다.
- `problem.cpp`에서 threshold를 20으로 바꾸고 결과가 `no matching window`인지 확인한다.
- `first_window_at_least()`와 반대로 마지막으로 조건을 만족하는 창을 찾는 `last_window_at_least()`를 작성한다.

통과 기준: 수정 후 `cmake --build build`가 성공하고 두 실행 파일의 테스트가 통과한다.

## 4단계: 초급자 기준 최종 점검

아래 항목을 모두 만족하면 오늘 학습을 완료한 것으로 본다.

- `std::string_view`와 `std::string`의 차이를 설명할 수 있다.
- `std::span`이 원본 배열을 소유하지 않는다는 점을 알고 있다.
- `std::optional`은 값이 없을 수 있음을 표현하고, `std::expected`는 실패 이유까지 담는다는 차이를 설명할 수 있다.
- RAII가 생성자와 소멸자를 이용한다는 점을 말할 수 있다.
- 템플릿 concept는 "이 타입은 이런 함수를 가져야 한다"는 요구사항임을 설명할 수 있다.

## 검증 로그

작성 시점 검증:

- `exercise_architecture`는 파싱, 오류 반환, 알림 발행, RAII 세션 복구 테스트를 포함한다.
- `exercise_window`는 정상 창, 조건 미달, 첫 창 일치, 창 크기 초과 입력을 테스트한다.
- 두 파일은 CMake로 독립 실행 파일을 생성하도록 구성했다.
