안녕하세요! 코드 알고리즘 최고 박사 이동훈입니다. 질문해주신 **16. 3Sum Closest** 문제는 배열에서 세 수의 합이 주어진 타겟 값에 가장 가까운 경우를 찾는 훌륭한 알고리즘 문제입니다.

모든 경우의 수를 찾는 브루트 포스(Brute Force) 방식은 $O(n^3)$의 시간이 걸리지만, 정렬(Sorting)과 **투 포인터(Two Pointers)** 기법을 활용하면 $O(n^2)$으로 최적화할 수 있습니다.

---

## 1. 알고리즘 동작 원리 (투 포인터 기법)

알고리즘의 핵심은 배열을 먼저 정렬한 뒤, 기준점 `i`를 하나 잡고 남은 배열의 양 끝에 포인터 `Left`와 `Right`를 배치하여 범위를 좁혀나가는 것입니다.

> **시각적 이해를 위한 과정 (Example 1 기준)**
> 정렬된 배열: `[-4, -1, 1, 2]`, 타겟 = 1
> * **기준점 i = 0 (`-4`)**
> * `[-4 (i), -1 (Left), 1, 2 (Right)]`
> * 현재 합: $-4 + (-1) + 2 = -3$
> * 타겟(1)보다 작으므로 합을 키우기 위해 `Left`를 오른쪽으로 이동합니다.
> 
> 
> * **포인터 이동 후**
> * `[-4 (i), -1, 1 (Left), 2 (Right)]`
> * 현재 합: $-4 + 1 + 2 = -1$
> * 여전히 타겟보다 작지만, 이게 최선입니다.
> 
> 
> * **기준점 i = 1 (`-1`)**
> * `[-4, -1 (i), 1 (Left), 2 (Right)]`
> * 현재 합: $-1 + 1 + 2 = 2$
> * 타겟(1)과 가장 가까우므로 (차이: 1), 정답은 2가 됩니다.
> 
> 
> 
> 

---

## 2. 언어별 예제 코드 및 상세 분석

각 언어별 표준 라이브러리와 함수를 적극 활용하여 작성된 실행 가능한 코드입니다.

### C++ (`.cpp`)

```cpp
#include <iostream>
#include <vector>
#include <algorithm> // std::sort 사용
#include <cmath>     // std::abs 사용

using namespace std;

class Solution {
public:
    int threeSumClosest(vector<int>& nums, int target) {
        // 1. 배열을 오름차순으로 정렬 (O(n log n))
        sort(nums.begin(), nums.end());
        
        int closestSum = nums[0] + nums[1] + nums[2]; // 초기 가장 가까운 합 설정
        
        // 2. 기준점 i를 0부터 n-3까지 반복
        for (int i = 0; i < nums.size() - 2; ++i) {
            int left = i + 1;             // 왼쪽 포인터는 i의 바로 오른쪽
            int right = nums.size() - 1;  // 오른쪽 포인터는 배열의 끝
            
            // 3. 투 포인터 탐색 진행
            while (left < right) {
                int currentSum = nums[i] + nums[left] + nums[right];
                
                // 현재 합이 타겟과 더 가깝다면 closestSum 업데이트
                if (abs(currentSum - target) < abs(closestSum - target)) {
                    closestSum = currentSum;
                }
                
                // 4. 합의 크기에 따라 포인터 이동
                if (currentSum < target) {
                    left++; // 합을 키워야 하므로 왼쪽 포인터 우측 이동
                } else if (currentSum > target) {
                    right--; // 합을 줄여야 하므로 오른쪽 포인터 좌측 이동
                } else {
                    return currentSum; // 타겟과 정확히 일치하면 즉시 반환
                }
            }
        }
        return closestSum;
    }
};

```

### Python (`.py`)

```python
from typing import List

class Solution:
    def threeSumClosest(self, nums: List[int], target: int) -> int:
        # 1. 파이썬 내장 Timsort를 이용한 배열 정렬 (O(n log n))
        nums.sort()
        
        closest_sum = float('inf') # 무한대로 초기화하여 첫 비교시 무조건 갱신되게 함
        
        # 2. 기준 원소 순회 (뒤에 2개의 원소를 남겨야 하므로 len - 2 까지)
        for i in range(len(nums) - 2):
            left = i + 1
            right = len(nums) - 1
            
            # 3. 양 끝에서 좁혀오는 투 포인터
            while left < right:
                current_sum = nums[i] + nums[left] + nums[right]
                
                # 내장 함수 abs()를 사용하여 절대값 비교
                if abs(current_sum - target) < abs(closest_sum - target):
                    closest_sum = current_sum
                
                # 4. 조건에 따른 포인터 조작
                if current_sum < target:
                    left += 1
                elif current_sum > target:
                    right -= 1
                else:
                    # 차이가 0인 완벽한 매칭이므로 더 탐색할 필요 없음
                    return current_sum 
                    
        return closest_sum

```

### C# (`.csx`)

```csharp
using System;

public class Solution {
    public int ThreeSumClosest(int[] nums, int target) {
        // 1. Array 클래스의 퀵정렬 기반 메서드 사용
        Array.Sort(nums);
        
        int closestSum = nums[0] + nums[1] + nums[2];
        
        // 2. 기준점 반복문
        for (int i = 0; i < nums.Length - 2; i++) {
            int left = i + 1;
            int right = nums.Length - 1;
            
            // 3. 포인터가 엇갈릴 때까지 반복
            while (left < right) {
                int currentSum = nums[i] + nums[left] + nums[right];
                
                // Math.Abs 메서드를 이용한 최적값 갱신
                if (Math.Abs(currentSum - target) < Math.Abs(closestSum - target)) {
                    closestSum = currentSum;
                }
                
                // 4. 합 크기에 따른 분기 처리
                if (currentSum < target) {
                    left++;
                } else if (currentSum > target) {
                    right--;
                } else {
                    return currentSum; // 정확히 일치하는 경우 조기 종료(Early Exit)
                }
            }
        }
        return closestSum;
    }
}

```

---

## 3. 복잡도 요약

| 지표 | 복잡도 | 설명 |
| --- | --- | --- |
| **Time Complexity** | $O(n^2)$ | 배열 정렬 $O(n \log n)$, $n$개의 기준점에 대한 $O(n)$ 포인터 탐색. |
| **Space Complexity** | $O(1)$ ~ $O(n)$ | 정렬 알고리즘에 따라 언어별로 사용하는 추가 메모리가 다릅니다. |

# mysolution

```csharp
public class Solution {
    public int ThreeSumClosest(int[] nums, int target) {
        var n = nums.Length;
        Array.Sort(nums);
        if(n < 3 ) return 0;
        int distance = nums[0] + nums[1] + nums[2];
        for(int i = 0; i < n - 2; i++ ){
            if( i > 0 && nums[i] == nums[i - 1]) continue;
            int left = i + 1;
            int right = n - 1;

            while(left < right) {
                var sum = nums[i] + nums[left] + nums[right];
                if( Math.Abs(distance - target) >= Math.Abs(sum - target) ) {
                    distance = sum;
                } 
                if( sum < target ){
                    left++;
                } else if( sum > target ){
                    right--;
                } else {
                    return sum;
                }
            }
        }
        return distance;
    }
}
```

### Cpp

```cpp
class Solution {
public:
    int threeSumClosest(vector<int>& nums, int target) {
        int n = nums.size();
        if( n < 3 ) return 0;
        std::sort(nums.begin(), nums.end());
        int distance = nums[0] + nums[1] + nums[2];
        for(int i{0}; i < n - 2; i++){
            //if( i > 0 && nums[i] == nums[i - 1]) continue;
            int left = i + 1;
            int right = n - 1;
            while(left < right) {
                int sum = nums[i] + nums[left] + nums[right];
                if( std::abs(distance - target) > std::abs(sum - target)){
                    distance = sum;
                }
                if( sum < target){
                    left++;
                } else if( sum > target ){
                    right--;
                } else {
                    return sum;
                }

            }
        }
        return distance;
    }
};
```