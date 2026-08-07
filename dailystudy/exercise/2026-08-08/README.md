# 2026-08-08 Modern C++ 학습 자료

## 오늘의 주제: `std::unique_ptr`로 소유권과 의존성 표현하기

`ReportService`는 출력 방법을 직접 만들지 않고 `std::unique_ptr<Sink>`를 생성자로 받는다. 이 구조는 의존성 역전과 단일 소유권을 코드의 타입으로 표현한다. 기반 클래스의 가상 소멸자는 파생 객체를 기반 포인터로 안전하게 파괴하며, `std::make_unique`는 자원 획득과 객체 생성을 한 식으로 묶는다.

- `struct`의 기본 접근은 `public`, `class`의 기본 접근은 `private`이다.
- `explicit` 생성자는 원치 않는 암시적 변환을 막고 `ReportService service{std::move(sink)};` 같은 직접 초기화를 요구한다.
- 이름 있는 `sink`는 lvalue이고 `std::move(sink)`는 xvalue다. 이동 후 원본 포인터는 비며 소유권은 서비스로 넘어간다.
- `std::make_unique<ConsoleSink>()`는 prvalue를 반환하고 목적 객체를 직접 초기화한다. 객체 수명은 소유한 `unique_ptr`의 수명에 묶인다.
- 가상 호출의 실제 간접 호출 방식과 로드·저장·비교·분기는 CPU, ABI, 컴파일러, 최적화 옵션에 따라 달라진다.

## 오늘의 ICPC/OJ 문제

- 문제 ID/제목: BOJ 1654, **랜선 자르기**
- 출처: [Baekjoon Online Judge](https://www.acmicpc.net/problem/1654)
- 핵심 알고리즘: 답에 대한 이분 탐색(매개변수 탐색)
- 공용 문서: [`../algorithm/binary-search-on-answer.md`](../algorithm/binary-search-on-answer.md)
- 복잡도: 시간 `O(K log M)`, 공간 `O(K)` (`M`은 가장 긴 랜선 길이)

길이 `L`로 필요한 개수 이상을 만들 수 있는지를 결정 문제로 바꾼다. 가능 구간은 작은 길이 쪽에 연속되므로, 가능한 가장 큰 `L`을 이분 탐색한다. 개수 합은 범위를 넘을 수 있어 `long long`을 사용한다.

## 빌드와 검증

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

CTest는 두 Modern C++ 예제의 정상 종료와 BOJ 공식 예제, 한 랜선, 경계 길이를 검증한다.

## 직접 해보기

1. `MemorySink`를 추가하고 저장된 문자열을 확인한다.
2. `std::move`를 제거했을 때 `unique_ptr` 복사가 금지되어 컴파일되지 않는 이유를 설명한다.
3. 이분 탐색의 `high`를 `max_length` 대신 `max_length + 1`로 둔다면 불변식과 갱신식을 다시 작성한다.
4. `made`가 `N` 이상이 되는 즉시 반복을 끝내도 정답이 유지되는 이유를 증명한다.
