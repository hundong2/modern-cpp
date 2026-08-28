# 접미 배열 Doubling과 Kasai LCP

## 정의

문자열 `S`의 접미사는 `S[i..n)`이고, **접미 배열(SA)** 은 모든 시작 위치 `i`를 접미사의 사전순으로 나열한 배열이다. **LCP 배열**은 `LCP[k] = lcp(S[SA[k-1]..], S[SA[k]..])`이며 첫 원소는 이전 접미사가 없어 0으로 둔다.

## 적용 조건

- 한 문자열에서 부분 문자열 검색, 서로 다른 부분 문자열 수, 반복 부분 문자열, 두 접미사의 공통 접두사 질의를 많이 처리한다.
- 단일 패턴 한 번 검색은 KMP가 더 단순하고, 여러 패턴을 한 번 훑는 문제는 Aho–Corasick이 알맞을 수 있다.
- 입력이 정적일 때 강하다. 문자열이 자주 삽입·삭제되는 문제에는 동적 자료구조를 따로 검토한다.

## 핵심 아이디어와 불변식

초기에는 각 접미사의 첫 문자로 그룹을 만든다. `half = 2^k` 단계 시작 시 `group[i]`는 `S[i..i+half)`의 사전순 동치류다. 길이 `2*half` 접두사는 앞 절반 그룹과 뒤 절반 그룹의 정수 쌍으로 완전히 비교할 수 있다.

문자열 끝에 입력 문자보다 작은 유일한 sentinel을 붙이면 suffix 정렬을 cyclic shift 정렬로 바꿀 수 있다. 이미 두 번째 key 순으로 정렬된 시작 위치를 `half`만큼 왼쪽 이동한 뒤 첫 key인 그룹 번호만 안정 계수 정렬한다. 따라서 단계당 `O(n)`, 단계 수 `O(log n)`이다.

Kasai 불변식은 시작 위치 `i`의 직전 이웃과 LCP가 `h`라면 `i+1` 접미사에서 적어도 앞의 `h-1`문자가 여전히 같다는 것이다. 각 반복 끝에 `h`를 최대 1만 줄이고 문자 일치 때만 늘리므로 전체 증가 횟수도 `O(n)`이다.

## 단계별 절차

1. sentinel을 붙이고 문자 값으로 안정 계수 정렬한다.
2. 같은 문자에는 같은 그룹 번호, 다른 문자에는 다음 번호를 준다.
3. `half = 1, 2, 4, ...`마다 정렬 위치를 `-half` 순환 이동한다.
4. 현재 그룹 번호로 안정 계수 정렬하고 두 그룹 쌍을 비교해 새 그룹을 만든다.
5. 모든 그룹이 다르거나 `half >= n`이면 sentinel 접미사를 제거한다.
6. SA의 역배열 `rank[start]`를 만든다.
7. 원문 시작 위치 순으로 직전 SA 이웃과 비교하고, 다음 위치로 갈 때 LCP 후보를 1 줄인다.

## 의사 코드

```text
group <- 문자별 순위
SA <- 문자별 안정 정렬
for half = 1; half < n; half *= 2:
    shifted <- SA의 각 위치를 half만큼 왼쪽 순환 이동
    SA <- shifted를 group key로 안정 계수 정렬
    new_group <- 인접한 (group[i], group[i+half]) 쌍 비교
    group <- new_group

rank[SA[k]] <- k
h <- 0
for i = 0 .. n-1:
    if rank[i] > 0:
        j <- SA[rank[i]-1]
        while S[i+h] == S[j+h]: h++
        LCP[rank[i]] <- h
        if h > 0: h--
```

## C++ 뼈대

아래 예제는 교육용으로 `std::sort`를 사용해 구조를 짧게 보인다. 따라서 SA 구성은 `O(n log²n)`이고, 오늘 BOJ 9248 제출 코드는 계수 정렬로 `O(n log n)`을 달성한다.

```cpp
#include <algorithm>  // 비교 함수로 접미 시작 위치를 정렬하는 std::sort를 선언한다.
#include <cstddef>    // 컨테이너 크기·인덱스용 std::size_t를 선언한다.
#include <iostream>   // 예제 결과를 쓰는 std::cout을 선언한다.
#include <numeric>    // 연속 정수로 채우는 std::iota를 선언한다.
#include <string>     // 입력 문자를 소유하는 std::string을 선언한다.
#include <vector>     // 접미 위치와 순위의 연속 저장소 std::vector를 선언한다.

// 반환형 vector<int>는 접미 시작 위치를 소유하며, const string& 매개변수는 호출자 문자열을 복사하지 않고 읽기만 한다.
[[nodiscard]] std::vector<int> suffix_array(const std::string& text) {
    // size()는 인자 없이 size_type을 O(1)에 반환하고 text를 바꾸지 않는다. 작은 예제라 int로 명시 변환한다.
    const int n{static_cast<int>(text.size())};
    // vector(count)는 n개 int를 0으로 값 초기화하고 동적 버퍼를 독점 소유한다.
    std::vector<int> suffix_order(static_cast<std::size_t>(n));
    // vector(count,value)는 n+1개 int를 -1 복사본으로 채운다. 마지막 -1은 문자열 밖 sentinel 순위다.
    std::vector<int> rank(static_cast<std::size_t>(n + 1), -1);
    std::vector<int> next_rank(static_cast<std::size_t>(n));

    // iota(first,last,value)는 [begin,end) 각 int에 0부터 연속 값을 쓰고 void를 반환한다. vector 크기·용량은 유지된다.
    // begin/end 반복자는 함수 동안 유효하며 재할당이 없고, O(n) 시간·추가 할당 없음·사용자 연산 예외 외 실패가 없다.
    std::iota(suffix_order.begin(), suffix_order.end(), 0);
    // 첫 단계 그룹은 unsigned char 사전순 값으로 시작한다.
    for (int index{0}; index < n; ++index) {
        rank[static_cast<std::size_t>(index)] =
            static_cast<unsigned char>(text[static_cast<std::size_t>(index)]);
    }

    // width는 비교가 이미 끝난 앞 절반 길이며 매 단계 *=2로 두 배가 된다.
    for (int width{1}; width < n; width *= 2) {
        // sort(first,last,comparison)는 suffix_order를 제자리 재배열하고 void를 반환한다.
        // 세 인자는 begin 반복자, end sentinel, rank/width/n을 비소유 참조 캡처한 lambda prvalue다.
        // 평균·최악 O(n log n) 비교, 추가 메모리는 구현 의존이며 반복자/참조는 원소 재배열을 관찰한다. 비교자는 strict weak ordering이어야 한다.
        std::sort(suffix_order.begin(), suffix_order.end(), [&](int left, int right) {
            // 첫 그룹이 다르면 int operator< 결과 bool을 즉시 반환한다.
            if (rank[static_cast<std::size_t>(left)] != rank[static_cast<std::size_t>(right)]) {
                return rank[static_cast<std::size_t>(left)] < rank[static_cast<std::size_t>(right)];
            }
            // ?: 조건 연산자는 뒤 절반이 범위 안이면 순위를, 아니면 sentinel -1을 값으로 고른다.
            const int left_second{left + width < n ? rank[static_cast<std::size_t>(left + width)] : -1};
            const int right_second{right + width < n ? rank[static_cast<std::size_t>(right + width)] : -1};
            return left_second < right_second;
        });

        next_rank[static_cast<std::size_t>(suffix_order[0])] = 0;  // 사전순 첫 접미사 그룹은 0이다.
        for (int order{1}; order < n; ++order) {
            const int previous{suffix_order[static_cast<std::size_t>(order - 1)]};
            const int current{suffix_order[static_cast<std::size_t>(order)]};
            const int previous_second{previous + width < n ? rank[static_cast<std::size_t>(previous + width)] : -1};
            const int current_second{current + width < n ? rank[static_cast<std::size_t>(current + width)] : -1};
            // ||는 첫 그룹이 다르면 뒤 비교를 생략하며, bool은 새 그룹 증가량 1/0을 결정한다.
            const bool different{rank[static_cast<std::size_t>(previous)] != rank[static_cast<std::size_t>(current)] ||
                                 previous_second != current_second};
            next_rank[static_cast<std::size_t>(current)] =
                next_rank[static_cast<std::size_t>(previous)] + (different ? 1 : 0);
        }
        // vector::swap(other)는 두 버퍼 소유권을 O(1)에 교환하고 void를 반환한다. 기본 allocator에서는 예외가 없다.
        rank.swap(next_rank);
    }
    return suffix_order;  // 반환 prvalue가 호출자 vector를 직접 초기화하거나 이동으로 버퍼 소유권을 넘긴다.
}

int main() {
    // 문자열 리터럴은 임시 string으로 변환되고 함수 호출 반환 prvalue가 const auto vector를 직접 초기화한다.
    const auto suffix_order{suffix_array("banana")};
    // range-for는 vector의 begin/end 구간에서 int를 하나씩 값 복사한다.
    for (const int index : suffix_order) {
        // operator<<는 1-based 위치와 공백을 cout 버퍼에 쓰고 같은 ostream&를 반환해 연쇄한다.
        std::cout << index + 1 << ' ';
    }
    std::cout << '\n';  // 마지막 개행을 쓰며 출력 실패는 기본적으로 스트림 상태 비트에 기록된다.
    return 0;
}
```

## 정확성 근거

초기 문자 정렬은 길이 1 접두사의 순서를 정확히 만든다. 어떤 단계에서 길이 `half` 그룹이 정확하다고 가정하면, 길이 `2*half` 접두사의 사전순은 앞/뒤 절반 그룹 쌍의 사전순과 같다. 안정 계수 정렬은 그 쌍의 순서를 정확히 만들고 새 그룹은 정확히 같은 쌍에만 같은 번호를 준다. 귀납적으로 충분히 긴 단계 뒤 전체 접미사 순서가 정확하다.

Kasai에서 접미사 `i`와 어떤 접미사가 앞 `h`문자 일치했다면 첫 글자를 제거한 두 접미사는 적어도 `h-1`문자 일치한다. 사전순 직전 이웃의 LCP는 그 하한보다 작을 수 없으므로 비교를 이어 시작해도 답을 놓치지 않는다.

## 복잡도

- 계수 정렬 doubling: 시간 `O(n log n)`, 추가 공간 `O(n)`.
- 비교 정렬 skeleton: 시간 `O(n log²n)`, 추가 공간 `O(n)`.
- Kasai LCP: 시간 `O(n)`, 추가 공간 `O(n)`.

## 흔한 실수

- 접미사 문자열을 `substr`로 모두 만들어 `O(n²)` 문자 저장과 비교 비용을 낸다.
- sentinel이 입력에도 나타나거나 다른 문자보다 작지 않다.
- 계수 정렬을 안정적으로 하지 않아 두 key 순서가 깨진다.
- `i+half` 경계를 잘못 읽거나 순환 shift와 일반 suffix 방식을 섞는다.
- BOJ 출력은 1-indexed SA지만 내부는 0-indexed라는 차이를 놓친다.
- LCP 첫 칸의 `x` 형식, height 감소, 문자열 끝 검사를 빠뜨린다.

## 변형

- 서로 다른 부분 문자열 수: `n(n+1)/2 - sum(LCP)`.
- 가장 긴 반복 부분 문자열: LCP 최댓값.
- 임의 두 접미사 LCP: LCP 배열 위 RMQ sparse table/segment tree.
- 여러 문자열: 서로 다른 sentinel을 둔 generalized suffix array.
- 더 강한 구성법: SA-IS 등 선형 시간 알고리즘이 있지만 구현 복잡도가 높다.

## 오늘 문제와의 연결

[2026-08-29 BOJ 9248 풀이](../2026-08-29/icpc_problem.cpp)는 최대 500,000문자를 위해 비교 정렬 대신 그룹 계수 정렬을 쓴다. `banana`, 동일 문자, 주기 문자열은 그룹 갱신과 Kasai 불변식을 동시에 드러내는 핵심 테스트다.

## 직접 해보기와 이해 검증

1. `banana`의 half 1, 2, 4 단계 그룹을 손으로 적는다.
2. 계수 정렬을 앞에서부터 배치해 안정성이 깨지는 입력을 찾는다.
3. 무작위 길이 30 이하 문자열에서 `substr` 기반 정답과 SA/LCP를 대조한다.
4. 서로 다른 부분 문자열 수 공식을 SA/LCP로 구현한다.
5. “단계 시작 group 불변식”과 “Kasai h-1 불변식”을 자료 없이 각각 두 문장으로 증명한다.
