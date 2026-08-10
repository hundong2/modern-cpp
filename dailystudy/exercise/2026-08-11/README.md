# 2026-08-11 Modern C++ 학습

## 오늘의 주제

`std::optional<T>`로 “값이 없을 수 있음”을 타입에 표현한다. `main.cpp`는 저장소 인터페이스와 구현을 분리하고, `class`의 private 상태를 public 멤버 함수로 보호한다. 조회 결과를 포인터나 특별한 정수 대신 `std::optional<Record>`로 반환하여 소유권과 수명을 분명히 한다.

`records`는 lvalue이므로 복사되고, `std::move(records)`는 xvalue로 바뀌어 이동 생성의 후보가 된다. `Record{...}`와 함수가 돌려주는 `std::optional<Record>{...}`은 prvalue이며 결과 객체에 직접 생성될 수 있다. 참조는 객체를 소유하지 않으므로 원본보다 오래 살아서는 안 된다. 실제 로드·저장·비교·조건 분기·함수 호출과 가상 간접 호출의 명령 형태는 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.

## 파일과 직접 해보기

- `main.cpp`: 추상 저장소, `override`, 가상 소멸자, `explicit`, `std::optional`, 복사·이동과 수명.
- `problem.cpp`: `const T&` 매개변수와 `std::optional<std::reference_wrapper<const T>>` 비소유 조회.
- `RecordStore copied = records;`가 왜 허용되지 않는지 확인하고 `RecordStore copied{records};`와 `RecordStore moved{std::move(records)};`의 차이를 설명한다.
- 초보자 검증: 조회 실패가 `0`이나 빈 문자열이 아니라 `std::nullopt`로 표현되는 이유와, 반환된 값이 저장소와 독립적인 수명을 갖는 이유를 말로 설명한다.

## 오늘의 ICPC 문제

- 문제: BOJ 11286 **절댓값 힙**
- 출처: [Baekjoon Online Judge](https://www.acmicpc.net/problem/11286)
- 핵심 알고리즘: [우선순위 큐와 사용자 정의 비교](../algorithm/priority-queue-custom-comparator.md)
- 복잡도: 삽입·삭제 한 번당 O(log N), 최솟값 확인 O(1), 공간 O(N)
- 검증: 공식 예제, 빈 큐, 절댓값 동률에서 작은 원본 값 우선인 경우를 CTest로 검사한다.

## 빌드와 실행

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
