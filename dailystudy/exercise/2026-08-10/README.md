# 2026-08-10 Modern C++ 학습

## 오늘의 주제

`std::ranges`로 알고리즘 호출 경계를 단순화하고, 소유 컨테이너와 `const` 참조를 구분한다. `main.cpp`의 값 전달 후 `std::move`는 xvalue를 만들고, 생성자는 멤버 초기화 목록으로 소유권을 받는다. 반환식의 `Report{...}`는 prvalue이며 C++17 이후 결과 객체로 직접 생성된다. 실제 로드·저장·비교·분기·호출 명령은 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.

## 파일과 실습

- `main.cpp`: `class`/`struct`, `explicit`, 이동, 수명, 복사 생략, `std::ranges::sort`.
- `problem.cpp`: 범위 템플릿, `const R&` 참조 바인딩, 람다와 `std::ranges::count_if`.
- 직접 해보기: `NameCatalog catalog = names;`가 왜 거부되는지 확인하고 `NameCatalog catalog{names};`와 이동 초기화의 차이를 설명한다.
- 초보자 검증: 이동 뒤 `names`는 유효하지만 값은 미지정 상태이며, `catalog`가 문자열 수명을 소유한다는 사실을 말로 설명한다.

## 오늘의 ICPC 문제

- 문제: BOJ 11053 **가장 긴 증가하는 부분 수열**
- 출처: [Baekjoon Online Judge](https://www.acmicpc.net/problem/11053)
- 핵심 알고리즘: [최장 증가 부분 수열](../algorithm/longest-increasing-subsequence.md)의 O(N²) 동적 계획법
- 복잡도: 시간 O(N²), 공간 O(N)
- 검증: 공식 예제, 완전 내림차순, 모든 값이 같은 수열을 CTest로 실행한다.

## 빌드와 실행

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
