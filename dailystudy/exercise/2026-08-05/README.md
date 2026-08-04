# 2026-08-05 Modern C++ 학습 자료

## 오늘의 주제: `std::pmr`와 메모리 자원 경계

`std::pmr::vector`는 컨테이너의 자료형과 메모리 할당 정책을 분리한다. `main.cpp`는 호출자가 `std::pmr::memory_resource`를 주입하고, 서비스가 그 자원을 사용하는 구조를 보여 준다. `problem.cpp`에서는 같은 아이디어를 작은 누적기 연습으로 확인한다.

- `std::pmr::monotonic_buffer_resource`: 미리 준비한 버퍼에서 빠르게 할당하고 자원 전체를 한 번에 정리한다.
- 비소유 포인터: 서비스는 자원을 소유하지 않으므로 자원이 서비스와 컨테이너보다 오래 살아야 한다.
- 값 범주: 이름 있는 객체는 lvalue, `Sample{...}`은 prvalue, `std::move(x)`는 xvalue다. 반환값은 복사 생략으로 목적지에 직접 만들어질 수 있다.
- 기계 관점: 원소 접근은 보통 주소 계산·로드/저장, 분기는 비교·조건 분기로 이어지지만 실제 명령은 CPU·ABI·컴파일러·최적화 옵션에 따라 달라진다.

## 오늘의 ICPC/OJ 문제

- 문제: BOJ 11724, **연결 요소의 개수**
- 출처: <https://www.acmicpc.net/problem/11724>
- 핵심 알고리즘: 인접 리스트와 반복형 깊이 우선 탐색(DFS)
- 공용 문서: [`../algorithm/graph-traversal-connected-components.md`](../algorithm/graph-traversal-connected-components.md)
- 복잡도: 시간 `O(N + M)`, 공간 `O(N + M)`

아직 방문하지 않은 정점을 발견할 때마다 연결 요소 수를 하나 늘리고, 그 정점에서 도달 가능한 모든 정점을 스택으로 방문한다. 각 정점은 한 번 방문되고 무방향 간선은 양 끝 인접 리스트에서 각각 한 번만 조사된다.

## 빌드와 검증

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

CTest는 두 학습 프로그램의 종료 코드와 BOJ 11724 예제 두 개의 출력을 검증한다. 추가로 빈 간선 그래프, 한 연결 요소, 여러 연결 요소를 직접 입력해 경계를 확인한다.

## 직접 해보기

1. `main.cpp`의 버퍼 크기를 줄이고 상위 자원 사용 여부를 관찰한다.
2. `problem.cpp`의 `std::pmr::vector<int>`를 일반 `std::vector<int>`로 바꾸어 생성자 계약 차이를 설명한다.
3. ICPC 풀이의 `std::vector<int> stack`을 `std::queue<int>`로 바꾸어 BFS로 만들고 결과가 같은 이유를 말한다.
