# 2026-08-21 Modern C++ 학습 자료

오늘은 실무에서 설정·요청 객체를 단계적으로 만드는 fluent builder에 **멤버 함수 참조 한정자 `&`, `&&`**를 적용한다. 이름 있는 builder는 보존하며 복사 스냅샷을 만들고, 곧 파괴될 임시 builder는 내부 자원을 이동해 불필요한 깊은 복사를 줄인다. 대회 문제는 BOJ 5052 「전화번호 목록」을 접두사 트라이로 해결한다.

## 오늘의 목표

- `function() &`, `function() &&`, `function() const &`가 수신 객체 `*this`의 값 범주에 따라 다른 오버로드를 고르는 이유를 설명한다.
- fluent API가 같은 객체의 참조를 반환할 때 객체 수명과 dangling 가능성을 추적한다.
- lvalue builder의 복사 스냅샷과 rvalue builder의 이동 소비를 실제 식에 연결한다.
- `struct`/`class`, `public`/`private`, 반환형 없는 생성자, `explicit`, 멤버 초기화 목록을 코드 위치에서 읽는다.
- 벡터 재할당과 문자열·vector 복사/이동, 소유권, 복사 생략, 반환 참조 수명을 설명한다.
- 트라이의 경로 불변식, terminal 표시, 두 방향 접두사 충돌 검사와 선형 복잡도를 구현한다.

## 생성 파일

- [`main.cpp`](main.cpp): lvalue 복사/rvalue 이동 정책을 분리한 실무 HTTP 요청 builder
- [`problem.cpp`](problem.cpp): 같은 참조 한정 패턴을 작은 보고서 builder로 직접 복습하는 실행 예제
- [`icpc_problem.cpp`](icpc_problem.cpp): BOJ 5052의 제출 가능한 접두사 트라이 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 높은 경고 수준과 CTest 7개 등록
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·표준 호출 계약·트라이를 자료 없이 증명하는 검증
- [`run_icpc_test.cmake`](run_icpc_test.cmake): 입력·출력·종료 코드를 비교하는 CTest 보조 스크립트
- [`../algorithm/prefix-trie.md`](../algorithm/prefix-trie.md): 공용 접두사 트라이 대표 문서

## Modern C++와 실무 아키텍처

### 1. 참조 한정자가 수신 객체의 사용 정책을 표현한다

```cpp
RequestBuilder& header(std::string, std::string) &;
RequestBuilder&& header(std::string, std::string) &&;
HttpRequest build() const &;
HttpRequest build() &&;
```

함수 끝의 `&`와 `&&`는 반환형이 아니라 암시적 객체 매개변수인 `*this`에 대한 조건이다. `reusable.header(...)`는 `reusable`이 lvalue라 `&` 오버로드를, `RequestBuilder{"/orders"}.header(...)`는 임시 prvalue라 `&&` 오버로드를 선택한다.

`build() const &`는 이름 있는 builder를 유지해야 하므로 문자열과 vector를 복사한다. `build() &&`는 수명이 끝날 객체를 소비하므로 `std::move`로 자원을 결과에 넘길 수 있다. 이 정책을 API 표면에 드러내면 호출자가 성능과 이후 사용 가능성을 식만 보고 판단할 수 있다.

### 2. fluent 반환 참조와 객체 수명

`header(...) &&`가 반환하는 `RequestBuilder&&`는 새 객체를 소유하지 않고 기존 임시 builder를 가리킨다. 임시 객체는 전체 식의 세미콜론까지 살아 있으므로 바로 이어지는 `build() &&`는 안전하다. 그 참조를 저장해 다음 문장까지 사용하면 임시 파괴 뒤 dangling 참조가 될 수 있다.

완성된 `HttpRequest`는 문자열과 헤더 vector를 독립적으로 소유한다. `headers()`가 반환하는 `const std::vector<Header>&`는 복사를 피하는 관찰 참조지만 `HttpRequest`보다 오래 저장하면 안 된다.

### 3. 값 범주·복사·이동·복사 생략

| 식 | 값 범주 | 선택과 수명 의미 |
|---|---|---|
| `reusable` | lvalue | `header() &`, `build() const &`; builder를 보존하고 결과 데이터를 복사한다. |
| `RequestBuilder{std::string{"/orders"}}` | prvalue | 임시 builder를 현재 전체 식에서 직접 만든다. |
| `std::move(*this)` | xvalue | 객체를 새로 만들지 않고 `&&` 연쇄가 가능하도록 값 범주만 바꾼다. |
| `.header(...).build()`의 수신 객체 | xvalue | `build() &&`가 내부 문자열/vector를 이동 후보로 삼는다. |
| `HttpRequest{...}` 반환 | prvalue | 호출자의 `snapshot`/`one_shot`을 직접 초기화해 복사 생략될 수 있다. |
| `snapshot.headers()` | const lvalue 참조 | snapshot 내부 vector를 빌리며 소유권과 수명을 연장하지 않는다. |

이동된 표준 문자열과 vector는 파괴·재대입할 수 있는 유효 상태지만 내용은 미지정이다. 기계 수준에서는 포인터·크기 로드/저장, 조건 비교·분기, 함수 호출이 생길 수 있으나 작은 문자열 최적화나 inlining 등으로 모습이 바뀐다. 구체 명령은 CPU, ABI, 표준 라이브러리 구현, 컴파일러와 최적화 옵션에 따라 달라 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- 문제: [BOJ 5052 - 전화번호 목록](https://www.acmicpc.net/problem/5052)
- 출처: Baekjoon Online Judge, ICPC 계열 문제. 문제 요약·입출력·제약·예제는 `icpc_problem.cpp` 상단에 한글로 재서술했다.
- 핵심 알고리즘: [접두사 트라이](../algorithm/prefix-trie.md)
- 핵심 불변식: 루트에서 현재 노드까지의 경로는 지금까지 읽은 전화번호 접두사와 정확히 같다.
- 충돌 판정: 경로 도중 terminal을 만나면 기존 번호가 새 번호의 접두사이고, 끝 노드에 자식이 있으면 새 번호가 기존 번호의 접두사다.
- 복잡도: 모든 전화번호 길이 합을 `S`라 하면 시간 `O(S)`, 숫자 자식 10개를 가진 노드가 최대 `S+1`개라 공간 `O(S)`다.
- 검증: 공개 예제, 역순 접두사, 중복, 충돌 없는 공통 접두사, 한 자리 접두사를 CTest로 분리한다.

트라이는 대회 문자열 필수 지식이다. 온라인 접두사 질의, 자동 완성, 이진수 최대 XOR, Aho–Corasick의 기반으로 이어진다. 한 번의 일괄 판정에는 정렬 후 인접 문자열 비교도 가능하므로 두 풀이의 구현·시간·메모리 차이를 함께 알아야 한다.

## 오늘 사용한 표준 라이브러리

| 심볼·실제 호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할·호출 계약 요점 | 대표 문서 |
|---|---|---|---|---|
| `std::string` 생성·복사·이동 | `<string>` | 타입·생성자 | URL·본문·전화번호 문자를 소유한다. 복사는 문자를 복제하고 이동은 버퍼를 넘길 수 있으며 이동 원본 값은 미지정이다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `string::append("\n")`, `append(text)` | `<string>` | 멤버 함수 | 수신 문자열 끝에 리터럴/빌린 문자열 문자를 복사하고 `string&`를 반환한다. 재할당 시 기존 관찰자가 무효화된다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `string::size()` | `<string>` | 멤버 함수 | 인자 없이 길이를 `size_type`으로 반환하며 상태를 바꾸지 않는 `O(1)` 관찰이다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::vector<Header/TrieNode>` 생성 | `<vector>` | 클래스 템플릿·생성자 | 동적 연속 저장소와 원소 수명을 소유한다. TrieNode initializer-list 생성은 루트 하나를 만든다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::push_back(...)` | `<vector>` | 멤버 함수 | rvalue 원소를 끝에 이동해 크기를 1 늘리고 void를 반환한다. 재할당 시 기존 관찰자가 무효화되며 상각 `O(1)`이다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::size()` | `<vector>` | 멤버 함수 | 현재 원소 수를 반환하고 vector를 바꾸지 않는다. 트라이는 마지막 노드 인덱스 계산에 쓴다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::array<int, 10>`·`operator[]` | `<array>` | 클래스 템플릿·연산자 | 숫자 자식 10개를 객체 내부에 고정 저장한다. `[]`는 범위 검사가 없어 digit 계약이 0..9를 보장해야 한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| 범위 `for`의 `begin/end` | `<string>`·`<array>` | 반복자 연산 | 수신 범위를 빌려 각 문자/자식 값을 순회한다. 순회 중 해당 컨테이너를 구조 변경하지 않아 반복자가 유지된다. | [표준 용어](../standard-library/terms.md) |
| `std::move(expression)` | `<utility>` | 함수 템플릿 | lvalue를 xvalue로 바꾸며 실제 이동과 이동 후 상태는 선택된 생성자/호출이 결정한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)` | `<iostream>` | 정적 멤버 함수 | 동기화 설정을 끄고 이전 bool은 버린다. 이후 C/C++ stdio의 임의 혼용 순서를 보장하지 않는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::cin.tie(nullptr)` | `<iostream>` | 멤버 함수 | 입력 전 자동 flush 연결을 해제하고 이전 포인터는 버린다. 스트림 소유권은 변하지 않는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| 스트림 `operator>>`, `operator<<` | `<iostream>` | 연산자 | 대상 lvalue/출력 상태를 바꾸고 같은 스트림 참조를 연쇄 반환한다. 실패는 기본적으로 상태 비트에 남는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::size_t` | `<cstddef>` | 부호 없는 정수 타입 | 컨테이너 인덱스와 크기를 표현한다. 음수와 섞지 않고 명시 변환 전에 문제 범위를 확인한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |

오늘 심볼과 호출은 기존 대표 표준 라이브러리 문서가 이미 다루므로 중복 문서는 만들지 않고 날짜별 색인만 갱신한다.

## 빌드와 검증

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-08-21 -B dailystudy/exercise/2026-08-21/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-08-21/build
ctest --test-dir dailystudy/exercise/2026-08-21/build --output-on-failure
& dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all
```

`build/`는 생성 산출물이므로 커밋하지 않는다.

## 직접 해보기

1. `RequestBuilder reusable`에서 `build() const &`를 두 번 호출해 서로 독립적인 두 결과가 생기는지 확인한다.
2. `auto&& dangling = RequestBuilder{...}.header(...);`를 다음 문장에서 쓰면 왜 잘못인지 전체 식과 임시 수명으로 설명한다.
3. `build() const &`를 지우고 lvalue 호출의 컴파일 오류를 확인한 뒤 API가 어떤 사용 정책을 강제하는지 적는다.
4. Header vector의 재할당 전 원소 참조를 저장했다가 push_back 뒤 쓰는 코드가 왜 위험한지 설명한다.
5. BOJ 풀이에서 끝 노드 자식 검사를 제거하고 `12345`, `123` 반례를 실행한다.
6. 트라이 풀이를 정렬+인접 `starts_with` 검사로 다시 작성하고 복잡도와 메모리를 비교한다.
