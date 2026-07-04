# System Architecture and Design Using Modern C++ 학습 노트

출처: <https://cppcon.org/class-2026-system-architecture-and-design-using-modern-cpp/>

이 폴더는 CppCon 2026의 `System Architecture and Design Using Modern C++`
강의 소개 페이지를 바탕으로 정리한 학습 자료입니다. 원 강의는 중급 이상 C++
개발자를 대상으로 시스템 아키텍처와 설계 의사결정을 다룹니다. 여기서는 같은
주제를 처음 접하는 학습자가 따라갈 수 있도록 기초 문법, 메모리 관점, 작은 예제
중심으로 재구성했습니다.

## 사이트 내용 분석

강의 페이지에서 강조하는 핵심은 다음과 같습니다.

- Modern C++ 시스템의 실패 원인과 반복되는 메타 패턴
- 아키텍처와 설계의 차이
- 비즈니스 제약과 기술 제약의 균형
- 타입 시스템, 대칭성, 불변식을 이용한 설계 단순화
- 데이터 흐름과 객체 상태 수명주기
- 어댑터를 통한 임피던스 불일치 해소
- 결합도, 복잡도, 부수 효과 관리
- 커스터마이징 포인트와 ADL
- 동시성, 병렬성, 스레딩 모델
- 넓은 계약과 좁은 계약
- 재사용 가능한 프레임워크와 애플리케이션 전용 코드의 경계

## 폴더 구성

```text
Course/
  README.md
  CMakeLists.txt
  01_architecture_vs_design/
    README.md
    architecture_vs_design.cpp
  02_types_contracts_invariants/
    README.md
    strong_types_and_contracts.cpp
  03_lifecycle_memory/
    README.md
    object_lifecycle_raii.cpp
  04_adapters_boundaries/
    README.md
    adapter_boundary.cpp
  05_concurrency_constraints/
    README.md
    thread_safe_queue.cpp
```

## 빌드 방법

```powershell
cmake -S Course -B Course/build
cmake --build Course/build
```

실행 파일은 `Course/build` 아래에 생성됩니다.

## 학습 순서

1. `01_architecture_vs_design`: 아키텍처와 설계의 차이를 작은 주문 처리 예제로 이해합니다.
2. `02_types_contracts_invariants`: 타입, 계약, 불변식으로 잘못된 상태를 줄이는 방법을 봅니다.
3. `03_lifecycle_memory`: 객체 수명주기, 스택/힙, RAII, 스마트 포인터를 설명합니다.
4. `04_adapters_boundaries`: 외부 API나 레거시 코드와 내부 도메인 모델을 분리하는 어댑터를 봅니다.
5. `05_concurrency_constraints`: 스레드, 공유 상태, 뮤텍스, 조건 변수로 처리량 제약을 다룹니다.

