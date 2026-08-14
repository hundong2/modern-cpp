# 2026-08-15 이해도 체크포인트

## Modern C++와 아키텍처

- [ ] `struct`의 기본 접근은 `public`, `class`의 기본 접근은 `private`임을 코드 위치로 설명한다.
- [ ] 생성자에는 반환형이 없고, `explicit`이 단일 인자 생성자의 뜻밖의 암시적 변환을 막는 이유를 말한다.
- [ ] `std::shared_lock`은 여러 조회자가 공유하고 `std::unique_lock`은 쓰기 작업이 단독 획득한다는 차이를 말한다.
- [ ] 잠금 객체의 소멸자가 잠금을 해제하므로 조기 반환과 예외에도 RAII가 유용함을 설명한다.
- [ ] `ConfigService`의 참조는 저장소를 소유하지 않으며, 저장소 수명이 더 길어야 함을 증명한다.
- [ ] `entry`는 lvalue, `std::move(value)`는 xvalue, `ConfigEntry{...}`는 prvalue임을 구분한다.
- [ ] 값 스냅샷 반환의 복사 비용과 잠금 밖 안전성 사이의 선택을 설명한다.
- [ ] 잠금의 기계 실행은 CPU·ABI·라이브러리·컴파일러에 따라 달라져 특정 명령으로 단정할 수 없음을 말한다.

## 실무 코드 검증

- [ ] `daily_main`이 `checkout=on version=2`를 출력하고 종료 코드 0을 반환한다.
- [ ] `daily_problem`이 `requests=3 errors=1`을 출력하고 종료 코드 0을 반환한다.
- [ ] `find("missing")`가 `std::nullopt`를 반환하는 테스트를 추가해 본다.
- [ ] 읽기 전용 함수가 `const`인데도 잠글 수 있도록 뮤텍스가 `mutable`인 이유를 설명한다.

## 오늘의 ICPC 문제

- 문제: [BOJ 2042 구간 합 구하기](https://www.acmicpc.net/problem/2042)
- 핵심 알고리즘: [펜윅 트리](../algorithm/fenwick-tree.md)
- [ ] `tree[i]`가 끝점 `i`인 길이 `lowbit(i)` 구간의 합이라는 불변식을 설명한다.
- [ ] 갱신에서 `i += lowbit(i)`, 누적 합에서 `i -= lowbit(i)`를 하는 이유를 그림으로 보인다.
- [ ] 대입 갱신을 `새 값 - 이전 값`의 차이 갱신으로 바꾸는 이유를 설명한다.
- [ ] 각 갱신·질의가 `O(log N)`, 전체 공간이 `O(N)`임을 인덱스의 최하위 1비트 변화로 증명한다.
- [ ] 공식 예제와 경계·음수 사례를 `ctest --test-dir build/2026-08-15 --output-on-failure`로 모두 통과시킨다.
