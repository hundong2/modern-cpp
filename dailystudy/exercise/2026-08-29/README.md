# 2026-08-29 Modern C++ 학습 자료

오늘은 C++20 `std::binary_semaphore`/`std::counting_semaphore`로 “데이터 준비”와 “동시 자원 상한”을 타입과 RAII 수명에 담는다. 실무 예제는 단일 갱신 mailbox와 연결 permit을 만들고, 대회 문제는 [BOJ 9248 - Suffix Array](https://www.acmicpc.net/problem/9248)를 접미 배열 doubling + 계수 정렬과 Kasai LCP로 해결한다.

## 오늘의 목표

- `struct` 기본 public, `class` 기본 private, 접근 지정자, 생성자·멤버 초기화 목록을 코드에서 읽는다.
- `binary_semaphore::release/acquire`의 counter 변화와 happens-before를 비원자 payload 안전성과 연결한다.
- `counting_semaphore<2>` permit을 이동 전용 RAII `Lease`로 감싸 누수와 이중 반환을 막는다.
- 이름 있는 객체의 lvalue, `ConfigUpdate{...}` prvalue, `std::move(object)` xvalue, 이동 뒤 유효하지만 값 미지정 상태를 설명한다.
- 접미 배열의 “길이 2^k 접두사 그룹” 불변식과 Kasai의 `height >= 이전 height-1` 불변식을 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): 세마포어로 단일 설정 갱신을 게시·수신하는 실무 mailbox
- [`problem.cpp`](problem.cpp): 이동 전용 RAII permit을 가진 용량 제한 게이트
- [`icpc_problem.cpp`](icpc_problem.cpp): BOJ 9248 제출 가능한 `O(N log N)` 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일과 CTest 7개 등록
- [`run_icpc_test.cmake`](run_icpc_test.cmake): 표준 입력·출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·호출 계약·알고리즘 증명 문제
- [`../algorithm/suffix-array-doubling-kasai.md`](../algorithm/suffix-array-doubling-kasai.md): 접미 배열과 LCP 공용 대표 문서
- [`../standard-library/concurrency-time-filesystem.md`](../standard-library/concurrency-time-filesystem.md): 세마포어 공용 호출 계약

## Modern C++와 실무 아키텍처

`SingleUpdateMailbox`는 payload 소유와 준비 신호를 한 경계에 감춘다. 생산자는 `slot_`을 쓴 뒤 `release()`, 소비자는 `acquire()`가 성공한 뒤 읽는다. 대응 release/acquire가 동기화하므로 이 한 번의 handoff에서는 slot을 atomic으로 만들 필요가 없다. 단, 미수신 값이 있는데 두 번 publish하면 binary semaphore 최대 counter 전제조건을 깨므로 범용 큐로 오해하면 안 된다.

`ConnectionGate::Lease`는 acquire한 permit의 반환 책임을 소유한다. 복사는 삭제하고 이동 생성자는 `std::exchange`로 원본 포인터를 null로 만들어 소멸자 하나만 release하게 한다. 이는 DB 연결 풀, GPU 작업 슬롯, 외부 API 동시 호출 제한처럼 “획득 성공 후 모든 제어 흐름에서 반드시 반환”해야 하는 실무 코드에 자주 쓰이는 RAII 패턴이다. 실제 연결 객체가 필요하면 Lease가 포인터/핸들을 함께 소유하도록 확장한다.

`mailbox`, `received`, 이름 있는 `update`는 lvalue다. `ConfigUpdate{7, ...}`는 prvalue이며 값 매개변수를 직접 초기화한다. `std::move(update)`는 xvalue로 바꿀 뿐이고 실제 문자열 버퍼 이동은 선택된 이동 대입이 수행한다. 반환 prvalue는 복사 생략으로 호출자 결과 객체를 직접 초기화할 수 있다. semaphore와 Lease의 비소유 포인터보다 소유 객체가 오래 살아야 한다.

기계 실행 관점에서 semaphore 호출은 counter의 원자적 읽기·수정, 조건 분기, 경쟁 시 대기/깨우기와 함수 호출로 구현될 수 있다. 접미 배열은 배열 로드·저장, 정수 비교·나머지·분기와 연속 메모리 순회를 수행한다. 구체 명령, 커널 대기 여부, 인라이닝과 벡터화는 CPU·ABI·표준 라이브러리·컴파일러·최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 9248 - Suffix Array](https://www.acmicpc.net/problem/9248), Baekjoon Online Judge
- 핵심 알고리즘: 접미 배열 doubling + 안정 계수 정렬, Kasai LCP ([대표 문서](../algorithm/suffix-array-doubling-kasai.md))
- 시간 복잡도: 접미 배열 `O(N log N)`, LCP `O(N)`, 출력 `O(N)`
- 공간 복잡도: 접미 순서·그룹·임시 배열·LCP 모두 합쳐 `O(N)`
- 대회 필수 지식: 모든 접미사를 실제 문자열로 만들지 않고 시작 인덱스와 두 개의 그룹 번호로 비교한다. LCP는 역순위 배열을 통해 현재 접미사의 직전 이웃만 본다.
- 우승권 포인트: 각 doubling 단계에서 비교 정렬을 쓰면 `O(N log²N)`이 될 수 있다. 그룹 번호를 key로 안정 계수 정렬하면 단계당 선형이다. sentinel은 입력 문자보다 작고 유일해야 한다.
- 검증: 공식 `banana`, 길이 1, 동일 문자, 주기 문자열, 모두 다른 오름차순 문자를 CTest로 확인한다. 추가로 무작위 짧은 문자열을 실제 접미사 정렬·직접 LCP와 대조한다.

## 오늘 사용한 표준 라이브러리

| 실제 타입·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할 | 대표 문서 |
|---|---|---|---|---|
| `std::binary_semaphore(0)`, `release()`, `acquire()` | `<semaphore>` | 타입 별칭·생성자·멤버 함수 | payload 쓰기를 게시하고 소비자의 읽기와 동기화한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::counting_semaphore<2>(2)`, `acquire()`, `release()` | `<semaphore>` | 클래스 템플릿·생성자·멤버 함수 | 동시 permit 수를 2로 제한하고 RAII 소멸에서 반환한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::jthread(callable)`, `join()` | `<thread>` | 클래스·생성자·멤버 함수 | 소비 작업과 실행 스레드 소유권을 묶고 join 뒤 결과 읽기를 동기화한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::string`, `push_back`, `size`, `operator[]` | `<string>` | 소유 타입·멤버 함수·연산자 | payload/입력을 소유하고 sentinel 추가·문자 접근을 수행한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::vector<int>(count)`, `size`, `operator[]`, `swap` | `<vector>` | 클래스 템플릿·생성자·멤버 함수 | 접미 배열·그룹·계수·LCP의 연속 저장소와 O(1) 버퍼 교환을 제공한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::move(value)`, `std::exchange(pointer,nullptr)` | `<utility>` | 함수 템플릿 | xvalue 변환과 이동 원본의 null 초기화를 분리해 소유권 이전을 표현한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)`, `std::cin.tie(nullptr)` | `<iostream>` | 정적 멤버·멤버 함수 | 온라인 저지의 일괄 입출력 설정을 바꾸고 반환값은 사용하지 않는다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::cin >> text`, `std::cout << value` | `<iostream>` | 연산자 오버로드 | 스트림 위치·상태와 대상/출력 버퍼를 갱신하며 같은 스트림 참조를 반환한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::size_t`, `std::ptrdiff_t` | `<cstddef>`, `<semaphore>` | 별칭 타입 | 컨테이너 크기·인덱스와 semaphore counter 계약 타입을 표현한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

```powershell
$kit = (Resolve-Path ../../../tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
ctest --test-dir build --output-on-failure
& ../tools/audit-standard-library-docs.ps1 -Scope all
```

예상 학습 출력은 `main.cpp`가 `7 edge.internal`, `problem.cpp`가 `1`이다. `build/`는 커밋하지 않는다.

## 직접 해보기

1. `publish()`에서 `release()`를 slot 대입 앞으로 옮기고 소비자가 오래된 payload를 볼 수 있는 이유를 설명한다.
2. `Lease` 복사를 허용했을 때 최대 counter 전제조건과 이중 release가 어떻게 깨지는지 그린다.
3. `std::move(update)`를 제거해 복사 횟수와 원본/목적지 상태를 비교한다.
4. 접미 배열의 계수 정렬을 `std::sort` 쌍 비교로 바꾸고 전체 복잡도를 유도한다.
5. Kasai에서 `--height`를 제거한 버전의 결과/시간 반례를 찾는다.
