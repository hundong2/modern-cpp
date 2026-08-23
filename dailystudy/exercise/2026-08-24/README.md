# 2026-08-24 Modern C++ 학습 자료

오늘은 복사 불가능한 자원을 캡처한 작업도 지연 실행 큐에 안전하게 넣는 C++23 `std::move_only_function`을 배운다. `TaskQueue`가 호출 대상 수명을 소유하고, 애플리케이션 계층은 등록 순서와 일괄 실행 정책을 캡슐화한다. 대회 문제는 Waterloo Programming Contest 2009의 Kattis `oceancurrents` 「Ocean Currents」를 0-1 BFS로 해결한다.

## 오늘의 목표

- `struct`의 기본 public과 `class`의 기본 private, `explicit` 생성자와 멤버 초기화 목록을 실제 코드에서 읽는다.
- `std::move_only_function<void()>`의 타입 소거, 이동 전용 소유권, 빈 상태와 간접 호출 계약을 설명한다.
- `unique_ptr` 이동 캡처가 람다를 복사 불가로 만드는 이유와 `std::function` 대신 오늘 타입이 필요한 이유를 말한다.
- lvalue·prvalue·xvalue, 참조 캡처 수명, 이동 뒤 상태, 반환 복사 생략을 실제 식에 연결한다.
- 0/1 간선에서 “비용 0은 덱 앞, 비용 1은 덱 뒤” 불변식과 `O(V+E)` 정확성을 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): 이동 전용 배포 명령을 소유·실행하는 실무 `TaskQueue`
- [`problem.cpp`](problem.cpp): `unique_ptr` 캡처 명령 두 개를 버퍼링하는 직접 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [Kattis oceancurrents - Ocean Currents](https://open.kattis.com/problems/oceancurrents)의 제출 가능한 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 높은 경고 수준, CTest 7개 등록
- [`run_icpc_test.cmake`](run_icpc_test.cmake): ICPC 표준 입력·출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·호출 계약·0-1 BFS 이해 검증
- [`../algorithm/zero-one-bfs.md`](../algorithm/zero-one-bfs.md): 0-1 BFS 공용 대표 문서

## Modern C++와 실무 아키텍처

`TaskQueue`는 “서로 다른 구체 작업을 한 소유 타입으로 받아 등록 순서대로 한 번 실행한다”는 정책을 맡는다. 구체 람다 타입을 호출자 밖으로 노출하지 않고 `void()` 포트만 받는다. 람다가 `unique_ptr<DeploymentRequest>`를 값으로 캡처하므로 복사할 수 없지만 이동할 수 있다. 복사 가능한 호출 대상을 요구하는 `std::function<void()>` 대신 C++23 `std::move_only_function<void()>`이 이 소유권을 정직하게 표현한다.

`request`와 `queue`는 이름 있는 lvalue다. `std::make_unique(...)`와 람다 식은 prvalue이고 목적 객체를 직접 초기화할 수 있다. `std::move(request)`는 request를 xvalue로 바꿀 뿐 실제 소유권 이전은 람다 캡처의 `unique_ptr` 이동 생성자가 수행한다. 이동 뒤 request는 표준 계약상 빈 포인터다. 람다가 다시 `Task`, 함수 값 매개변수, vector 원소로 이동되며 최종 Queue 원소가 요청 수명을 소유한다.

`ledger`는 참조 캡처되어 소유권과 수명이 연장되지 않는다. 오늘은 ledger가 queue와 작업 실행보다 오래 살아 안전하다. 비동기 큐로 확장한다면 참조 대상 수명, 동기화, 예외 전달, 종료 정책을 별도로 설계해야 한다. `move_only_function::operator()`는 타입 소거된 간접 호출 경계지만 실제 기계 코드가 반드시 특정 가상 호출 명령이 되는 것은 아니다. CPU, ABI, 라이브러리 구현, 컴파일러와 최적화 옵션에 따라 인라인·간접 호출·할당 형태가 달라진다.

## 오늘의 ICPC 문제

- ID·제목·출처: [Kattis `oceancurrents` - Ocean Currents](https://open.kattis.com/problems/oceancurrents), Waterloo Programming Contest 2009-02-08 / Kattis Online Judge
- 핵심 알고리즘: 0-1 BFS ([`../algorithm/zero-one-bfs.md`](../algorithm/zero-one-bfs.md))
- 시간 복잡도: 질문 하나 `O(RC)`, Q개 전체 `O(QRC)`
- 공간 복잡도: 거리 배열과 덱을 합쳐 `O(RC)`
- 대회 필수 지식: 가중치가 0/1뿐이면 우선순위 큐 다익스트라의 최소 후보 선택을 덱 앞/뒤 삽입으로 대체해 `O(V+E)`로 줄인다.
- 검증: Kattis 공개 예제, 단일 칸, 정방향 0비용/역방향 1비용, 대각선 해류, 격자 경계를 CTest로 비교한다.

## 오늘 사용한 표준 라이브러리

같은 심볼의 긴 일반 설명은 공용 대표 문서에 두고, 각 C++ 호출 바로 위에는 오늘 수신 객체·실제 인자·반환·상태 변화가 적혀 있다.

| 실제 타입·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할 | 대표 문서 |
|---|---|---|---|---|
| `std::move_only_function<void()>{callable}` | `<functional>` | 클래스 템플릿·생성자 | 이동 전용 람다를 값으로 소유·타입 소거하고 Queue의 단일 작업 타입을 만든다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `move_only_function::operator bool()` | `<functional>` | 명시적 변환 연산자 | 호출 대상 보유 여부를 상태 변화 없이 검사해 빈 작업 등록을 거부한다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `move_only_function::operator()()` | `<functional>` | 멤버 호출 연산자 | 데이터 인자 없이 저장 호출 대상을 간접 호출하고 오늘의 `void` 반환은 버린다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `std::make_unique<T>(arg)` | `<memory>` | 함수 템플릿 | 요청/int를 한 번 동적 생성하고 독점 소유 포인터 prvalue를 반환한다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `std::unique_ptr<T>` 이동·`operator*` | `<memory>` | 클래스 템플릿·연산자 | 요청 소유권을 람다로 옮기고 실행 중 pointee lvalue를 빌린다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `std::move(value)` | `<utility>` | 함수 템플릿 | lvalue를 xvalue로 바꿔 후속 이동 생성자가 소유권을 이전할 기회를 준다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::vector<Task>{}` / `vector(count,value)` | `<vector>` | 클래스 템플릿·생성자 | 이동 전용 작업, 격자 행, 최단 거리 배열을 연속 저장소에 소유한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::reserve(count)` | `<vector>` | 멤버 함수 | 크기는 유지하고 예상 작업 용량을 확보해 이후 재할당을 줄인다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::push_back(task)` | `<vector>` | 멤버 함수 | Task 소유권을 끝 원소로 이동하고 크기를 1 늘린다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::clear()` | `<vector>` | 멤버 함수 | 모든 작업과 캡처 자원을 파괴해 크기를 0으로 만들고 용량은 보장하지 않는다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector/string/array::size`, `operator[]` | 각 컨테이너 헤더 | 멤버 함수·연산자 | 격자 크기를 관찰하고 범위 불변식 아래 원소 참조를 얻는다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::deque<int>{}` | `<deque>` | 클래스 템플릿·생성자 | 0-1 BFS의 앞/뒤 우선순위 후보를 양끝 큐로 소유한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `deque::empty()` / `front()` | `<deque>` | const 멤버 함수·멤버 함수 | 빈 상태를 검사하고 pop 전에 첫 정점 참조의 값을 복사한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `deque::push_front(value)` / `push_back(value)` | `<deque>` | 멤버 함수 | 0비용 후보는 앞, 1비용 후보는 뒤에 복사해 거리 순서 불변식을 유지한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `deque::pop_front()` | `<deque>` | 멤버 함수 | 값을 반환하지 않고 첫 정점을 제거하므로 먼저 `front()` 값을 복사한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::array<int,8>` | `<array>` | 클래스 템플릿 | 여덟 방향 변화량을 객체 내부 고정 크기 저장소에 보관한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::numeric_limits<int>::max()` | `<limits>` | 정적 멤버 함수 | 상태 없이 int 최댓값을 반환하고 4로 나눠 안전한 INF를 만든다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)` | `<iostream>` | 정적 멤버 함수 | C/C++ 스트림 동기화를 끄고 이전 bool 반환은 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::cin.tie(nullptr)` | `<iostream>` | 멤버 함수 | 입력 전 cout 자동 flush 연결을 해제하고 이전 포인터는 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `operator>>` / `operator<<` | `<iostream>` | 비멤버 연산자 오버로드 | 입력 대상·출력 버퍼와 스트림 상태를 갱신하고 스트림 참조를 연쇄한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

```powershell
$kit = (Resolve-Path ../../../tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
ctest --test-dir build --output-on-failure
& ../tools/audit-standard-library-docs.ps1 -Scope all
```

예상 학습 프로그램 출력은 `main.cpp`가 `2 9 worker`, `problem.cpp`가 `12`다. CTest 7개와 전체 표준 라이브러리 감사가 모두 통과해야 한다. `build/`는 생성 산출물이므로 커밋하지 않는다.

## 직접 해보기

1. `std::move_only_function<void()>`을 `std::function<void()>`으로 바꿔 unique_ptr 캡처 람다에서 나는 컴파일 오류를 읽고 복사 요구를 설명한다.
2. 첫 람다의 `&ledger`를 값 캡처로 바꾸고 원본 Ledger가 갱신되지 않는 이유를 객체 복사 관점에서 설명한다.
3. 빈 `Task{}`를 enqueue해 false가 반환되고 Queue 상태가 유지되는 테스트를 추가한다.
4. 0-1 BFS의 `push_front`와 `push_back`을 서로 바꾸고 CTest 실패 반례를 찾는다.
5. `visited`를 첫 삽입 때 고정하는 잘못된 구현에 `0→1(1), 0→2(0), 2→1(0)` 반례를 적용한다.
