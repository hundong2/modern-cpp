# 2026-08-07 Modern C++ 학습 자료

## 오늘의 주제: `std::variant`로 유효한 상태만 표현하기

`JobState`와 `Connection`은 여러 상태 중 하나만 존재하도록 모델링한다. 정수 상태 코드와 서로 맞지 않는 필드가 동시에 존재하는 오류를 타입으로 줄인다. `std::visit`은 활성 타입별 동작을 한곳에 모으고, `std::get_if`는 포인터의 null 여부로 안전하게 상태를 확인한다.

- `struct`의 기본 접근은 `public`, `class`는 `private`이다.
- `explicit` 생성자는 의도하지 않은 암시적 변환을 막고 `Job job{2};` 직접 초기화는 허용한다.
- `state_ = Running{0}`에서 오른쪽은 prvalue이며 기존 활성 객체의 수명이 끝나고 새 객체가 시작된다.
- `job`은 lvalue, `std::move(name)`은 xvalue다. 이동은 소유 자원을 재사용할 기회를 주며, 반환 prvalue는 목적지에 직접 생성되어 복사 생략될 수 있다.
- `std::visit`의 분기와 함수 호출, 로드·저장·비교의 실제 기계 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.

## 오늘의 ICPC/OJ 문제

- 문제: BOJ 2178, **미로 탐색**
- 출처: [Baekjoon Online Judge](https://www.acmicpc.net/problem/2178)
- 핵심 알고리즘: 무가중 격자 그래프의 너비 우선 탐색(BFS)
- 공용 문서: [`../algorithm/breadth-first-search-unweighted-grid.md`](../algorithm/breadth-first-search-unweighted-grid.md)
- 복잡도: 시간 `O(NM)`, 공간 `O(NM)`

FIFO 큐가 거리가 작은 칸부터 처리한다. 따라서 어떤 칸을 처음 발견했을 때 기록한 `이전 거리 + 1`이 최단거리다. 거리 배열의 0을 미방문 표식으로 함께 사용한다.

## 빌드와 검증

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

CTest는 두 학습 프로그램의 정상 종료와 ICPC 공식 예제, 단일 칸 보강 사례, 열린 2×2 미로를 검증한다.

## 직접 해보기

1. `Failed` 상태를 만드는 `fail(int)` 멤버 함수를 추가하고 출력이 바뀌는지 확인한다.
2. `std::get_if`가 반환하는 포인터가 null인 경우를 일부러 만들고 역참조 전에 검사해야 하는 이유를 설명한다.
3. `icpc_problem.cpp`에서 거리를 별도 배열 대신 미로 문자에 덮어쓰면 어떤 타입/범위 문제가 생기는지 적는다.
4. 손으로 3×3 미로의 큐와 거리표를 매 단계 추적하여 최초 발견이 최단거리임을 확인한다.
