# 2026-08-18 이해 점검

## Modern C++ 문법·수명·아키텍처

- [ ] Pimpl이 공개 헤더의 구현 의존성과 재컴파일 범위를 줄이는 과정, 동적 할당·간접 접근이라는 비용을 함께 설명한다.
- [ ] `class Impl;`이 불완전 타입이어도 `unique_ptr<Impl>` 멤버를 선언할 수 있는 이유와 소멸자를 Impl 정의 뒤에 두는 이유를 말한다.
- [ ] 생성자에는 반환형이 없고 `explicit MessageFormatter(std::string)`이 막는 암시 변환과 허용하는 직접 초기화를 예로 든다.
- [ ] `class`와 `struct`의 기본 접근 차이, `public`/`private`, 멤버 변수와 멤버 초기화 목록의 역할을 오늘 코드에서 찾는다.
- [ ] `formatter` lvalue, `std::move(formatter)` xvalue, `format()` 결과 prvalue를 참조 바인딩·이동·복사 생략과 연결한다.
- [ ] 이동 뒤 원본 객체의 “유효하지만 값은 미지정” 계약과 객체 수명 종료 때 unique_ptr가 수행하는 RAII 정리를 설명한다.
- [ ] 이동 전용 Pimpl과 깊은 복사 Pimpl의 의미 차이, Rule of Five에서 명시한 다섯 특수 멤버를 나열한다.
- [ ] copy-and-swap이 자기 대입과 할당 실패에서 기존 값을 보존하는 순서를 설명한다.
- [ ] 포인터 간접 접근·함수 호출·분기가 기계 코드로 내려가도 CPU·ABI·컴파일러·최적화에 따라 실제 명령이 달라짐을 말한다.

## 표준 라이브러리 호출 계약

공용 문서: [표준 라이브러리 학습 지도](../standard-library/README.md)

- [ ] `std::make_unique<Impl>(std::move(prefix))`의 템플릿 인자, 실제 인자의 타입·값 범주·소유권, 반환형, 할당 실패와 호출 뒤 상태를 자료 없이 설명한다.
- [ ] `std::move(formatter)`가 직접 이동하지 않는 이유와 어느 생성자 호출이 실제 소유권을 바꾸는지 설명한다.
- [ ] `std::swap(left.impl_, right.impl_)`의 두 입력, void 반환, 교환 뒤 소유권·pointee 수명, 복잡도와 예외 계약을 설명한다.
- [ ] `impl_->delays_.push_back(milliseconds)`에서 수신 객체의 정확한 타입, 인자 복사, void 반환, 크기 변화, 재할당과 관찰자 무효화, 예외 보장을 설명한다.
- [ ] `for (const int& delay : impl_->delays_)`가 얻는 begin/end, 참조 수명, 빈 범위 동작과 복잡도를 설명한다.
- [ ] `pending.front()`와 `pending.pop()`의 전제조건, 반환값 차이, 제거 전 복사가 필요한 이유를 설명한다.
- [ ] `pending.empty()`의 수신 객체, 인자 없음, bool 반환, 상태 불변과 복잡도를 설명한다.
- [ ] `std::ios::sync_with_stdio(false)`와 `std::cin.tie(nullptr)` 각각의 입력, 반환값, 전역 스트림 상태 변화와 금지되는 사용을 설명한다.
- [ ] `std::cin >> first >> second`와 `std::cout << answer << '\n'`이 반환하는 스트림 참조, 입력/출력 상태 변화와 오류 계약을 설명한다.
- [ ] `ancestor[level][vertex]`의 `vector::operator[]`가 반환하는 참조, 범위 전제조건, 무검사 접근과 수명·무효화 조건을 설명한다.

## 오늘의 ICPC 문제

- 문제: [BOJ 11438 LCA 2](https://www.acmicpc.net/problem/11438)
- 대표 문서: [이진 리프팅 최소 공통 조상](../algorithm/binary-lifting-lca.md)
- 복잡도: 전처리 `O(N log N)`, 질의 `O(log N)`, 공간 `O(N log N)`

- [ ] `ancestor[k][v] = ancestor[k-1][ancestor[k-1][v]]` 점화식이 정확한 이유를 2^(k-1) 점프 두 번으로 증명한다.
- [ ] BFS가 루트 1에서 모든 정점의 깊이와 직계 부모를 정확히 한 번 정하는 이유를 트리의 유일 경로 성질로 설명한다.
- [ ] 깊이가 다른 두 정점을 먼저 같은 깊이로 만드는 이유와 큰 level부터 점프해도 정답을 건너뛰지 않는 이유를 말한다.
- [ ] 동시 점프 반복 중 “두 정점의 깊이는 같고 LCA는 두 정점보다 위에 있다”는 불변식을 설명한다.
- [ ] 반복 종료 뒤 `ancestor[0][first]`가 LCA인 이유를 두 정점은 다르지만 직계 부모는 같다는 사실로 증명한다.
- [ ] N과 M이 각각 100,000일 때 한 칸씩 부모를 따라가는 방법과 이진 리프팅의 최악 연산 수 차이를 비교한다.

## 초보자 검증 절차

1. [`main.cpp`](main.cpp), [`problem.cpp`](problem.cpp), [`icpc_problem.cpp`](icpc_problem.cpp)를 줄별로 읽고 각 `#include`, 기본 타입, `{}` 초기화, 함수, 분기, 반복, 컨테이너 호출의 바로 가까운 설명을 확인한다.
2. 각 표준 호출의 인자 수만큼 타입·값 범주·소유권 의미가 적혔고 반환값을 저장·연쇄·무시하는지 대조한다.
3. `4-5`, `4-3`, `2-4` 질의를 종이에 풀어 깊이 정렬과 동시 점프 결과가 각각 `2, 1, 2`인지 확인한다.
4. `../algorithm/binary-lifting-lca.md` 링크가 실제 대표 문서를 가리키고 새 중복 알고리즘 문서가 없는지 검사한다.
5. 경고 옵션으로 세 실행 파일을 빌드하고 `daily_main`은 `[prod] ready`, `daily_problem`은 `700 1500`을 출력하는지 확인한다.
6. CTest 여섯 개와 전체 표준 라이브러리 감사가 모두 통과해야 체크를 완료한다.
