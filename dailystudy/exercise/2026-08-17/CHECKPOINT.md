# 2026-08-17 이해 점검

## Modern C++ 문법과 객체 수명

- [ ] `struct`의 기본 접근은 `public`, `class`의 기본 접근은 `private`임을 오늘 코드의 실제 타입으로 설명한다.
- [ ] 생성자에는 반환형이 없고, 생성자 매개변수와 멤버 초기화 목록이 각각 어떤 역할인지 설명한다.
- [ ] `explicit AtomicConfigStore(shared_ptr<...>)`가 막는 암시 변환과 올바른 중괄호 직접 초기화 예를 쓴다.
- [ ] `std::atomic<std::shared_ptr<const AppConfig>>`의 각 템플릿 층과 `const`의 적용 대상을 설명한다.
- [ ] `initial`은 lvalue, `std::move(initial)`은 xvalue, `AppConfig{...}`는 prvalue임을 실제 식과 연결한다.
- [ ] `std::move` 자체는 이동하지 않고 이동 생성·대입이 선택될 수 있는 xvalue를 만든다는 점을 말한다.
- [ ] `shared_ptr` 복사와 이동이 참조 횟수 및 원본 포인터 상태에 미치는 차이를 설명한다.
- [ ] 반환 prvalue의 복사 생략과 `shared_ptr`가 보장하는 객체 수명을 구분한다.
- [ ] `RetryService`의 참조 멤버가 비소유이며 저장소보다 오래 살아서는 안 되는 이유를 설명한다.

## 동시성과 실무 아키텍처

- [ ] 불변 설정을 새 객체로 만들어 한 번에 게시하면 제자리 변경보다 추론하기 쉬운 이유를 말한다.
- [ ] release 저장 이전의 객체 초기화와 acquire 로드 이후의 읽기 사이에 필요한 공개 관계를 설명한다.
- [ ] `memory_order_relaxed`도 원자성은 보장하지만 다른 메모리 작업의 순서를 전달하지 않음을 설명한다.
- [ ] 두 relaxed 카운터를 따로 읽은 스냅샷이 하나의 전역 시점을 뜻하지 않는 반례를 든다.
- [ ] 포트 `IConfigSnapshotSource`와 어댑터 `AtomicConfigStore`를 분리했을 때 테스트와 구현 교체가 쉬워지는 이유를 말한다.
- [ ] 가상 함수 호출, 원자 load/store, 조건 분기의 실제 기계 명령은 CPU·ABI·컴파일러·최적화에 따라 달라짐을 설명한다.

## 오늘의 ICPC 문제

- 문제: [UVa 11402 Ahoy, Pirates!](https://onlinejudge.org/index.php?Itemid=8&option=com_onlinejudge&page=show_problem&problem=2397)
- 핵심 문서: [지연 전파 세그먼트 트리](../algorithm/lazy-segment-tree.md)
- 시간·공간: 빌드 `O(N)`, 명령당 `O(log N)`, 전체 `O(N + Q log N)`, 공간 `O(N)`
- [ ] `ones_[node]`와 `lazy_[node]`가 각각 어떤 사실을 항상 보존하는지 불변식으로 말한다.
- [ ] 완전 포함 갱신에서 자식까지 내려가지 않아도 부모의 구간 합이 정확한 이유를 설명한다.
- [ ] 일부 포함 질의 전에 `push`가 필요한 이유와 완전 포함 질의에는 필요 없는 이유를 말한다.
- [ ] 대입 뒤 반전과 반전 뒤 대입의 합성 결과가 다른 예를 직접 계산한다.
- [ ] `invert + invert = none`, `set_zero + invert = set_one`인 이유를 0/1 값 하나로 증명한다.
- [ ] 한 구간이 `O(log N)`개의 표준 구간으로 분해되어 명령당 복잡도가 `O(log N)`인 이유를 설명한다.
- [ ] 트리 배열을 `4*N`으로 잡는 이유와 `std::vector`가 그 메모리를 소유하는 방식을 설명한다.

## 초보자 검증 절차

1. [`main.cpp`](main.cpp), [`problem.cpp`](problem.cpp), [`icpc_problem.cpp`](icpc_problem.cpp)를 한 줄씩 읽고 설명 대상 코드 바로 위에 한글 주석이 있는지 확인한다.
2. 세 파일의 모든 `#include`, 기본 타입, 변수, `{}` 초기화, 연산자, 분기, 반복, 함수, 컨테이너와 표준 라이브러리 호출의 목적을 말한다.
3. ICPC 구현 근처의 `../algorithm/lazy-segment-tree.md`가 실제 대표 문서로 연결되는지 연다.
4. `00101` 예제에서 네 질의 답 `2, 4, 3, 1`을 배열을 직접 바꾸며 계산한다.
5. CMake 경고 옵션으로 세 실행 파일을 빌드하고 CTest 다섯 개를 모두 통과시킨다.
6. `daily_main`이 `retry_limit=5`, `daily_problem`이 `accepted=2, rejected=1`을 출력하는지 확인한다.
7. 자료를 보지 않고 `compose` 표와 `apply/push/update/query` 네 역할을 빈 종이에 다시 작성한다.
