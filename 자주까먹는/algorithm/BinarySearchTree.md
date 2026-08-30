## 이진 탐색 내부 동작 원리

이진 탐색은 **업다운 게임**과 완벽히 같은 원리입니다. 항상 탐색 범위의 중간값(Mid)을 찔러보고, 타겟과 비교하여 절반을 버리는 과정을 반복해 $O(\log n)$의 압도적인 속도를 달성합니다.

**[탐색 시뮬레이션: Target = 7]**

```text
1단계: 초기 상태
인덱스:   0    1    2    3    4    5    6
배열:   [ 1,   3,   5,   7,   9,  11,  15 ]
         ^              ^              ^
        Left           Mid           Right
        (Mid 3번 인덱스 값은 7. 한 번에 발견!)

만약 Target이 11 이라면?
2단계: Mid(7) < Target(11) 이므로 왼쪽 절반을 버림
배열:   [ 1,   3,   5,   7,   9,  11,  15 ]
                              ^    ^    ^
                             Left Mid Right
                             (Mid 5번 인덱스 값은 11. 발견!)

```

---

## 언어별 핵심 구현 및 라인별 분석

각 언어별로 내부 원리를 직접 구현한 코드와, 실무에서 사용하는 표준 라이브러리 활용법을 모두 제공합니다.

### 1. Python (`solution.py`)

```python
import bisect # 표준 이진 탐색 라이브러리

def binary_search(nums: list[int], target: int) -> int:
    # --- [내부 원리 직접 구현] ---
    left = 0
    right = len(nums) - 1 # 배열의 양 끝 인덱스 설정
    
    while left <= right: # 교차하기 전까지 반복
        mid = left + (right - left) // 2 # 오버플로우 방지용 중간 인덱스 계산
        
        if nums[mid] == target:
            return mid # 정답을 찾으면 인덱스 반환
        elif nums[mid] < target:
            left = mid + 1 # 중간값보다 타겟이 크면, 오른쪽 절반으로 범위 축소
        else:
            right = mid - 1 # 중간값보다 타겟이 작으면, 왼쪽 절반으로 범위 축소
            
    # --- [표준 라이브러리 활용] ---
    # idx = bisect.bisect_left(nums, target)
    # if idx < len(nums) and nums[idx] == target:
    #     return idx
        
    return -1 # 찾지 못함

```

### 2. C++ (`solution.cpp`)

```cpp
#include <iostream>
#include <vector>
#include <algorithm> // binary_search, lower_bound 포함

using namespace std;

int binarySearch(const vector<int>& nums, int target) {
    // --- [내부 원리 직접 구현] ---
    int left = 0;
    int right = nums.size() - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        
        if (nums[mid] == target) return mid;
        if (nums[mid] < target) left = mid + 1;
        else right = mid - 1;
    }
    
    // --- [표준 라이브러리 활용] ---
    // bool exists = binary_search(nums.begin(), nums.end(), target); // 존재 여부만 반환
    // auto it = lower_bound(nums.begin(), nums.end(), target); // 실제 위치 반환
    // if (it != nums.end() && *it == target) return distance(nums.begin(), it);
    
    return -1;
}

```

### 3. C# (`solution.csx`)

```csharp
using System;

public int BinarySearch(int[] nums, int target) {
    // --- [내부 원리 직접 구현] ---
    int left = 0;
    int right = nums.Length - 1;
    
    while (left <= right) {
        int mid = left + (right - left) / 2; // Int32 최대값 오버플로우 방지
        
        if (nums[mid] == target) return mid;
        if (nums[mid] < target) left = mid + 1;
        else right = mid - 1;
    }
    
    // --- [표준 라이브러리 활용] ---
    // int index = Array.BinarySearch(nums, target);
    // if (index >= 0) return index; // 찾으면 0 이상의 인덱스 반환, 못 찾으면 음수 반환
    
    return -1;
}

```


정렬된 2차원 배열은 수학적인 나눗셈과 나머지 연산을 통해 완벽하게 1차원 배열처럼 취급하여 $O(\log(m \times n))$의 속도로 이진 탐색을 수행할 수 있습니다.

**2차원 배열의 1차원화 (Index Mapping)**

행(Row)의 개수가 `m`이고 열(Col)의 개수가 `n`인 행렬이 순차적으로 정렬되어 있다고 가정해 보겠습니다.

```text
[ 2차원 좌표계 ]           [ 1차원 가상 인덱스 (n=4) ]
   0   1   2   3
0 [ 1,  3,  5,  7]   =>    0   1   2   3 
1 [10, 11, 16, 20]   =>    4   5   6   7 
2 [23, 30, 34, 60]   =>    8   9  10  11 

```

1차원 가상 인덱스 `mid`를 도출했다면, 이를 다시 2차원 행렬의 `[row][col]` 위치로 변환하는 공식은 다음과 같습니다.

* **Row (행):** `mid / n` (인덱스를 열의 개수로 나눈 몫)
* **Col (열):** `mid % n` (인덱스를 열의 개수로 나눈 나머지)

예를 들어 인덱스 `6`을 2차원 배열 좌표로 바꾸면 `6 / 4 = 1` (1행), `6 % 4 = 2` (2열)이 되어 `matrix[1][2]`인 `16`을 정확히 가리키게 됩니다.

---

**1. Python 구현 및 라인별 분석 (solution.py)**

Python의 표준 라이브러리인 `bisect`는 1차원 리스트에 최적화되어 있으므로, 메모리를 낭비하며 2차원을 1차원으로 펴는(`flatten`) 대신 인덱스 매핑을 적용한 순수 이진 탐색 로직을 작성하는 것이 최고의 CS 아키텍처입니다.

```python
class Solution:
    def searchMatrix(self, matrix: list[list[int]], target: int) -> bool:
        # 1. 예외 처리: 빈 행렬이 들어오면 즉시 False 반환
        if not matrix or not matrix[0]:
            return False
            
        m, n = len(matrix), len(matrix[0])
        left, right = 0, m * n - 1 # 2차원 배열을 1차원으로 펼쳤을 때의 양 끝 인덱스
        
        # 2. 이진 탐색 코어 루프 시작
        while left <= right:
            mid = left + (right - left) // 2 # 오버플로우 방지 및 정수 몫 연산
            
            # 3. 1차원 mid 인덱스를 2차원 좌표로 매핑하여 값을 추출
            mid_val = matrix[mid // n][mid % n]
            
            # 4. 값 비교 및 탐색 범위 축소
            if mid_val == target:
                return True
            elif mid_val < target:
                left = mid + 1
            else:
                right = mid - 1
                
        return False

# 실행 예제
# sol = Solution()
# print(sol.searchMatrix([[1,3,5,7],[10,11,16,20],[23,30,34,60]], 3)) # True

```

* `m, n = len(matrix), len(matrix[0])`: 전체 행과 열의 길이를 추출하여 총 원소의 개수를 파악합니다.
* `left, right = 0, m * n - 1`: 배열의 가장 처음(0)과 가장 마지막 인덱스를 설정합니다.
* `mid = left + (right - left) // 2`: 파이썬은 정수 오버플로우가 없지만, 다른 언어와의 호환성 및 알고리즘 정석을 지키기 위한 연산입니다. `//`를 통해 소수점을 버립니다.
* `matrix[mid // n][mid % n]`: 가상의 1차원 배열 인덱스를 물리적인 2차원 배열 좌표로 치환하여 값을 가져오는 이 알고리즘의 핵심 라인입니다.

---

**2. C++ 구현 및 라인별 분석 (solution.cpp)**

```cpp
#include <vector>
#include <iostream>

using namespace std;

class Solution {
public:
    bool searchMatrix(vector<vector<int>>& matrix, int target) {
        // 1. 방어적 코드: 행렬의 행이나 열이 비어있는 경우 탐색 종료
        if (matrix.empty() || matrix[0].empty()) return false;
        
        int m = matrix.size();
        int n = matrix[0].size();
        
        // 2. 초기 포인터 세팅 (시작과 끝)
        int left = 0;
        int right = m * n - 1;
        
        // 3. 이진 탐색 수행
        while (left <= right) {
            int mid = left + (right - left) / 2; // C++에서 int 오버플로우 방지용 연산
            
            // 4. 몫과 나머지 연산을 통한 2차원 원소 접근
            int mid_val = matrix[mid / n][mid % n];
            
            if (mid_val == target) {
                return true;
            }
            if (mid_val < target) {
                left = mid + 1; // 타겟이 크면 탐색 범위를 오른쪽으로 
            } else {
                right = mid - 1; // 타겟이 작으면 탐색 범위를 왼쪽으로
            }
        }
        
        return false;
    }
};

// int main() {
//     Solution sol;
//     vector<vector<int>> matrix = {{1,3,5,7},{10,11,16,20},{23,30,34,60}};
//     cout << sol.searchMatrix(matrix, 3) << endl; // 1 (true)
//     return 0;
// }

```

* `if (matrix.empty() || matrix[0].empty())`: 런타임 에러(Segmentation Fault)를 막기 위한 필수적인 안전 장치입니다.
* `int right = m * n - 1;`: 인덱스는 0부터 시작하므로 총 원소의 갯수에서 1을 뺀 값이 가장 마지막 인덱스가 됩니다.
* `int mid_val = matrix[mid / n][mid % n];`: 열의 개수 `n`으로 나눈 몫이 행(Row), 나머지가 열(Col)이 됩니다.

---

**3. C# 구현 및 라인별 분석 (solution.csx)**

```csharp
using System;

public class Solution {
    public bool SearchMatrix(int[][] matrix, int target) {
        // 1. null 체크 및 길이 검증
        if (matrix == null || matrix.Length == 0 || matrix[0].Length == 0) {
            return false;
        }
        
        int m = matrix.Length;
        int n = matrix[0].Length;
        
        int left = 0;
        int right = m * n - 1;
        
        // 2. 이진 탐색 루프 (왼쪽 포인터가 오른쪽 포인터를 역전할 때까지)
        while (left <= right) {
            // Int32의 최대값인 21억을 넘는 덧셈 오버플로우 원천 차단
            int mid = left + (right - left) / 2;
            
            // 3. 인덱스 트랜스레이션 (1D -> 2D)
            int midVal = matrix[mid / n][mid % n];
            
            if (midVal == target) {
                return true; // 찾았을 경우 즉시 true 반환
            }
            
            if (midVal < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        
        return false; // 범위를 모두 좁혔음에도 없다면 false 반환
    }
}

// var sol = new Solution();
// int[][] matrix = new int[][] {
//     new int[] {1, 3, 5, 7},
//     new int[] {10, 11, 16, 20},
//     new int[] {23, 30, 34, 60}
// };
// Console.WriteLine(sol.SearchMatrix(matrix, 3)); // True

```

* `if (matrix == null || matrix.Length == 0)`: C#에서 참조 타입인 배열이 객체로 할당되지 않았을 경우(`null`)를 대비한 가장 강력한 유효성 검사입니다.
* `matrix.Length` / `matrix[0].Length`: 재기지 배열(Jagged Array) 형태인 `int[][]`에서 각각 행렬의 세로 길이와 가로 길이를 가져옵니다.

이 문제는 행렬의 우측 상단(Top-Right) 모서리를 이진 탐색 트리(BST)의 루트 노드로 취급하여 범위를 좁혀나가는 **스텝 탐색(Saddleback Search)** 기법으로 $O(M + N)$ 시간에 해결할 수 있습니다. 다음 행의 첫 값이 이전 행의 끝 값보다 크다는 보장이 없으므로 1차원 배열로 펼칠 수는 없지만, 각 행과 열이 정렬되어 있다는 기하학적 특성을 이용하는 것이 핵심입니다.

**탐색 시뮬레이션 (Target = 5)**

우측 상단 모서리에서 시작하면 다음과 같은 절대적인 규칙이 성립합니다.

* **현재 값보다 왼쪽:** 무조건 작습니다.
* **현재 값보다 아래쪽:** 무조건 큽니다.

이 규칙을 이용해 매 단계마다 정답이 절대 있을 수 없는 행(Row)이나 열(Column)을 하나씩 통째로 제거하며 이동합니다.

```text
탐색 시작 지점 (Top-Right)
                      ↓
[  1,   4,   7,  11, *15* ]  -> 15 > 5 (타겟이 작으므로 왼쪽 열로 이동: ←)
[  2,   5,   8,  12,  19  ]
[  3,   6,   9,  16,  22  ]

               ↓
[  1,   4,   7, *11*,  15 ]  -> 11 > 5 (타겟이 작으므로 왼쪽 열로 이동: ←)
[  2,   5,   8,  12,  19  ]

          ↓
[  1,   4,  *7*,  11,  15 ]  -> 7 > 5 (타겟이 작으므로 왼쪽 열로 이동: ←)
[  2,   5,   8,  12,  19  ]

     ↓
[  1,  *4*,  7,   11,  15 ]  -> 4 < 5 (타겟이 크므로 아래 행으로 이동: ↓)
[  2,   5,   8,   12,  19 ]

     ↓
[  1,   4,   7,   11,  15 ]
[  2,  *5*,  8,   12,  19 ]  -> 5 == 5 (발견! 탐색 종료)

```

---

### Python 구현 및 라인별 분석 (`solution.py`)

Python은 다차원 리스트의 인덱싱이 직관적입니다. 별도의 복잡한 라이브러리 없이 행과 열의 포인터를 조작하는 것이 가장 파이썬다운(Pythonic) 해결책입니다.

```python
class Solution:
    def searchMatrix(self, matrix: list[list[int]], target: int) -> bool:
        # 1. 방어적 코드: 행렬이 비어있는 경우 즉시 종료
        if not matrix or not matrix[0]:
            return False
            
        m, n = len(matrix), len(matrix[0])
        
        # 2. 시작점 설정: 우측 상단 모서리 (Row 0, Col n-1)
        row = 0
        col = n - 1
        
        # 3. 행렬 경계를 벗어나지 않을 때까지 반복 (O(M + N))
        while row < m and col >= 0:
            current = matrix[row][col]
            
            # 4. 값 비교 및 포인터 이동
            if current == target:
                return True      # 타겟 발견
            elif current > target:
                col -= 1         # 현재 값이 타겟보다 크면, 더 작은 값을 찾기 위해 왼쪽으로 이동
            else:
                row += 1         # 현재 값이 타겟보다 작으면, 더 큰 값을 찾기 위해 아래로 이동
                
        # 탐색 범위를 모두 벗어나면 타겟이 없는 것
        return False

# 실행 예제
# sol = Solution()
# print(sol.searchMatrix([[1,4,7,11,15],[2,5,8,12,19],[3,6,9,16,22]], 5)) # True

```

* `row = 0`, `col = n - 1`: 탐색의 시작점이 되는 최우측 상단 인덱스입니다. 이 지점은 해당 행에서 가장 크고, 해당 열에서 가장 작은 값입니다.
* `while row < m and col >= 0:`: `row`는 아래로만 내려가고, `col`은 왼쪽으로만 이동하므로 행렬의 경계를 벗어나면 루프가 종료됩니다.

---

### C++ 구현 및 라인별 분석 (`solution.cpp`)

C++에서는 `std::vector`를 순회할 때 인덱스 참조가 매우 빠릅니다.

```cpp
#include <vector>
#include <iostream>

using namespace std;

class Solution {
public:
    bool searchMatrix(vector<vector<int>>& matrix, int target) {
        // 1. 방어적 코드: 행렬 길이 예외 처리
        if (matrix.empty() || matrix[0].empty()) return false;
        
        int m = matrix.size();
        int n = matrix[0].size();
        
        // 2. 우측 상단 좌표 초기화
        int row = 0;
        int col = n - 1;
        
        // 3. O(M + N) 탐색 루프
        while (row < m && col >= 0) {
            int current = matrix[row][col];
            
            // 4. 이진 탐색 트리와 동일한 분기 처리
            if (current == target) {
                return true; 
            } else if (current > target) {
                col--; // 왼쪽 열로 이동
            } else {
                row++; // 아래 행으로 이동
            }
        }
        
        return false;
    }
};

// int main() {
//     Solution sol;
//     vector<vector<int>> matrix = {{1,4,7,11,15},{2,5,8,12,19},{3,6,9,16,22}};
//     cout << sol.searchMatrix(matrix, 5) << endl; // 1 (true)
//     return 0;
// }

```

* `int current = matrix[row][col];`: 매번 2차원 배열에 접근하여 캐시 지역성(Cache Locality)을 다소 해칠 수 있으나, 이동 횟수가 최대 $M + N$ 번으로 고정되어 있어 퍼포먼스 하락은 무시할 수준입니다.
* `col--` 및 `row++`: 각각 트리 구조에서 왼쪽 자식, 오른쪽 자식으로 이동하는 것과 완벽히 동일한 동작을 수행합니다.

---

### C# 구현 및 라인별 분석 (`solution.csx`)

C#에서는 다차원 배열 `[,]` 대신 가변 배열(Jagged Array) `[][]`가 메모리 구조상 접근 속도가 미세하게 더 빠르기 때문에 알고리즘 문제에서 널리 쓰입니다.

```csharp
using System;

public class Solution {
    public bool SearchMatrix(int[][] matrix, int target) {
        // 1. 널 참조 및 빈 배열 검증
        if (matrix == null || matrix.Length == 0 || matrix[0].Length == 0) {
            return false;
        }
        
        int m = matrix.Length;
        int n = matrix[0].Length;
        
        // 2. 초기 위치 설정 (Top-Right)
        int row = 0;
        int col = n - 1;
        
        // 3. 루프 실행
        while (row < m && col >= 0) {
            int current = matrix[row][col];
            
            if (current == target) {
                return true; // 정답 리턴
            }
            
            // 4. 값 비교에 따른 포인터 변경
            if (current > target) {
                col--; // 타겟보다 크면 열을 줄인다 (왼쪽 이동)
            } else {
                row++; // 타겟보다 작으면 행을 늘린다 (아래 이동)
            }
        }
        
        return false;
    }
}

// var sol = new Solution();
// int[][] matrix = new int[][] {
//     new int[] {1, 4, 7, 11, 15},
//     new int[] {2, 5, 8, 12, 19},
//     new int[] {3, 6, 9, 16, 22}
// };
// Console.WriteLine(sol.SearchMatrix(matrix, 5)); // True

```

* `if (matrix == null || ...)`: 객체 지향 언어에서 발생할 수 있는 `NullReferenceException` 및 `IndexOutOfRangeException`을 원천 차단하는 가장 훌륭한 작성 습관입니다.

