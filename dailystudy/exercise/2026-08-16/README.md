# 2026-08-16 Modern C++ 학습 자료

## 오늘의 목표

- 실무에서 문자열 대신 `std::filesystem::path`로 경로를 표현하고, 경로 결합·확장자 조회·정규화를 타입 안전하게 수행한다.
- `std::error_code`를 받는 파일 시스템 API로 예상 가능한 I/O 실패를 예외 없이 호출 경계에서 처리한다.
- 파일 시스템 접근을 `DirectoryCatalog`에 모아 서비스의 책임과 운영체제 I/O 경계를 분리한다.
- 방향 그래프의 강결합 요소(SCC)를 코사라주 알고리즘의 두 번의 DFS로 `O(V+E)`에 구한다.

## 생성 파일

- [`main.cpp`](main.cpp): `std::filesystem::directory_iterator`와 `std::error_code`를 사용하는 실무형 파일 카탈로그
- [`problem.cpp`](problem.cpp): 절대 경로와 `..` 탈출을 거부하는 업로드 상대 경로 검증 연습
- [`icpc_problem.cpp`](icpc_problem.cpp): [BOJ 2150 Strongly Connected Component](https://www.acmicpc.net/problem/2150) 제출 가능 풀이
- [`CMakeLists.txt`](CMakeLists.txt): 세 실행 파일과 CTest 검증 정의
- [`CHECKPOINT.md`](CHECKPOINT.md): 문법·수명·파일 경계·SCC 이해를 증명하는 점검표
- [`../algorithm/strongly-connected-components-kosaraju.md`](../algorithm/strongly-connected-components-kosaraju.md): 코사라주 SCC 공용 알고리즘 문서

## Modern C++와 실무 아키텍처

`std::filesystem::path`는 운영체제별 경로 구분과 결합 규칙을 캡슐화한다. `DirectoryCatalog`는 루트 경로를 값으로 소유하고 디렉터리 순회라는 I/O 세부 사항을 한곳에 둔다. 호출자는 `FileRecord` 값 목록만 받으므로 `directory_entry`의 수명이나 반복자 상태를 밖으로 노출하지 않는다. `std::error_code` 오버로드는 “권한 없음, 사라진 파일”처럼 예상 가능한 운영 실패를 호출자가 명시적으로 분기하게 한다.

`problem.cpp`는 외부 입력 경로를 저장 루트와 결합하기 전에 절대 경로와 `..` 구성 요소를 거부한다. 이는 기본적인 lexical 방어이며, 실제 보안 경계에서는 심볼릭 링크·junction·TOCTOU와 권한도 별도로 검증해야 한다.

이름 있는 `root`, `records`는 lvalue다. `std::move(root)`는 xvalue로 바꾸어 `path` 내부 자원 이동의 후보가 되게 한다. `FileRecord{...}`와 함수 반환 벡터는 prvalue이며 결과 객체에 직접 생성되거나 이동될 수 있다. `const path&`는 복사 없이 기존 객체에 바인딩하지만 소유하지 않으므로 원본보다 오래 저장하면 안 된다. 디렉터리 순회에는 시스템 호출, 메모리 로드·저장·비교·조건 분기·함수 호출이 포함될 수 있지만 정확한 명령과 호출 방식은 운영체제, 표준 라이브러리, CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.

## 오늘의 ICPC 문제

- ID·제목·출처: [BOJ 2150 Strongly Connected Component](https://www.acmicpc.net/problem/2150), Baekjoon Online Judge
- 핵심 알고리즘: 코사라주 강결합 요소 분해, 공용 문서 [`../algorithm/strongly-connected-components-kosaraju.md`](../algorithm/strongly-connected-components-kosaraju.md)
- 왜 필수인가: SCC는 “서로 왕복 가능한 정점”을 하나로 압축해 일반 방향 그래프를 DAG로 바꾸는 대회 핵심 도구다. 2-SAT, 의존성 순환 탐지, 상태 공간 압축의 기반이 된다.
- 복잡도: 정방향·역방향 그래프 생성과 두 번의 DFS는 `O(V+E)`, 문제의 정렬 출력은 최악 `O(V log V)`, 저장 공간은 `O(V+E)`다.
- 검증: 공식 예제 형태, 단일 SCC, 간선 없는 정점들을 CTest에서 실제 출력과 비교한다.

## 빌드와 실행

저장소 루트에서 다음을 실행한다.

```powershell
$kit = (Resolve-Path tools/w64devkit/bin).Path
$env:Path = "$kit;$env:Path"
cmake -S dailystudy/exercise/2026-08-16 -B build/2026-08-16 -G "MinGW Makefiles"
cmake --build build/2026-08-16
ctest --test-dir build/2026-08-16 --output-on-failure
```

## 직접 해보기

1. `main.cpp`에서 확장자가 `.cpp`인 파일만 남기고, 대소문자 구분 정책을 어느 계층이 가져야 하는지 설명한다.
2. `problem.cpp`에 `sub/../safe.txt`를 넣어 현재 정책이 왜 거부하는지 확인하고, 정규화 후 허용하는 정책과 장단점을 비교한다.
3. SCC 예제의 첫 DFS 종료 순서를 손으로 적고, 그 역순으로 역그래프 DFS를 시작해야 하는 이유를 설명한다.
4. SCC를 각각 한 정점으로 압축한 그래프에 사이클이 생기면 왜 모순인지 증명한다.
