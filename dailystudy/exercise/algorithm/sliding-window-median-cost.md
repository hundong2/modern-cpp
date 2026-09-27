# 슬라이딩 윈도우 중앙값 비용

## 정의

길이 `k`인 연속 구간이 한 칸씩 이동할 때, 각 구간의 중앙값과 중앙값까지의 절댓값 거리 합을 동적으로 유지하는 기법이다. 현재 창을 `W`라 하고 중앙값 하나를 `m`이라 하면 비용은 다음과 같다.

```text
cost(W) = sum(|x - m| for x in W)
```

절댓값 합은 중앙값에서 최소가 된다. `k`가 홀수이면 중앙값이 하나이고, 짝수이면 정렬했을 때 가운데 두 값 사이의 어떤 값도 같은 최소 비용을 만든다. 구현에서는 일관되게 **아래쪽 중앙값(lower median)** 을 선택한다.

## 적용 조건

- 고정 길이 연속 구간마다 중앙값 또는 중앙값까지의 절댓값 합이 필요하다.
- 한 칸 이동할 때 원소 하나가 빠지고 하나가 들어오는 온라인 갱신을 활용할 수 있다.
- 값의 중복이 있을 수 있으므로, 삭제할 원소를 값만으로 구별하면 안 된다.
- `O(nk)` 정렬 또는 전수 계산은 느리고 `O(n log k)`가 필요한 입력 크기다.
- 값의 절댓값과 `k`의 곱이 32비트 범위를 넘을 수 있어 합은 64비트 정수로 저장해야 한다.

창 길이가 매우 작거나 질의 수가 적으면 매번 정렬하는 단순 구현이 더 이해하기 쉬울 수 있다. 값의 좌표 범위가 작고 정적이라면 빈도 배열이나 펜윅 트리도 대안이다.

## 자료구조와 핵심 불변식

각 원소를 `Entry{value, index}`로 저장한다. 비교 순서는 `(value, index)`의 사전식 순서다. `index`가 전역에서 유일하므로 값이 같아도 각 창 원소가 서로 다른 키가 되고, 나가는 위치를 정확히 찾아 한 개만 삭제할 수 있다.

두 개의 정렬 다중집합을 둔다.

- `lower`: 정렬된 창의 작은 쪽 `ceil(s / 2)`개
- `upper`: 나머지 큰 쪽 `floor(s / 2)`개
- `lower_sum`: `lower`에 든 모든 `value`의 합
- `upper_sum`: `upper`에 든 모든 `value`의 합

현재 저장 원소 수를 `s`라 할 때 공개 연산이 끝날 때마다 다음 불변식을 만족시킨다.

1. **분할 완전성**: 현재 창의 각 `Entry`는 `lower`와 `upper` 중 정확히 한 곳에 있다.
2. **순서 불변식**: 두 집합이 모두 비어 있지 않다면 `max(lower) < min(upper)`가 `Entry` 순서로 성립한다. 값만 보면 `max(lower).value <= min(upper).value`다.
3. **크기 불변식**: `lower.size() = ceil(s / 2)`, `upper.size() = floor(s / 2)`다.
4. **합 불변식**: 두 합 변수는 각 파티션 원소의 `value` 합과 정확히 같다.
5. **중앙값 불변식**: `s > 0`이면 `max(lower).value`가 아래쪽 중앙값이다.

## 비용 공식

`m = max(lower).value`, `L = lower.size()`, `U = upper.size()`라 하자. 순서 불변식 때문에 `lower`의 값은 모두 `m` 이하이고 `upper`의 값은 모두 `m` 이상이다.

```text
sum(m - x for x in lower) = m * L - lower_sum
sum(x - m for x in upper) = upper_sum - m * U

cost = m * L - lower_sum + upper_sum - m * U
```

따라서 중앙값을 얻은 뒤 비용은 `O(1)`에 계산된다. `multiset`을 순회해 절댓값을 다시 더하면 창마다 `O(k)`가 되어 전체 목표 복잡도를 잃는다.

## 단계별 절차

### 1. 원소 삽입

1. `lower`가 비었거나 새 `Entry`가 현재 `max(lower)` 이하이면 `lower`에 삽입하고 `lower_sum`에 값을 더한다.
2. 그렇지 않으면 `upper`에 삽입하고 `upper_sum`에 값을 더한다.
3. 두 집합의 전체 크기로부터 목표 `lower` 크기 `ceil(s / 2)`를 계산한다.
4. `lower`가 크면 그 최댓값을 `upper`로, 작으면 `upper`의 최솟값을 `lower`로 옮긴다.

삽입 위치를 중앙값과 비교했기 때문에 옮길 경계 원소 하나는 항상 반대 파티션의 모든 원소와 올바른 순서를 이룬다.

### 2. 원소 삭제

1. 빠질 위치의 정확한 `Entry{value, index}`를 만든다.
2. `lower.find(entry)`로 먼저 찾고, 있으면 `lower`에서 지우며 `lower_sum`에서 값을 뺀다.
3. 없으면 분할 완전성에 의해 `upper`에 반드시 있으므로 그곳에서 찾아 지우고 `upper_sum`을 갱신한다.
4. 삽입과 같은 크기 재균형을 수행한다.

값만 키로 지우는 `erase(value)`는 같은 값이 여러 개일 때 어느 위치가 나갔는지 표현하지 못한다. 특히 `multiset::erase(key)`는 동등한 키를 모두 지울 수 있으므로 이 문제에서는 정확한 iterator 한 개를 지워야 한다.

### 3. 창 이동과 답 계산

처음 `k`개를 넣고 비용을 출력한다. 이후 오른쪽 끝 위치 `r = k .. n-1`마다 다음을 반복한다.

1. `Entry{a[r-k], r-k}` 삭제
2. `Entry{a[r], r}` 삽입
3. 비용 공식으로 답 출력

삭제 후 잠깐 크기가 `k-1`인 상태에서도 재균형하고, 삽입 후 다시 크기 `k` 상태로 재균형한다. 각 공개 연산 뒤 불변식이 완성되므로 연산 순서를 독립적으로 검증할 수 있다.

## 의사코드

```text
add(e):
    if lower is empty or e <= max(lower):
        lower.insert(e)
        lower_sum += e.value
    else:
        upper.insert(e)
        upper_sum += e.value
    rebalance()

remove(e):
    it = lower.find(e)
    if it exists:
        lower_sum -= e.value
        lower.erase(it)
    else:
        it = upper.find(e)  // e가 현재 창에 있다는 전제에서 반드시 성공
        upper_sum -= e.value
        upper.erase(it)
    rebalance()

rebalance():
    target = ceil((lower.size + upper.size) / 2)
    while lower.size > target:
        move max(lower) to upper and update both sums
    while lower.size < target:
        move min(upper) to lower and update both sums

cost():
    m = max(lower).value
    return m * lower.size - lower_sum
         + upper_sum - m * upper.size
```

## C++ 뼈대

아래 코드는 구조만 보여 주는 축약형이다. 오늘의 제출 코드는 호출 계약, 입력·출력, 수치 경계를 더 자세히 설명한다.

```cpp
#include <set>

struct Entry {
    long long value;
    int index;
};

bool operator<(const Entry& left, const Entry& right) noexcept {
    if (left.value != right.value) {
        return left.value < right.value;
    }
    return left.index < right.index;
}

class MedianCost {
    std::multiset<Entry> lower;
    std::multiset<Entry> upper;
    long long lower_sum{};
    long long upper_sum{};

    // add/remove 뒤 경계 원소를 옮겨 ceil(s/2) : floor(s/2)를 복구한다.
    void rebalance();

public:
    void add(Entry entry);
    void remove(Entry entry);
    [[nodiscard]] long long cost();
};
```

## 정확성 근거

### 보조정리 1: 중앙값은 절댓값 합을 최소화한다

정렬된 값에서 후보 `m`을 오른쪽으로 한 단위 움직이면 `m` 왼쪽 원소가 만드는 비용은 그 개수만큼 늘고, 오른쪽 원소가 만드는 비용은 그 개수만큼 줄어든다. 중앙값 전에는 오른쪽 원소가 더 많아 이동이 비용을 줄이고, 중앙값을 지난 뒤에는 왼쪽 원소가 더 많아 이동이 비용을 늘린다. 그러므로 최소점은 중앙값이며, 짝수 개일 때는 가운데 두 값 사이 전체가 최소 구간이다.

### 보조정리 2: 삽입 뒤 재균형은 순서 불변식을 보존한다

새 원소가 `max(lower)` 이하이면 `lower`에, 더 크면 `upper`에 넣으므로 삽입 직후 두 파티션 사이 순서는 유지된다. `lower`가 너무 크면 그 최댓값만 `upper`로 옮긴다. 남은 `lower`의 모든 원소는 이동 원소 이하이고, 기존 `upper` 원소도 이동 전 경계 이상이므로 순서가 유지된다. 반대 이동도 `upper`의 최솟값에 대해 대칭적으로 성립한다.

### 보조정리 3: 삭제 뒤 재균형은 모든 불변식을 복구한다

삭제는 정확한 인덱스가 붙은 원소 한 개만 제거하므로 나머지 원소의 상대 순서를 바꾸지 않는다. 삭제된 파티션의 합에서 같은 값을 빼므로 합 불변식도 유지된다. 한 원소 삭제로 목표 크기와 실제 크기의 차이는 최대 1이고, 경계 원소를 한 번 옮기면 크기 불변식이 복구된다. 보조정리 2와 같은 경계 논리로 순서도 유지된다.

### 보조정리 4: 비용 공식은 실제 절댓값 합과 같다

중앙값 `m` 이하인 `lower`에서는 각 항이 `m-x`, `m` 이상인 `upper`에서는 각 항이 `x-m`이다. 두 파티션에서 각각 합을 전개하면 저장한 두 합을 사용하는 공식과 정확히 같아진다.

### 정리

초기에는 두 집합이 비어 있어 불변식이 자명하다. 각 삽입과 삭제는 보조정리 2와 3에 의해 불변식을 보존한다. 따라서 모든 완성된 창에서 `max(lower)`는 중앙값이고, 보조정리 1과 4에 의해 출력한 값은 가능한 최소 절댓값 합이다. 각 위치는 고유 인덱스로 정확히 한 번 들어오고 나가므로 모든 창의 답을 빠짐없이 정확히 출력한다.

## 시간·공간 복잡도

- `multiset` 삽입과 키 탐색: 각각 `O(log k)`
- 이미 찾은 iterator 한 개 삭제: 상각 `O(1)`; 다만 오늘 삭제 경로는 먼저 `find`하므로 탐색까지 합치면 `O(log k)`
- 재균형: 삽입 또는 삭제 한 번에 이동 횟수가 상수이고, 이동마다 `O(log k)`
- 비용 계산: `O(1)`
- 전체: `n`개 삽입과 `n-k`개 삭제에 `O(n log k)` 시간
- 두 집합과 입력 배열: `O(k) + O(n)` 공간. 스트리밍 입력과 원형 버퍼를 쓰면 입력 보관을 `O(k)`로 줄일 수 있다.

## 수치 경계

CSES 1077의 `1 <= x_i <= 10^9`, `k <= 2 * 10^5`에서 한 파티션 합과 `median * count`는 최대 `2 * 10^14` 규모다. 이는 signed 64비트 정수의 최댓값보다 충분히 작지만 32비트 `int`는 넘는다. 따라서 다음 값은 모두 `long long`이어야 한다.

- 입력 원소 값
- `lower_sum`, `upper_sum`
- 중앙값과 크기의 곱
- 최종 비용

크기 자체와 위치 인덱스는 최대 `2 * 10^5`이므로 `int`로 표현 가능하다. 비용 식의 곱셈 전에 중앙값이 이미 `long long`이므로 컨테이너 크기를 `long long`으로 변환하면 64비트 곱셈이 보장된다.

## 흔한 실수

1. `multiset<long long>`에서 `erase(value)`를 사용해 같은 값 전부를 지운다.
2. 값만 저장한 뒤 나가는 중복값이 어느 파티션에 있는지 잘못 판단한다.
3. `lower`와 `upper`의 크기만 맞추고 `max(lower) <= min(upper)`를 보장하지 않는다.
4. 이동할 때 원소는 옮기지만 `lower_sum` 또는 `upper_sum` 갱신을 빠뜨린다.
5. 짝수 창에서 어느 중앙값을 택했는지와 목표 파티션 크기가 일치하지 않는다.
6. `--lower.end()`를 빈 집합에서 수행하거나 `*upper.begin()`을 빈 집합에서 수행한다.
7. 합과 `median * size`를 `int`로 계산해 signed overflow를 일으킨다.
8. 매 창마다 두 집합을 순회해 비용을 계산하여 `O(nk)`로 퇴화한다.
9. iterator를 지운 뒤 다시 역참조하거나 증가시킨다. `multiset` 삭제는 지운 원소의 iterator만 무효화한다.

## 변형

- **중앙값만 출력**: 두 합 변수를 없애고 `max(lower)`만 읽는다.
- **위쪽 중앙값**: `lower` 크기를 `floor(s/2)`, `upper`를 `ceil(s/2)`로 두고 `min(upper)`를 택한다.
- **두 힙 + 지연 삭제**: 최대 힙과 최소 힙으로도 `O(n log k)`가 가능하지만, 중복 원소의 유효 개수와 지연 삭제 맵을 별도로 관리해야 한다.
- **좌표 압축 + 펜윅 트리**: 값 빈도 트리로 k번째 값을 찾고 값 합 트리를 함께 두면 중앙값과 비용을 구할 수 있다. 값 종류가 많아도 `O(log n)`이며 다수의 순위 질의로 확장하기 좋다.
- **가중 중앙값 비용**: 각 원소에 가중치가 있으면 개수 대신 누적 가중치를 기준으로 분할하고, `value * weight` 합을 유지한다.
- **가변 길이 창**: 공개 `add/remove`가 매번 전체 원소 수로 목표 크기를 다시 계산하므로 같은 구조를 사용할 수 있다.

## 실행 관점

균형 트리 기반 `multiset`은 각 갱신에서 키 비교와 포인터를 통한 노드 탐색, 조건 분기가 일어난다. 연속 메모리 자료구조보다 캐시 지역성이 낮을 수 있지만, 임의 원소 삭제와 정렬 순서 유지가 모두 필요하므로 구현 단순성과 최악 `O(log k)` 보장이 강점이다. 실제 명령 배치, 분기 예측 결과, 할당 전략은 컴파일러·표준 라이브러리 구현·최적화 옵션·CPU에 따라 달라지므로 특정 어셈블리 형태로 단정하지 않는다.

## 오늘 문제와의 연결

- 문제: [CSES 1077 - Sliding Window Cost](https://cses.fi/problemset/task/1077/)
- 각 길이 `k` 창에서 어떤 하나의 값으로 모두 바꾸는 최소 총 이동 비용을 요구한다.
- 최소점이 중앙값이라는 성질과 두 파티션의 합 공식을 결합한다.
- 제출 구현은 `std::multiset<Entry>` 두 개, 고유 원래 인덱스, 64비트 합을 사용한다.
- 공식 제약에서 전체 시간은 `O(n log k)`, 추가 창 자료구조 공간은 `O(k)`다.
- 구현 파일: `../2026-09-28/icpc_problem.cpp`
