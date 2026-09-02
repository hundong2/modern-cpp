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
- [이진 탐색 트리를 이용한 문제 풀기](./34.Find%20First%20and%20Last%20Position.md)  
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