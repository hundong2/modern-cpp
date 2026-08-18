## **"11. Container With Most Water"** 문제

## 1. 문제 분석 및 핵심 원리

이 문제의 목표는 $x$축과 두 개의 수직선(기둥)이 만드는 직사각형의 최대 넓이(Area)를 구하는 것입니다.

* **넓이 공식:** $Area = \min(\text{height}[left], \text{height}[right]) \times (right - left)$
* **핵심 딜레마:** 넓이를 최대화하려면 기둥 사이의 간격(너비)이 넓어야 하고, 두 기둥 중 짧은 기둥의 높이(높이)가 높아야 합니다.

### 직관적인 그래프 시각화

주어진 예제 1 `height = [1,8,6,2,5,4,8,3,7]`을 그래프로 그려보겠습니다.

```text
높이(height)
 8 |    [ ]                     [ ]
 7 |    [ ]                     [ ]       [ ] <--- 우측 끝 기둥(index 8)
 6 |    [ ]   [ ]               [ ]       [ ]
 5 |    [ ]   [ ]       [ ]     [ ]       [ ]
 4 |    [ ]   [ ]       [ ] [ ] [ ]       [ ]
 3 |    [ ]   [ ]       [ ] [ ] [ ] [ ]   [ ]
 2 |    [ ]   [ ] [ ]   [ ] [ ] [ ] [ ]   [ ]
 1 |[ ] [ ]   [ ] [ ]   [ ] [ ] [ ] [ ]   [ ]
 0 +-------------------------------------------
     0   1     2   3     4   5   6   7     8   (인덱스)
         ^                                 ^
        left                             right

```

가장 직관적인 방법은 모든 쌍을 다 비교하는 완전 탐색(Brute Force)입니다. 하지만 이 경우 시간 복잡도가 $O(N^2)$이 되어, $N$이 $10^5$인 제약 조건에서는 시간 초과(Time Limit Exceeded)가 발생합니다.

### 최고 아키텍트의 해결책: 투 포인터 (Two Pointers)

가장 넓은 너비에서 시작해서 너비를 줄여나가며 더 높은 기둥을 찾는 전략을 취합니다.

1. 가장 왼쪽(`left = 0`)과 가장 오른쪽(`right = 8`)에 포인터를 둡니다. (현재 너비는 최대)
2. 현재 상태에서 물의 넓이를 구하고, 최댓값을 갱신합니다.
3. **포인터 이동 규칙 (가장 중요):** 두 기둥 중 **높이가 더 낮은 쪽의 포인터를 안쪽으로 이동**시킵니다.
* *왜 낮은 쪽을 이동시킬까요?* 너비는 포인터를 움직일 때마다 무조건 1씩 줄어듭니다. 만약 높은 쪽 기둥을 안쪽으로 움직인다면, 새롭게 만들어지는 직사각형의 높이는 여전히 기존의 '낮은 기둥'에 의해 제한되므로 넓이가 무조건 줄어듭니다. 반면, 낮은 쪽 기둥을 움직이면 더 높은 기둥을 만나 높이가 커져서 전체 넓이가 증가할 "가능성"이 생기기 때문입니다.



---

## 2. 언어별 예제 코드 및 상세 주석 설명

세 가지 주요 언어(Python, C++, C#)로 실행 가능한 코드를 작성해 드립니다. 각 언어의 표준 라이브러리 함수(예: `max`, `min`)를 적극 활용했습니다.

### Python 구현 (`container.py`)

Python의 내장 함수 `max()`와 `min()`을 사용하여 간결하게 구현할 수 있습니다.

```python
def maxArea(height: list[int]) -> int:
    # left는 배열의 시작 인덱스, right는 배열의 끝 인덱스를 가리킵니다.
    left = 0
    right = len(height) - 1
    
    # 지금까지 발견한 가장 큰 물의 양(넓이)을 저장할 변수입니다.
    max_area = 0
    
    # 두 포인터가 교차하기 전까지 반복합니다. (같은 기둥을 가리키면 너비가 0이 되므로 < 사용)
    while left < right:
        # 직사각형의 너비(width)는 오른쪽 인덱스에서 왼쪽 인덱스를 뺀 값입니다.
        width = right - left
        
        # 직사각형의 높이(h)는 두 기둥 중 '더 짧은' 기둥의 높이에 의해 결정됩니다.
        h = min(height[left], height[right])
        
        # 현재 컨테이너의 넓이를 계산합니다.
        current_area = width * h
        
        # 기존에 저장된 최대 넓이와 현재 넓이를 비교하여 더 큰 값으로 갱신합니다.
        max_area = max(max_area, current_area)
        
        # 핵심 로직: 더 짧은 기둥 쪽의 포인터를 안쪽으로 한 칸 이동시킵니다.
        # 더 높은 기둥을 찾아 넓이를 최대화할 가능성을 탐색하기 위함입니다.
        if height[left] < height[right]:
            left += 1  # 왼쪽 기둥이 낮으면 왼쪽 포인터를 오른쪽으로 이동
        else:
            right -= 1 # 오른쪽 기둥이 낮거나 같으면 오른쪽 포인터를 왼쪽으로 이동
            
    # 탐색이 끝나면 찾은 최대 넓이를 반환합니다.
    return max_area

# 실행 예제
if __name__ == "__main__":
    test_height = [1, 8, 6, 2, 5, 4, 8, 3, 7]
    result = maxArea(test_height)
    print(f"최대 담을 수 있는 물의 양: {result}") # 출력: 49

```

---

### C++ 구현 (`container.cpp`)

C++에서는 `<algorithm>` 헤더의 `std::max`와 `std::min`을 사용하여 성능을 극대화합니다.

```cpp
#include <iostream>
#include <vector>
#include <algorithm> // std::max, std::min 사용을 위한 표준 알고리즘 헤더

using namespace std;

class Solution {
public:
    int maxArea(vector<int>& height) {
        // 투 포인터 초기화
        int left = 0;
        int right = height.size() - 1;
        
        int max_area = 0; // 최대 넓이 저장 변수
        
        while (left < right) {
            // 현재 높이는 두 기둥 중 작은 값
            // std::min은 두 값 중 더 작은 값을 반환하는 표준 라이브러리 함수입니다.
            int current_height = std::min(height[left], height[right]);
            
            // 현재 너비 계산
            int current_width = right - left;
            
            // std::max를 사용하여 기존 max_area와 현재 계산된 넓이 중 더 큰 값을 저장합니다.
            max_area = std::max(max_area, current_height * current_width);
            
            // 포인터 이동 로직
            if (height[left] < height[right]) {
                left++; // 왼쪽을 안쪽으로
            } else {
                right--; // 오른쪽을 안쪽으로
            }
        }
        
        return max_area;
    }
};

// 실행 예제
int main() {
    Solution sol;
    vector<int> test_height = {1, 8, 6, 2, 5, 4, 8, 3, 7};
    int result = sol.maxArea(test_height);
    cout << "최대 담을 수 있는 물의 양: " << result << endl; // 출력: 49
    return 0;
}

```

---

### C# 스크립트 구현 (`container.csx`)

C#에서는 `System.Math` 클래스의 `Math.Max`와 `Math.Min` 메서드를 활용합니다. `.csx` 포맷이므로 네임스페이스 선언 없이 바로 실행 가능한 스크립트 형태로 작성되었습니다.

```csharp
using System;

public class Solution {
    public int MaxArea(int[] height) {
        // 배열의 양 끝점을 가리키는 포인터 설정
        int left = 0;
        int right = height.Length - 1;
        int maxArea = 0;
        
        // 포인터가 겹치지 않는 동안 반복
        while (left < right) {
            // Math.Min을 사용하여 두 기둥 중 수위의 한계(더 낮은 높이)를 구합니다.
            int currentHeight = Math.Min(height[left], height[right]);
            int currentWidth = right - left;
            
            // Math.Max를 활용하여 현재까지의 최대 면적을 갱신합니다.
            maxArea = Math.Max(maxArea, currentHeight * currentWidth);
            
            // 더 낮은 기둥을 버리고, 안쪽에 있을지도 모르는 더 높은 기둥을 향해 이동합니다.
            if (height[left] < height[right]) {
                left++;
            } else {
                right--;
            }
        }
        
        return maxArea;
    }
}

// 실행 예제 (csx 스크립트 실행)
int[] testHeight = new int[] { 1, 8, 6, 2, 5, 4, 8, 3, 7 };
Solution sol = new Solution();
int result = sol.MaxArea(testHeight);
Console.WriteLine($"최대 담을 수 있는 물의 양: {result}"); // 출력: 49

```

---

## 3. 복잡도 분석 (Complexity Analysis)

최종적으로 이 알고리즘 아키텍처가 얼마나 효율적인지 증명해 드리겠습니다.

* **시간 복잡도 (Time Complexity):** $O(N)$
* 배열의 길이 $N$에 대하여, 두 개의 포인터(`left`, `right`)가 각각 반대편에서 시작하여 배열을 정확히 한 번만 순회합니다. 중첩된 루프가 없으므로 연산 속도가 매우 빠릅니다.


* **공간 복잡도 (Space Complexity):** $O(1)$
* 포인터 변수(`left`, `right`)와 최대 넓이를 저장할 변수(`max_area`) 등 상수 개의 추가 메모리만 사용합니다. 입력 배열의 크기가 아무리 커져도 추가 메모리를 요구하지 않으므로 메모리 효율이 극대화된 알고리즘입니다.



투 포인터의 원리를 확실히 체득하셨기를 바랍니다. 추가로 알고리즘의 특정 부분이나 다른 유형의 문제에 대해 궁금하신 점이 있다면 언제든 편하게 말씀해 주십시오.