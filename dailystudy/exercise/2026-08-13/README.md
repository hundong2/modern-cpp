# 2026-08-13 Modern C++ 학습 자료

## 오늘의 목표

- `std::shared_ptr`의 공유 소유권과 `std::weak_ptr`의 비소유 관찰을 구분한다.
- `weak_ptr::lock()`이 반환한 임시 `shared_ptr`로 객체 수명을 안전하게 연장한다.
- lvalue, prvalue, xvalue, 참조 바인딩, 복사·이동·복사 생략을 실제 식과 연결한다.
- 아키텍처에서 소유 관계와 관찰 관계를 타입으로 드러내 순환 소유권을 피한다.

## 파일 안내

- `main.cpp`: `Session`을 공유 소유하고 `weak_ptr`로 안전하게 관찰한다.
- `problem.cpp`: 저장소를 소유하지 않는 서비스와 만료 뒤 동작을 직접 검증한다.
- `icpc_problem.cpp`: BOJ 1717 「집합의 표현」의 제출 가능한 완전한 풀이이다.
- `CHECKPOINT.md`: 개념·실행·알고리즘 이해를 증명하는 점검표다.

## 값 범주와 수명 읽기

`store` 같은 이름 있는 변수는 lvalue다. `std::move(name)`은 이름을 xvalue로 바꾸어 문자열 자원을 이동할 기회를 준다. `std::make_shared<Session>(...)`과 `reader.execute()`의 결과는 prvalue이며 목적 객체를 직접 초기화할 수 있어 복사 생략 대상이다. `const std::string&` 반환은 복사를 피하지만 참조 대상 객체보다 오래 보관하면 안 된다. `weak_ptr` 자체는 수명을 늘리지 않고, `lock()` 성공으로 얻은 `shared_ptr`만 해당 범위에서 수명을 연장한다.

## 오늘의 ICPC 문제

- 문제: [BOJ 1717 집합의 표현](https://www.acmicpc.net/problem/1717)
- 핵심 알고리즘: 분리 집합(Union-Find), 경로 압축, 랭크 기준 합치기
- 문서: [`../algorithm/disjoint-set-union.md`](../algorithm/disjoint-set-union.md)
- 복잡도: 초기화 O(n), m개 연산 O(m alpha(n)) 상각, 공간 O(n)
- 검증: 공식 예제 성격의 입력과 연결 사슬/분리 집합 입력을 CTest로 자동 실행한다.

## 빌드와 실행

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

직접 연습: `problem.cpp`에서 `weak_ptr`를 `shared_ptr`로 바꾼 뒤 `store.reset()` 이후 결과와 참조 횟수를 예상하고 확인하라. 이어 `icpc_problem.cpp`에서 경로 압축 대입을 제거하고 정답은 유지되지만 성능이 어떻게 달라지는지 설명하라.

