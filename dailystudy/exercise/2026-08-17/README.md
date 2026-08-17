# 2026-08-17 Modern C++ 학습 자료

## 오늘의 목표

- C++20 `std::atomic<std::shared_ptr<const T>>`로 읽기가 많은 설정을 불변 스냅샷으로 게시한다.
- `release` 저장과 `acquire` 로드가 객체의 완성된 상태를 전달하는 이유를 이해한다.
- `relaxed` 원자 카운터가 값 하나의 원자성은 보장하지만 다른 메모리의 순서는 전달하지 않음을 구분한다.
- 포트와 어댑터, 비소유 참조 주입, 불변 값 객체로 동시성 경계를 작게 유지한다.
- 지연 전파 세그먼트 트리에서 구간 대입과 반전의 합성 순서 및 구간 합 불변식을 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): `std::atomic<std::shared_ptr<const AppConfig>>`로 설정 스냅샷을 게시하는 실무형 저장소와 서비스
- [`problem.cpp`](problem.cpp): `std::atomic<std::uint64_t>`와 `memory_order_relaxed`로 독립 요청 통계를 집계하는 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [UVa 11402 Ahoy, Pirates!](https://onlinejudge.org/index.php?Itemid=8&option=com_onlinejudge&page=show_problem&problem=2397) 제출 가능 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일 빌드, 경고 옵션, CTest 다섯 개 정의
- [`run_icpc_test.cmake`](run_icpc_test.cmake): 표준 입력 실행과 출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·메모리 순서·알고리즘 이해 검증
- [`../algorithm/lazy-segment-tree.md`](../algorithm/lazy-segment-tree.md): 중복 검사 후 추가한 지연 전파 세그먼트 트리 대표 문서

## 오늘 사용한 표준 라이브러리

공통 계약과 용어는 [`../standard-library/README.md`](../standard-library/README.md), 현재 날짜의 전체 심볼은 [`../standard-library/by-date.md`](../standard-library/by-date.md)에서 찾는다.

| 심볼·헤더 | 종류와 현재 역할 | 반환·복잡도·수명 주의점 |
|---|---|---|
| `std::atomic<T>` `<atomic>` | 원자 값을 보관하는 클래스 템플릿 | `load/store/fetch_add`는 중간 상태를 노출하지 않지만 주변 비원자 데이터까지 자동 보호하지 않는다. |
| `std::memory_order_release/acquire` `<atomic>` | 원자 연산 순서 상수 | 완성된 설정 객체를 게시·관찰하는 happens-before 관계를 만든다. |
| `std::memory_order_relaxed` `<atomic>` | 가장 약한 원자 순서 | 카운터 한 값의 원자성은 유지하지만 다른 메모리 공개 순서를 만들지 않는다. |
| `std::shared_ptr<T>` `<memory>` | 공유 소유권 클래스 템플릿 | 복사는 참조 횟수를 늘리고 이동은 소유권 한 몫을 넘긴다. 가리키는 `T`의 동시 변경은 별도 보호가 필요하다. |
| `std::make_shared<T>` `<memory>` | 객체와 제어 블록을 만드는 함수 템플릿 | `shared_ptr<T>` prvalue를 반환하며 보통 한 번의 할당으로 생성한다. |
| `std::move` `<utility>` | xvalue 캐스트 함수 템플릿 | 직접 데이터를 옮기지 않고 이동 생성·대입이 선택될 기회를 준다. |
| `std::vector<T>` `<vector>` | 트리 노드를 소유하는 동적 연속 컨테이너 | 인덱스 접근 `O(1)`, 재할당 시 기존 포인터·참조·반복자가 무효화될 수 있다. |
| `std::string` `<string>` | 압축 해제한 0/1 문자를 소유 | `append`는 선형으로 문자를 추가하며 용량 부족 시 재할당한다. |
| `std::size_t` `<cstddef>` | 크기와 인덱스용 부호 없는 타입 | 음수 `int`를 변환하기 전에 유효 범위를 확인한다. |
| `std::uint64_t` `<cstdint>` | 정확히 64비트인 경우 제공되는 부호 없는 타입 | 카운터 범위를 넓히지만 산술 오버플로는 모듈러로 감싼다. |
| `std::cin`, `std::cout` `<iostream>` | 표준 입출력 스트림 객체 | `>>`, `<<`는 스트림 참조를 반환해 연쇄하며 입력 실패 상태와 flush 정책을 확인한다. |
| `std::ios::sync_with_stdio(false)` `<iostream>` | C/C++ 스트림 동기화 설정 함수 | 이전 설정을 `bool`로 반환하며 호출 뒤 C stdio와 임의로 섞지 않는다. |

세부 설명: [동시성](../standard-library/concurrency-time-filesystem.md), [컨테이너](../standard-library/containers-and-views.md), [소유권](../standard-library/ownership-and-vocabulary-types.md), [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md).

## Modern C++와 실무 아키텍처

설정 객체는 생성이 끝난 뒤 바꾸지 않는 `const AppConfig`다. 작성자는 새 객체를 완성한 뒤 `release`로 `shared_ptr`를 원자 저장하고, 독자는 `acquire`로 포인터를 원자 로드한다. 독자가 얻은 `shared_ptr`는 다음 설정이 게시된 뒤에도 자신이 읽던 옛 객체의 수명을 연장한다. 따라서 긴 읽기 잠금이나 잠금 해제 뒤 무효가 되는 내부 참조를 외부로 노출하지 않는다. 이 패턴은 읽기가 압도적으로 많은 기능 플래그, 라우팅 표, 정책 설정에서 자주 쓰인다.

`IConfigSnapshotSource`는 서비스가 요구하는 읽기 계약이고 `AtomicConfigStore`는 동시성 세부 구현이다. `RetryService`는 포트를 `const&`로 빌리므로 저장소를 소유하지 않는다. 저장소가 서비스보다 오래 살아야 한다는 수명 계약이 있다. 가상 함수는 구현 교체를 가능하게 하지만 호출은 가상 간접 호출이 될 수 있다. 정확한 로드·저장·비교·조건 분기·함수 호출 명령과 비용은 CPU, ABI, 표준 라이브러리, 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

`main.cpp`의 `initial`과 `store`는 이름 있는 lvalue다. `std::move(initial)`은 xvalue로 바꾸어 공유 포인터의 소유권 한 몫을 이동할 기회를 준다. `AppConfig{...}`와 `make_shared`의 반환은 prvalue이며 목적 객체를 직접 초기화한다. 반환 prvalue와 값 객체는 C++17 이후 보장되는 복사 생략 또는 이동 최적화의 대상이 된다. `shared_ptr<const AppConfig>`의 `const`는 설정 내용을 바꾸지 못하게 하지만 참조 횟수 관리 자체는 계속 가능하다.

`problem.cpp`의 통계는 다른 데이터의 게시 순서와 무관한 독립 숫자이므로 `memory_order_relaxed`가 알맞다. 단, 두 카운터를 차례로 로드한 `RequestSnapshot`은 하나의 원자적 시점 전체를 보장하지 않는다. 둘 사이에 강한 일관성이 필요하면 잠금이나 별도의 버전 프로토콜을 설계해야 한다.

## 기초 문법을 코드에서 찾기

- `struct AppConfig`와 `struct RequestSnapshot`의 기본 접근은 `public`, `class`의 기본 접근은 `private`다.
- 생성자는 클래스와 이름이 같고 반환형이 없다. `explicit AtomicConfigStore(...)`는 단일 인자 암시 변환을 막으며 `AtomicConfigStore store{...};` 직접 초기화는 허용한다.
- `using`은 새 타입을 만드는 문법이 아니라 긴 타입에 별칭을 붙인다. 오늘 ICPC 문서는 별칭보다 `enum class LazyOp`로 연산 집합을 강하게 모델링한다.
- `std::atomic<std::shared_ptr<const AppConfig>>`에서 꺾쇠 안은 템플릿 인자다. 바깥 원자는 공유 포인터 교체를, 안쪽 `const AppConfig`는 설정 불변성을 표현한다.
- `const T&`는 기존 객체를 복사 없이 읽지만 소유하지 않는다. `shared_ptr<T>`는 공유 소유권으로 객체 수명을 연장한다.
- `if`, `else`, `for`, `switch`, `return`, `==`, `&&`, `+`, `-`, `*`, `[]`, `->`, `.`의 의미는 각 코드 줄 바로 위 주석에서 확인한다.

## 오늘의 ICPC 문제

- ID·제목·출처: [UVa 11402 Ahoy, Pirates!](https://onlinejudge.org/index.php?Itemid=8&option=com_onlinejudge&page=show_problem&problem=2397), UVa Online Judge / ICPC Live Archive
- 공식 문제 PDF: [Problem H — Ahoy, Pirates!](https://onlinejudge.org/contests/193-a6da26e8/11402.pdf)
- 핵심 알고리즘: [지연 전파 세그먼트 트리](../algorithm/lazy-segment-tree.md)
- 복잡도: 초기 문자열 조립과 트리 빌드 `O(N)`, 각 변경·질의 `O(log N)`, 전체 `O(N + Q log N)`, 저장 공간 `O(N)`
- 대회 필수성: 세그먼트 트리는 구간 정보를 결합하는 표준 도구다. 지연 전파는 구간 전체 변경을 잎마다 수행하지 않고 대표 노드에 미루며, 대입·덧셈·반전처럼 연산 합성이 필요한 고급 구간 문제의 기반이 된다.

각 노드는 담당 구간의 1 개수를 항상 정확히 보관한다. 자식은 아직 최신이 아닐 수 있지만, `lazy[node]`가 자식에 나중에 적용할 합성 연산을 나타낸다. 완전 포함 갱신은 노드 합과 지연 값만 `O(1)`에 바꾼다. 일부 포함 질의나 갱신은 `push`로 부모 연산을 자식에 먼저 전파한다. 새 대입은 이전 연산을 덮어쓰고, 반전 두 번은 사라지며, 0 대입 뒤 반전은 1 대입이 된다. 이 순서를 거꾸로 합성하면 오답이다.

## 빌드와 검증

저장소 루트의 w64devkit을 사용하는 PowerShell 예시다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-08-17 -B build/2026-08-17 -G "MinGW Makefiles"
cmake --build build/2026-08-17
ctest --test-dir build/2026-08-17 --output-on-failure
./build/2026-08-17/daily_main.exe
./build/2026-08-17/daily_problem.exe
```

CTest는 두 Modern C++ 실행 파일의 정상 종료, 대입·반전·질의 합성, 이중 반전, 반복 블록과 경계 인덱스를 검증한다. 추가로 모든 C++ 파일을 다시 읽어 `#include`, 타입, 초기화, 함수, 분기, 반복, 컨테이너 호출, 값 범주, 수명, 원자 메모리 순서, 알고리즘 불변식의 가까운 한글 주석을 확인하고 `../algorithm/lazy-segment-tree.md` 링크가 실제 파일을 가리키는지 검사한다.

## 직접 해보기

1. `main.cpp`에서 `shared_ptr<const AppConfig>`의 `const`를 제거하고 독자가 설정을 바꾸게 하면 어떤 데이터 경쟁과 설계 문제가 생기는지 설명한다.
2. `memory_order_release/acquire`를 모두 `relaxed`로 바꾸었을 때 포인터 값의 원자성은 남지만 새 객체의 다른 필드 공개 계약이 왜 약해지는지 말한다.
3. `problem.cpp`의 두 카운터 합이 항상 특정 불변식을 만족해야 한다면 현재 `snapshot()`만으로 충분하지 않은 실행 순서를 그린다.
4. ICPC 코드에서 `compose(set_zero, invert)`와 `compose(invert, set_zero)`를 손으로 계산해 연산 합성이 교환법칙을 만족하지 않음을 보인다.
5. `lazy_[node]`를 합성하지 않고 새 연산으로 무조건 덮어쓰도록 바꾼 뒤 이중 반전 CTest가 왜 실패하는지 추적한다.
6. `string::append`, `vector`의 크기 생성자, `atomic::load/store/fetch_add` 각각의 반환값·복잡도·무효화 또는 메모리 순서 계약을 공용 문서 없이 설명한다.
