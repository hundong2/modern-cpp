# 2026-09-02 Modern C++ 학습 자료

오늘은 외부 저장이 실패하면 이미 바꾼 메모리 상태를 자동으로 되돌리는 **RAII 보상 트랜잭션 가드**를 만든다. 가드는 보상 람다를 값으로 소유하고, 성공 시 `commit()`, 그 밖의 반환·예외 경로에서는 소멸자로 rollback한다. 대회 문제는 [CSES 2077 - Necessary Cities](https://cses.fi/problemset/task/2077)를 DFS 방문 순서와 low-link로 해결한다.

## 오늘의 목표

- 소멸자를 정상·조기 반환·예외가 공유하는 정리 경계로 사용하고, 보상 콜백의 `noexcept` 계약을 설명한다.
- `struct` 기본 `public`, `class` 기본 `private`, 생성자의 무반환형, `explicit`, 멤버 초기화 목록, 삭제된 복사 연산을 실제 코드에서 구분한다.
- lvalue·prvalue·xvalue, 참조 바인딩, `std::move`, 람다 캡처 소유권, 객체 수명, 복사 생략을 오늘 식과 연결한다.
- 단절점의 `discovered`/`low` 불변식, 루트와 비루트 판정 차이, 반복 DFS 종료 시점을 증명한다.
- CSES 한계에서도 호출 스택이 넘치지 않는 `O(N+M)` 반복 구현과 `O(N+M)` 공간을 검증한다.

## 생성 파일

- [`main.cpp`](main.cpp): 재고 예약과 영속화 실패를 다루는 재사용 가능한 RAII rollback 가드
- [`problem.cpp`](problem.cpp): 두 계좌 이체의 부분 성공을 없애는 직접 연습 예제
- [`icpc_problem.cpp`](icpc_problem.cpp): CSES 2077 제출 가능한 반복형 low-link 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 경고 옵션, CTest 6개
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·표준 호출 계약·단절점 이해 검증
- [`run_icpc_test.cmake`](run_icpc_test.cmake): ICPC 표준 입력과 정확한 출력을 비교하는 도우미
- [`../algorithm/articulation-points-low-link.md`](../algorithm/articulation-points-low-link.md): 단절점 low-link 대표 공용 문서

## Modern C++와 실무 아키텍처

`ScopeRollback<Rollback>`은 보상 작업을 값으로 소유하는 이동 전용 책임 객체다. 예약 서비스는 먼저 메모리 상태를 바꾸고 즉시 guard를 만든다. DB·메시지 발행 같은 후속 단계가 실패해 `return`하거나 예외가 전파되면 자동 객체가 역순으로 소멸하며 원상 복구한다. 모두 성공했을 때만 `commit()`으로 보상을 비활성화한다. 이 패턴은 파일 임시 교체, DB unit of work, mutex가 아닌 논리적 자원 해제, 여러 시스템 사이의 보상 트랜잭션에 자주 쓰인다. 단, 분산 시스템에서 원격 부작용을 완벽히 원복할 수 있다는 뜻은 아니므로 보상 작업의 멱등성·실패 기록·재시도 정책이 별도로 필요하다.

`ScopeRollback rollback{lambda}`에서 람다 식은 고유 closure 타입의 prvalue다. 추론 가이드가 템플릿 인자를 정하고 값 매개변수를 직접 초기화할 수 있다. 생성자 안에서 이름이 붙은 `rollback`은 lvalue이며 `std::move(rollback)`은 xvalue 식일 뿐이다. 실제 상태 이동은 `Rollback` 이동 생성자가 수행한다. guard가 callback을 값으로 소유하므로 지역 `units`는 값 캡처하지만, `this`와 `Account&` 캡처는 비소유라 guard가 실행될 때 대상 객체가 살아 있어야 한다. 오늘은 guard가 함수의 대상 참조보다 먼저 파괴되어 이 수명 계약을 만족한다.

`explicit` 생성자는 `ReservationService service = stock;` 같은 암시 변환을 막고 `ReservationService service{stock};` 직접 초기화를 허용한다. 생성자는 반환형이 없고 멤버 초기화 목록이 참조 멤버를 최초 바인딩한다. `struct StockRecord`의 필드는 기본 public, `class ReservationService`와 guard의 필드는 기본 private다. 복사 생성/대입을 삭제한 이유는 같은 보상 책임이 두 객체에 복제되어 두 번 실행되는 것을 막기 위해서다.

기계 실행 관점에서 상태 변경은 멤버 load·산술·store, 조건은 비교와 분기, guard 소멸은 callback 호출로 나타날 수 있다. 템플릿과 람다는 인라인되어 호출 자체가 사라질 수도 있다. 구체 명령은 CPU·ABI·컴파일러·표준 라이브러리·최적화에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [CSES 2077 - Necessary Cities](https://cses.fi/problemset/task/2077), CSES Problem Set / Advanced Techniques
- 핵심 알고리즘: [단절점 DFS low-link](../algorithm/articulation-points-low-link.md)
- 핵심 불변식: `low[v]`는 DFS 트리에서 `v`의 서브트리가 트리 간선과 최대 한 개의 back edge로 도달 가능한 가장 이른 방문 순서다.
- 판정: 비루트 `v`는 어떤 자식 `u`가 `low[u] >= discovered[v]`이면 단절점이고, 루트는 DFS 트리 자식이 둘 이상일 때만 단절점이다.
- 구현 선택: N=100,000 일자 그래프에서 재귀 호출 스택 위험을 피하도록 `next_edge`와 `vector<int>`로 함수 프레임을 명시한다.
- 복잡도: 모든 정점·무방향 간선을 상수 번 처리해 시간 `O(N+M)`, 인접 목록과 상태 배열·명시적 스택 공간 `O(N+M)`
- 대회 필수 이유: low-link는 단절선, 2-정점/2-간선 연결 요소, block-cut tree로 확장되는 ICPC 그래프 핵심 지식이다. 네트워크 단일 장애점 분석에도 직접 연결된다.

## 오늘 사용한 표준 라이브러리

| 실제 타입·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할·호출 계약 요약 | 대표 문서 |
|---|---|---|---|---|
| `std::move(rollback)` | `<utility>` | 함수 템플릿 | 이름 있는 callback 매개변수 lvalue를 xvalue로 바꾼 `Rollback&&`를 반환한다. 반환 참조는 멤버 생성에 즉시 쓰며 실제 이동·예외는 closure 타입 생성자에 달린다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::vector(count, value)` | `<vector>` | 클래스 템플릿·생성자 | 그래프와 DFS 배열이 원소를 소유한다. count개 초기화에 선형 시간·공간, 할당 실패 가능성이 있다. | [컨테이너·뷰](../standard-library/containers-and-views.md) |
| `vector::reserve(count)` | `<vector>` | 멤버 함수 | stack size는 유지하고 capacity를 확보한다. 재할당 시 기존 관찰자는 무효이며 반환은 void다. | [컨테이너·뷰](../standard-library/containers-and-views.md) |
| `vector::push_back(value)` | `<vector>` | 멤버 함수 | 인접 정점/DFS 프레임 값을 복사해 size를 1 늘린다. 분할 상환 O(1), 재할당 시 포인터·참조·반복자가 무효다. | [컨테이너·뷰](../standard-library/containers-and-views.md) |
| `vector::size()` | `<vector>` | 멤버 함수 | 수신 vector를 유지하며 부호 없는 원소 수를 O(1)에 반환해 인덱스 범위를 증명한다. | [컨테이너·뷰](../standard-library/containers-and-views.md) |
| `vector::empty/back/pop_back` | `<vector>` | 멤버 함수 | 빈 상태 검사 후 마지막 프레임 값을 복사하고 제거한다. `back/pop_back`은 빈 수신 객체에서 UB이며 모두 O(1)이다. | [컨테이너·뷰](../standard-library/containers-and-views.md) |
| `std::min(a, b)` | `<algorithm>` | 함수 템플릿 | 두 int lvalue를 const 참조로 읽고 작은 값의 참조를 반환한다. 즉시 복사 저장하며 입력은 유지되고 O(1)이다. | [알고리즘·ranges](../standard-library/algorithms-and-ranges.md) |
| `std::ios::sync_with_stdio(false)` | `<iostream>` | 정적 멤버 함수 | C stdio 동기화를 끄고 이전 bool 설정을 반환하지만 무시한다. 이후 C/C++ 입출력을 섞지 않는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::cin.tie(nullptr)` | `<iostream>` | 멤버 함수 | 입력 전 자동 flush 연결을 해제하고 이전 `ostream*`를 반환하지만 무시한다. 스트림 소유권은 유지된다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| 스트림 `operator>>`, `operator<<` | `<iostream>` | 연산자 함수 | 대상 lvalue를 갱신하거나 값을 출력하고 같은 스트림 참조를 반환한다. 위치·상태가 바뀌며 실패는 기본적으로 상태 비트에 남는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

저장소 루트의 PowerShell에서 실행한다. `build/`는 생성 산출물이므로 커밋하지 않는다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-09-02 -B dailystudy/exercise/2026-09-02/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-09-02/build
ctest --test-dir dailystudy/exercise/2026-09-02/build --output-on-failure
powershell -ExecutionPolicy Bypass -File dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all
```

CTest는 두 RAII 실행 예제, CSES 공개 예제, 일자 그래프, DFS 루트, 사이클+꼬리를 검증한다. 추가로 작은 무작위 연결 그래프에서 각 정점을 실제로 제거한 뒤 BFS 연결 요소 수를 세는 brute force와 비교하고, N=100,000 일자 그래프로 반복 DFS의 시간·스택 안전성을 확인한다.

## 직접 해보기

1. `rollback.commit()`을 지우고 성공 예약까지 원복되는지 확인해 guard 상태 전이를 설명한다.
2. callback에서 예외를 던지는 코드를 작성하고 `noexcept` 소멸자 계약상 왜 프로그램 종료 위험이 있는지 말한다.
3. `ScopeRollback first = lambda;`가 explicit 때문에 실패하고 직접 초기화는 되는지 확인한다.
4. 단절점 코드에서 루트 별도 규칙을 지우고 삼각형의 첫 DFS 정점이 잘못 표시되는 반례를 추적한다.
5. 작은 그래프의 `discovered`와 `low` 값을 DFS 종료 순서마다 손으로 적는다.
6. CHECKPOINT를 자료 없이 풀고 실제 표준 호출마다 수신 객체, 각 입력, 반환, 상태 변화, 수명·무효화·오류를 말한다.
