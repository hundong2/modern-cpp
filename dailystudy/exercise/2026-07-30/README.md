# 2026-07-30 — `std::chrono` 강한 시간 타입과 테스트 가능한 시계

오늘은 C++20의 `std::chrono`로 “숫자 3”이 3초인지 3밀리초인지 타입에 담고, 현재 시간을 직접 읽는 대신 `Clock` 인터페이스를 주입합니다. 핵심 아키텍처는 다음과 같습니다.

`RetryPolicy`(규칙) → `RetryService`(유스케이스) → `Clock`(포트) ← `ManualClock`(테스트 어댑터)

유스케이스가 운영체제 시계에 직접 의존하지 않으므로 테스트는 기다리지 않고 시간을 앞으로 보낼 수 있습니다. `std::chrono::duration<Rep, Period>`에서 `Rep`은 저장 숫자 타입, `Period`는 한 틱의 단위인 템플릿 인자입니다. 서로 다른 시간 단위는 `duration_cast`로 의도를 드러내어 변환합니다.

## 실행

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

- `main.cpp`: 완성 예제입니다. 포트/어댑터, 가상 호출, `unique_ptr` 소유권, 강한 시간 타입을 함께 봅니다.
- `problem.cpp`: TODO를 직접 완성하는 작은 연습입니다. 현재 정답 상태로도 컴파일되며, 주석 지시에 따라 일부 식을 바꾸어 다시 검증합니다.
- `CHECKPOINT.md`: 말로 설명하고 코드를 바꾸며 이해를 증명하는 초보자 검증표입니다.

## 값 범주·수명·복사와 이동

이름 있는 `clock`과 `service`는 lvalue입니다. `std::make_unique<ManualClock>(...)`의 결과는 prvalue이고, 생성한 객체를 가리키는 소유권 포인터가 만들어집니다. `std::move(clock)`은 lvalue를 이동 가능한 xvalue로 표현할 뿐 그 자체가 객체를 옮기지는 않습니다. 이어지는 `unique_ptr` 이동 생성이 소유권을 옮기며 원본은 비게 됩니다.

`RetryPolicy{...}`와 함수의 `Decision{...}` 반환값은 prvalue입니다. C++17 이후에는 반환 목적지에 직접 구성되는 복사 생략이 보장되는 경우가 있어 불필요한 임시 복사·이동이 없습니다. `const Clock&`는 기존 객체에 바인딩되는 비소유 참조이므로, 참조 대상이 사용하는 동안 살아 있어야 합니다. 이 예제에서는 `RetryService`가 `unique_ptr<Clock>`을 소유해 그 조건을 만족시킵니다.

## 기계 실행 관점

개념적으로 멤버를 읽는 로드, 마감 시각과 현재 시각의 비교, `if` 조건 분기, 함수 호출, `Clock::now()`의 가상 간접 호출이 생길 수 있습니다. 시간 값을 멤버에 기록할 때는 저장도 일어날 수 있습니다. 다만 실제 명령, 레지스터, 인라이닝 및 가상 호출 제거 여부는 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않습니다.

## 직접 해보기

1. `main.cpp`의 재시도 간격을 `3s`에서 `1500ms`로 바꾸고 기대 결과도 수정합니다.
2. `ManualClock::advance`에 음수 시간이 들어오면 거부하도록 조건문을 추가합니다.
3. `problem.cpp`의 `expired`를 `<`로 바꾸었을 때 마감 시각과 정확히 같은 시간이 왜 실패하는지 설명합니다.
4. `Clock`을 소유하지 않고 `const Clock&`만 저장하는 설계로 바꾼다면 누가 수명을 책임져야 하는지 적습니다.
