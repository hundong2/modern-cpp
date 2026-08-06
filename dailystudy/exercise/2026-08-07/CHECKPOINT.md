# 2026-08-07 이해 점검

## C++ 문법과 객체 수명

- [ ] `std::variant<Idle, Running, Failed>`가 동시에 한 상태만 보관한다는 뜻을 설명한다.
- [ ] `struct`와 `class`의 기본 접근 차이, `public`/`private`의 역할을 코드 위치와 연결한다.
- [ ] 생성자에 반환형이 없는 이유와 `explicit Job(int)`, 생성자 매개변수, 멤버 초기화 목록을 설명한다.
- [ ] `job`, `Running{0}`, `std::move(name)`을 각각 lvalue, prvalue, xvalue로 구분한다.
- [ ] const 참조 바인딩, 포인터 null 검사, 복사·이동·복사 생략, 활성 variant 객체 수명을 설명한다.

## 아키텍처와 실행 관점

- [ ] 상태 타입이 잘못된 필드 조합을 막는 방식을 정수 상태 코드와 비교한다.
- [ ] `std::visit` 방문자가 상태별 정책을 분리하는 방식을 설명한다.
- [ ] 로드·저장·비교·조건 분기·함수 호출의 구체 명령이 환경과 최적화에 의존함을 말한다.

## ICPC: BOJ 2178 미로 탐색

- [ ] [`../algorithm/breadth-first-search-unweighted-grid.md`](../algorithm/breadth-first-search-unweighted-grid.md)의 BFS 층 불변식을 설명한다.
- [ ] `distance == 0`이 미방문을 뜻하고 시작 거리가 1이어야 하는 이유를 설명한다.
- [ ] 최초 발견 거리가 최단인 정확성 근거와 시간 `O(NM)`, 공간 `O(NM)`을 증명한다.
- [ ] 공식 예제의 답 15, 단일 칸 답 1, 열린 2×2 답 3을 직접 실행해 확인한다.

## 초보자 검증

- [ ] 방향 배열 네 쌍을 종이에 적고 각 인덱스가 어느 이웃을 만드는지 확인한다.
- [ ] 경계, 벽, 방문 완료 조건 하나씩 제거했을 때 생기는 실패를 예측한다.
- [ ] CMake 빌드와 CTest 5개가 모두 통과하는지 확인한다.
