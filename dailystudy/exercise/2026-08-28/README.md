# 2026-08-28 Modern C++ 학습 자료

오늘은 여러 worker가 같은 출력 스트림에 진단 정보를 남기되 한 줄 레코드의 문자가 서로 섞이지 않게 하는 실무 경계를 C++20 `std::osyncstream`으로 만든다. 일반 `std::ostream`을 포트로 주입해 운영에서는 파일/콘솔에, 테스트에서는 `std::ostringstream`에 기록할 수 있다. 대회 문제는 BOJ 2261 「가장 가까운 두 점」을 x축 분할, y축 병합, 중앙 띠 후보 제한으로 `O(N log N)`에 해결한다.

## 오늘의 목표

- `struct`의 기본 public과 `class`의 기본 private, `public`/`private`, 멤버 변수, 생성자·멤버 초기화 목록을 실제 코드에서 읽는다.
- `explicit ConcurrentAuditLog(std::ostream&)`가 암시 변환을 막는 이유와 비소유 참조 포트의 수명 조건을 설명한다.
- `std::osyncstream(ostream&)`, 연쇄 `operator<<`, 소멸 시 emit의 입력·출력·상태 변화와 예외 계약을 말한다.
- `AuditEvent{...}`·lambda의 prvalue, 이름 있는 `event` lvalue, `std::move(event)` xvalue, lambda 이동 캡처를 소유권 이전과 연결한다.
- 최근접 점 분할 정복의 “함수 반환 시 구간은 y정렬” 불변식과 중앙 띠에서 상수 개 후보만 남는 이유를 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): `ostream&` 포트에 레코드 단위 동시 로그를 쓰는 실무 서비스
- [`problem.cpp`](problem.cpp): 두 metric 레코드를 섞이지 않게 쓰고 완전성을 검증하는 직접 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [BOJ 2261 - 가장 가까운 두 점](https://www.acmicpc.net/problem/2261)의 제출 가능한 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 높은 경고 수준, CTest 7개 등록
- [`run_icpc_test.cmake`](run_icpc_test.cmake): ICPC 표준 입력·출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·호출 계약·분할 정복 이해 검증
- [`../algorithm/divide-and-conquer-closest-pair.md`](../algorithm/divide-and-conquer-closest-pair.md): 최근접 점 분할 정복 공용 대표 문서
- [`../standard-library/io-parsing-and-utilities.md`](../standard-library/io-parsing-and-utilities.md): 스트림·`osyncstream` 공용 호출 계약

## Modern C++와 실무 아키텍처

`ConcurrentAuditLog`는 로그 형식을 책임지고 실제 저장 장소는 `std::ostream&`로 빌린다. 이 의존성 역전 덕분에 서비스는 파일, 콘솔, 메모리 버퍼의 구체 타입을 알 필요가 없다. 참조 멤버는 포트를 소유하지 않으므로 `sink → log → workers` 순으로 생성하고 모든 worker를 합류시킨 뒤 `sink`를 읽는 수명 순서가 계약의 일부다.

일반 스트림의 여러 번 `operator<<`를 서로 다른 스레드에서 수행하면 레코드 조각이 섞일 수 있다. 각 호출이 지역 `std::osyncstream`을 만들면 문자는 동기 버퍼에 모이고 객체 수명 끝의 emit에서 같은 최종 stream buffer를 사용하는 다른 `osyncstream`과 섞이지 않는 덩어리로 전달된다. 출력 순서 자체는 스케줄러에 따라 달라지므로 테스트는 순서를 고정하지 않고 각 완성 레코드와 줄 수를 검증한다. 운영에서 한 줄마다 동기 버퍼와 잠금 비용이 생기므로 아주 높은 처리량에서는 배치 크기와 비동기 큐를 함께 측정해야 한다.

`events`와 각 `event`는 이름 있는 lvalue다. `std::move(event)` 결과는 xvalue일 뿐이며 실제 `std::string` 버퍼 소유권 이전은 lambda의 `owned` 초기화가 수행한다. lambda 식은 prvalue이고 `jthread`가 callable을 이동 소유한다. 이동된 `events` 원소는 유효하지만 값이 미지정이므로 다시 읽지 않는다. `AuditEvent{...}`와 반환 `AuditSummary{...}`는 prvalue이며 결과 객체를 직접 초기화하는 복사 생략이 가능하다.

기계 실행 관점에서 각 로그는 문자열 문자 로드, 버퍼 저장, 길이/용량 비교, 필요 시 할당, 동기화된 emit, 함수 호출로 이어질 수 있다. 분할 정복 풀이는 좌표 로드·정수 뺄셈/곱셈·비교·조건 분기와 재귀 호출을 수행한다. 다만 구체적인 명령, 잠금 구현, 인라이닝, 복사 생략, 벡터화는 CPU·ABI·표준 라이브러리·컴파일러·최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 2261 - 가장 가까운 두 점](https://www.acmicpc.net/problem/2261), Baekjoon Online Judge
- 핵심 알고리즘: 최근접 점 분할 정복 ([`../algorithm/divide-and-conquer-closest-pair.md`](../algorithm/divide-and-conquer-closest-pair.md))
- 시간 복잡도: 최초 x정렬 `O(N log N)` + 각 재귀 깊이의 y병합/띠 검사 `O(N)`, 전체 `O(N log N)`
- 공간 복잡도: 점 배열과 공유 작업 배열 `O(N)`, 재귀 스택 `O(log N)`
- 대회 필수 지식: 왼쪽·오른쪽 내부 최솟값만 취하면 중앙선을 가로지르는 더 가까운 쌍을 놓친다. 폭 `2√d`의 띠를 y순으로 훑되 y차 제곱이 `d` 이상이면 멈춰야 한다.
- 우승권 포인트: 재귀마다 y정렬을 다시 하면 `O(N log²N)`이다. 두 반쪽이 반환될 때 이미 y정렬이라는 불변식을 세우고 선형 병합해 한 log를 제거한다.
- 검증: 공식 예제, 중복 좌표, 중앙 띠 교차 최적해, 좌표 경계, 같은 y좌표를 CTest로 비교하고 별도 무작위 입력을 `O(N²)` 완전 탐색과 대조한다.

## 오늘 사용한 표준 라이브러리

긴 일반 계약은 공용 대표 문서에 두고 각 C++ 첫 의미 있는 호출 바로 위에는 오늘의 수신 객체·각 인자·반환·상태 변화가 적혀 있다.

| 실제 타입·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할 | 대표 문서 |
|---|---|---|---|---|
| `std::osyncstream(output)` / 소멸 emit | `<syncstream>` | 클래스·생성자·소멸자 | 비소유 `ostream`의 stream buffer를 감싸 한 레코드를 모아 수명 끝에 섞이지 않게 전달한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `osyncstream::operator<<(value)` | `<ostream>`, `<syncstream>` | 연산자 오버로드 | 문자·문자열·정수를 동기 버퍼에 쓰고 같은 스트림 참조를 반환해 연쇄한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::ostringstream()` / `str()` | `<sstream>` | 클래스·생성자·멤버 함수 | 테스트가 소유하는 메모리 버퍼를 만들고 합류 뒤 새 `string` 스냅샷을 복사한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::ostream&` | `<ostream>` | 타입·비소유 참조 | 로그 저장 장소를 구체 구현과 분리하는 출력 포트다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::string` / `std::string_view` | `<string>`, `<string_view>` | 소유 타입·비소유 뷰 타입 | 이벤트 이름과 캡처 결과는 소유하고, metric 이름은 호출 동안만 읽는다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::jthread(callable)`, `joinable()`, `join()` | `<thread>` | 클래스·생성자·멤버 함수 | 이동 캡처 lambda를 실행 소유하고 명시적 합류로 sink 읽기 전 수명·동기화를 보장한다. | [동시성](../standard-library/concurrency-time-filesystem.md) |
| `std::vector<T>(count)` / 초기화 목록 | `<vector>` | 클래스 템플릿·생성자 | 이벤트, worker, 점, 공유 작업 공간의 연속 저장소를 소유한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `vector::size()` / `begin()` / `end()` / `operator[](index)` | `<vector>` | const 멤버·멤버 연산자 | 원소 수와 반복자 구간을 읽고, 검증한 인덱스로 점에 O(1) 접근한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `vector::reserve(count)` / `emplace_back(callable)` | `<vector>` | 멤버 함수·멤버 함수 템플릿 | 재할당을 미리 피하고 끝에 jthread를 직접 생성한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `string::find(pattern)` / `string::npos` | `<string>` | const 멤버 함수·상수 | 합류 뒤 완성 레코드가 있는지 찾고 문자열은 유지한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::move(event)` | `<utility>` | 함수 템플릿 | event lvalue를 xvalue로 바꿔 lambda가 문자열 소유권을 이동받게 한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::sort(first,last,comparison)` | `<algorithm>` | 함수 템플릿 | 점을 x순으로, 작은 재귀 구간을 y순으로 제자리 재배열한다. | [알고리즘](../standard-library/algorithms-and-ranges.md) |
| `std::min(left,right)` | `<algorithm>` | 함수 템플릿 | 두 부분 문제와 후보 거리 중 작은 `Distance`를 선택한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::numeric_limits<Distance>::max()` | `<limits>` | 클래스 템플릿·정적 함수 | 작은 구간 완전 탐색의 안전한 초기 상한을 만든다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::size_t` / `std::ptrdiff_t` | `<cstddef>` | 기본 별칭 타입 | 크기·인덱스를 표현하고 반복자 오프셋에는 부호 있는 차이 타입을 쓴다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)` / `std::cin.tie(nullptr)` | `<iostream>` | 정적 멤버·멤버 함수 | 일괄 온라인 저지 입력에서 C 동기화와 자동 flush 연결을 해제한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::cin >> value` / `std::cout << value` | `<iostream>` | 연산자 오버로드 | 입력 위치와 대상 값, 출력 버퍼·상태를 갱신하고 스트림 참조를 연쇄한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

```powershell
$kit = (Resolve-Path ../../../tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
ctest --test-dir build --output-on-failure
& ../tools/audit-standard-library-docs.ps1 -Scope all
```

예상 학습 출력은 `main.cpp`가 `3 1`, `problem.cpp`가 `2 1`이다. CTest 7개, 무작위 완전 탐색 대조, 전체 표준 라이브러리 감사까지 통과해야 한다. `build/`는 생성 산출물이므로 커밋하지 않는다.

## 직접 해보기

1. `std::osyncstream`을 제거하고 각 레코드를 여러 `operator<<`로 직접 출력했을 때 보장할 수 없는 것을 설명한다.
2. `sink.str()`를 worker 합류 전에 호출하는 코드를 만들고 왜 데이터 경쟁 가능성이 생기는지 수명·동기화 관점에서 말한다.
3. `std::move(event)`를 제거했을 때 lambda 캡처가 어떤 복사를 수행하고 원본 `events`가 어떻게 달라지는지 확인한다.
4. 최근접 점 풀이에서 중앙 띠 검사를 제거하고 좌우 내부 최솟값보다 교차 쌍이 가까운 반례를 실행한다.
5. 매 재귀마다 `std::sort`로 y정렬하는 버전과 오늘의 선형 병합 버전의 점근 복잡도를 유도한다.
