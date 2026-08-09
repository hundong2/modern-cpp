# 2026-08-09 Modern C++ 학습 자료

## 오늘의 주제: `std::span`으로 소유권과 읽기 경계 분리하기

`std::vector`가 데이터를 소유하고, `std::span<const int>`가 연속 메모리를 복사 없이 읽는 구조를 연습한다. 서비스 계층은 저장 방식을 몰라도 되고, 호출자는 데이터의 수명이 `span`보다 길어야 한다.

- `main.cpp`: `struct` 값 객체, `class` 서비스, `explicit` 생성자, 참조·수명·값 범주를 설명한다.
- `problem.cpp`: `std::span`을 받는 순수 계산 함수와 초보자 연습을 제공한다.
- `span`은 포인터와 길이를 가진 비소유 뷰다. 원본을 이동·삭제하거나 재할당하면 뷰가 무효가 될 수 있다.
- 함수 호출은 인자 준비·분기·로드를 동반할 수 있지만 실제 명령은 CPU, ABI, 컴파일러와 최적화 옵션에 따라 달라진다.

## 오늘의 ICPC/OJ 문제

- 문제 ID/제목: BOJ 1149, **RGB거리**
- 출처: [Baekjoon Online Judge](https://www.acmicpc.net/problem/1149)
- 핵심 알고리즘: 상태 기반 동적 계획법
- 공용 문서: [`../algorithm/dynamic-programming-state-transition.md`](../algorithm/dynamic-programming-state-transition.md)
- 복잡도: 시간 `O(N)`, 공간 `O(1)`(입력 저장을 제외한 DP 상태)

`dp[color]`을 지금까지 칠한 집 중 마지막 집이 `color`일 때의 최소 비용으로 둔다. 다음 집은 같은 색을 쓸 수 없으므로 다른 두 상태의 최솟값에 현재 비용을 더한다.

## 빌드와 검증

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

CTest는 Modern C++ 예제 2개와 ICPC 공식 예제 2개, 단일 집 경계를 검증한다.

## 직접 해보기

1. `main.cpp`의 `std::span<const int>`를 `std::span<int>`로 바꾸고 원본 변경 가능성이 어떻게 달라지는지 설명한다.
2. 임시 `std::vector<int>`에서 오래 보관하는 `span`을 만들면 왜 수명 문제가 생기는지 설명한다.
3. RGB 풀이를 `N x 3` 표로 바꾸고 결과와 복잡도를 비교한다.
4. 각 최소 비용과 함께 직전 색을 저장해 실제 색 배열을 복원한다.
