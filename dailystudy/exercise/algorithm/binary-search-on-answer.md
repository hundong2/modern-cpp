# 답에 대한 이분 탐색(매개변수 탐색)

## 정의와 적용 조건

최적값을 직접 구성하기 어려울 때 후보 `x`가 가능한지를 판정하는 결정 함수 `possible(x)`로 바꾸고, 단조성을 이용해 경계를 찾는 알고리즘이다. 후보가 정렬된 정수 구간이어야 하며, `참→거짓` 또는 `거짓→참`처럼 판정 결과가 한 번만 바뀌어야 한다.

## 핵심 아이디어와 불변식

랜선 길이 `x`가 가능하면 그보다 짧은 길이도 가능하다. 따라서 가능한 값은 왼쪽에 연속된다. 닫힌 구간 구현에서 `answer`는 지금까지 확인한 가능한 최댓값이고, `[low, high]`만 아직 답 후보라는 불변식을 유지한다.

## 단계별 절차

1. 최솟값 `low`와 최댓값 `high`를 안전하게 정한다.
2. `mid = low + (high-low)/2`를 계산한다.
3. `possible(mid)`가 참이면 `answer=mid`, `low=mid+1`로 더 큰 값을 찾는다.
4. 거짓이면 `high=mid-1`로 불가능한 오른쪽을 버린다.
5. `low>high`이면 `answer`를 반환한다.

## 의사 코드

```text
answer = 실패 시 기본값
while low <= high:
    mid = low + (high - low) / 2
    if possible(mid):
        answer = mid
        low = mid + 1
    else:
        high = mid - 1
return answer
```

## 컴파일 가능한 C++ 뼈대

```cpp
// <iostream>은 입출력을, <vector>는 후보 판정 데이터를 제공한다.
#include <iostream>
#include <vector>

// const 참조는 벡터를 복사하지 않고 읽으며, limit 이상 만들 수 있는지 반환한다.
bool possible(const std::vector<long long>& values, long long limit, long long unit) {
    long long count{0}; // 중괄호 초기화로 0을 명시한다.
    for (const long long value : values) { // 모든 원소를 한 번씩 읽는다.
        count += value / unit; // 정수 나눗셈으로 완전한 단위 개수를 센다.
        if (count >= limit) { return true; } // 조건을 만족하면 조기 반환한다.
    }
    return false;
}

int main() {
    const std::vector<long long> values{8, 8}; // 템플릿 인자 long long인 벡터를 직접 초기화한다.
    long long low{1};
    long long high{8};
    long long answer{0};
    while (low <= high) {
        const long long mid{low + (high - low) / 2};
        if (possible(values, 4, mid)) {
            answer = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    std::cout << answer << '\n';
    return 0;
}
```

## 정확성 근거

판정 함수가 참인 `mid`에서는 `mid` 이하를 다시 볼 필요가 없고 `mid`를 답으로 보존한 채 더 큰 구간만 탐색한다. 거짓인 `mid`에서는 단조성상 `mid` 이상이 모두 거짓이므로 제거해도 최적해를 잃지 않는다. 매 반복마다 후보 구간이 줄고, 종료할 때 미검사 후보가 없으므로 기록한 `answer`가 가능한 최댓값이다.

## 시간·공간 복잡도

후보 범위의 크기를 `M`, 한 판정 비용을 `T`라 하면 시간은 `O(T log M)`이다. 랜선 문제에서 판정은 `K`개를 보므로 `O(K log M)`, 입력 저장 공간은 `O(K)`, 판정의 추가 공간은 `O(1)`이다.

## 흔한 실수

- 길이 0을 후보로 넣어 0으로 나눈다.
- `low=mid`처럼 구간이 줄지 않는 갱신으로 무한 반복한다.
- 합계나 중간값을 `int`로 두어 오버플로한다.
- 단조성을 증명하지 않고 이분 탐색을 적용한다.
- 최솟값 찾기와 최댓값 찾기의 경계 갱신을 섞는다.

## 변형

- 거짓에서 참으로 바뀌는 구간의 최소 참 값 찾기
- 실수 구간에서 고정 횟수 또는 오차 한계까지 탐색하기
- `lower_bound`/`upper_bound`로 정렬 컨테이너의 경계 찾기
- 생산량, 거리, 용량, 시간 같은 최적화 값을 결정 문제로 바꾸기

## 오늘 문제와의 연결

2026-08-08 BOJ 1654에서는 `possible(L) = 길이 L인 조각을 N개 이상 만들 수 있음`이다. `L`이 가능하면 더 작은 길이도 가능하므로 참에서 거짓으로 바뀌는 마지막 참을 찾는다.

## 직접 해보기와 이해 검증

1. `[1, 8]`에서 각 반복의 `low`, `mid`, `high`, `answer`를 표로 적는다.
2. `possible(x)`의 결과가 `참, 거짓, 참`처럼 다시 바뀌는 문제에 적용할 수 없는 이유를 말한다.
3. 모든 후보가 불가능할 때의 반환값을 문제 정의에 맞게 정한다.
4. 같은 뼈대를 최소 가능 값 탐색으로 바꾸고 갱신식을 비교한다.
