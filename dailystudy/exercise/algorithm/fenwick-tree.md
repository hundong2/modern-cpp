# 펜윅 트리(Binary Indexed Tree)

## 정의와 적용 조건

펜윅 트리는 배열의 **한 점 갱신**과 **접두 구간 집계**를 모두 `O(log N)`에 처리하는 1차원 자료구조다. 대표 연산은 한 원소에 값을 더하고 `1..i`의 합을 구하는 것이다. 덧셈처럼 결합법칙이 있고 역연산(뺄셈)으로 두 접두 결과의 차이를 구할 수 있을 때 닫힌 구간 `[left, right]`의 합도 빠르게 얻는다.

다음 조건에 특히 잘 맞는다.

- 배열 값이 실행 중 바뀌고 합·빈도 누적 질의가 반복된다.
- 필요한 연산이 점 갱신과 접두 질의 또는 이를 조합한 구간 질의다.
- 세그먼트 트리의 다양한 병합 정보보다 짧은 구현과 작은 상수가 중요하다.

단순 누적 합은 갱신이 없을 때 질의 `O(1)`이지만 값 하나가 바뀌면 뒤쪽 누적 합을 `O(N)` 수정해야 한다. 펜윅 트리는 갱신과 질의를 모두 `O(log N)`으로 균형 잡는다. 임의 구간 최솟값처럼 역연산으로 접두 결과를 제거할 수 없는 연산은 일반 세그먼트 트리가 더 자연스럽다.

## 핵심 아이디어와 불변식

1 기반 인덱스 `i`의 `lowbit(i) = i & -i`는 이진 표현에서 가장 낮은 1비트의 값이다. `tree[i]`는 다음 구간의 합을 저장한다.

```text
tree[i] = A[i - lowbit(i) + 1] + ... + A[i]
```

예를 들어 `i=12`는 이진수 `1100`이고 `lowbit(12)=4`이므로 `tree[12]`는 `A[9..12]`를 담당한다. 이것이 항상 유지해야 하는 핵심 불변식이다.

- 갱신: `A[index]`에 `delta`를 더하면 `index`를 포함하는 모든 담당 구간에 `delta`를 더해야 한다. `index += lowbit(index)`로 그 상위 구간을 차례로 방문한다.
- 접두 합: `tree[index]`를 결과에 더한 뒤 `index -= lowbit(index)`로 이미 센 담당 구간을 제거한다. 방문 구간들은 서로 겹치지 않고 정확히 `A[1..index]`를 덮는다.
- 구간 합: `[left, right] = [1, right] - [1, left-1]`이므로 두 접두 합의 차를 쓴다.

`-i`는 2의 보수 표현과 연결해 이해할 수 있다. C++에서 `std::size_t` 같은 부호 없는 정수의 음수화는 모듈러 산술로 정의되므로 `i & -i`가 최하위 1비트를 안전하게 남긴다. 0에서 `lowbit(0)`은 0이라 진행하지 못하므로 외부 배열 인덱스 0을 갱신 루프에 넣어서는 안 된다.

## 단계별 절차

### 점 추가 갱신

1. 외부 인덱스를 1 이상으로 받는다.
2. `index < tree.size()`인 동안 현재 `tree[index]`에 `delta`를 더한다.
3. `index += lowbit(index)`로 현재 점을 포함하는 다음 큰 담당 구간으로 이동한다.
4. 저장 범위를 벗어나면 끝낸다.

### 접두 합

1. 결과를 0으로 초기화한다.
2. `index > 0`인 동안 `tree[index]`를 결과에 더한다.
3. `index -= lowbit(index)`로 이미 더한 구간의 바로 왼쪽으로 이동한다.
4. 인덱스가 0이면 결과를 반환한다.

### 대입 갱신

펜윅 트리의 기본 갱신은 “더하기”다. `A[position] = new_value` 대입은 현재 값을 별도 배열에 보관하고 `delta = new_value - old_value`를 계산해 `add(position, delta)`로 바꾼다. 이후 현재 값 배열도 새 값으로 갱신해야 다음 차이가 정확하다.

## 의사 코드

```text
add(index, delta):
    while index <= N:
        tree[index] += delta
        index += index & -index

prefix_sum(index):
    result = 0
    while index > 0:
        result += tree[index]
        index -= index & -index
    return result

range_sum(left, right):
    return prefix_sum(right) - prefix_sum(left - 1)
```

## 컴파일 가능한 C++ 뼈대

```cpp
// <cstddef>는 인덱스 타입 std::size_t를 제공한다.
#include <cstddef>
// <iostream>은 예제 출력을 위한 std::cout을 제공한다.
#include <iostream>
// <vector>는 펜윅 트리 저장 배열을 제공한다.
#include <vector>

// class는 tree_를 private으로 숨겨 불변식이 공개 연산을 통해서만 바뀌게 한다.
class FenwickTree final {
public:
    // explicit 생성자는 정수가 객체로 암시 변환되는 것을 막고, 반환형은 없다.
    explicit FenwickTree(std::size_t size) : tree_(size + 1, 0) {}

    // index 위치에 delta를 더하는 점 갱신 함수다.
    void add(std::size_t index, long long delta) {
        while (index < tree_.size()) { // 상위 담당 구간이 존재하는 동안 반복한다.
            tree_[index] += delta;      // 현재 담당 구간 합을 갱신한다.
            index += lowbit(index);     // 다음 상위 담당 구간으로 이동한다.
        }
    }

    // const는 트리 값을 바꾸지 않는 조회임을 나타낸다.
    [[nodiscard]] long long prefix_sum(std::size_t index) const {
        long long result{}; // 기본 정수를 0으로 중괄호 초기화한다.
        while (index > 0) { // 1 기반 인덱스를 0까지 줄인다.
            result += tree_[index]; // 겹치지 않는 담당 구간을 더한다.
            index -= lowbit(index); // 이미 더한 구간 길이만큼 왼쪽으로 이동한다.
        }
        return result; // 값 복사로 누적 합을 반환한다.
    }

    [[nodiscard]] long long range_sum(std::size_t left, std::size_t right) const {
        return prefix_sum(right) - prefix_sum(left - 1); // 두 접두 합의 차다.
    }

private:
    [[nodiscard]] static std::size_t lowbit(std::size_t index) {
        return index & -index; // & 비트 연산자가 최하위 1비트만 남긴다.
    }

    std::vector<long long> tree_{}; // 템플릿 인자는 저장할 합의 타입이다.
};

// main은 [3,1,4,1,5]를 만들고 구간 합과 갱신을 검증한다.
int main() {
    FenwickTree tree{5}; // 크기 5인 객체를 직접 초기화한다.
    const std::vector<long long> values{3, 1, 4, 1, 5}; // vector가 다섯 값을 소유한다.
    for (std::size_t index{}; index < values.size(); ++index) { // 모든 원소를 순회한다.
        tree.add(index + 1, values[index]); // 0 기반 vector 위치를 1 기반 트리 위치로 바꾼다.
    }
    std::cout << tree.range_sum(2, 4) << '\n'; // 1+4+1인 6을 출력한다.
    tree.add(3, 6); // 세 번째 값을 4에서 10으로 만들 차이 6을 더한다.
    std::cout << tree.range_sum(2, 4) << '\n'; // 1+10+1인 12를 출력한다.
    return 0; // 정상 종료를 알린다.
}
```

## 정확성 근거

### 갱신의 정확성

`index`에서 시작해 `lowbit`을 더해 방문하는 각 노드의 담당 구간은 원래 위치를 포함한다. 반대로 그 위치를 포함하는 펜윅 담당 구간은 이 상위 이동 경로에 정확히 한 번 나타난다. 따라서 모든 관련 노드에 같은 `delta`를 더하면 `tree[i]`가 자기 담당 구간 합이라는 불변식이 보존된다.

### 접두 합의 정확성

현재 `index`에서 `tree[index]`가 담당하는 구간을 결과에 더한 후 그 길이 `lowbit(index)`만큼 왼쪽으로 이동한다. 새 구간은 이전 구간과 겹치지 않는다. 인덱스가 0이 될 때까지 이 과정을 반복하면 방문한 구간들의 합집합이 정확히 원래의 `[1, index]`가 되므로 접두 합이 정확하다.

### 구간 합의 정확성

접두 구간 `[1, right]`는 `[1, left-1]`과 `[left, right]`의 서로 겹치지 않는 합이다. 앞 접두 합에서 뒤 접두 합을 빼면 원하는 닫힌 구간만 남는다.

## 시간·공간 복잡도

- `add`: 인덱스에 최하위 1비트를 더할 때마다 더 큰 2의 거듭제곱 경계로 이동하므로 `O(log N)`
- `prefix_sum`: 인덱스의 1비트를 하나씩 지우므로 `O(log N)`
- `range_sum`: 접두 합 두 번이므로 `O(log N)`
- 저장 공간: 크기 `N+1` 배열 하나이므로 `O(N)`
- N개 값을 각각 `add`로 초기화하면 `O(N log N)`이며, 담당 구간을 이용한 `O(N)` 빌드 변형도 있다.

## 흔한 실수

- 0 기반 인덱스를 그대로 넣어 `lowbit(0)==0` 때문에 무한 반복한다.
- 구간 합에서 `prefix_sum(left)`를 빼서 왼쪽 끝 원소까지 없애는 오프바이원 오류를 낸다.
- 대입 명령에서 새 값을 그대로 더해 버리고 `new-old` 차이를 더하지 않는다.
- 현재 값 배열을 대입 뒤 갱신하지 않아 두 번째 변경부터 차이가 틀린다.
- 합을 `int`에 저장해 큰 입력에서 오버플로를 일으킨다.
- 입력 명령의 오른쪽 위치와 새 값의 타입 의미가 다른데 한 타입으로 무비판적으로 해석한다.

## 변형

- **구간 더하기 + 한 점 조회**: 차분 배열을 펜윅 트리로 관리해 `[l,r]` 갱신을 `add(l,x)`, `add(r+1,-x)`로 바꾼다.
- **구간 더하기 + 구간 합**: 펜윅 트리 두 개를 사용해 접두 합 공식을 구성한다.
- **빈도와 k번째 원소**: 값별 빈도를 저장하고 이진 리프팅으로 누적 빈도가 k 이상이 되는 최소 인덱스를 `O(log N)`에 찾는다.
- **2차원 펜윅 트리**: 격자 점 갱신과 직사각형 합을 `O(log^2 N)`에 처리하지만 공간 사용을 주의한다.
- **선형 빌드**: 각 `tree[i]` 값을 부모 `i+lowbit(i)`에 전달하여 초기화를 `O(N)`으로 줄일 수 있다.

## 오늘 문제와의 연결

2026-08-15의 [BOJ 2042 구간 합 구하기](../2026-08-15/icpc_problem.cpp)는 값 대입과 구간 합이 섞여 있다. `values[b]`로 이전 값을 기억하고 `delta=c-values[b]`를 펜윅 트리에 더한다. 질의는 `prefix_sum(c)-prefix_sum(b-1)`로 답한다. N이 최대 백만이므로 매 질의마다 구간을 순회하는 `O(N)` 방식보다 이 `O(log N)` 구조가 대회에서 필수적이다.

## 직접 해보기와 초보자 검증

1. 길이 8인 배열을 그리고 각 `tree[i]`가 담당하는 구간을 적는다.
2. 3번째 값에 5를 더할 때 방문하는 인덱스 `3,4,8`을 이진수로 써 본다.
3. `prefix_sum(7)`이 방문하는 `7,6,4`의 담당 구간이 겹치지 않는지 확인한다.
4. 위 C++ 예제를 컴파일해 `6`, `12`가 출력되는지 확인한다.
5. `left=1`일 때 `left-1`이 0이고 접두 합 함수가 즉시 0을 반환하는 경계를 설명한다.
6. 대입 갱신을 두 번 연속 수행하고 현재 값 배열 갱신을 빼면 왜 틀리는지 수치 예로 증명한다.
