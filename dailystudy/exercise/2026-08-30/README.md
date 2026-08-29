# 2026-08-30 Modern C++ 학습 자료

오늘은 실무 생산자–소비자 경계를 C++ `std::condition_variable`의 술어 대기와 명시적 `close()` 프로토콜로 만든다. 작업 저장소·잠금·대기 조건을 `BlockingJobQueue` 안에 캡슐화해 lost wakeup, 허위 깨움, 종료 교착을 피한다. 대회 문제는 [CSES 2102 - Finding Patterns](https://cses.fi/problemset/task/2102)를 Aho–Corasick automaton으로 해결한다.

## 오늘의 목표

- `struct` 기본 public, `class` 기본 private, `public`/`private`, 생성자·멤버 초기화 목록과 `explicit`을 실제 줄에서 읽는다.
- `condition_variable::wait(lock, predicate)`가 mutex를 풀고 다시 잠그며 술어를 반복 검사하는 계약을 설명한다.
- queue의 핵심 불변식 `0 <= size <= capacity`, 닫힌 뒤 push 금지, 닫혀도 남은 작업은 배출을 증명한다.
- lvalue인 이름 있는 `job`, `Job{...}` prvalue, `std::move(job)` xvalue, 이동 뒤 수명과 소유권을 연결한다.
- Aho–Corasick의 trie 전이, 실패 링크, BFS 깊이, 역방향 일치 전파 불변식을 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): 용량 제한·종료 프로토콜을 가진 실무 blocking queue
- [`problem.cpp`](problem.cpp): 결과 한 개를 안전하게 게시하고 기다리는 작은 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): CSES 2102 제출 가능한 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일과 CTest 7개 등록
- [`run_icpc_test.cmake`](run_icpc_test.cmake): 표준 입력·출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·호출 계약·알고리즘 증명 문제
- [`../algorithm/aho-corasick-multiple-pattern-matching.md`](../algorithm/aho-corasick-multiple-pattern-matching.md): Aho–Corasick 공용 대표 문서
- [`../standard-library/concurrency-time-filesystem.md`](../standard-library/concurrency-time-filesystem.md): 조건 변수·잠금 공용 호출 계약

## Modern C++와 실무 아키텍처

`BlockingJobQueue`는 `deque<Job>`, `closed_`, mutex, 두 조건 변수를 함께 소유한다. 모든 공유 상태 읽기·쓰기는 같은 mutex 아래에서 일어나며, `not_empty_`는 “작업 있음 또는 닫힘”, `not_full_`은 “공간 있음 또는 닫힘” 술어를 기다린다. 알림은 상태 자체가 아니므로 먼저 잠금 아래 상태를 바꾸고 대기자는 항상 술어를 재검사한다. `close()`는 양쪽 대기 집합을 모두 깨워 생산자와 소비자가 종료를 관찰하게 한다.

실무에서는 로그 처리, DB write-behind, 네트워크 송신, 스레드 풀 작업 전달에 이 구조가 반복된다. 운영 코드라면 `push`/`pop`에 timeout 또는 `stop_token`, 예외 정책, metric, 다중 close의 멱등성, destructor 시 대기자 수명 계약을 추가한다. 큐가 파괴될 때 기다리는 스레드가 남아 있으면 조건 변수·mutex 접근이 수명 밖으로 나가므로 소유 계층이 먼저 작업을 중지하고 join해야 한다.

이름 있는 `queue`, `job`, `result`는 lvalue다. `Job{1, "alpha"}`와 함수가 값으로 반환하는 객체는 prvalue이며 목적 저장소에 직접 생성될 수 있다. `std::move(job)`은 xvalue를 만들 뿐 실제 payload 이동은 선택된 `Job`/`string` 이동 생성·대입이 수행한다. 이동된 원본은 파괴·대입 가능한 유효 상태지만 문자열 내용은 미지정이다. `optional<Job>`이 반환 객체를 소유해 큐 잠금이 풀린 뒤에도 작업 수명이 유지된다.

기계 실행 관점에서 잠금·대기는 원자 상태 load/store/RMW, 비교, 조건 분기, 함수 호출, 경쟁 시 스케줄러 대기·깨우기로 구현될 수 있다. Aho–Corasick은 연속 배열의 전이 load/store와 인덱스 계산·분기를 주로 수행한다. 구체 명령, futex/커널 진입, 캐시 효과, 인라이닝은 CPU·ABI·표준 라이브러리·컴파일러·최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [CSES 2102 - Finding Patterns](https://cses.fi/problemset/task/2102), CSES Problem Set
- 핵심 알고리즘: Aho–Corasick trie + 실패 링크 BFS + 역방향 일치 전파 ([대표 문서](../algorithm/aho-corasick-multiple-pattern-matching.md))
- 시간 복잡도: 패턴 총길이 `P`, text 길이 `N`, 패턴 수 `K`에 대해 `O(26P + N + K)`, 고정 알파벳에서는 `O(P+N+K)`
- 공간 복잡도: 26개 완성 전이를 가진 상태마다 고정 배열을 두므로 `O(26P + P + K)`
- 대회 필수 지식: 실패 링크는 현재 문자열의 가장 긴 proper suffix이면서 trie 접두사인 상태다. BFS로 부모 실패 링크가 먼저 완성돼야 자식 실패 링크를 계산할 수 있다.
- 우승권 포인트: 패턴별 검색 `O(NP)`를 피하고 text를 한 번만 읽는다. 검색 중 방문 상태를 표시한 뒤 BFS 역순으로 실패 조상에 전파하면 중복 패턴과 suffix 패턴을 모두 선형에 판정한다.
- 검증: 공식 예제, 다단 실패 suffix, 동일 문자 중첩, 중복 패턴, 완전 불일치를 CTest로 확인하고 무작위 입력은 단순 `string::find`와 대조한다.

## 오늘 사용한 표준 라이브러리

| 실제 타입·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할 | 대표 문서 |
|---|---|---|---|---|
| `std::condition_variable`, `wait(lock,predicate)`, `notify_one()`, `notify_all()` | `<condition_variable>` | 클래스·멤버 함수 템플릿·멤버 함수 | queue 상태 술어가 참일 때까지 mutex를 양보하고 작업/종료 변경을 알린다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::mutex`, `std::unique_lock`, `std::scoped_lock` | `<mutex>` | 클래스·클래스 템플릿·생성자 | wait 가능한 잠금과 범위 잠금으로 공유 deque/종료 상태를 보호한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::jthread(callable)`, `join()` | `<thread>` | 클래스·생성자·멤버 함수 | 생산자/소비자 실행 수명을 소유하고 결과 읽기 전에 합류한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::deque<Job>`, `push_back`, `front`, `pop_front`, `size`, `empty` | `<deque>` | 클래스 템플릿·멤버 함수 | 양끝 큐 저장과 용량·빈 상태 술어를 제공한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::optional<T>`, `emplace`, `value`, `operator bool` | `<optional>` | 클래스 템플릿·멤버 함수·변환 연산자 | 정상 종료/결과 미준비와 소유 값을 타입으로 구분한다. | [소유권](../standard-library/ownership-and-vocabulary-types.md) |
| `std::array<int,26>`, `fill`, `operator[]` | `<array>` | 클래스 템플릿·멤버 함수·연산자 | automaton의 고정 소문자 전이를 상태마다 저장한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::queue<int>`, `push`, `front`, `pop`, `empty` | `<queue>` | 컨테이너 어댑터·멤버 함수 | 실패 링크를 부모 깊이부터 구축하는 BFS frontier를 소유한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::vector<T>`, `emplace_back`, `push_back`, `size`, `operator[]` | `<vector>` | 클래스 템플릿·멤버 함수·연산자 | automaton 상태와 BFS/패턴 끝 상태 인덱스를 연속 저장한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::string`, `append`, `operator==` | `<string>` | 소유 타입·멤버 함수·연산자 | 작업 payload, text, 패턴을 소유하고 학습 결과를 결합한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::move(value)` | `<utility>` | 함수 템플릿 | lvalue를 xvalue로 바꿔 문자열/작업 소유권 이동 후보를 만든다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)`, `std::cin.tie(nullptr)`, 스트림 `>>`/`<<` | `<iostream>` | 정적 함수·멤버 함수·연산자 | 온라인 저지 입출력 설정과 문자열/정수 읽기·YES/NO 출력을 수행한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::size_t` | `<cstddef>` | 타입 별칭 | 컨테이너 크기와 음수 없는 인덱스를 표현한다. | [비트·바이트](../standard-library/bit-and-byte-utilities.md) |

## 빌드와 검증

```powershell
$kit = (Resolve-Path ../../../tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
ctest --test-dir build --output-on-failure
& ../tools/audit-standard-library-docs.ps1 -Scope all
```

예상 학습 출력은 `main.cpp`가 `alpha+beta`, `problem.cpp`가 `ready`다. `build/`는 커밋하지 않는다.

## 직접 해보기

1. 술어 없는 `wait(lock)`로 바꾸고 허위 깨움 또는 알림 선행에서 어떤 불변식이 깨지는지 설명한다.
2. `close()`에서 `not_full_` 알림을 제거했을 때 가득 찬 큐의 생산자가 종료되지 않는 실행 순서를 그린다.
3. `push(Job)`를 `push(const Job&)`와 `push(Job&&)` 오버로드로 나눠 lvalue/prvalue의 복사·이동 차이를 관찰한다.
4. Aho–Corasick의 완성 전이 대신 검색 중 실패 링크를 while로 따라가는 구현과 복잡도를 비교한다.
5. 길이 1~40 무작위 text/pattern 묶음을 생성해 각 답을 `text.find(pattern) != npos`와 대조한다.
