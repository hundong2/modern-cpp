# 2026-08-20 Modern C++ 학습 자료

오늘은 실무 입력 모델과 유효한 도메인 모델을 분리한다. C++20 **지정 초기화(designated initialization)** 로 설정 DTO를 읽기 쉽게 만들고, `class`의 `private` 멤버·`explicit` 생성자·검증 서비스로 불변식이 깨진 객체가 핵심 로직 안에 들어오지 못하게 한다. 대회 문제는 BOJ 6086 「최대 유량」을 Edmonds–Karp로 해결한다.

## 오늘의 목표

- `struct` 집합체에 `.member = value` 형식으로 값을 지정하고, 지정자 순서와 생략 멤버의 규칙을 설명한다.
- 외부 입력 DTO와 도메인 객체를 분리해 “편한 생성”과 “항상 유효한 상태”라는 서로 다른 책임을 동시에 만족시킨다.
- `class`의 기본 `private`, `public` 접근 지정자, 멤버 변수, 반환형 없는 생성자, 멤버 초기화 목록을 코드에서 읽는다.
- `explicit`이 막는 암시 변환과 `ReplicaCount{3}` 같은 올바른 직접 초기화를 구분한다.
- lvalue·prvalue·xvalue, 참조 바인딩, 문자열 복사·이동, 객체 수명, 소유권, 복사 생략을 실제 식과 연결한다.
- 잔여 그래프, 역방향 간선, 증가 경로, 병목, 최대 유량-최소 컷 정리를 설명하고 Edmonds–Karp를 제출 가능한 코드로 구현한다.

## 생성 파일

- [`main.cpp`](main.cpp): 지정 초기화 DTO → 검증 서비스 → 불변 도메인 객체라는 실무 배포 계획 경계
- [`problem.cpp`](problem.cpp): 지정 초기화, `explicit` 직접 초기화와 클래스 캡슐화를 다시 쓰는 짧은 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): BOJ 6086의 완전한 Edmonds–Karp 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 높은 경고 수준, 공개·경계 CTest 등록
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·호출 계약·최대 유량을 자료 없이 설명하는 검증
- [`run_icpc_test.cmake`](run_icpc_test.cmake): 입력·출력과 종료 코드를 비교하는 CTest 보조 스크립트
- [`../algorithm/maximum-flow-edmonds-karp.md`](../algorithm/maximum-flow-edmonds-karp.md): 공용 최대 유량·Edmonds–Karp 대표 문서

## Modern C++와 실무 아키텍처

### 1. DTO는 편하게 만들고 도메인 객체는 유효하게 만든다

`RawDeployment`는 `struct` 집합체다. 외부 설정과 같은 단순 데이터는 다음처럼 필드 이름을 보면서 만들 수 있다.

```cpp
RawDeployment request{.service = "billing", .replicas = 3, .canary = true};
```

지정자는 멤버 **선언 순서**를 지켜야 하며, 이름을 잘못 쓰면 컴파일 오류다. 생략한 멤버는 멤버 기본 초기값을 사용한다. 집합체를 그대로 핵심 모델로 쓰면 `replicas = -7` 같은 상태도 쉽게 만들어지므로, 경계에서만 DTO로 사용한다.

`DeploymentPlanner::make`는 빈 서비스 이름과 `1..100` 밖의 복제 수를 거부한다. 성공한 뒤에는 `DeploymentPlan`의 `private` 멤버를 외부에서 직접 바꿀 수 없다. 이 분리는 실무의 JSON/YAML/API 요청 모델과 비즈니스 모델 사이에 자주 필요하다.

### 2. 생성자와 `explicit`

`ReplicaCount`의 생성자에는 반환형이 없고 `: value_{value}` 멤버 초기화 목록이 본문 전에 멤버를 직접 초기화한다. `explicit` 때문에 `ReplicaCount count = 3;`은 거부되고 `ReplicaCount count{3};`은 허용된다. 숫자 하나가 복제 수라는 의미를 갖는 지점을 코드에 드러내는 강한 타입이다.

### 3. 값 범주·참조·수명·소유권

| 식 | 값 범주 | 소유권·수명 의미 |
|---|---|---|
| `request`, `result`, `plan` | lvalue | 이름과 안정된 저장 위치가 있다. `plan`은 `result` 내부 성공값을 빌린다. |
| `RawDeployment{...}`, `DeploymentPlan{...}` | prvalue | 새 결과 객체를 계산하며 목적 객체를 직접 초기화해 복사를 생략할 수 있다. |
| `std::move(request)` | xvalue | `request`의 자원을 이동할 수 있음을 표현할 뿐, 실제 이동은 선택된 생성자가 수행한다. |
| `result.value()` | lvalue 참조 | `result` 내부 객체를 빌리며 `result`가 파괴·재대입되면 참조를 더 쓰면 안 된다. |
| `plan.service()` | `const std::string&` | 문자열 복사를 피하지만 `plan`보다 오래 저장할 수 없는 비소유 관찰자다. |

`make(std::move(request))` 뒤 원래 `request`는 파괴 가능한 유효 상태지만 문자열 값은 미지정이다. 성공 결과는 prvalue로 반환되어 `result` 내부에 직접 만들어질 수 있다. 컴파일러는 로드·저장·문자열 이동·조건 비교·분기·함수 호출을 만들거나 최적화로 없앨 수 있다. 구체 명령은 CPU, ABI, 표준 라이브러리 구현과 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- 문제: [BOJ 6086 - 최대 유량](https://www.acmicpc.net/problem/6086)
- 출처: Baekjoon Online Judge, 원 문제의 파이프 네트워크를 요약해 설명하고 원문 전체는 복제하지 않았다.
- 핵심 알고리즘: [최대 유량과 Edmonds–Karp](../algorithm/maximum-flow-edmonds-karp.md)
- 핵심 불변식: `0 <= flow(u,v) <= capacity(u,v)`와 유량 보존을 유지하고, 역방향 잔여 용량으로 이전 선택을 취소할 수 있다.
- 정답 근거: BFS 증가 경로가 더 없으면 잔여 그래프에서 도달 가능한 정점 집합이 컷을 만들고, 현재 유량이 그 컷 용량과 같아 최대다.
- 복잡도: Edmonds–Karp `O(VE^2)` 시간, 용량·유량 행렬과 인접 목록 `O(V^2+E)` 공간. 이 문제는 `V=52`, `N<=700`이다.
- 대회 필수 지식: 최대 유량은 최소 컷, 이분 매칭, 간선/정점 분할, 서로소 경로와 이어진다. 단순히 탐욕적으로 경로에 흘리고 끝내지 말고 **역방향 잔여 간선**으로 과거 선택을 되돌릴 수 있어야 한다.

검증은 공개 예제, 평행 간선 누적, 소문자 정점, 두 갈래 경로를 CTest로 나눠 수행한다. 각 테스트는 종료 코드와 정규화한 전체 출력을 함께 비교한다.

## 오늘 사용한 표준 라이브러리

| 심볼·실제 호출 | 선언 헤더 | 종류 | 현재 코드에서의 역할·호출 계약 요점 | 대표 문서 |
|---|---|---|---|---|
| `std::string` 생성 | `<string>` | 타입·생성자 | 문자열 리터럴을 복사해 서비스 이름을 소유하며 할당 실패가 가능하다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `string::empty()` | `<string>` | 멤버 함수 | 인자 없이 크기 0 여부를 `bool`로 반환하며 문자열 상태를 바꾸지 않는 `O(1)` 관찰이다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::move(request)` / `std::move(service)` | `<utility>` | 함수 템플릿 | lvalue를 xvalue로 변환하고 소유권 이동 가능성을 표현한다. 실제 이동과 이동 후 상태는 선택된 생성자가 결정한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::expected<DeploymentPlan, PlanError>` 생성 | `<expected>` | 클래스 템플릿·생성자 | 성공 객체 또는 오류 하나를 소유하며 반환 prvalue로 `result`를 직접 초기화한다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `std::unexpected<PlanError>{error}` | `<expected>` | 클래스 템플릿·생성자 | 오류 값을 소유해 `expected`의 오류 대안을 선택하게 한다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `expected::operator bool()` | `<expected>` | 연산자 | 성공 여부를 반환하고 수신 객체를 바꾸지 않는 `O(1)` 관찰이다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `expected::value()` / `error()` | `<expected>` | 멤버 함수 | 활성 대안의 참조를 반환한다. 잘못된 대안 접근 계약과 반환 참조의 수신 객체 종속 수명을 구분한다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `std::array`와 `operator[]` | `<array>` | 클래스 템플릿·연산자 | 52개 정점 행렬을 고정 저장한다. `[]`는 범위 검사를 하지 않아 알고리즘 불변식이 인덱스를 보장해야 한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::vector<int>` 생성 | `<vector>` | 클래스 템플릿·생성자 | 각 정점의 이웃을 소유하는 빈 동적 배열을 만든다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::push_back(to)` | `<vector>` | 멤버 함수 | 값을 복사해 크기를 1 늘린다. 재할당 시 그 vector의 기존 반복자·참조가 무효화되고 상각 `O(1)`이다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| 범위 `for`의 `begin/end` | `<array>`·`<vector>` | 반복자 연산 | 컨테이너를 빌려 순회한다. 순회 중 구조를 바꾸지 않아 반복자 수명을 유지한다. | [표준 용어](../standard-library/terms.md) |
| `std::queue<int>` 생성 | `<queue>` | 컨테이너 어댑터·생성자 | 빈 FIFO 대기열과 기반 컨테이너를 소유한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `queue::push/empty/front/pop` | `<queue>` | 멤버 함수 | 삽입, 빈 상태 관찰, 첫 원소 참조, 제거를 수행한다. `front/pop`은 비어 있지 않아야 하고 `pop`은 값을 반환하지 않는다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::min(bottleneck, residual)` | `<algorithm>` | 함수 템플릿 | 두 `int` lvalue를 빌려 작은 값의 참조를 반환하고 즉시 복사해 병목을 갱신한다. `O(1)`이다. | [알고리즘과 ranges](../standard-library/algorithms-and-ranges.md) |
| `std::numeric_limits<int>::max()` | `<limits>` | 클래스 템플릿·정적 함수 | 상태 없이 `int` 최댓값을 반환해 병목 초기값으로 쓴다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::ios::sync_with_stdio(false)` | `<iostream>` | 정적 멤버 함수 | 전역 스트림 동기화 설정을 바꾸고 이전 상태는 버린다. C stdio 혼용 순서를 더는 보장하지 않는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::cin.tie(nullptr)` | `<iostream>` | 멤버 함수 | 입력 전 자동 flush 연결을 해제하고 이전 포인터는 버린다. 스트림 소유권은 바뀌지 않는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| 스트림 `operator>>` / `operator<<` | `<iostream>` | 연산자 | 입력 대상 lvalue를 갱신하거나 출력 상태를 바꾸고 같은 스트림 참조를 연쇄 반환한다. I/O 실패는 상태 비트로 남는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |

공용 표준 라이브러리 설명은 기존 대표 문서에서 모두 찾을 수 있으므로 오늘은 중복 문서를 만들지 않는다.

## 빌드와 검증

저장소에 포함된 w64devkit을 PowerShell에서 사용한다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-08-20 -B dailystudy/exercise/2026-08-20/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-08-20/build
ctest --test-dir dailystudy/exercise/2026-08-20/build --output-on-failure
& dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all
```

`build/`는 생성 산출물이므로 커밋하지 않는다.

## 직접 해보기

1. `RawDeployment`에 `.region = "ap-northeast-2"`를 선언 순서에 맞게 추가하고 `DeploymentPlan`이 소유하도록 바꾼다.
2. 지정자 순서를 일부러 바꿔 컴파일 오류를 확인한 뒤 왜 C++ 지정 초기화가 선언 순서를 요구하는지 설명한다.
3. `ReplicaCount count = 3;`이 실패하고 `ReplicaCount count{3};`이 성공하는지 확인한다.
4. `make(request)`와 `make(std::move(request))`에서 문자열 복사·이동과 호출 뒤 `request` 상태 차이를 적는다.
5. ICPC 풀이에서 역방향 `flow[vertex][previous] -= bottleneck`을 제거한 구현이 일반 최대 유량에서 왜 틀릴 수 있는지 반례를 그린다.
6. BOJ 풀이를 용량 행렬 없이 `Edge{to, reverse, capacity}` 인접 간선 구조로 다시 구현해 본다.
