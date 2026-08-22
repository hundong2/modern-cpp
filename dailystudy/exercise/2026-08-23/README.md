# 2026-08-23 Modern C++ 학습 자료

오늘은 여러 백엔드 구성 요소가 모두 준비된 뒤에만 서비스를 공개하는 실무 시작 경계를 C++20 `std::latch`로 표현한다. 카운터는 정확히 한 번 0으로 내려가며 다시 닫히지 않는다. `std::jthread`가 작업 스레드 수명을 RAII로 소유하고, `std::atomic<int>`는 준비 작업 합계를 데이터 경쟁 없이 모은다. 대회 문제는 BOJ 11657 「타임머신」을 Bellman–Ford로 해결한다.

## 오늘의 목표

- `struct`의 기본 public과 `class`의 기본 private, `explicit` 생성자와 멤버 초기화 목록을 실제 코드에서 읽는다.
- `std::latch` 생성·`count_down`·`wait`·`arrive_and_wait`의 수신 객체, 각 입력, 반환값과 상태 변화를 설명한다.
- `std::jthread`의 단일 스레드 소유권과 소멸 시 합류, 참조 캡처 대상의 수명을 추적한다.
- lvalue·prvalue·`std::move`가 만드는 xvalue, 람다 init-capture 이동과 반환 복사 생략을 실제 식에 연결한다.
- 음수 간선 완화, 순회 횟수 불변식, 도달 가능한 음수 사이클 판정과 `O(VE)` 복잡도를 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): 세 구성 요소의 준비 보고를 기다리는 `StartupCoordinator` 실무 예제
- [`problem.cpp`](problem.cpp): 두 작업을 같은 시작선에서 해제하는 `arrive_and_wait` 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [BOJ 11657 타임머신](https://www.acmicpc.net/problem/11657)의 제출 가능한 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, Threads 연결, 높은 경고 수준, CTest 8개 등록
- [`run_icpc_test.cmake`](run_icpc_test.cmake): ICPC 표준 입력·출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·호출 계약·Bellman–Ford 이해 검증
- [`../algorithm/bellman-ford.md`](../algorithm/bellman-ford.md): Bellman–Ford 공용 대표 문서

## Modern C++와 실무 아키텍처

`StartupCoordinator`는 “구성 요소가 모두 준비되기 전에는 진행하지 않는다”는 정책을 한 객체에 모은다. 작업자는 `complete()`만 알고, 시작을 기다리는 호출자는 `wait_until_ready()`만 안다. `std::latch`는 일회성 시작 준비·종료 카운트다운에 알맞고, 반복 단계 동기화가 필요하면 재사용 가능한 `std::barrier`를 검토해야 한다.

`coordinator`는 이름 있는 lvalue이고 작업 람다는 이를 참조로 빌린다. 따라서 coordinator가 세 `jthread`보다 먼저 파괴되면 안 된다. `Component{...}`와 `launch(...)` 반환은 prvalue이며 목적 객체를 직접 초기화할 수 있다. `std::move(component)`는 이름 있는 component를 xvalue로 바꿀 뿐이고, 실제 문자열 버퍼 이전은 선택된 `Component`/`string` 이동 생성자가 수행한다. 이동 뒤 원본은 파괴 가능한 유효 상태지만 값은 미지정이다.

`std::jthread`는 복사 불가·이동 가능 소유권 타입이다. 오늘 코드는 반환 prvalue의 보장된 복사 생략을 통해 실행 스레드 소유권을 각 지역 변수에 직접 만든다. 함수 끝에서 역순으로 소멸하며 아직 joinable이면 중지를 요청하고 합류한다. 작업 함수는 stop token을 받지 않으므로 요청에 반응하지 않지만, 짧은 작업이라 정상 종료한 뒤 합류한다.

소스 수준의 원자 로드·저장·비교·조건 분기·함수 호출·스레드 대기와 깨우기는 기계 코드와 일대일 대응하지 않는다. CPU, ABI, 컴파일러, 표준 라이브러리 구현과 최적화 옵션에 따라 인라인·운영체제 대기·스핀·명령 재배치 형태가 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 11657 타임머신](https://www.acmicpc.net/problem/11657), Baekjoon Online Judge
- 핵심 알고리즘: Bellman–Ford와 도달 가능한 음수 사이클 판정 ([`../algorithm/bellman-ford.md`](../algorithm/bellman-ford.md))
- 시간 복잡도: `O(NM)`
- 공간 복잡도: 간선 목록과 거리 배열을 합쳐 `O(N+M)`
- 대회 필수 지식: 음수 가중치에서는 다익스트라의 방문 확정 논리가 깨진다. `i`회 순회 뒤 최대 `i`개 간선 경로가 반영된다는 불변식과 N번째 완화를 이용한다.
- 검증: 공식 기본 예제, 도달 가능한/불가능한 음수 사이클, 평행 간선, 음수 경로를 CTest로 비교한다.

## 오늘 사용한 표준 라이브러리

같은 심볼의 긴 일반 설명은 공용 대표 문서에 두고, 각 C++ 호출 바로 위에는 오늘 수신 객체·실제 인자·반환·상태 변화가 적혀 있다.

| 실제 타입·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할 | 대표 문서 |
|---|---|---|---|---|
| `std::latch{count}` 생성 | `<latch>` | 클래스·생성자 | 0 이상 참가자 수를 복사해 일회성 카운터를 만들며 위반한 수는 전제조건 위반이다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `latch::count_down()` | `<latch>` | 멤버 함수 | 기본 인자 1만큼 카운터를 낮추고 0이면 대기자와 동기화하며 `void`를 반환한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `latch::wait()` | `<latch>` | const 멤버 함수 | 카운터를 바꾸지 않고 0을 관찰할 때까지 기다리며 `void`를 반환한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `latch::arrive_and_wait()` | `<latch>` | 멤버 함수 | 기본 인자 1만큼 낮춘 뒤 0까지 기다리는 도착과 대기를 한 호출로 묶는다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::jthread{callable}` 생성·소멸 | `<thread>` | 클래스·생성자·소멸자 | 람다를 소유해 새 스레드를 시작하고 객체 수명 끝에 중지 요청·합류한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::atomic<int>{0}` 생성 | `<atomic>` | 클래스 템플릿·생성자 | 완료 작업량을 0으로 초기화하고 여러 스레드가 공유하는 저장소를 소유한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `atomic::fetch_add(value, relaxed)` | `<atomic>` | 멤버 함수 | 첫 인자만큼 원자 증가하고 증가 전 값을 반환하지만 오늘 코드는 버린다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `atomic::load(relaxed)` | `<atomic>` | const 멤버 함수 | 원자 저장값을 바꾸지 않고 `int` 스냅샷을 반환해 최종 검증에 쓴다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::move(component)` | `<utility>` | 함수 템플릿 | lvalue를 xvalue로 바꿔 람다 캡처가 문자열 소유권을 이동할 기회를 준다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::string{"..."}` 생성 | `<string>` | 클래스·생성자 | 리터럴 문자를 복사해 Component가 독립 수명으로 소유한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::ptrdiff_t` | `<cstddef>` | 기본 정수 타입 별칭 | latch 참가자 수를 표현하며 음수도 표현하므로 생성 전 허용 범위를 지켜야 한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::size_t` | `<cstddef>` | 기본 정수 타입 별칭 | 검증된 비음수 도시 수·번호를 vector 크기와 인덱스 타입으로 명시 변환한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::vector<Edge>{}` / `vector(count,value)` | `<vector>` | 클래스 템플릿·생성자 | 간선과 거리 배열을 연속 메모리에 소유한다. 할당 실패가 가능하다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::reserve(count)` | `<vector>` | 멤버 함수 | 크기는 유지하고 M개 간선 용량을 먼저 확보해 이후 재할당을 피한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::push_back(edge)` | `<vector>` | 멤버 함수 | Edge lvalue를 복사해 크기를 1 늘리며 예약 용량 안에서는 기존 참조를 유지한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::operator[]` | `<vector>` | 멤버 연산자 | 검사 없이 `T&`를 반환하므로 도시 번호 범위 불변식이 안전을 책임진다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::numeric_limits<long long>::max()` | `<limits>` | 정적 멤버 함수 | 상태 없이 최댓값을 반환하고 4로 나눠 안전한 INF를 만든다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)` | `<iostream>` | 정적 멤버 함수 | C/C++ 스트림 동기화를 끄고 이전 bool은 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::cin.tie(nullptr)` | `<iostream>` | 멤버 함수 | 입력 전 cout 자동 flush 연결을 해제하고 이전 포인터는 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `operator>>` / `operator<<` | `<iostream>` | 비멤버 연산자 오버로드 | 스트림 참조를 반환해 연쇄하고 입력 대상·출력 버퍼와 상태 비트를 갱신한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

저장소의 w64devkit을 쓰는 PowerShell 예시는 다음과 같다. `build/`는 생성 산출물이므로 커밋하지 않는다.

```powershell
$kit = (Resolve-Path ../../../tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
ctest --test-dir build --output-on-failure
& ../tools/audit-standard-library-docs.ps1 -Scope all
```

예상 학습 프로그램 출력은 `main.cpp`가 `18`, `problem.cpp`가 `10`이다. CTest 8개가 모두 통과해야 한다.

## 직접 해보기

1. `StartupCoordinator{3}`을 `{2}`로 바꾸되 세 작업이 모두 `count_down()`하게 하여 전제조건 위반이 왜 미정의 동작인지 설명한 뒤 되돌린다.
2. `Component` 람다 캡처에서 `std::move`를 지우고 문자열 복사 횟수와 원본 상태 차이를 설명한다.
3. 반복 가능한 두 단계 처리에 latch를 재사용하려 하지 말고 `std::barrier`가 필요한 이유를 적는다.
4. BOJ 풀이의 `distance[from] == INF` 분기를 지우고 도달 불가능한 음수 사이클 CTest가 왜 위험한지 추적한다.
5. Bellman–Ford의 N번째 순회를 없애면 도달 가능한 음수 사이클 테스트가 어떤 잘못된 출력을 만드는지 확인한다.
