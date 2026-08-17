# 2026-08-18 Modern C++ 학습 자료

## 오늘의 목표

- Pimpl(pointer to implementation)로 공개 인터페이스와 구현 의존성을 분리해 빌드 경계와 ABI 경계를 작게 유지한다.
- 불완전 타입을 `std::unique_ptr`로 소유할 때 소멸자와 특수 멤버 함수를 구현 파일에서 정의해야 하는 이유를 이해한다.
- 이동 전용 Pimpl과 깊은 복사가 가능한 값 의미 Pimpl을 비교하고 Rule of Five를 실제 코드에 적용한다.
- BOJ 11438을 통해 대회에서 필수적인 이진 리프팅, 트리 깊이 정렬, 조상 점화식과 불변식을 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): 이동 전용 `MessageFormatter` Pimpl과 컴파일 방화벽 예제
- [`problem.cpp`](problem.cpp): 깊은 복사, copy-and-swap, 강한 예외 보장을 갖는 `RetrySchedule` 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [BOJ 11438 LCA 2](https://www.acmicpc.net/problem/11438) 제출 가능한 완전한 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 경고 옵션, CTest 여섯 개 정의
- [`run_icpc_test.cmake`](run_icpc_test.cmake): 표준 입력 실행과 출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·소유권·표준 호출 계약·LCA 이해 검증
- [`../algorithm/binary-lifting-lca.md`](../algorithm/binary-lifting-lca.md): 이진 리프팅 LCA 공용 대표 문서

## Modern C++와 실무 아키텍처

Pimpl은 공개 클래스가 구현 클래스를 전방 선언하고 `unique_ptr<Impl>`만 보관하는 방식이다. 사용자는 구현 멤버를 볼 필요가 없어 구현 헤더 변경에 따른 재컴파일 범위를 줄일 수 있고, 공유 라이브러리에서는 공개 객체의 데이터 배치를 안정시키는 데 도움을 준다. 다만 동적 할당과 포인터 간접 접근 비용이 생기고 인라인 최적화 기회가 줄 수 있으므로 모든 작은 타입에 기계적으로 적용하지 않는다.

`main.cpp`의 `MessageFormatter`는 복사를 금지하고 이동만 허용한다. `formatter`는 lvalue, `std::move(formatter)`는 xvalue, `format()`이 반환하는 새 `std::string`은 prvalue다. 이동 뒤 원본은 파괴 가능한 유효 상태지만 구현 포인터가 비었으므로 다시 `format()`하지 않는다. `problem.cpp`의 복사 생성자는 새 Impl을 만들어 vector까지 깊게 복사하고, 복사 대입은 임시 복사본을 먼저 완성한 뒤 교환하여 실패 전 상태를 보존한다. 반환 prvalue는 목적 객체를 직접 초기화하는 복사 생략 대상이다.

생성자는 반환형이 없고 멤버 초기화 목록은 함수 본문 전에 멤버를 직접 초기화한다. `explicit`은 단일 인자 생성자를 통한 암시 변환을 막지만 `RetrySchedule{...}` 같은 직접 초기화는 허용한다. `class`의 기본 접근은 `private`, `struct`의 기본 접근은 `public`이며 `public`/`private` 지정자는 이후 선언의 접근 권한을 바꾼다.

Pimpl의 포인터 접근은 메모리 로드, 문자열 처리는 문자 load/store, 조건문은 비교와 분기를 만들 수 있다. 정확한 호출·간접 접근·복사·이동 명령은 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라지므로 특정 어셈블리 명령으로 단정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 11438 LCA 2](https://www.acmicpc.net/problem/11438), Baekjoon Online Judge
- 핵심 알고리즘: [이진 리프팅 최소 공통 조상](../algorithm/binary-lifting-lca.md)
- 복잡도: BFS와 조상 표 전처리 `O(N log N)`, 질의 하나 `O(log N)`, 전체 `O((N+M) log N)`, 공간 `O(N log N)`
- 대회 필수 지식: 트리 질의가 많을 때 `up[k][v] = up[k-1][up[k-1][v]]` 점화식으로 2의 거듭제곱만큼 점프한다. 먼저 두 정점 깊이를 같게 하고 큰 점프부터 서로 다른 조상으로 올리면 마지막 두 정점의 부모가 LCA라는 불변식을 유지한다. 경로 질의, 트리 DP, 거리 계산과 결합되는 대표 기반 기술이다.

## 오늘 사용한 표준 라이브러리

공통 계약은 [`../standard-library/README.md`](../standard-library/README.md)에서 시작하고 오늘 날짜 색인은 [`../standard-library/by-date.md`](../standard-library/by-date.md)에서 찾는다.

| 심볼·호출 | 선언 헤더 | 종류 | 현재 코드에서의 역할과 호출 계약 요점 | 대표 문서 |
|---|---|---|---|---|
| `std::unique_ptr<Impl>` | `<memory>` | 클래스 템플릿 | 불완전 Impl 한 개를 독점 소유하고 외부 소멸자 정의 시 정확히 한 번 파괴한다. | [소유권](../standard-library/ownership-and-vocabulary-types.md) |
| `std::make_unique<Impl>(arg)` | `<memory>` | 함수 템플릿 | arg를 전달해 Impl을 할당·생성하고 `unique_ptr<Impl>` prvalue를 반환하며 실패하면 예외가 난다. | [소유권](../standard-library/ownership-and-vocabulary-types.md) |
| `std::move(object)` | `<utility>` | 함수 템플릿 | lvalue를 xvalue로 바꾸어 문자열·vector·Pimpl 소유권 이동 오버로드를 선택할 기회를 준다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::swap(left.impl_, right.impl_)` | `<utility>` | 함수 템플릿 | 두 unique_ptr의 소유권을 `noexcept`, `O(1)`에 교환하고 pointee는 파괴하지 않는다. | [알고리즘](../standard-library/algorithms-and-ranges.md) |
| `std::string`, `operator+` | `<string>` | 클래스·연산자 | 문자를 소유하고 두 입력을 유지한 채 결합 문자열 prvalue를 선형 시간·공간에 만든다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::vector<T>` 생성자·`operator[]` | `<vector>` | 클래스 템플릿·멤버·연산자 | 연속 저장소를 소유하고 크기/값 생성자는 원소를 초기화하며 `[]`는 범위 검사 없이 참조를 반환한다. | [컨테이너](../standard-library/containers-and-views.md) |
| `vector::push_back(value)` | `<vector>` | 멤버 함수 | 끝에 값을 복사해 크기를 늘리고 용량 부족 재할당 시 모든 관찰자를 무효화할 수 있다. | [컨테이너](../standard-library/containers-and-views.md) |
| `std::queue<int>`, `push/front/pop/empty` | `<queue>` | 어댑터·멤버 함수 | BFS의 FIFO를 유지하며 `front/pop` 전에 비어 있지 않아야 하고 `pop`은 값을 반환하지 않는다. | [컨테이너](../standard-library/containers-and-views.md) |
| 범위 `for`의 `begin/end` | `<vector>` | 반복자 연산 | vector를 바꾸지 않는 동안 유효한 반복자로 모든 원소를 선형 순회한다. | [공통 용어](../standard-library/terms.md) |
| `std::ios::sync_with_stdio(false)`, `cin.tie(nullptr)` | `<iostream>` | 정적 함수·멤버 함수 | C/C++ 동기화와 입력 전 자동 flush 연결을 해제하며 이전 상태/연결 반환값은 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::cin >> value`, `std::cout << value` | `<iostream>` | 객체·연산자 | 같은 스트림 참조를 반환해 연쇄하고 입력은 대상 lvalue, 출력은 버퍼·상태를 변경한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-08-18 -B build/2026-08-18 -G "MinGW Makefiles"
cmake --build build/2026-08-18
ctest --test-dir build/2026-08-18 --output-on-failure
./build/2026-08-18/daily_main.exe
./build/2026-08-18/daily_problem.exe
powershell -ExecutionPolicy Bypass -File dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all
```

CTest는 두 학습 실행 파일의 종료 코드와 가지형·편향형·루트/동일 정점·공개 예제 LCA 출력을 검증한다. 완료 전에 세 C++ 파일을 다시 읽어 각 표준 호출의 수신 객체, 모든 인자, 반환값 사용, 상태 변화, 복잡도, 할당·무효화·수명·오류 계약 및 알고리즘 링크를 대조한다.

## 직접 해보기

1. `MessageFormatter`의 이동 생성자를 지우고 `std::move(formatter)` 줄의 컴파일 오류가 독점 소유권과 어떻게 연결되는지 설명한다.
2. `RetrySchedule` 복사 생성자에서 `make_unique` 대신 포인터만 공유하려 할 때 어떤 소유권 오류가 생기는지 말한다.
3. copy-and-swap을 직접 대입으로 바꾸고 vector 복사 중 예외가 나면 원래 객체에 필요한 보장이 무엇인지 비교한다.
4. `icpc_problem.cpp`에서 조상 표를 한 단계만 저장하도록 바꾸고 긴 편향 트리에서 질의 시간이 왜 `O(N)`이 되는지 측정한다.
5. 두 정점 거리 `depth[a] + depth[b] - 2 * depth[lca]`를 출력하는 새 함수를 구현하고 CTest를 추가한다.
