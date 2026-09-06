# algorithm 

- [01.MAX, MIN value 확인](#reverse-integer). 
- [02.IsDigit(C#)](#isdigit-1). 
    - [02-1. isdigit(C++)](#isdigit). 
- [03. Stack & Queue](./StackAndQueue.md). 
- [04. Priority Queue](./priority_queue.md). 
    - [Problem with leetcode](./23.Merge%20k%20Sorted%20Lists.md). 
- [25. Sorting with 3 points](./25.Reverse%20Nodes%20in%20k-Group.md). 
- [26. Using Unique, Distance functions](./26.DuplicateRemove.md). 
    - `std::unique`, `std::distance` 사용법 
- [string find function using string::npos ](./28.FindSubstring.md). 
    - `string.find`
    - [string npos](../basic/string.md). 
- [shift calculator with <<](./29.Divide%20Two%20Integers.md). 
    - shift 연산
- [hash map을 활용한 two point sliding window 구현](./30.TwoPointSlidingWindow.md). 
    - hard problem + using dictionary of csharp, map of cpp
- [std::reverse, std::swap 을 활용한 순열 문제](./31.Next%20Permutation.md). 
    - c++ std::reverse, std::swap
    - c# Array.Reverse, (A, B) = (B, A) -> swap 
- [stack을 이용한 유효한 () 구문 찾기 문제](./32.Longest%20Valid%20Parentheses.md)  
- [(중요!) 이진 탐색 트리를 이용한 문제 풀기](./34.Find%20First%20and%20Last%20Position.md)  
    - [Binary Search Tree](./BinarySearchTree.md). 
    - `mid = left + (right - left)/2` 를 구한 뒤 `mid` 값을 기준으로 target값이 크면 left = mid;
    -  작으면 right = mid로 바꾼 뒤 진행하는 방법
    - `std::lower_bound`, `std::upper_bound` from `<algorithm>` library 사용
    - iterator 에서 간격 구하는 방법 `distance(.begin(), .iterator )`
    - c#의 경우 upper, lower bound를 직접 구현해야 함. 
    - [35 problem lower bound를 활용한 문제](./35.Search%20Insert%20Position.md). 
- [using hash set and array 2dim](./36.ValidSudoku.md). 
    - c# HashMap
    - cpp array 
- [Using 2 dim array search for matching sudoku](./37.SudokuSolver.md)
    - [0~9)까지 i문을 돌리면서 현재 row와 col에서 행, 렬에 포함된 숫자가 있는지 체크하고, 현재 row,col기준에 속한 박스에서도 해당 값이 존재하는지 체크 
    - rowBox = 3 * ( r/ 3) + i/3 : 3배수 내에 
    - colBox = 3 * ( c/ 3) + i%3 : 3배수 내에
    - board[rowBox][colBox] == value 로 체크 
- [stringBuilder for C# and using string reserve + two pointer problem](./38.CountAndSay.md). 
    - two pointer를 활용한 문제 해결
    - StringBuilder, Operator+ 를 사용하지 않고 Array 고정으로 연산 속도를 올리는 방법 등
- [Vector push_back, pop_back](./39.CombinationSum.md). 
    - dotnet Add, RemoveAt (C#). 
    - 중복 허용 `BFS` 알고리즘
- [중복을 허용하지 않는 BFS 알고리즘 문제 ](./40.CombinationSum2.md). 
    - `if (i > idx && candidates[i] == candidates[i - 1]) continue;`. 이 핵심. 시작 부분에서 중복 된 값이 DFS 로 빠지지 않도록 막는 역할을 함. 
    - 예를 들어, `1, 1, 2, ... `일때 1 이 들어가고 다음 1이 들어가는 것을 막아줌 즉, 1, 2 또는 1, 2 가 두번 Depth가 빠지는 것을 막아줌. 
- [Cycle sort algorithm - O(n) 시간, 공간 복잡도 O(1)사용 예](./41.FirstMissingPositive.md%20). 
    - 1, 2, 3, 4, 5, ..., n 이상 적인 값일 경우 0번 에는 1, 1번에는 2 값이 들어가야함. 따라서 i 를 이동하면서 해당 포인트의 값을 배열의 array위치로 써서 swap해준다. i 포인트에서 swap이 모두 끝날때 까지 while loop를 돌리면서 진행. 
    - sorting을 마친 뒤 이상적인 값 위치에 값이 있는지 확인 만약 i 위치에서 i+1이 아니라면 i+1이 결핍이라는 뜻. 
    - 만약 전체 수행 했지만 결핍이 없다면 n + 1이 결핍값 
- [(자주 출제!) 투포인트 정석문제 (채워진 물 깊이 구하기)](./42.TrappingRainWater.md)  
    - left, right point 를 통해 left_max, right_max 를 비교하여 현재 left, right포인트에서 물의 깊이를 구하는 알고리즘 
    - O(N)의 시간으로 물의 깊이를 구할 수 있다. 
- [문자열 곱셈- 세로셈 곱셈(Grade-School Multiplication)](./43.MultiplyStrings.md)  
    - carry 되는 데이터에 대해 합을 저장 한 뒤 추후 해당 array에 해당하는 값 연산 시 올림 수 계산
    - number1, number2의 합만큼 array를 할당 후 저장 
    - number1 * number2 에서 i+j자리에 값 연산에 대하여 i+j+1부터 저장 
    - 243 * 34 일때 끝자리 수부터 곱하면 i=2, j=1 => i+j = 3 이고 4자리에 값을 일의 자리를 저장 i+j만큼 할당 된 array의 경우 (0,1,2,3, i + j, i + j + 1) 까지 생성 되기 때문. 
- **DP, Dynamic Programming Problem**
    - [(Hard))egular Expression](./regular_expression_hard.md)  
        - 'c*' 일 경우 문자열이 0이거나 c가 1이상 반복되는 경우임. 
    - [Hard Wild Card Problem](./44.WildCardMatching.md). 
        - `*` 일때는 없거나 1개 이상인 경우를 따져야 하고, dp에서 대각선 앞 s, p 문자열에서 s문자 하나 앞(s문자 없거나, 하나 먹혔을 때)와 p하나 앞(* 없을때) 이기때문 `dp[i][j] = dp[i-1][j] || dp[i][j-1]`
        - `?`이고 p와 s 문자가 같을 때 이므로 대각 선 앞 `dp[i][j] = dp[i-1][j-1]` 이다. 
- [Greedy Algorithm을 이용한 Count Search](./45.JumpGame2.md). 
    - O(n) 동안 Counting 할 수 있는 알고리즘 
    - 다음 최대 index에서 최대 일 경우 까지 모든 경우들은 그 최대 이하이므로 최대까지 Search하고 난 뒤 count를 수행. 끝까지 검색한 후 break후 결과 리턴 
    - C#에서는 배열의 내장 함수 `Length`를 적극 활용, Count()는 Linq함수 이므로 성능 저하 유발 
- [Permutation 조합 만들기 문제](./46.Permutation.md). 
    - C#의 경우 DFS 를 통해 결과 도출
    - C++의 경우 `Algorithm`라이브러리의 `next_permutation` function을 사용하여 문제를 해결 ( [std::next_permutation](./next_permutation.md)). 


## C++

### reverse integer 

1. INT_MAX, INT_MIN

- `#include<climits>` 
- [example code - reverse integer](./reverse_integer.cpp). 
- [reverse integer for c#](#reverse-integer-for-c). 
- [reverse integer for python]()

### isdigit

- `#include<cctype>` - `isdigit()` 
- [string to integer problem](./stringtointeger.cpp). 

## C#

### reverse integer for c#

1. Int32.MaxValue, Int32.MinValue 

- `using System`
- [example code - reverse integer](./reverse_integer.csx). 

### IsDigit

- 숫자인지 체크 

```csharp
using System;

public class Solution {
    public int MyAtoi(string s) {
        int i = 0;
        int n = s.Length;
        int sign = 1;
        int result = 0;

        // [State 1] 공백 건너뛰기
        while (i < n && s[i] == ' ') {
            i++;
        }

        if (i == n) return 0;

        // [State 2] 부호 확인
        if (s[i] == '+' || s[i] == '-') {
            sign = (s[i] == '-') ? -1 : 1;
            i++;
        }

        // [State 3] 숫자 파싱
        // char.IsDigit()를 사용하여 숫자인지 안전하게 확인
        while (i < n && char.IsDigit(s[i])) {
            int digit = s[i] - '0'; // 문자를 정수형 숫자로 변환

            // [오버플로우 방어 - Clamping]
            // Int32.MaxValue = 2147483647
            if (result > int.MaxValue / 10 || (result == int.MaxValue / 10 && digit > 7)) {
                // 한계 초과 시 부호에 맞춰 Max/Min 리턴
                return sign == 1 ? int.MaxValue : int.MinValue;
            }

            result = result * 10 + digit;
            i++;
        }

        return result * sign;
    }
}

// [실행 예제]
// var sol = new Solution();
// Console.WriteLine(sol.MyAtoi("4193 with words")); // 출력: 4193
```

