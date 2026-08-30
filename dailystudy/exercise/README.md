# Modern C++ 날짜별 연습 안내

이 폴더는 하루에 하나의 작은 프로그램을 빌드·실행하면서 문법, 객체 수명, 설계 경계를 함께 익히는 과정입니다. 날짜 폴더 안에서 `README.md → main.cpp → problem.cpp → CHECKPOINT.md` 순서로 학습하세요.

| 날짜 | 핵심 주제 | C++ 표준 |
|---|---|---:|
| [2026-07-14](./2026-07-14/README.md) | 입력 파싱, `span`, `string_view`, `expected`, RAII, concept | C++23 |
| [2026-07-15](./2026-07-15/README.md) | 명령 라우팅, 오류 모델, RAII, concept | C++23 |
| [2026-07-16](./2026-07-16/README.md) | `variant`, 방문자, 명령·이벤트 아키텍처 | C++20 |
| [2026-07-17](./2026-07-17/README.md) | 저장소 인터페이스, 다형성, `optional`, 복사·이동 | C++20 |
| [2026-07-18](./2026-07-18/README.md) | 강한 타입, `enum class`, `expected`, 검증 서비스 | C++23 |
| [2026-07-19](./2026-07-19/README.md) | RAII, `unique_ptr`, 소유권, 의존성 주입 | C++20 |
| [2026-07-20](./2026-07-20/README.md) | `variant` 상태 모델, `visit`, `if constexpr` | C++17 |
| [2026-07-21](./2026-07-21/README.md) | `expected`, 명시적 오류 계약, 계층형 처리 파이프라인 | C++23 |
| [2026-07-22](./2026-07-22/README.md) | `span`, 비소유 범위, concept 경계 | C++20 |
| [2026-07-23](./2026-07-23/README.md) | `jthread`, `stop_token`, 협력적 취소 | C++20 |
| [2026-07-24](./2026-07-24/README.md) | 코루틴 프레임, 핸들 소유권, 지연 시퀀스 | C++20 |
| [2026-07-25](./2026-07-25/README.md) | ranges/views 지연 파이프라인 | C++20 |
| [2026-07-26](./2026-07-26/README.md) | concept 기반 정책 계약 | C++20 |
| [2026-07-27](./2026-07-27/README.md) | 삼방향 비교, `tuple`, 정렬 모델 | C++20 |
| [2026-07-28](./2026-07-28/README.md) | `shared_ptr`, `weak_ptr`, 순환 소유권 | C++20 |
| [2026-07-29](./2026-07-29/README.md) | `source_location`, 진단 포트 | C++20 |
| [2026-07-30](./2026-07-30/README.md) | `chrono` 강한 시간 타입, 시계 주입 | C++20 |
| [2026-07-31](./2026-07-31/README.md) | 수학 상수, 정책 객체, 소유권 주입 | C++20 |
| [2026-08-01](./2026-08-01/README.md) | `string_view`, 명령 파싱 수명 경계 | C++20 |
| [2026-08-02](./2026-08-02/README.md) | `optional`, 인터페이스 주입, 다익스트라 | C++20 |
| [2026-08-03](./2026-08-03/README.md) | 완벽 전달, `variant`, 다익스트라 복습 | C++20 |
| [2026-08-04](./2026-08-04/README.md) | RAII 소유권과 크루스칼 MST | C++20 |
| [2026-08-05](./2026-08-05/README.md) | `std::pmr`, 메모리 자원, 그래프 연결 요소 | C++20 |
| [2026-08-06](./2026-08-06/README.md) | `std::function` 파이프라인, 위상 정렬 | C++20 |
| [2026-08-07](./2026-08-07/README.md) | `variant` 상태 모델과 격자 BFS | C++20 |
| [2026-08-08](./2026-08-08/README.md) | `unique_ptr` 의존성 주입, 답 이분 탐색 | C++20 |
| [2026-08-09](./2026-08-09/README.md) | `span` 서비스 경계와 상태 DP | C++20 |
| [2026-08-10](./2026-08-10/README.md) | ranges 정렬, 이동, LIS | C++20 |
| [2026-08-11](./2026-08-11/README.md) | `optional`, 참조 래퍼, 사용자 비교 힙 | C++20 |
| [2026-08-12](./2026-08-12/README.md) | RAII 타이머, 시계 포트, 슬라이딩 윈도우 | C++20 |
| [2026-08-13](./2026-08-13/README.md) | 공유·약한 소유권과 Union-Find | C++20 |
| [2026-08-14](./2026-08-14/README.md) | Strategy, `unique_ptr`, 다익스트라 | C++20 |
| [2026-08-15](./2026-08-15/README.md) | 읽기-쓰기 잠금과 펜윅 트리 | C++20 |
| [2026-08-16](./2026-08-16/README.md) | `filesystem`, 오류 코드, 코사라주 SCC | C++20 |
| [2026-08-17](./2026-08-17/README.md) | atomic 불변 스냅샷과 지연 세그먼트 트리 | C++20 |
| [2026-08-18](./2026-08-18/README.md) | Pimpl·Rule of Five와 이진 리프팅 LCA | C++20 |
| [2026-08-19](./2026-08-19/README.md) | `bit_cast`·엔디언 와이어 경계와 KMP 문자열 검색 | C++23 |
| [2026-08-20](./2026-08-20/README.md) | 지정 초기화 DTO·도메인 불변식 경계와 Edmonds–Karp 최대 유량 | C++23 |
| [2026-08-21](./2026-08-21/README.md) | 참조 한정 fluent builder와 접두사 트라이 | C++20 |
| [2026-08-22](./2026-08-22/README.md) | unordered_map 노드 핸들 키 재지정과 플로이드–워셜 | C++20 |
| [2026-08-23](./2026-08-23/README.md) | `std::latch` 시작 준비 게이트와 Bellman–Ford | C++20 |
| [2026-08-24](./2026-08-24/README.md) | `move_only_function` 이동 전용 작업 큐와 0-1 BFS | C++23 |
| [2026-08-26](./2026-08-26/README.md) | `mdspan` 다차원 비소유 분석 뷰와 단조 스택 | C++23 |
| [2026-08-27](./2026-08-27/README.md) | `barrier` 반복 단계 동기화와 증강 경로 이분 매칭 | C++20 |
| [2026-08-28](./2026-08-28/README.md) | `osyncstream` 레코드 경계와 최근접 점 분할 정복 | C++20 |
| [2026-08-29](./2026-08-29/README.md) | semaphore handoff·RAII permit과 접미 배열·Kasai LCP | C++20 |
| [2026-08-30](./2026-08-30/README.md) | `condition_variable` blocking queue와 Aho–Corasick 다중 검색 | C++20 |
| [2026-08-31](./2026-08-31/README.md) | `std::format` 표현 계층과 Heavy-Light Decomposition | C++20 |

## 표준 라이브러리 설명 찾기

- [표준 라이브러리 학습 지도](./standard-library/README.md): 설명 품질 기준과 분야별 대표 문서
- [날짜별 사용 색인](./standard-library/by-date.md): 현재 48개 날짜, 125개 C++ 파일의 헤더와 `std::` 심볼
- [심볼 색인](./standard-library/symbol-index.md): 127개 현재 심볼의 역할과 상세 문서 링크
- [공통 용어](./standard-library/terms.md): 반복자, 뷰, 술어, 소유권, 무효화, 예외 보장, 원자성
- [표준 헤더](./standard-library/headers.md): 현재 사용하는 51개 헤더와 대표 선언

코드의 인접 주석은 현재 호출의 인자·반환값·상태 변화를 설명한다. 공용 문서는 전제조건, 복잡도, 객체 수명, 참조·반복자 무효화, 오류·예외 계약을 설명한다. 새 심볼은 다음 감사로 누락 여부를 검사한다.

```powershell
& dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all
```

## 공통 학습 순서

1. README의 아키텍처와 예상 출력을 읽습니다.
2. `main.cpp`의 한글 주석을 따라 객체의 생성·복사·이동·소멸을 표시합니다.
3. 그날의 표준 심볼을 날짜별 색인에서 찾아 매개변수·반환값·복잡도·무효화 규칙을 확인합니다.
4. 실행 전에 출력과 assert 결과를 종이에 예측합니다.
5. 로컬 w64devkit으로 구성·빌드하고 CTest와 표준 라이브러리 문서 감사를 실행합니다.
6. `problem.cpp`를 일부 가린 뒤 직접 다시 작성합니다.
7. CHECKPOINT를 자료 없이 풀고, 실기 변경까지 통과해야 다음 날짜로 넘어갑니다.

## 전체 검증

저장소 루트의 PowerShell에서 날짜별로 아래 명령을 실행합니다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
$dates = Get-ChildItem dailystudy/exercise -Directory | Where-Object Name -Match '^\d{4}-\d{2}-\d{2}$'
foreach ($date in $dates) {
    cmake -S $date.FullName -B "$($date.FullName)/build" -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
    cmake --build "$($date.FullName)/build"
    ctest --test-dir "$($date.FullName)/build" --output-on-failure
}
& dailystudy/exercise/tools/audit-standard-library-docs.ps1 -Scope all
```

`build/`는 생성 파일이므로 Git에 커밋하지 않습니다.

## 값 범주를 읽는 최소 기준

- **lvalue**: 이름과 식별 가능한 저장 위치가 있는 식입니다. 이름 있는 변수는 타입이 `T&&`여도 식으로 사용하면 lvalue입니다.
- **prvalue**: 값을 계산하거나 새 객체를 초기화하는 순수한 값입니다. `T{...}`와 대부분의 값 반환 함수 호출이 해당합니다.
- **xvalue**: 수명이 끝나기 전 자원을 이전할 수 있는 객체를 나타냅니다. `std::move(object)` 결과가 대표적입니다.
- `std::move`는 실제 데이터를 옮기지 않고 식을 xvalue로 변환합니다. 실제 이동 여부는 선택된 생성자·대입 연산자가 결정합니다.
- 어셈블리는 소스 코드와 일대일 대응하지 않습니다. CPU, ABI, 컴파일러, 디버그/릴리스와 최적화 옵션에 따라 load·store·분기·호출이 사라지거나 합쳐질 수 있습니다.
