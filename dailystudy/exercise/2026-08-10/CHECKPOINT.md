# 2026-08-10 체크포인트

## Modern C++ 이해 증명

- [ ] lvalue `names`를 복사하는 직접 초기화와 `std::move(names)` xvalue를 이동하는 초기화의 차이를 설명한다.
- [ ] prvalue `Report{...}`의 복사 생략과 반환 객체 수명을 설명한다.
- [ ] `explicit`이 암시적 변환을 막고 `{}` 직접 초기화는 허용하는 이유를 설명한다.
- [ ] `struct`의 기본 public과 `class`의 기본 private 접근 차이를 코드에서 찾는다.
- [ ] `const R&`가 데이터를 소유하지 않으므로 원본 수명보다 오래 보관하면 안 되는 이유를 설명한다.

## ICPC 검증

- 문제: BOJ 11053 **가장 긴 증가하는 부분 수열** — <https://www.acmicpc.net/problem/11053>
- 알고리즘: `algorithm/longest-increasing-subsequence.md`, 점화식 `dp[i] = max(dp[i], dp[j] + 1)`.
- 불변식: `i` 계산 시 모든 `j < i` 상태가 확정되고, `dp[i]`는 i에서 끝나는 최장 길이다.
- 복잡도: 이중 반복문 시간 O(N²), 두 vector 공간 O(N).
- [ ] 공식 예제의 출력 4를 손으로 추적했다.
- [ ] 내림차순과 중복 값 입력의 출력이 1임을 확인했다.
- [ ] 모든 실행 파일을 빌드하고 CTest 5개가 통과했다.
