# 2026-08-26 Modern C++ 학습 자료

오늘은 연속 메모리를 복사하지 않고 행·열 좌표로 읽는 C++23 `std::mdspan`을 배운다. `MetricBatch`가 `vector<int>` 저장소를 소유하고, `BatchAnalyzer`는 읽기 전용 2차원 뷰만 빌려 저장 책임과 계산 책임을 분리한다. 대회 문제는 BOJ 6549 「히스토그램에서 가장 큰 직사각형」을 단조 증가 스택으로 `O(n)`에 해결한다.

## 오늘의 목표

- `struct`의 기본 public과 `class`의 기본 private, 생성자·멤버 초기화 목록·`explicit`을 실제 코드에서 읽는다.
- `std::mdspan<T, Extents>`가 원소를 소유하지 않고 포인터·extent·매핑을 값으로 보관한다는 계약을 설명한다.
- `std::dextents<std::size_t, 2>`, `extent(axis)`, 다차원 `operator[]`의 인자·반환·범위 전제조건을 말한다.
- lvalue·prvalue·xvalue, vector 이동, mdspan 복사, 반환 복사 생략, 원본 재할당과 뷰 수명을 실제 식에 연결한다.
- 단조 스택의 증가 불변식, `start`의 의미, pop 순간 오른쪽 경계 확정과 `O(n)` 상각 분석을 증명한다.

## 생성 파일

- [`main.cpp`](main.cpp): 연속 측정값 소유자와 읽기 전용 2차원 분석 뷰를 분리한 실무 예제
- [`problem.cpp`](problem.cpp): 쓰기 가능한 `mdspan`으로 정사각 격자 업데이트를 적용하는 직접 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [BOJ 6549 - 히스토그램에서 가장 큰 직사각형](https://www.acmicpc.net/problem/6549)의 제출 가능한 완전 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일, 높은 경고 수준, CTest 8개 등록
- [`run_icpc_test.cmake`](run_icpc_test.cmake): ICPC 표준 입력·출력 비교 도우미
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·호출 계약·단조 스택 이해 검증
- [`../algorithm/monotonic-stack-largest-rectangle.md`](../algorithm/monotonic-stack-largest-rectangle.md): 단조 스택 공용 대표 문서

## Modern C++와 실무 아키텍처

`MetricBatch`는 행·열 수와 `vector<int>` 버퍼의 수명을 함께 소유한다. `BatchAnalyzer`는 `ConstMetricView`만 값으로 받아 저장 방식이나 할당 정책을 소유하지 않는다. 행마다 별도 vector를 두는 `vector<vector<int>>`와 달리 오늘 저장소는 한 연속 버퍼라서 할당이 하나이고 행 우선 순회에서 지역성이 좋다. 다만 `mdspan`은 크기 곱과 실제 버퍼 길이가 맞는지 검사하지 않으므로 소유 클래스가 `rows * columns == samples.size()` 불변식을 지켜야 한다.

`samples`와 `batch`는 이름 있는 lvalue다. `std::move(samples)`는 samples를 xvalue로 바꾸고, `MetricBatch`의 값 매개변수와 멤버 vector 이동 생성자가 버퍼 소유권을 옮길 기회를 준다. 이동 뒤 samples는 유효하지만 내용은 지정되지 않는다. `batch.view()`가 반환하는 mdspan은 prvalue이며 포인터와 extent만 복사해 Analyzer를 직접 초기화한다. 원소는 전혀 복사되지 않고, 뷰를 복사해도 Batch의 수명이 연장되지 않는다.

`view_[row, column]`은 매핑으로 오프셋을 계산하고 원소 참조를 반환한다. 소스 수준에서는 로드·비교·조건 분기·함수 호출로 이해할 수 있지만 실제 명령과 인라이닝 여부는 CPU, ABI, 표준 라이브러리 구현, 컴파일러와 최적화 옵션에 따라 달라진다. `mdspan`이 자동 경계 검사나 스레드 동기화를 추가한다고 가정하지 않는다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 6549 - 히스토그램에서 가장 큰 직사각형](https://www.acmicpc.net/problem/6549), Baekjoon Online Judge
- 핵심 알고리즘: 단조 증가 스택 ([`../algorithm/monotonic-stack-largest-rectangle.md`](../algorithm/monotonic-stack-largest-rectangle.md))
- 시간 복잡도: 테스트 케이스 하나 `O(n)`
- 공간 복잡도: 높이 배열과 단조 스택을 합쳐 `O(n)`
- 대회 필수 지식: “다음으로 더 작은 원소”가 나타나는 순간 스택에서 꺼낸 높이의 최대 확장 구간이 확정되며, 각 원소가 최대 한 번 push/pop되어 중첩 while도 전체 선형이다.
- 검증: BOJ 공개 예제, 단일 막대, 증가·감소 수열, 모두 0, 32비트 초과 넓이를 CTest로 비교한다.

## 오늘 사용한 표준 라이브러리

같은 심볼의 긴 일반 설명은 공용 대표 문서에 두고, 각 C++ 호출 바로 위에는 오늘 수신 객체·실제 인자·반환·상태 변화가 적혀 있다.

| 실제 타입·호출 | 선언 헤더 | 항목 종류 | 현재 코드에서의 역할 | 대표 문서 |
|---|---|---|---|---|
| `std::mdspan<T, std::dextents<std::size_t, 2>>` | `<mdspan>` | 클래스 템플릿·별칭 템플릿 | 연속 int 버퍼를 원소 복사 없이 실행 시간 2차원 크기로 바라본다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `mdspan(pointer, rows, columns)` | `<mdspan>` | 생성자 | 비소유 포인터와 두 extent를 값으로 보관해 기본 `layout_right` 매핑을 만든다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `mdspan::extent(axis)` | `<mdspan>` | const 멤버 함수 | 축 번호 1을 받아 열 수를 반환하고 뷰와 원본을 유지한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `mdspan::operator[](row,column)` | `<mdspan>` | 멤버 연산자 | 두 인덱스로 O(1) 원소 참조를 얻으며 호출자가 범위를 보장한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::vector<T>{...}` / `vector(count,value)` | `<vector>` | 클래스 템플릿·생성자 | 측정값·격자·높이·스택 원소의 연속 저장소와 수명을 소유한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::data()` | `<vector>` | 멤버 함수 | 소유 버퍼의 첫 포인터를 반환해 mdspan에 비소유로 전달한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::size()` / `empty()` | `<vector>` | const 멤버 함수 | 원소 수와 빈 상태를 관찰하며 컨테이너를 바꾸지 않는다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::reserve(count)` | `<vector>` | 멤버 함수 | 단조 스택 크기는 0인 채 용량을 확보해 재할당을 피한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::back()` / `pop_back()` | `<vector>` | 멤버 함수 | 마지막 미완료 막대를 복사한 뒤 제거해 오른쪽 경계를 확정한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `vector::push_back(value)` | `<vector>` | 멤버 함수 | 새 증가 높이와 최좌측 시작을 스택 끝에 소유한다. | [컨테이너와 뷰](../standard-library/containers-and-views.md) |
| `std::move(samples)` | `<utility>` | 함수 템플릿 | vector lvalue를 xvalue로 바꿔 버퍼 소유권 이동을 허용한다. | [입출력·유틸리티](../standard-library/io-parsing-and-utilities.md) |
| `std::max(best,area)` | `<algorithm>` | 함수 템플릿 | 두 long long을 const 참조로 비교해 큰 값을 best에 복사한다. | [알고리즘·ranges](../standard-library/algorithms-and-ranges.md) |
| `std::ios::sync_with_stdio(false)` | `<iostream>` | 정적 멤버 함수 | C/C++ 스트림 동기화를 끄고 이전 bool 반환은 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `std::cin.tie(nullptr)` | `<iostream>` | 멤버 함수 | 입력 전 cout 자동 flush 연결을 해제하고 이전 포인터는 버린다. | [입출력](../standard-library/io-parsing-and-utilities.md) |
| `operator>>` / `operator<<` | `<iostream>` | 비멤버 연산자 오버로드 | 입력 대상·출력 버퍼와 스트림 상태를 갱신하고 스트림 참조를 연쇄한다. | [입출력](../standard-library/io-parsing-and-utilities.md) |

## 빌드와 검증

```powershell
$kit = (Resolve-Path ../../../tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S . -B build -G "MinGW Makefiles" "-DCMAKE_CXX_COMPILER=g++.exe"
cmake --build build
ctest --test-dir build --output-on-failure
& ../tools/audit-standard-library-docs.ps1 -Scope all
```

예상 학습 프로그램 출력은 `main.cpp`가 `12 5 10 7`, `problem.cpp`가 `15`다. CTest 8개와 전체 표준 라이브러리 감사가 모두 통과해야 한다. `build/`는 생성 산출물이므로 커밋하지 않는다.

## 직접 해보기

1. `MetricBatch`보다 오래 사는 위치에 `ConstMetricView`를 저장해 보고 어떤 시점부터 댕글링인지 객체 파괴 순서로 설명한다.
2. view를 만든 뒤 소유 vector에 `push_back`해 재할당을 유도하고 기존 포인터·mdspan이 왜 무효인지 말한다.
3. `std::layout_left`를 명시한 별칭을 만들고 같은 버퍼의 좌표별 값이 어떻게 달라지는지 표로 추적한다.
4. 단조 스택에서 같은 높이를 매번 push하도록 바꾸고 정답과 복잡도, start 중복이 어떻게 달라지는지 분석한다.
5. 끝의 높이 0 센티널 반복을 제거하고 증가 수열 CTest가 왜 실패하는지 손으로 확인한다.
