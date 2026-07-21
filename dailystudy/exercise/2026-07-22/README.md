# 2026-07-22 Modern C++ 일일 연습

오늘은 C++20 `std::span`으로 **소유권과 읽기 경계**를 분리한다. 저장소는 `std::vector<int>`를 소유하고, 서비스는 복사 없이 `std::span<const int>`만 빌린다. 이 구분은 “누가 메모리를 살려 두는가?”를 API에 드러내는 작은 아키텍처 연습이다.

## 학습 목표

- `std::vector`(소유)와 `std::span`(비소유 뷰)의 역할을 구별한다.
- `const`, 포인터, 참조, 객체 수명과 값 범주를 실제 식에서 판별한다.
- C++20 `concept`로 템플릿 인자에 필요한 연산을 문서화한다.
- 저장소, 계산 정책, 서비스 계층을 작게 분리한다.

## 빌드와 실행

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
ctest --test-dir build --output-on-failure
./build/span_architecture.exe
./build/span_problem.exe
```

`build/`는 생성 산출물이므로 커밋하지 않는다.

## 아키텍처 흐름

1. `ScoreStore`가 점수 벡터의 수명과 소유권을 가진다.
2. `scores()`는 내부 원소를 가리키는 `std::span<const int>`를 반환한다.
3. `ScoreService<Policy>`가 뷰와 정책을 받아 평균을 계산한다.
4. 호출자는 결과 값만 받고 저장 방식에는 의존하지 않는다.

`span` 자체를 복사해도 원소는 복사되지 않는다. 보통 시작 주소와 길이 같은 작은 정보만 복사된다. 따라서 원본 벡터가 파괴되거나 재할당되면 뷰가 무효가 될 수 있다. **뷰보다 소유자가 오래 살아야 한다.**

## 값 범주·바인딩·수명 지도

- `store`는 이름 있는 객체이므로 lvalue다.
- `store.scores()`의 반환값은 임시 `span`인 prvalue다. `const auto view = ...`에서 새 `span` 객체로 초기화되며, 원소를 소유하지 않는다.
- `std::move(store)`는 `store`를 xvalue로 바꿀 뿐 즉시 이동시키지는 않는다. 이동 생성자 같은 소비 연산이 있어야 실제 이동이 일어난다.
- `const int& first = view.front()`는 lvalue 원소에 const lvalue 참조를 바인딩한다. 복사는 없다.
- 값으로 반환한 `ScoreStore`는 C++17 이후 보장되는 복사 생략 대상이 될 수 있다. 반면 `span`의 복사 생략 여부와 무관하게 원소 소유권은 생기지 않는다.

## 기계 실행 관점

`span` 순회는 개념적으로 주소에서 값을 로드하고, 합계를 저장하며, 끝과 비교하고 조건 분기한다. 정책 호출은 일반 함수 호출이지만 최적화 시 인라인될 수 있다. 가상 인터페이스를 쓴다면 객체의 동적 타입을 통해 간접 호출할 수 있으나 오늘의 템플릿 정책은 컴파일 시간 결합이다. 실제 명령과 호출 방식은 CPU, ABI, 컴파일러, 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 직접 해보기

1. `main.cpp`의 `PositiveOnly`를 `AllScores` 정책으로 바꿔 0 이하도 평균에 포함한다.
2. `problem.cpp`의 TODO를 먼저 가리고 직접 다시 작성한다.
3. `ScoreStore::add` 뒤에 벡터가 재할당될 수 있는데, 그 전에 얻은 `span`을 계속 쓰면 왜 위험한지 설명한다.
4. `std::span<const int>`를 `const std::vector<int>&`로 바꿀 때 결합도가 어떻게 달라지는지 비교한다.

이제 `main.cpp`를 읽고 실행한 뒤 `problem.cpp`, `CHECKPOINT.md` 순으로 진행한다.
