# 2026-08-31 Modern C++ 학습 자료

오늘은 C++20 `std::format`으로 도메인 값과 표현 문자열의 경계를 분리한다. `DeploymentFormatter`와 `StoragePresenter`가 형식 정책을 소유하고, 입력 `struct`는 값만 운반한다. 대회 문제는 [BOJ 13510 - 트리와 쿼리 1](https://www.acmicpc.net/problem/13510)을 Heavy-Light Decomposition(HLD)과 세그먼트 트리로 해결한다.

## 오늘의 목표

- 문자열 더하기 대신 컴파일 시간 형식 검사를 받는 `std::format` 호출의 입력·반환·할당·오류 계약을 설명한다.
- `struct` 기본 `public`과 `class` 기본 `private`, `explicit` 생성자, 멤버 초기화 목록을 실제 코드에서 구분한다.
- lvalue·prvalue·xvalue, 참조 바인딩, 복사·이동, 소유권, 수명, 복사 생략을 오늘 식과 연결한다.
- HLD의 light 간선 절반 감소 불변식과 간선 값을 자식 위치에 저장하는 규칙을 증명한다.
- 간선 갱신 `O(log N)`, 경로 최댓값 `O(log² N)`, 전체 공간 `O(N)`을 구현과 테스트로 검증한다.

## 생성 파일

- [`main.cpp`](main.cpp): `std::format` 기반 배포 진단 formatter와 batch service
- [`problem.cpp`](problem.cpp): 용량 snapshot을 표현하는 직접 연습 예제
- [`icpc_problem.cpp`](icpc_problem.cpp): BOJ 13510 제출 가능한 재귀 없는 HLD 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 경고 옵션, CTest 검증
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·표준 호출 계약·HLD 이해 검증
- [`run_icpc_test.cmake`](run_icpc_test.cmake): 표준 입력과 정확한 출력을 비교하는 CTest 도우미
- [`../algorithm/heavy-light-decomposition.md`](../algorithm/heavy-light-decomposition.md): HLD 대표 공용 문서

## Modern C++와 실무 아키텍처

`DeploymentEvent`는 외부 입력에 가까운 DTO라 `struct`의 공개 필드를 사용한다. `DeploymentFormatter`는 환경 문자열을 `private`로 소유하고 유효한 형식만 반환한다. 서비스가 formatter를 값으로 소유하므로 호출자가 사라져도 표현 정책의 수명이 안전하다. 실제 운영 코드에서는 형식 결과를 로그 sink, HTTP adapter, metric exporter에 넘기며 도메인 계층이 `std::cout`에 직접 의존하지 않게 한다.

`DeploymentFormatter{std::string{"prod"}}`는 explicit 생성자를 직접 초기화한다. `std::string{"prod"}`는 prvalue이고, 값 매개변수 `environment`는 함수 안에서 이름이 있으므로 lvalue다. `std::move(environment)`가 그 식을 xvalue로 바꾸어 `environment_`의 이동 생성을 허용한다. `render()`가 반환하는 `std::string` prvalue는 호출자의 변수나 `vector` 원소를 직접 초기화하거나 이동한다. 컴파일러는 언어가 허용하는 복사 생략을 적용할 수 있지만, 구체적인 buffer 이동·할당은 구현과 최적화에 따라 달라진다.

기계 실행 관점에서 format은 인자를 읽는 load, 자리표시자 처리, 결과 buffer 저장과 함수 호출을 포함할 수 있다. 조건문은 분기로, vector 확장은 할당과 기존 원소 이동으로 구현될 수 있다. 하지만 실제 명령과 가상 호출 여부는 CPU·ABI·표준 라이브러리 구현·컴파일러·최적화 옵션에 따라 사라지거나 합쳐질 수 있으므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 13510 - 트리와 쿼리 1](https://www.acmicpc.net/problem/13510), Baekjoon Online Judge
- 핵심 알고리즘: [Heavy-Light Decomposition](../algorithm/heavy-light-decomposition.md) + 반복형 최대 세그먼트 트리
- 핵심 불변식: light 간선을 내려가면 서브트리 크기가 절반 이하가 되므로 한 경로는 `O(log N)`개 heavy 체인 구간으로 나뉜다.
- 간선 표현: 부모-자식 간선의 값을 자식 정점 `position`에 저장해 간선 번호 갱신을 한 점 갱신으로 바꾼다.
- 복잡도: 전처리 `O(N)`(세그먼트 초기 점 갱신 포함 구현은 `O(N log N)`), 갱신 `O(log N)`, 경로 질의 `O(log² N)`, 공간 `O(N)`
- 대회 필수 이유: 동적 트리 경로 합·최대·최소·XOR와 lazy 갱신 문제로 확장되는 ICPC 핵심 패턴이다. 다만 정적 질의면 LCA+prefix처럼 더 단순한 풀이를 먼저 검토한다.

## 오늘 사용한 표준 라이브러리

| 실제 타입·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할·호출 계약 요약 | 대표 문서 |
|---|---|---|---|---|
| `std::format(fmt, args...)` | `<format>` | 함수 템플릿 | 형식 문자열과 참조 전달 인자를 읽고 새 소유 `string`을 반환한다. 원본은 유지되고 결과 길이만큼 시간·공간·할당이 들 수 있다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::string` 생성·이동 | `<string>`, `<utility>` | 타입·생성자·함수 템플릿 | 환경/결과 문자를 소유하며 `std::move`는 이름 있는 매개변수를 xvalue로 바꾼다. 이동 원본은 유효하지만 값은 미지정이다. | [문자열·컨테이너](../standard-library/containers-and-views.md), [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::string_view` 생성·읽기 | `<string_view>` | 타입·생성자 | 문자열 리터럴을 비소유로 보며 호출 뒤 리터럴 수명은 유지된다. 일반 문자열을 볼 때는 원본 수명이 더 길어야 한다. | [문자열·컨테이너](../standard-library/containers-and-views.md) |
| `std::vector(count, value)` | `<vector>` | 클래스 템플릿·생성자 | N+1개 배열과 인접 리스트를 소유한다. 원소 수에 선형 초기화·공간, 할당 실패 가능성이 있다. | [문자열·컨테이너](../standard-library/containers-and-views.md) |
| `vector::reserve(count)` | `<vector>` | 멤버 함수 | size는 유지하고 capacity를 확보한다. 재할당하면 기존 포인터·참조·반복자가 무효화된다. | [문자열·컨테이너](../standard-library/containers-and-views.md) |
| `vector::assign(count, value)` | `<vector>` | 멤버 함수 | 기존 원소를 파괴하고 count개 값으로 교체한다. 반환은 void, O(count)이며 기존 관찰자는 모두 무효화된다. | [문자열·컨테이너](../standard-library/containers-and-views.md) |
| `vector::push_back(value)` | `<vector>` | 멤버 함수 | 끝에 값을 복사/이동해 size를 1 늘린다. 분할 상환 O(1), 재할당 시 무효화와 예외 가능성이 있다. | [문자열·컨테이너](../standard-library/containers-and-views.md) |
| `vector::empty/back/pop_back` | `<vector>` | 멤버 함수 | 빈 상태를 검사한 뒤 마지막 원소를 읽고 제거한다. `back/pop_back`은 비어 있으면 UB이며 O(1)이다. | [문자열·컨테이너](../standard-library/containers-and-views.md) |
| `vector::size()` | `<vector>` | 멤버 함수 | 수신 vector를 바꾸지 않고 `size_type` 원소 수를 O(1)에 반환한다. | [문자열·컨테이너](../standard-library/containers-and-views.md) |
| `std::max(a, b)` | `<algorithm>` | 함수 템플릿 | 두 값을 비교해 큰 값의 참조를 반환하며 입력은 유지된다. HLD/세그먼트 최대 결합은 O(1)이다. | [알고리즘·ranges](../standard-library/algorithms-and-ranges.md) |
| `std::pair<int,int>` 생성·`first/second` | `<utility>` | 클래스 템플릿·생성자·멤버 | 체인 시작/머리와 간선 양 끝점을 값으로 소유한다. 두 int의 수명은 pair와 같다. | [소유권·어휘 타입](../standard-library/ownership-and-vocabulary-types.md) |
| `std::ios::sync_with_stdio(false)` | `<iostream>` | 정적 멤버 함수 | 동기화 설정을 바꾸고 이전 bool을 반환하지만 무시한다. 이후 C stdio와 섞지 않는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::cin.tie(nullptr)` | `<iostream>` | 멤버 함수 | 자동 flush 연결을 해제하고 이전 ostream 포인터를 반환하지만 무시한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| 스트림 `operator>>`, `operator<<` | `<iostream>` | 연산자 함수 | 입력 대상/출력 값을 참조로 처리하고 같은 스트림 참조를 반환한다. 스트림 위치·상태가 바뀌며 실패는 상태 비트에 남는다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

저장소 루트의 PowerShell에서 실행한다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-08-31 -B dailystudy/exercise/2026-08-31/build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build dailystudy/exercise/2026-08-31/build
ctest --test-dir dailystudy/exercise/2026-08-31/build --output-on-failure
powershell -ExecutionPolicy Bypass -File dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all
```

CTest는 두 학습 실행 파일과 네 HLD 사례를 확인한다. 추가 검증은 작은 무작위 트리에서 매 쿼리마다 BFS로 실제 경로를 복원해 최댓값을 구하는 brute force와 비교하고, N=100,000 일자/별 트리로 수명·인덱스·스택 경계를 확인한다. `build/`는 생성 산출물이라 커밋하지 않는다.

## 직접 해보기

1. `std::format` 자리표시자 하나를 지우고 컴파일 시간 형식 검사가 어떤 진단을 내는지 확인한다.
2. `DeploymentFormatter formatter = std::string{"prod"};`가 explicit 때문에 실패하고 직접 초기화는 성공하는 이유를 말한다.
3. `events`의 `string_view`가 지역 `string`을 가리키게 바꾼 뒤 원본이 먼저 파괴되는 위험을 그린다.
4. HLD 코드의 마지막 `+ 1`을 제거해 LCA의 부모 간선이 잘못 포함되는 반례를 만든다.
5. 최댓값을 경로 합으로 바꾸고 같은 무작위 테스트를 통과시킨다.
6. CHECKPOINT를 자료 없이 풀고 각 표준 호출의 인자 수만큼 값 범주·소유권·상태 변화를 말한다.
