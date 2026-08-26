# 2026-08-27 Modern C++ 학습 자료

오늘은 여러 worker가 준비 단계를 모두 끝낸 뒤 게시 단계로 함께 넘어가는 실무 파이프라인을 C++20 `std::barrier`로 표현한다. `std::latch`와 달리 barrier는 단계가 끝날 때 참가자 수가 다시 설정되어 반복 사용할 수 있고, 완료 함수는 각 단계마다 정확히 한 번 실행된다. 대회 문제는 BOJ 11375 「열혈강호」를 증강 경로 기반 이분 매칭으로 해결한다.

## 오늘의 목표

- `struct`의 기본 public과 `class`의 기본 private, 반환형·매개변수·`const` 멤버를 실제 코드에서 읽는다.
- `std::barrier<Completion>` 생성자의 참가자 수·완료 callable 소유권과 `arrive_and_wait()`의 단계 전환 계약을 설명한다.
- `std::jthread`의 RAII 수명, lambda 값/참조 캡처, `atomic::fetch_add/load`와 barrier 동기화의 역할을 구분한다.
- lvalue `on_phase_complete`, `std::move(on_phase_complete)` xvalue, lambda prvalue, `ReleaseReport{...}` prvalue와 복사 생략을 연결한다.
- 이분 그래프, 매칭, 교대 경로, 증강 경로와 “증강 한 번마다 매칭 크기가 1 증가한다”는 불변식을 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): 준비→게시의 두 단계 릴리스 파이프라인과 반복 barrier 완료 함수
- [`problem.cpp`](problem.cpp): 두 worker가 합산→검증 단계를 맞추는 직접 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [BOJ 11375 - 열혈강호](https://www.acmicpc.net/problem/11375)의 제출 가능한 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 높은 경고 수준, CTest 7개 등록
- [`run_icpc_test.cmake`](run_icpc_test.cmake): ICPC 표준 입력·출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·호출 계약·이분 매칭 이해 검증
- [`../algorithm/bipartite-matching-augmenting-path.md`](../algorithm/bipartite-matching-augmenting-path.md): 증강 경로 이분 매칭 공용 대표 문서
- [`../standard-library/concurrency-time-filesystem.md`](../standard-library/concurrency-time-filesystem.md): barrier·jthread·atomic 공용 호출 계약

## Modern C++와 실무 아키텍처

`ReleaseCoordinator`는 “준비 작업은 모두 끝나야 게시로 넘어간다”는 단계 순서만 책임진다. 각 `std::jthread`는 실행 자원을 RAII로 소유하며, 별도 블록의 끝에서 모두 합류한 뒤에만 결과를 읽는다. `std::barrier`는 단순 카운터가 아니라 단계 경계다. 마지막 참가자가 도착하면 완료 함수를 한 번 실행하고 다음 단계용 카운트를 다시 연다. 참가자가 도중에 빠져야 한다면 `arrive_and_drop()`을 고려해야 하며, 오늘처럼 모든 worker가 두 단계에 반드시 도착한다는 불변식을 깨면 영원히 기다릴 수 있다.

`weights`와 `on_phase_complete`는 이름 있는 lvalue다. `std::move(on_phase_complete)`는 callable을 xvalue로 표현할 뿐 실제 소유권 이동은 barrier 생성자가 수행한다. `workers.emplace_back(...)`의 lambda 식은 prvalue이며 jthread가 callable을 소유한다. lambda 내부의 `weight`는 값 복사라 독립적이지만 atomic과 barrier는 참조만 빌린다. worker가 이 객체들보다 먼저 종료된다는 범위와 join 순서가 수명 안전성의 핵심이다. `ReleaseReport{...}`는 prvalue로 반환 결과를 직접 초기화할 수 있다.

원자 덧셈은 합계 자체의 데이터 경쟁만 막는다. 오늘 코드에서 “준비가 끝난 뒤 게시”라는 순서는 `memory_order_relaxed`가 아니라 `barrier::arrive_and_wait()`의 단계 완료 동기화가 만든다. 소스 수준에서는 로드·저장·원자 read-modify-write·비교·조건 분기·함수 호출로 이해할 수 있지만 실제 명령, lock-free 여부, 인라이닝은 CPU·ABI·표준 라이브러리·컴파일러·최적화 옵션에 따라 달라진다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 11375 - 열혈강호](https://www.acmicpc.net/problem/11375), Baekjoon Online Judge
- 핵심 알고리즘: 증강 경로 기반 이분 매칭(Kuhn) ([`../algorithm/bipartite-matching-augmenting-path.md`](../algorithm/bipartite-matching-augmenting-path.md))
- 시간 복잡도: 최악 `O(N * E)` (`N`은 직원 수, `E`는 가능한 직원-일 관계 수)
- 공간 복잡도: `O(N + M + E)`
- 대회 필수 지식: 새 직원을 빈 일에 바로 넣는 탐욕만으로는 부족하다. 현재 담당자를 다른 일로 옮기는 교대 경로가 자유 일에 닿으면 간선 선택을 뒤집어 매칭 크기를 정확히 1 늘린다.
- 우승권 확장: 정점·간선 수가 커지면 모든 자유 왼쪽 정점에서 BFS 층을 만들고 DFS로 최단 증강 경로를 묶어 찾는 Hopcroft–Karp `O(E√V)`를 선택한다.
- 검증: 공개 예제, 재배정이 필요한 최소 반례, 완전 이분 그래프, 간선 없음, 일이 더 많은 경계를 CTest로 비교한다.

## 오늘 사용한 표준 라이브러리

긴 일반 계약은 공용 대표 문서에 두고 각 C++ 첫 의미 있는 호출 바로 위에는 오늘의 수신 객체·각 인자·반환·상태 변화가 적혀 있다.

| 실제 타입·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할 | 대표 문서 |
|---|---|---|---|---|
| `std::barrier<Completion>(expected, completion)` | `<barrier>` | 클래스 템플릿·생성자 | 양수 참가자 수와 이동한 완료 callable을 소유해 반복 단계 경계를 만든다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `barrier::arrive_and_wait()` | `<barrier>` | 멤버 함수 | 현재 단계 도착을 기록하고 마지막 도착·완료 함수 종료까지 기다린 뒤 다음 단계를 연다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::jthread(callable)` / 소멸자 | `<thread>` | 클래스·생성자·소멸자 | lambda를 소유해 즉시 실행하고 범위 끝에서 중지 요청과 join을 수행한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `jthread::joinable()` / `join()` | `<thread>` | 멤버 함수 | 스레드 소유 상태를 검사하고 명시적 합류 뒤 joinable 상태를 해제한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::atomic<int>::fetch_add(value, relaxed)` | `<atomic>` | 멤버 함수 | int 차이값을 원자 가산하고 증가 전 값은 버리며 다른 메모리 순서는 추가하지 않는다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `atomic::load(relaxed)` | `<atomic>` | 멤버 함수 | 저장값을 바꾸지 않고 int 스냅샷을 반환해 결과에 저장한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::memory_order_relaxed` | `<atomic>` | 열거 상수 | 원자성·수정 순서만 요구하고 단계 동기화는 barrier에 맡긴다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::vector<T>(count,value)` / `{...}` | `<vector>` | 클래스 템플릿·생성자 | 작업량·인접 목록·매칭 상태의 연속 저장소와 수명을 소유한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `vector::size()` / `empty()` / `reserve(count)` | `<vector>` | const 멤버·멤버 함수 | 참가자 수·빈 상태를 읽고 jthread 저장 용량을 미리 확보한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `vector::emplace_back(callable)` / `push_back(value)` | `<vector>` | 멤버 함수 템플릿·멤버 함수 | jthread를 끝에서 직접 만들거나 인접 간선 값을 추가한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `vector::operator[](index)` / `array::operator[](index)` | 컨테이너별 헤더 | 멤버 연산자 | 범위 검사 없이 O(1) 원소 참조를 반환하며 호출자가 인덱스를 보장한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::array<std::jthread,2>` | `<array>` | 클래스 템플릿·생성 | 고정된 두 스레드 소유자를 객체 안에 직접 보관한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::move(on_phase_complete)` | `<utility>` | 함수 템플릿 | lambda lvalue를 xvalue로 바꿔 barrier가 callable을 이동 소유하게 한다. | [유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)` / `std::cin.tie(nullptr)` | `<iostream>` | 정적 멤버·멤버 함수 | C/C++ 동기화와 입력 전 자동 flush 연결을 해제하고 이전 상태 반환은 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::cin >> value` / `std::cout << value` | `<iostream>` | 연산자 오버로드 | 스트림 위치·버퍼·상태를 갱신하고 스트림 참조를 연쇄한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

```powershell
$kit = (Resolve-Path ../../../tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
ctest --test-dir build --output-on-failure
& ../tools/audit-standard-library-docs.ps1 -Scope all
```

예상 학습 출력은 `main.cpp`가 `9 18 2`, `problem.cpp`가 `14`다. CTest 7개와 전체 표준 라이브러리 감사가 모두 통과해야 한다. `build/`는 생성 산출물이므로 커밋하지 않는다.

## 직접 해보기

1. worker 하나의 두 번째 `arrive_and_wait()`를 제거하고 왜 다른 worker가 끝나지 않는지 단계 참가자 수로 설명한다(실행 시 중단 준비).
2. `memory_order_relaxed`를 `seq_cst`로 바꿔도 barrier 자체를 제거할 수 없는 이유를 말한다.
3. 완료 함수에서 단계 번호를 기록하고 정확히 두 번 호출되는지 검증한다.
4. BOJ 풀이에서 재귀 재배정을 제거하고 `직원1={일1,일2}, 직원2={일1}` 반례가 왜 1만 만드는지 확인한다.
5. 방문 stamp 대신 매 증강 시도마다 `vector<bool>`을 새로 초기화하고 복잡도 차이를 계산한다.
