# 2026-07-23 Modern C++ 일일 학습

오늘은 C++20 `std::jthread`와 `std::stop_token`으로 **작업의 소유권과 협력적 취소 경계**를 설계한다. `std::thread`는 직접 `join()`해야 하지만 `std::jthread`는 객체가 파괴될 때 중지를 요청하고 합류한다. 다만 취소는 강제 종료가 아니다. 작업 함수가 토큰을 확인하고 스스로 빠져나와야 한다.

## 학습 목표

- `std::jthread`의 RAII 수명 관리와 `std::stop_token`의 비소유 관찰 역할을 구분한다.
- 애플리케이션 계층(`JobRunner`)과 작업 전략 계층(`IJob`)을 분리한다.
- `explicit`, 멤버 초기화 목록, 가상 함수, `unique_ptr`, 이동 의미론을 실제 코드에서 읽는다.
- lvalue·prvalue·xvalue, 참조 바인딩, 복사·이동·복사 생략과 객체 수명을 식에 연결한다.

## 빌드와 실행

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
ctest --test-dir build --output-on-failure
./build/jthread_architecture.exe
./build/jthread_problem.exe
```

`build/`는 생성 산출물이므로 커밋하지 않는다.

## 아키텍처 흐름

1. `CountingJob`은 실제 작업 규칙을 구현한다.
2. `IJob`은 실행 계층이 구체 타입을 몰라도 호출할 수 있는 포트다.
3. `JobRunner`는 `unique_ptr<IJob>`로 작업을 독점 소유하고 `jthread`로 실행 수명을 관리한다.
4. `request_stop()`은 요청만 기록하며, 작업의 반복문이 `stop_requested()`를 확인해 종료한다.

## 값 범주·수명·소유권

- 이름 있는 `runner`와 `job`은 lvalue다.
- `std::make_unique<CountingJob>(...)`의 반환값은 prvalue이며 `unique_ptr<IJob>` 멤버로 이동된다.
- `std::move(job)`은 이름 있는 lvalue를 xvalue로 바꿀 뿐 이동 자체는 아니다. 이동 생성자가 실행될 때 포인터 소유권이 옮겨진다.
- 람다가 받은 `std::stop_token token`은 값으로 복사되는 작은 관찰 핸들이다. 중지 상태를 소유하는 `jthread`보다 오래 독립적으로 작업을 살려 두지 않는다.
- `make_runner()`의 prvalue 반환은 C++17 이후 보장되는 복사 생략으로 호출자의 객체를 직접 초기화할 수 있다.
- `IJob&`는 기존 객체에 바인딩되는 lvalue 참조이며 소유하지 않는다. `unique_ptr`가 살아 있는 동안만 안전하다.

## 기계 실행 관점

반복문은 대략 중지 상태와 카운터를 로드하고, 비교 결과에 따라 조건 분기하며, 가상 `step()`은 런타임 타입을 통한 간접 호출이 될 수 있다. 출력과 `sleep_for`는 표준 라이브러리 함수 호출이다. 원자적인 중지 상태의 관찰에는 스레드 사이 동기화가 필요하다. 실제 명령과 호출 방식은 CPU, ABI, 컴파일러, 표준 라이브러리, 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정할 수 없다.

## 직접 해보기

1. `CountingJob`의 한계를 5에서 3으로 바꾸고 출력 횟수를 예측한다.
2. `problem.cpp`의 `run_until_stopped`를 보지 않고 다시 작성한다.
3. `request_stop()`을 제거했을 때 `jthread` 소멸자의 중지 요청이 어떻게 작동하는지 확인한다.
4. `unique_ptr`를 복사하려 해 보고 컴파일 오류가 소유권 규칙을 어떻게 증명하는지 설명한다.

이제 `main.cpp`, `problem.cpp`, `CHECKPOINT.md` 순서로 진행한다.
