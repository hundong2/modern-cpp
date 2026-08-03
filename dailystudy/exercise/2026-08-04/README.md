# 2026-08-04 — C++20 정책 객체와 의존성 역전

오늘은 `std::unique_ptr`로 정책 객체의 소유권을 옮기고, 추상 인터페이스에 의존하는 애플리케이션 서비스를 만든다. `main.cpp`는 `DiscountPolicy` 포트와 `FixedRatePolicy` 어댑터를 조립하고, `problem.cpp`는 같은 설계를 직접 완성하는 작은 연습이다.

## 코드 가까이에서 확인할 문법

- `struct`의 기본 접근은 `public`, `class`의 기본 접근은 `private`이다.
- 생성자에는 반환형이 없고, `explicit`은 단일 인자 생성자의 원치 않는 암시적 변환을 막는다.
- `std::unique_ptr`는 단독 소유권을 나타낸다. `std::move` 결과는 xvalue이며 이동 생성/대입의 후보가 된다.
- 이름 있는 객체는 lvalue, `Money{10000}` 같은 임시 객체는 prvalue다. 반환값은 복사 생략으로 목적지에 직접 생성될 수 있다.
- 가상 함수 호출은 런타임 간접 호출일 수 있다. 실제 로드·저장·비교·분기·호출 명령은 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.

## 오늘의 ICPC 문제

- 문제: BOJ 1197 **최소 스패닝 트리**
- 출처: [Baekjoon Online Judge](https://www.acmicpc.net/problem/1197)
- 핵심 알고리즘: 크루스칼 + 서로소 집합(DSU), 공용 문서 [`../algorithm/kruskal-minimum-spanning-tree.md`](../algorithm/kruskal-minimum-spanning-tree.md)
- 복잡도: 간선 정렬이 지배하므로 시간 `O(E log E)`, 간선과 DSU 저장 공간 `O(V+E)`
- 검증: 공식 예제를 CTest로 실행해 출력 `3`을 비교하며, 직접 실행 시 음수 가중치·중복 간선·이미 연결된 간선 건너뛰기도 점검한다.

## 빌드와 검증

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 직접 해보기

1. `FixedRatePolicy` 대신 정액 할인 정책을 구현해 생성자 주입으로 교체한다.
2. `problem.cpp`의 TODO를 먼저 가리고 직접 작성한 뒤 출력과 종료 코드를 확인한다.
3. ICPC 코드에서 선택한 간선 수가 `V-1`이 되는 이유를 불변식으로 설명한다.
