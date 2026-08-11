# 2026-08-12 이해 점검

## Modern C++

- [ ] 생성자에 반환형이 없고 `explicit`이 단일 인자 암시 변환을 막는다고 설명할 수 있다.
- [ ] `struct`의 기본 접근은 `public`, `class`의 기본 접근은 `private`임을 코드에서 찾는다.
- [ ] `string_view`가 비소유라서 원본 문자열 수명보다 오래 쓰면 안 된다고 설명한다.
- [ ] `clock` lvalue, `std::move(clock)` xvalue, 함수 반환 임시 객체 prvalue를 구분한다.
- [ ] `unique_ptr` 이동 전후의 소유권과 객체 소멸 시점을 추적한다.
- [ ] 복사 생략이 반환 prvalue를 결과 객체에 직접 만들 수 있음을 설명한다.
- [ ] 가상 간접 호출의 기계 명령이 CPU·ABI·컴파일러·최적화에 따라 달라짐을 안다.

## ICPC

- 문제: BOJ 1806 **부분합** — <https://www.acmicpc.net/problem/1806>
- 알고리즘: [`../algorithm/two-pointers-sliding-window.md`](../algorithm/two-pointers-sliding-window.md)
- 불변식: `sum`은 현재 창의 합이며, 양수 조건 때문에 양 끝 이동의 합 변화 방향이 단조롭다.
- 복잡도: 시간 `O(N)`, 입력 공간 `O(N)`, 추가 공간 `O(1)`

## 검증 기록

- [x] w64devkit GCC 16.1.0으로 CMake 구성과 세 실행 파일 빌드 성공
- [x] `daily_main`, `daily_problem` 직접 실행 성공
- [x] CTest 5/5: 공식 예제·단일 원소·불가능 입력 포함 전체 통과
- [x] 모든 C++ 파일을 다시 읽고 코드 가까운 한글 주석을 확인
- [x] 알고리즘 링크가 실제 대표 문서를 가리키는지 확인
