# algorithm 

- [01.MAX, MIN value 확인](#reverse-integer). 
- [02.IsDigit(C#)](#isdigit-1). 
    - [02-1. isdigit(C++)](#isdigit). 
- [03. Stack & Queue](./StackAndQueue.md). 
- [04. Priority Queue](./priority_queue.md). 
    - [Problem with leetcode](./23.Merge%20k%20Sorted%20Lists.md). 
- [25. Sorting with 3 points](./25.Reverse%20Nodes%20in%20k-Group.md). 
    


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