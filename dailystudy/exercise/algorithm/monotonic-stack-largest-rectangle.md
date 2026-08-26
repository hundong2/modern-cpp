# 단조 스택과 히스토그램 최대 직사각형

## 정의

단조 스택(monotonic stack)은 스택 아래에서 위로 값이 계속 증가하거나 감소하도록 원소를 넣고 빼는 자료구조 기법이다. BOJ 6549에서는 아직 오른쪽 경계가 정해지지 않은 막대의 높이를 **엄격히 증가**하게 보관한다. 새로 만난 높이가 더 낮으면 높은 막대들을 꺼내며 그 높이가 만들 수 있는 최대 너비와 넓이를 확정한다.

## 적용 조건

- 배열의 각 원소에 대해 “왼쪽/오른쪽에서 처음 더 작은(또는 큰) 원소”를 선형 시간에 찾아야 한다.
- 어떤 후보의 종료 시점이 뒤에서 나타나는 임계값 하나로 확정된다.
- 후보마다 전체 구간을 다시 훑는 `O(n^2)` 풀이를 피해야 한다.
- 히스토그램 최대 직사각형, 다음 큰 원소, 주가 기간, 일일 온도, 빗물 경계 문제에 자주 적용된다.

값 자체의 정렬이 목적이면 일반 정렬을 사용한다. 임의 구간 최소 질의가 반복되면 세그먼트 트리·희소 테이블 등 다른 구조가 더 알맞을 수 있다.

## 핵심 아이디어와 불변식

스택 원소를 `(height, start)`로 둔다.

- `height`: 아직 오른쪽으로 더 확장할 수 있는 막대 높이
- `start`: 현재 `height` 이상인 연속 구간이 시작된 가장 왼쪽 인덱스
- 불변식: 스택의 `height`는 아래에서 위로 엄격히 증가한다.

인덱스 `i`에서 현재 높이 `h`를 볼 때 스택 위 높이가 `h`보다 크면 그 원소를 꺼낸다. 꺼낸 높이 `old_height`는 `start`부터 `i-1`까지 모두 유지됐고, 위치 `i`의 더 낮은 막대 때문에 더 오른쪽으로 갈 수 없다.

```text
width = i - start
area = old_height * width
```

꺼낸 원소의 `start`를 현재 높이의 시작으로 넘긴다. 현재의 더 낮은 높이는 그 왼쪽 구간 전체에서도 유지되기 때문이다. 같은 높이는 더 이른 `start`를 가진 기존 원소 하나만 남긴다.

마지막 인덱스 뒤에 높이 `0`인 센티널을 한 번 처리하면 남아 있는 양의 높이가 모두 pop되어 별도 정리 코드가 필요 없다.

## 단계별 절차

1. 빈 스택과 최대 넓이 `best=0`을 만든다.
2. 실제 인덱스 `0..n-1`과 끝 센티널 인덱스 `n`을 순회한다.
3. 실제 위치의 현재 높이는 배열 값, 센티널 높이는 0으로 둔다.
4. `start=i`로 시작한다.
5. 스택 위 높이가 현재 높이보다 큰 동안 꺼낸다.
6. 각 pop에서 `height * (i-start)`로 넓이를 계산하고 최대값을 갱신한다.
7. 현재의 `start`를 꺼낸 원소의 더 이른 `start`로 바꾼다.
8. 현재 높이가 양수이고 스택 위보다 크면 `(current_height,start)`를 넣는다.
9. 센티널 처리 뒤 최대값을 반환한다.

## 의사 코드

```text
stack = empty
best = 0

for i from 0 through n:
    current = 0 if i == n else height[i]
    start = i

    while stack is not empty and stack.top.height > current:
        closing = stack.pop()
        best = max(best, closing.height * (i - closing.start))
        start = closing.start

    if current > 0 and (stack is empty or stack.top.height < current):
        stack.push((current, start))

return best
```

## 컴파일 가능한 C++ 뼈대

```cpp
#include <algorithm> // std::max를 선언한다.
#include <cstddef>   // std::size_t를 선언한다.
#include <iostream>  // std::cout을 선언한다.
#include <vector>    // std::vector를 선언한다.

struct OpenBar {              // struct는 기본 public인 값 레코드다.
    long long height{};       // 넓이 곱셈을 64비트로 유지한다.
    std::size_t start{};      // 이 높이가 시작 가능한 가장 왼쪽 위치다.
};

long long largest_rectangle(const std::vector<long long>& heights) {
    std::vector<OpenBar> stack{}; // vector의 뒤를 스택 꼭대기로 쓴다.
    stack.reserve(heights.size());
    long long best{};

    for (std::size_t index{}; index <= heights.size(); ++index) {
        const long long current{index == heights.size() ? 0 : heights[index]};
        std::size_t start{index};

        while (!stack.empty() && stack.back().height > current) {
            const OpenBar closing{stack.back()}; // pop 전에 값을 복사한다.
            stack.pop_back();                    // pop_back은 값을 반환하지 않는다.
            const auto width{static_cast<long long>(index - closing.start)};
            best = std::max(best, closing.height * width);
            start = closing.start; // 더 낮은 막대가 왼쪽으로 확장한다.
        }

        if (current > 0 && (stack.empty() || stack.back().height < current)) {
            stack.push_back(OpenBar{current, start});
        }
    }
    return best;
}

int main() {
    const std::vector<long long> heights{2, 1, 4, 5, 1, 3, 3};
    const long long answer{largest_rectangle(heights)};
    std::cout << answer << '\n'; // 높이 4, 너비 2인 직사각형의 넓이 8
    return answer == 8 ? 0 : 1;
}
```

## 정확성 근거

### 보조정리 1: 높이 단조성

처음 스택은 비어 있어 단조다. 새 높이보다 큰 원소를 모두 pop하면 스택은 비었거나 위 높이가 새 높이 이하이다. 같은 높이는 추가하지 않고 더 큰 높이만 push하므로 아래에서 위로 엄격히 증가한다. 따라서 매 반복 뒤 불변식이 유지된다.

### 보조정리 2: pop 때 최대 너비가 확정된다

꺼낸 `(height,start)`에 대해 `start..i-1`의 모든 막대는 `height` 이상이다. 그렇지 않았다면 더 낮은 막대를 만난 이전 시점에 이미 pop됐어야 한다. 위치 `i`는 `height`보다 낮으므로 이 높이의 직사각형은 오른쪽으로 더 확장할 수 없다. `start`보다 왼쪽은 이 원소가 push될 때 이미 더 낮은 막대 또는 배열 경계로 막혔다. 따라서 너비 `i-start`가 이 높이의 가능한 최대 너비다.

### 보조정리 3: 최적 직사각형을 빠뜨리지 않는다

어떤 최적 구간의 최소 높이를 `h`라 하자. 그 구간에서 `h`가 처음 나타날 때 또는 더 높은 원소들이 pop된 뒤 `(h,start)`가 보존된다. 구간 오른쪽 다음의 더 낮은 높이 또는 끝 센티널을 만날 때 보조정리 2에 의해 `h`가 만들 수 있는 최대 구간 넓이를 계산한다. 이는 선택한 최적 구간의 넓이 이상이며, 실제 히스토그램을 벗어나지 않으므로 최적값과 같다.

### 정리

모든 계산 넓이는 실제 연속 구간으로 만들 수 있다. 보조정리 3에 따라 최적 구간의 넓이도 반드시 계산되므로 반환한 최대값은 정확하다.

## 시간·공간 복잡도

- 각 막대는 스택에 최대 한 번 push되고 최대 한 번 pop된다.
- 겉 for와 안 while을 따로 곱하지 않는다. 전체 push/pop 횟수는 `2n` 이하이므로 시간은 `O(n)`이다.
- 스택에 최악의 경우 증가 수열 전체가 들어가므로 추가 공간은 `O(n)`이다.
- 입력 높이 배열까지 함께 보관하면 총 보조 저장은 여전히 `O(n)`이다.

높이가 최대 `10^9`, 너비가 최대 `10^5`이면 넓이는 `10^14`까지 가능하다. 32비트 `int`가 아니라 `long long`으로 높이·너비 변환·곱셈·답을 계산한다.

## 흔한 실수

1. pop한 원소의 `start`를 현재 높이에 전달하지 않는다. 더 낮은 막대가 왼쪽으로 확장 가능한 구간을 잃는다.
2. 끝 센티널 또는 별도 flush를 빼먹는다. 증가 수열의 후보가 한 번도 pop되지 않는다.
3. `back()`이 반환한 참조를 보관한 채 `pop_back()`한다. 제거 원소 참조는 무효이므로 먼저 값으로 복사한다.
4. 같은 높이를 계속 넣으면서 더 이른 start를 잃는다. 같은 높이는 가장 왼쪽 후보 하나로 합친다.
5. 면적 곱셈을 int로 먼저 하고 long long에 저장한다. 대입 전에 이미 오버플로할 수 있으므로 피연산자를 64비트로 만든다.
6. `index <= size()` 루프에서 센티널일 때도 `heights[index]`를 읽는다. 조건 연산이나 별도 분기로 범위 밖 접근을 막는다.
7. while이 중첩됐다는 이유로 `O(n^2)`라 단정한다. 각 원소의 생애 전체 push/pop 횟수로 상각 분석한다.

## 변형

- **다음으로 더 큰 원소**: 값 또는 인덱스를 감소 스택에 두고 더 큰 값이 오면 답을 확정한다.
- **일일 온도/주가 기간**: 인덱스를 저장해 거리나 기간을 계산한다.
- **양쪽 더 작은 원소 배열**: 왼쪽·오른쪽 경계를 각각 구한 뒤 각 막대의 면적을 계산한다.
- **최대 1 직사각형**: 이진 행렬의 각 행을 누적 높이 히스토그램으로 바꿔 매 행 같은 알고리즘을 적용한다.
- **원형 배열**: 인덱스를 두 바퀴 순회하되 중복 후보와 최대 너비를 제한한다.
- **구간 최소 기반 분할 정복**: 세그먼트 트리로 최소 높이 위치를 찾는 대안이 있지만 일반적으로 `O(n log n)`이고 구현이 더 크다.

## 오늘 문제와의 연결

2026-08-26의 BOJ 6549에서 `OpenBar{height,start}`는 아직 더 오른쪽으로 확장 가능한 직사각형 후보다. 새 높이가 낮아지는 순간 후보의 최대 오른쪽 경계가 확정된다. 높이 0 센티널이 마지막 후보까지 닫고, `long long`이 최대 `10^14` 넓이를 안전하게 담는다.

대회에서 반드시 떠올릴 질문은 다음과 같다.

- 각 원소의 왼쪽·오른쪽에서 처음 더 작은/큰 원소가 필요한가?
- 후보의 답이 한쪽 임계값을 처음 만나는 순간 확정되는가?
- 중첩 반복을 각 원소 한 번 push/pop으로 상각할 수 있는가?

세 질문이 맞으면 단조 스택으로 `O(n)`을 검토한다.

## 직접 해보기와 초보자 검증

1. `[2,1,4,5,1,3,3]`에서 매 인덱스의 스택 `(height,start)`와 best를 표로 쓴다.
2. `[1,2,3,4,5]`에서 센티널을 제거해 누락되는 후보를 찾는다.
3. `[5,4,3,2,1]`에서 start가 매 pop 뒤 어떻게 0까지 전달되는지 추적한다.
4. `[3,3,3]`에서 같은 높이를 하나만 보관해도 넓이 9가 나오는 이유를 설명한다.
5. 각 막대의 왼쪽·오른쪽 첫 작은 인덱스를 두 번의 단조 스택 순회로 구하는 버전을 작성한다.
6. 위 컴파일 가능한 예제를 높은 경고 수준으로 빌드하고 출력 8과 종료 코드 0을 확인한다.
