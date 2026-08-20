# LeetCode 15번 

**'3Sum'** 문제는 알고리즘 면접의 단골 손님이자, 정렬(Sorting)과 **투 포인터(Two Pointers)** 기법의 아름다운 조화를 보여주는 핵심 문제입니다. 무작정 3중 반복문을 돌리면 O(N³)의 시간 복잡도로 인해 `Time Limit Exceeded`가 발생하지만, 투 포인터를 활용하면 O(N²)으로 획기적으로 단축할 수 있습니다.

초보자분들도 직관적으로 이해하실 수 있도록 그림과 함께 완벽하게 해부해 드리겠습니다.

---

## 🎨 직관적인 알고리즘 시각화 (Visual Explanation)

이 문제의 핵심은 "중복을 어떻게 피할 것인가?"와 "어떻게 탐색 범위를 줄일 것인가?"입니다.
이를 위해 가장 먼저 **배열을 오름차순으로 정렬**합니다.

예제 1: `nums = [-1, 0, 1, 2, -1, -4]`

**1단계: 배열 정렬**
정렬된 배열: `[-4, -1, -1, 0, 1, 2]`

**2단계: 고정 포인터 `i`와 투 포인터 `left`, `right`의 이동**
배열을 순회하며 `i`를 고정시키고, `i` 다음 위치부터 `left`, 맨 끝을 `right`로 둡니다.

```text
[Step 1] i = 0 일 때 (고정값: -4)
  [-4, -1, -1,  0,  1,  2]
   ^    ^               ^
   i   left           right
 
  합계: (-4) + (-1) + 2 = -3
  결과: 0보다 작으므로, 합을 키우기 위해 left 포인터를 우측으로 이동합니다 (left++).

[Step 2] i = 1 일 때 (고정값: -1)
  [-4, -1, -1,  0,  1,  2]
        ^   ^           ^
        i  left       right

  합계: (-1) + (-1) + 2 = 0  => 정답 발견! [-1, -1, 2] 저장.
  중복을 피하기 위해 left와 right를 각각 다음 다른 숫자가 나올 때까지 이동시킵니다.

  [-4, -1, -1,  0,  1,  2]
        ^       ^   ^
        i     left right

  합계: (-1) + 0 + 1 = 0  => 정답 발견! [-1, 0, 1] 저장.

```

**[핵심 원리]**

1. 배열이 정렬되어 있으므로, 세 수의 합이 0보다 **작으면** `left`를 우측으로 이동(값을 키움).
2. 합이 0보다 **크면** `right`를 좌측으로 이동(값을 줄임).
3. **중복 건너뛰기:** 이전 값과 동일한 값이라면 탐색할 필요가 없으므로 과감히 건너뜁니다 (`i`, `left`, `right` 모두 해당).

---

## 💻 Python 예제 코드 (3sum.py)

파이썬은 리스트 컴프리헨션과 내장 정렬 함수 `sort()`를 사용하여 매우 간결하게 작성할 수 있습니다.

```python
# 3sum.py
from typing import List

class Solution:
    def threeSum(self, nums: List[int]) -> List[List[int]]:
        # 결과를 저장할 리스트
        result = []
        
        # 1. 배열을 오름차순으로 정렬 (O(N log N))
        nums.sort()
        n = len(nums)
        
        # 2. 기준점 i를 0부터 n-3까지 순회 (세 수를 뽑아야 하므로 끝에서 두 자리는 남김)
        for i in range(n - 2):
            # [중복 건너뛰기 1] 현재 값이 이전 값과 같다면 동일한 결과가 나오므로 패스
            if i > 0 and nums[i] == nums[i - 1]:
                continue
            
            # 투 포인터 초기화
            left = i + 1      # i의 바로 다음 인덱스
            right = n - 1     # 배열의 맨 끝 인덱스
            
            # 3. 투 포인터 탐색 (O(N))
            while left < right:
                current_sum = nums[i] + nums[left] + nums[right]
                
                if current_sum < 0:
                    # 합이 0보다 작으면 값을 키워야 하므로 left를 우측으로 이동
                    left += 1
                elif current_sum > 0:
                    # 합이 0보다 크면 값을 줄여야 하므로 right를 좌측으로 이동
                    right -= 1
                else:
                    # 합이 0을 만족하는 경우!
                    result.append([nums[i], nums[left], nums[right]])
                    
                    # [중복 건너뛰기 2] left 포인터의 다음 값이 현재와 같다면 건너뜀
                    while left < right and nums[left] == nums[left + 1]:
                        left += 1
                    # [중복 건너뛰기 3] right 포인터의 이전 값이 현재와 같다면 건너뜀
                    while left < right and nums[right] == nums[right - 1]:
                        right -= 1
                        
                    # 중복을 모두 건너뛴 후 포인터 이동
                    left += 1
                    right -= 1
                    
        return result

# 실행 예제
if __name__ == "__main__":
    solution = Solution()
    print("Example 1:", solution.threeSum([-1, 0, 1, 2, -1, -4]))
    print("Example 2:", solution.threeSum([0, 1, 1]))
    print("Example 3:", solution.threeSum([0, 0, 0]))

```

---

## 💻 C++ 예제 코드 (3sum.cpp)

C++에서는 STL의 `<vector>`와 `<algorithm>` 헤더의 `std::sort`를 적극적으로 활용합니다.

```cpp
// 3sum.cpp
#include <iostream>
#include <vector>
#include <algorithm> // std::sort 사용을 위함

using namespace std;

class Solution {
public:
    vector<vector<int>> threeSum(vector<int>& nums) {
        vector<vector<int>> result;
        int n = nums.size();
        
        // 1. std::sort를 이용한 오름차순 정렬 (기본적으로 O(N log N)인 IntroSort 사용)
        sort(nums.begin(), nums.end());
        
        // 배열의 길이가 3 미만이면 빈 결과 반환
        if (n < 3) return result;
        
        // 2. 기준 포인터 i 순회
        for (int i = 0; i < n - 2; ++i) {
            // 정렬된 상태이므로 nums[i]가 0보다 크면 더 이상 합이 0이 될 수 없음 (조기 종료 최적화)
            if (nums[i] > 0) break;
            
            // i 중복 제거
            if (i > 0 && nums[i] == nums[i - 1]) continue;
            
            int left = i + 1;
            int right = n - 1;
            
            // 3. 투 포인터 탐색 로직
            while (left < right) {
                int sum = nums[i] + nums[left] + nums[right];
                
                if (sum == 0) {
                    result.push_back({nums[i], nums[left], nums[right]});
                    
                    // left 중복 제거
                    while (left < right && nums[left] == nums[left + 1]) left++;
                    // right 중복 제거
                    while (left < right && nums[right] == nums[right - 1]) right--;
                    
                    // 정답을 하나 찾았으므로 양쪽 포인터 모두 안쪽으로 한 칸씩 이동
                    left++;
                    right--;
                } 
                else if (sum < 0) {
                    left++; // 합이 부족하므로 증가
                } 
                else {
                    right--; // 합이 넘치므로 감소
                }
            }
        }
        return result;
    }
};

// 실행 예제
int main() {
    Solution sol;
    vector<int> nums = {-1, 0, 1, 2, -1, -4};
    vector<vector<int>> res = sol.threeSum(nums);
    
    cout << "Result: ";
    for (const auto& triplet : res) {
        cout << "[";
        for (int num : triplet) cout << num << " ";
        cout << "] ";
    }
    cout << endl;
    return 0;
}

```

---

## 💻 C# 스크립트 예제 코드 (3sum.csx)

C#의 경우 `Array.Sort`를 사용하고, 반환형이 인터페이스인 `IList<IList<int>>`를 요구하므로 `List<IList<int>>`로 객체를 생성하여 반환합니다. C# 스크립트(.csx) 환경에서 바로 실행 가능한 구조입니다.

```csharp
// 3sum.csx
using System;
using System.Collections.Generic;

public class Solution {
    public IList<IList<int>> ThreeSum(int[] nums) {
        IList<IList<int>> result = new List<IList<int>>();
        
        // 1. C# 표준 라이브러리를 통한 배열 정렬 (O(N log N))
        Array.Sort(nums);
        
        int n = nums.Length;
        
        // 2. 고정 포인터 i 순회
        for (int i = 0; i < n - 2; i++) {
            // [최적화] 가장 작은 수인 nums[i]가 양수면 0을 만들 수 없음
            if (nums[i] > 0) break;
            
            // i 중복 제거
            if (i > 0 && nums[i] == nums[i - 1]) continue;
            
            int left = i + 1;
            int right = n - 1;
            
            // 3. left와 right를 교차하지 않을 때까지 반복
            while (left < right) {
                int sum = nums[i] + nums[left] + nums[right];
                
                if (sum == 0) {
                    // C# 다차원 컬렉션 추가 방식
                    result.Add(new List<int> { nums[i], nums[left], nums[right] });
                    
                    // 다음 포인터가 동일한 값이면 스킵하여 중복 트리플릿 생성 방지
                    while (left < right && nums[left] == nums[left + 1]) left++;
                    while (left < right && nums[right] == nums[right - 1]) right--;
                    
                    left++;
                    right--;
                } 
                else if (sum < 0) {
                    left++;
                } 
                else {
                    right--;
                }
            }
        }
        
        return result;
    }
}

// 실행 예제
var solution = new Solution();
int[] nums = { -1, 0, 1, 2, -1, -4 };
var result = solution.ThreeSum(nums);

Console.WriteLine("결과 리스트:");
foreach(var triplet in result) {
    Console.WriteLine($"[{string.Join(", ", triplet)}]");
}

```

---


1. **`nums.sort()` / `Array.Sort()**`
* 왜 정렬을 할까요? 정렬을 해야 투 포인터(작으면 오른쪽으로, 크면 왼쪽으로 이동)라는 방향성을 가진 탐색이 가능해집니다. 정렬되지 않은 배열에서는 포인터를 어디로 옮길지 결정할 수 없습니다.


2. **`if (i > 0 && nums[i] == nums[i - 1]) continue;`**
* 첫 번째 원소(`nums[i]`)의 중복을 체크합니다. 정렬되어 있기 때문에 같은 숫자는 연속해서 나타납니다. 첫 번째 원소가 같으면 찾게 될 투 포인터 조합도 완전히 같아지므로, 연산의 낭비를 막고 문제의 조건인 "unique triplets"를 만족시키기 위해 건너뜁니다.


3. **`while (left < right && nums[left] == nums[left + 1]) left++;`**
* 정답(합이 0)을 찾았을 때 실행되는 줄입니다. 현재 `left`와 다음 `left`의 값이 같다면 굳이 계산할 필요 없이 스킵합니다. 이 줄이 없다면 `[-1, -1, -1, 2, 2]` 같은 배열에서 `[-1, -1, 2]`가 중복해서 출력되는 참사가 벌어집니다.



시간 복잡도는 정렬에 O(N log N), 외부 루프가 N번, 내부 while 루프가 N번 도달하므로 합쳐서 O(N²)이 됩니다. 공간 복잡도는 정렬 알고리즘에 따라 O(1) 또는 O(N)을 차지합니다.

## My Code 

```cpp
#include<numeric>

class Solution {
public:
    vector<vector<int>> threeSum(vector<int>& nums) {
        const auto& sum = std::accumulate(nums.begin(), nums.end(), 0);
        if( nums.size() == 3 && sum == 0 ) return { nums };
        else if( nums.size() < 3 ) return {};
        std::sort(nums.begin(), nums.end());
        int n = nums.size();
        vector<vector<int>> result {};
        for(int i{0}; i < n - 2; i++){
            if( nums[i] > 0 ) break;
            if( i > 0 && nums[i] == nums[i-1] ) continue;

            int left = i + 1;
            int right = n - 1;
            while(left < right ){
                int sum = nums[i] + nums[left] + nums[right];
                if( sum == 0 ){
                    result.push_back({nums[i], nums[left], nums[right]});
                    while(left < right && nums[left] == nums[left+1]) left++;
                    while(left < right && nums[right] == nums[right-1]) right--;
                    left++; right--;
                }
                else if(sum < 0){
                    left++;
                }
                else{
                    right--;
                }
            }
        }

        return result;
    }
};
```

## C#

```csharp
public class Solution {
    public IList<IList<int>> ThreeSum(int[] nums) {
        var result = new List<IList<int>>();
        int n = nums.Length;
        Array.Sort(nums);
        for(int i = 0; i < n - 2; i++ ){
            if( i > 0 && nums[i] == nums[i-1]) continue;
            int left = i + 1;
            int right = n - 1;
            while(left < right)
            {
                int sum = nums[i] + nums[left] + nums[right];
                switch(sum){
                    case 0:
                        result.Add(new List<int>{nums[i], nums[left], nums[right]});
                        while(left < right && nums[left] == nums[left + 1]) left++;
                        while(left < right && nums[right] == nums[right -1]) right--;
                        left++; right--;
                        break;
                    case < 0:
                        left++;
                        break;
                    default:
                        right--;
                        break;
                }

            }
        }
        return result;
    }
}
```