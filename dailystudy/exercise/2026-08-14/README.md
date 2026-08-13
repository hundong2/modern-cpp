# 2026-08-14 Modern C++ 학습 자료

## 오늘의 목표
- Strategy 인터페이스와 `std::unique_ptr`로 정책, 소유권, 수명을 분리한다.
- lvalue·prvalue·xvalue, 참조 바인딩, 이동, 복사 생략을 실제 식과 연결한다.

## 파일
- `main.cpp`: 인터페이스와 단일 소유권 의존성 주입
- `problem.cpp`: 이동 전용 서비스 연습
- `icpc_problem.cpp`: [BOJ 1916 최소비용 구하기](https://www.acmicpc.net/problem/1916) 제출 풀이
- `CHECKPOINT.md`: 이해를 증명하는 점검표

## 오늘의 ICPC 문제
- ID·제목·출처: [BOJ 1916 최소비용 구하기](https://www.acmicpc.net/problem/1916), Baekjoon Online Judge
- 핵심 알고리즘: 다익스트라 ([`../algorithm/dijkstra.md`](../algorithm/dijkstra.md))
- 복잡도: 시간 `O((V+E) log V)`, 공간 `O(V+E)`
- 검증: 공식 예제와 평행 간선/우회 경로 사례를 CTest로 비교한다.

`policy`는 lvalue, `std::move(policy)`는 xvalue, `make_unique` 결과는 prvalue다. `unique_ptr`은 복사되지 않고 이동되어 단일 소유권을 보존한다. 반환 prvalue는 목적 객체를 직접 초기화해 복사 생략될 수 있다. 실제 실행에는 로드·저장·비교·분기·함수 호출·가상 간접 호출이 포함될 수 있지만 구체 명령은 CPU·ABI·컴파일러·최적화에 따라 달라진다.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

직접 연습: 다른 할인 정책을 추가하고 `Checkout` 수정 없이 교체하라.
