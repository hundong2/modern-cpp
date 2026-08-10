# 2026-08-11 체크포인트

## Modern C++ 이해 증명

- [ ] `std::optional<Record>`가 특별한 id나 null 포인터보다 실패를 명확히 표현하는 이유를 설명한다.
- [ ] lvalue `records`, `std::move(records)` xvalue, `Record{...}` prvalue가 각각 복사·이동·직접 생성에 어떤 영향을 주는지 설명한다.
- [ ] 비소유 `const T&`와 값을 소유하는 반환 객체의 수명 차이를 설명한다.
- [ ] `struct`의 기본 public과 `class`의 기본 private, public/private 접근 지정자의 역할을 코드에서 찾는다.
- [ ] 생성자에 반환형이 없고, `explicit`이 암시적 변환을 막으며 `{}` 직접 초기화는 허용하는 이유를 설명한다.
- [ ] 기반 클래스의 가상 소멸자와 `override`가 아키텍처 경계에서 필요한 이유를 설명한다.

## ICPC 검증

- 문제: BOJ 11286 **절댓값 힙** — <https://www.acmicpc.net/problem/11286>
- 알고리즘: `algorithm/priority-queue-custom-comparator.md`; 우선순위 키는 `(절댓값, 원본 값)`의 오름차순이다.
- 불변식: `heap.top()`은 저장된 값 중 절댓값이 가장 작고, 동률이면 원본 값이 가장 작다.
- 복잡도: 각 삽입·삭제 O(log N), top/empty O(1), 전체 O(N log N), 공간 O(N).
- [ ] 공식 예제에서 출력 순서를 손으로 추적한다.
- [ ] 빈 큐의 0 출력과 `-3, 3`, `-2, 2` 동률 순서를 확인한다.
- [ ] 모든 실행 파일을 빌드하고 CTest 5개가 통과하는지 확인한다.
