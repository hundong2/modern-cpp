# 2026-08-08 체크포인트

## Modern C++와 아키텍처

- [ ] `unique_ptr`가 복사 불가이고 이동 가능한 이유를 단일 소유권과 연결해 설명한다.
- [ ] lvalue `sink`, xvalue `std::move(sink)`, `make_unique`가 만든 prvalue를 구분한다.
- [ ] 기반 클래스의 가상 소멸자가 필요한 이유와 가상 간접 호출 비용이 구현 의존적임을 설명한다.
- [ ] `struct`/`class`의 기본 접근, `public`/`private`, 생성자와 멤버 초기화 목록, `explicit`의 역할을 코드에서 찾는다.
- [ ] 참조는 비소유 별칭이고 `unique_ptr`는 소유권이라는 차이를 설명한다.

## 오늘의 ICPC 문제

- 문제: BOJ 1654 **랜선 자르기** — <https://www.acmicpc.net/problem/1654>
- 핵심: 답에 대한 이분 탐색, 단조 결정 함수
- 문서: [`../algorithm/binary-search-on-answer.md`](../algorithm/binary-search-on-answer.md)
- 복잡도: 시간 `O(K log M)`, 공간 `O(K)`

## 정확성 검증

- [ ] `can_make(L)`이 참이면 모든 `1..L`도 참인 단조성을 설명한다.
- [ ] 반복 시작마다 `answer`가 지금까지 찾은 가능한 최대 길이라는 불변식을 설명한다.
- [ ] 가능한 `mid`에서는 오른쪽, 불가능한 `mid`에서는 왼쪽만 남겨도 답을 버리지 않음을 증명한다.
- [ ] 공식 예제의 결과가 `200`인지 손으로 확인한다.
- [ ] `1 1 / 1`과 같이 최소 길이가 1인 경계를 확인한다.

## 초보자 자기 검증

1. `/`가 정수 나눗셈임을 예로 설명한다.
2. `const auto& cable`에서 `const`, 참조, 타입 추론의 역할을 각각 말한다.
3. `low <= high`와 `mid = low + (high-low)/2`가 종료와 오버플로 방지에 어떻게 관여하는지 설명한다.
4. `long long` 없이 개수 합을 계산할 때 생길 수 있는 문제를 설명한다.
