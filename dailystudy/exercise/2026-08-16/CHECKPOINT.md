# 2026-08-16 이해 점검

## Modern C++ 문법과 수명

- [ ] `struct FileRecord`의 멤버가 기본 `public`이고 `class DirectoryCatalog`의 멤버가 기본 `private`인 차이를 설명한다.
- [ ] 생성자에는 반환형이 없으며 `explicit DirectoryCatalog(path)`가 단일 인자 암시적 변환을 막는 이유를 말한다.
- [ ] 생성자 매개변수 `root`는 값으로 받고 멤버 초기화 목록에서 `std::move(root)` 하는 방식의 복사·이동 비용을 설명한다.
- [ ] 이름 있는 `root`는 lvalue, `std::move(root)`는 xvalue, `FileRecord{...}`는 prvalue임을 실제 식과 연결한다.
- [ ] `const std::filesystem::path&`가 객체를 소유하지 않으며 참조 대상보다 오래 보관하면 안 되는 이유를 말한다.
- [ ] 반환 벡터가 복사 생략 또는 이동될 수 있고 지역 원소의 수명과 반환 객체 수명이 어떻게 이어지는지 설명한다.

## 실무 파일 시스템 경계

- [ ] `std::filesystem::path`와 문자열 이어 붙이기의 차이를 설명한다.
- [ ] 예외 오버로드 대신 `std::error_code&` 오버로드를 선택한 이유와 오류 확인 시점을 찾는다.
- [ ] 반복 중 파일이 삭제될 수 있으므로 `directory_entry` 조회도 실패할 수 있음을 설명한다.
- [ ] lexical 경로 검사가 심볼릭 링크·junction·TOCTOU 공격까지 해결하지 못한다는 한계를 말한다.
- [ ] 실제 실행의 시스템 호출·로드·비교·분기 형태를 특정 어셈블리 명령으로 단정할 수 없는 이유를 설명한다.

## 오늘의 ICPC 문제

- 문제: [BOJ 2150 Strongly Connected Component](https://www.acmicpc.net/problem/2150)
- 핵심 알고리즘: [코사라주 강결합 요소](../algorithm/strongly-connected-components-kosaraju.md)
- 시간·공간: DFS 자체 `O(V+E)`, 출력 정렬 포함 최악 `O(V log V)`, 공간 `O(V+E)`
- [ ] 첫 DFS 종료 순서의 역순이 SCC 원천 컴포넌트부터 방문하게 하는 이유를 말한다.
- [ ] 역그래프 DFS 한 번이 정확히 하나의 SCC만 모은다는 불변식을 설명한다.
- [ ] 모든 정점이 정확히 한 컴포넌트에 들어감을 `visited` 배열과 바깥 반복문으로 증명한다.
- [ ] 재귀 DFS의 호출 스택 위험과 반복형 DFS 대안을 설명한다.

## 검증 과정

1. 세 C++ 파일을 다시 읽고 각 `#include`, 타입, 변수, 초기화, 연산자, 분기, 반복, 함수, 컨테이너 호출 가까이에 한글 주석이 있는지 확인한다.
2. `icpc_problem.cpp`의 알고리즘 링크가 실제 공용 문서로 연결되는지 확인한다.
3. w64devkit로 세 실행 파일을 빌드하고 학습 실행 파일을 실행한다.
4. CTest의 공식 예제 형태·단일 SCC·고립 정점 검증을 모두 통과시킨다.
5. 경고 옵션 빌드에서 새 경고가 없는지 확인한다.
