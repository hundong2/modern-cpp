# 2026-08-22 Modern C++ 학습 자료

오늘은 실무 레지스트리에서 식별자만 바꿔야 할 때 C++17 `std::unordered_map` **노드 핸들**을 사용한다. `extract`로 원소 소유권을 잠시 컨테이너 밖으로 옮기고, `node.key()`를 변경한 뒤 `insert`로 되돌려 mapped 객체의 불필요한 복사를 피한다. 대회 문제는 BOJ 11404 「플로이드」를 모든 쌍 최단 경로 동적 계획법으로 해결한다.

## 오늘의 목표

- `struct`의 기본 public과 `class`의 기본 private 접근 차이를 실제 레코드·서비스 역할과 연결한다.
- `using Table = ...`, `explicit` 생성자, 값 매개변수, 멤버 초기화 목록을 읽는다.
- `extract → node.key()/mapped() → insert`의 소유권과 상태 변화를 호출별로 설명한다.
- lvalue, prvalue, `std::move`가 만드는 xvalue, 이동 후 유효하지만 미지정인 상태를 실제 식에서 찾는다.
- 플로이드–워셜의 `k` 단계 불변식과 점화식을 증명하고 `O(N³)`/`O(N²)`를 계산한다.

## 생성 파일

- [`main.cpp`](main.cpp): 세션 ID를 노드 핸들로 안전하게 재지정하는 실무 레지스트리
- [`problem.cpp`](problem.cpp): 같은 패턴을 자산 카탈로그에서 직접 읽고 변형하는 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [BOJ 11404 플로이드](https://www.acmicpc.net/problem/11404)의 제출 가능한 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일과 6개 CTest 등록
- [`run_icpc_test.cmake`](run_icpc_test.cmake): 표준 입력·출력·종료 코드 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·소유권·표준 호출·알고리즘 이해 검증
- [`../algorithm/floyd-warshall-all-pairs-shortest-path.md`](../algorithm/floyd-warshall-all-pairs-shortest-path.md): 공용 플로이드–워셜 대표 문서

## Modern C++와 실무 아키텍처

### 1. 레지스트리가 변경 규칙을 소유한다

`Session`은 이름과 세대 번호를 묶은 값 레코드라 `struct`가 자연스럽다. `SessionRegistry`는 키 충돌 검사, 누락 처리, 노드 추출과 재삽입 순서를 지켜야 하므로 `class`의 private `sessions_` 뒤에 상태를 감춘다. 호출자는 `rename`과 `find`만 사용해 중간 상태를 만들 수 없다.

노드 핸들은 캐시 키·세션 ID·라우팅 이름처럼 **mapped 객체는 그대로 두고 연관 컨테이너의 키만 바꾸는 유지보수 작업**에 유용하다. 일반적인 `erase + emplace`는 mapped 값을 새 원소로 복사/이동해야 하고, 둘 사이에서 소유권이 코드 밖으로 흩어진다. 노드 핸들은 분리된 원소를 이동 전용 값으로 명시해 그 소유권을 한 객체가 추적한다.

### 2. 호출별 상태 전이

| 식 | 입력 상태 | 출력·호출 뒤 상태 |
|---|---|---|
| `sessions_.contains(new_id)` | map이 원소를 소유하고 `new_id`는 함수의 string lvalue | bool만 반환하며 map과 문자열은 유지된다. |
| `sessions_.extract(old_id)` | 원본 키가 map에 있거나 없다 | 있으면 map 크기가 1 줄고 반환 node가 key/mapped를 소유한다. 없으면 빈 node다. |
| `node.key() = std::move(new_id)` | 비어 있지 않은 node, 이름 있는 `new_id` lvalue | `std::move` 결과는 xvalue이고 새 키가 node에 소유된다. `new_id`는 유효하지만 값은 미지정이다. |
| `++node.mapped().generation` | node가 Session을 소유한다 | `mapped()`의 lvalue 참조를 통해 같은 Session의 정수 저장값만 1 증가한다. |
| `sessions_.insert(std::move(node))` | node가 원소를 독점 소유한다 | 성공 시 map이 다시 소유하고 node는 빈다. 결과는 성공 여부·위치·실패 노드를 묶는다. |

컨테이너는 동시 접근을 자동으로 직렬화하지 않는다. 오늘 예제의 사전 `contains`와 후속 `insert` 사이에 다른 스레드가 같은 map을 변경한다면 데이터 경쟁이며 검사-행동 원자성도 없다. 공유 레지스트리라면 외부 mutex 또는 단일 소유 실행기 같은 동기화 경계가 필요하다.

### 3. 값 범주·참조 바인딩·수명

- `initial`과 `sessions_`는 이름 있는 **lvalue**다.
- `std::string{"alpha"}`와 `Session{...}`는 새 목적 객체를 초기화하는 **prvalue**다.
- `std::move(initial)`과 `std::move(node)`는 자원 이전 가능성을 표현하는 **xvalue**다. `std::move` 자체는 이동하지 않고 선택된 이동 생성자·오버로드가 실제 소유권을 옮긴다.
- `sessions_.extract(old_id)`의 node_type 반환값은 지역 `node`를 직접 초기화할 수 있어 복사 생략 또는 이동으로 이어진다. node_type은 복사할 수 없고 이동으로만 소유권을 넘긴다.
- `find`가 반환한 `const Session*`은 비소유 포인터다. Registry를 파괴하거나 해당 원소를 삭제한 뒤에는 댕글링한다. 재해시는 반복자를 무효화하지만 `unordered_map` 원소의 포인터·참조는 일반적으로 유지된다. 연산별 계약을 확인해야 한다.
- 이동된 `initial`, `new_id`, `node`는 파괴·재대입 가능한 유효 상태지만, 표준이 구체 값을 약속하지 않는 곳에서는 내용을 가정하지 않는다.

기계 실행 관점에서는 해시 계산, 버킷 포인터 추적, 키 비교, 조건 분기, 노드 링크 분리·연결, 문자열 포인터/크기 이동이 생길 수 있다. 실제 load·store·호출·분기와 inlining 여부는 CPU, ABI, 표준 라이브러리 구현, 컴파일러와 최적화 옵션에 따라 달라 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 11404 플로이드](https://www.acmicpc.net/problem/11404), Baekjoon Online Judge
- 핵심 알고리즘: [플로이드–워셜 모든 쌍 최단 경로](../algorithm/floyd-warshall-all-pairs-shortest-path.md)
- 점화식: `dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j])`
- 불변식: `k` 반복이 끝나면 중간 도시로 `1..k`만 허용한 모든 `i→j` 최소 비용이 저장된다.
- 복잡도: 시간 `O(N³)`, 거리 행렬 공간 `O(N²)`
- 검증: 공개 예제, 병렬 간선, 중간 도시 경유 개선, 단일 도시 경계를 CTest로 분리한다.

플로이드–워셜은 대회에서 반드시 알아야 할 모든 쌍 최단 경로의 기준 알고리즘이다. `N`이 수백이고 질의가 모든 쌍이면 구현과 증명이 단순하다. 병렬 간선에 `min`을 적용하는 초기화, `k`를 가장 바깥에 두는 반복 순서, INF 덧셈 방지는 자주 틀리는 필수 포인트다.

## 오늘 사용한 표준 라이브러리

| 심볼·실제 호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할·호출 계약 요점 | 대표 문서 |
|---|---|---|---|---|
| `std::unordered_map<std::string, T>` 생성·이동 | `<unordered_map>` | 클래스 템플릿·생성자 | 해시 키와 값을 노드로 소유한다. 값 매개변수에서 Registry 멤버로 이동해 소유권 경계를 만든다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `unordered_map::contains(key)` | `<unordered_map>` | const 멤버 함수 | key를 const 참조로 빌려 평균 `O(1)`에 bool을 반환하고 수신 map을 바꾸지 않는다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `unordered_map::extract(key)` | `<unordered_map>` | 멤버 함수 | 일치 원소를 map에서 분리해 이동 전용 `node_type`으로 반환한다. 성공 시 크기가 1 줄고 원소 소유권이 node로 이동한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `node_type::empty()/key()/mapped()` | `<unordered_map>` | 멤버 함수 | 소유 여부를 관찰하고, 비어 있지 않은 노드의 키·mapped 값에 lvalue 참조로 접근한다. 참조 수명은 노드/재삽입 원소에 묶인다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `unordered_map::insert(std::move(node))` | `<unordered_map>` | 멤버 함수 | node 소유권을 map에 이전하고 `insert_return_type`을 반환한다. 성공 시 node가 비고 재해시는 반복자를 무효화할 수 있다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `unordered_map::emplace(key, value)` | `<unordered_map>` | 멤버 함수 템플릿 | key/value prvalue를 원소 생성으로 전달한다. 평균 `O(1)`, pair 반환은 오늘 코드에서 무시하고 재해시·할당 가능성이 있다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `unordered_map::find(key)`, `end()` | `<unordered_map>` | const 멤버 함수 | const_iterator를 반환해 비소유 조회를 한다. map은 유지되고 재해시·삭제 뒤 반복자 수명에 주의한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::string` 생성·비교·이동 대입 | `<string>` | 타입·생성자·연산자 | ID/파일명 문자를 소유한다. 비교는 문자를 읽고, node 키 대입은 new ID 자원을 이동할 수 있다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::move(expression)` | `<utility>` | 함수 템플릿 | lvalue를 xvalue로 바꾸며 실제 이동과 이동 후 상태는 선택된 생성자·대입·insert가 결정한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::vector<T>(count, value)`·`operator[]` | `<vector>` | 클래스 템플릿·생성자·연산자 | 거리 행렬의 행과 값을 복사 소유한다. `[]`는 범위 검사가 없어 1..N 인덱스 계약을 코드가 보장한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::min(a, b)` | `<algorithm>` | 함수 템플릿 | 두 const lvalue를 빌려 작은 값의 const 참조를 반환하며 직접 간선·경유 비용 갱신에 즉시 복사한다. | [알고리즘과 ranges](../standard-library/algorithms-and-ranges.md) |
| `std::numeric_limits<long long>::max()` | `<limits>` | 클래스 템플릿·정적 멤버 함수 | 표현 가능한 최댓값을 반환하며 안전한 INF 산술의 기준을 만든다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)`, `std::cin.tie(nullptr)` | `<iostream>` | 정적·일반 멤버 함수 | 표준 스트림 전역 동기화와 자동 flush 연결을 바꾸고 이전 반환값은 버린다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::cin`, `std::cout`과 스트림 `operator>>`, `operator<<` | `<iostream>` | 객체·연산자 | 입력 대상/출력 상태를 바꾸고 같은 스트림 참조를 연쇄 반환한다. 실패는 기본적으로 상태 비트에 남는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::size_t` | `<cstddef>` | 부호 없는 정수 타입 | 1..N의 검증된 int 인덱스를 vector의 `size_type` 계열로 명시 변환한다. 음수는 변환 전에 배제한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |

노드 핸들 호출 계약은 기존 `containers-and-views.md`의 `unordered_map` 대표 섹션을 보강해 관리한다. 같은 컨테이너 문서를 새로 만들지 않는다.

## 빌드와 검증

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-08-22 -B dailystudy/exercise/2026-08-22/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-08-22/build
ctest --test-dir dailystudy/exercise/2026-08-22/build --output-on-failure
& dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all
```

`build/`는 생성 산출물이므로 커밋하지 않는다.

## 직접 해보기

1. `rename("missing", "new")`을 추가해 빈 node가 원본 map을 바꾸지 않는지 확인한다.
2. `rename("beta", "occupied")`에서 목적지 선검사를 지우고 `insert_return_type.node`가 무엇을 소유하는지 추적한다.
3. `node.key() = std::move(new_id)` 뒤 new_id가 반드시 빈 문자열이라고 가정하면 왜 잘못인지 설명한다.
4. `find` 반환 포인터를 Registry 파괴 뒤 사용하지 못하는 이유를 객체 수명으로 설명한다.
5. BOJ 풀이에서 병렬 간선 `min`을 단순 대입으로 바꾸고 CTest 반례가 실패하는지 확인한다.
6. `via` 반복을 가장 안쪽으로 옮긴 뒤 어떤 불변식이 깨지는지 말한다.
