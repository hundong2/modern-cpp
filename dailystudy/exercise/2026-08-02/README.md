# 2026-08-02 — `std::optional`과 명시적인 조회 실패

오늘은 “값이 없을 수 있음”을 마법 값 `-1` 대신 `std::optional<T>`로 표현하고, 저장소 포트와 메모리 어댑터를 분리한다. 호출자는 `has_value()` 또는 `if (result)`로 성공 여부를 확인한 뒤 `*result`로 값을 읽는다.

## 파일과 실행

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

- `main.cpp`: 완성 예제. `UserRepository` 포트, `MemoryUserRepository` 어댑터, `GreetingService` 응용 서비스의 역할을 살핀다.
- `problem.cpp`: `TODO`를 고치며 `optional`의 성공/실패 분기를 연습한다.
- `icpc_problem.cpp`: BOJ 1753 **최단경로**의 제출 가능한 다익스트라 풀이이다.
- `CHECKPOINT.md`: 말하기, 수정, 실행으로 이해를 증명한다.

## 값 범주·수명·소유권

`repository`, `service`, `result`처럼 이름 있는 식은 lvalue이다. `User{1, "Ada"}`와 함수가 반환하는 `std::optional<User>` 임시 결과는 prvalue이며, 반환 목적지에 직접 만들어져 복사 생략될 수 있다. `std::move(repository)`는 lvalue를 xvalue로 표현해 `unique_ptr`의 소유권을 서비스로 옮긴다. 이동 뒤 원래 포인터는 비어 있고, 저장소 객체의 수명은 서비스가 끝날 때까지 이어진다. `const User& user{*result}`는 optional 내부 객체에 바인딩되므로 `result`보다 오래 살아서는 안 된다.

개념적으로 조회는 키 로드·비교·조건 분기와 함수 호출을, 가상 포트 호출은 동적 타입에 따른 간접 호출을 포함할 수 있다. 실제 로드·저장·레지스터·명령 선택과 가상 호출 제거는 CPU, ABI, 컴파일러, 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- 문제 ID/제목: BOJ 1753, **최단경로**
- 출처: [Baekjoon Online Judge](https://www.acmicpc.net/problem/1753)
- 핵심 알고리즘: 다익스트라 — 공용 문서 [`../algorithm/dijkstra.md`](../algorithm/dijkstra.md)
- 복잡도: 인접 리스트와 이진 최소 힙에서 시간 `O((V+E) log V)`, 공간 `O(V+E)`
- 검증: 공식 예제를 CTest 입력으로 실행해 기대 출력과 완전 일치시키고, 도달 불가 정점의 `INF` 출력도 확인한다.

## 직접 해보기

1. 조회 ID를 `7`에서 `8`로 바꾸고 실패 분기를 확인한다.
2. `optional` 대신 `-1`을 쓰면 유효한 값과 실패가 어떻게 섞이는지 설명한다.
3. ICPC 예제에서 정점 1이 확정되는 순간의 거리 배열과 최소 힙 내용을 손으로 적는다.

