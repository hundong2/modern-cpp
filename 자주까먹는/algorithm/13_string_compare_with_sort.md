# 문자열 배열에서 가장 긴 공통 접두사(Longest Common Prefix, LCP)

문자열 배열에서 가장 긴 공통 접두사(Longest Common Prefix, LCP)를 찾는 문제군요. 이 문제는 컴퓨터 과학에서 문자열 처리(String Processing)의 기초를 다지기에 아주 훌륭한 문제입니다.

초보자분들도 이해하기 쉽게, 직관적인 그림과 함께 가장 우아하고 효율적인 '정렬(Sorting)을 활용한 알고리즘'으로 이 문제를 박살 내보겠습니다!

---

### 💡 1. 알고리즘 개념: 왜 '정렬'을 사용할까요?

가장 직관적인 방법은 첫 번째 단어의 첫 글자부터 다른 모든 단어의 글자와 수직으로 비교(Vertical Scanning)하는 것입니다. 하지만 표준 라이브러리의 **정렬(Sort) 함수**를 적극 활용하면 코드가 훨씬 짧아지고 논리가 우아해집니다.

알파벳 사전순(Lexicographical order)으로 문자열을 정렬하면 어떤 일이 발생할까요?
**가장 비슷한 문자열끼리 모이고, 가장 다른 문자열은 배열의 양 끝(처음과 끝)으로 밀려납니다.**

즉, 배열을 정렬한 뒤 '가장 첫 번째 문자열'과 **'가장 마지막 문자열'** 두 개만 비교해서 공통 접두사를 찾으면, 그 사이에 있는 모든 문자열도 당연히 그 접두사를 공유하게 됩니다!

#### 🎨 직관적인 그림 설명

`strs = ["flower", "flow", "flight"]` 가 주어졌다고 가정해 봅시다.

**[Step 1] 배열을 알파벳 사전순으로 정렬합니다.**

```text
정렬 전:
1. f l o w e r
2. f l o w
3. f l i g h t

정렬 후 (사전순):
[0] f l i g h t   <-- First (가장 앞)
[1] f l o w
[2] f l o w e r   <-- Last (가장 뒤)

```

**[Step 2] 처음(First)과 끝(Last)만 비교합니다.**

```text
First : f l i g h t
Last  : f l o w e r
        | | | 
비교  : O O X  (3번째 인덱스 'i'와 'o'에서 다름!)

```

* `f` (같음) ➡️ `l` (같음) ➡️ `i`와 `o` (다름!)
* 따라서 가장 긴 공통 접두사는 "fl"이 됩니다. 가운데 있는 "flow"는 볼 필요도 없습니다.

#### ⏱ 시간 및 공간 복잡도

* **시간 복잡도:** $O(N \log N \times M)$ (여기서 $N$은 배열의 길이, $M$은 문자열의 최대 길이). 정렬에 드는 시간입니다.
* **공간 복잡도:** $O(1)$ 또는 $O(M)$ (정렬 알고리즘 및 언어별 슬라이싱 메커니즘에 따라 다름).

---

### 💻 2. Python 구현 (`.py`)

Python의 내장 `sort()` 함수는 **Timsort**라는 매우 강력하고 안정적인 하이브리드 정렬 알고리즘을 사용합니다. 현실의 데이터에서 엄청나게 빠른 속도를 자랑합니다.

```python
# longest_common_prefix.py

def longestCommonPrefix(strs: list[str]) -> str:
    # 1. 예외 처리: 배열이 비어있다면 빈 문자열 반환
    if not strs:
        return ""
    
    # 2. 파이썬 표준 라이브러리인 Timsort를 이용해 사전순 정렬 (O(N log N * M))
    strs.sort()
    
    # 3. 정렬된 배열의 첫 번째 문자열과 마지막 문자열을 가져옴
    first = strs[0]
    last = strs[-1]
    
    i = 0
    # 4. 두 문자열의 길이를 넘지 않는 선에서, 같은 인덱스의 문자가 동일한지 확인
    while i < len(first) and i < len(last) and first[i] == last[i]:
        i += 1  # 문자가 같으면 인덱스 증가
        
    # 5. 파이썬의 문자열 슬라이싱을 사용하여 0부터 i-1번째 인덱스까지 잘라서 반환
    return first[:i]

# --- 실행 예제 ---
if __name__ == "__main__":
    example1 = ["flower", "flow", "flight"]
    print(f"Example 1 Output: '{longestCommonPrefix(example1)}'")  # 출력: 'fl'
    
    example2 = ["dog", "racecar", "car"]
    print(f"Example 2 Output: '{longestCommonPrefix(example2)}'")  # 출력: ''

```

---

### 💻 3. C++ 구현 (`.cpp`)

C++에서는 `<algorithm>` 헤더의 `std::sort`를 사용합니다. 이는 일반적으로 **IntroSort** (Quick Sort + Heap Sort + Insertion Sort)로 구현되어 있어 최악의 경우에도 $O(N \log N)$을 보장하는 최고 성능의 알고리즘입니다.

```cpp
// longest_common_prefix.cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm> // std::sort 사용을 위한 헤더

using namespace std;

class Solution {
public:
    string longestCommonPrefix(vector<string>& strs) {
        // 1. 배열이 비어있는 경우 빈 문자열 반환
        if(strs.empty()) return "";
        
        // 2. std::sort를 사용하여 사전순(Lexicographical)으로 오름차순 정렬
        sort(strs.begin(), strs.end());
        
        // 3. 정렬 후 가장 첫 문자열과 가장 마지막 문자열 선택
        string first = strs.front(); // strs[0] 과 동일
        string last = strs.back();   // strs[strs.size() - 1] 과 동일
        
        int i = 0;
        // 4. 두 문자열의 문자를 앞에서부터 하나씩 비교
        // 사이즈를 넘어가지 않게 방어막(i < size)을 치고, 문자가 같을 때만 i 증가
        while(i < first.size() && i < last.size() && first[i] == last[i]) {
            i++;
        }
        
        // 5. 첫 번째 문자열에서 0번째 인덱스부터 i개의 문자를 부분 문자열(substr)로 추출
        return first.substr(0, i);
    }
};

// --- 실행 예제 ---
int main() {
    Solution sol;
    vector<string> example1 = {"flower", "flow", "flight"};
    cout << "Example 1 Output: '" << sol.longestCommonPrefix(example1) << "'" << endl; // 출력: 'fl'
    
    vector<string> example2 = {"dog", "racecar", "car"};
    cout << "Example 2 Output: '" << sol.longestCommonPrefix(example2) << "'" << endl; // 출력: ''
    
    return 0;
}

```

---

### 💻 4. C# 구현 (`.csx` - C# Script)

C#에서는 `Array.Sort()` 또는 `List.Sort()`를 사용합니다. C#의 정렬 역시 내부적으로 **IntroSort**를 기반으로 하여 매우 안정적이고 빠릅니다.

```csharp
// longest_common_prefix.csx
using System;

public class Solution {
    public string LongestCommonPrefix(string[] strs) {
        // 1. 입력 배열 null 및 빈 배열 체크 방어 코드
        if (strs == null || strs.Length == 0) return "";
        
        // 2. C# 내장 Array.Sort를 통해 알파벳 사전순으로 제자리(In-place) 정렬
        Array.Sort(strs);
        
        // 3. 첫 문자열과 끝 문자열 가져오기
        string first = strs[0];
        string last = strs[strs.Length - 1]; // 최신 C#에서는 strs[^1] 로도 사용 가능
        
        int i = 0;
        // 4. 인덱스 범위를 초과하지 않으면서 두 문자가 일치할 때까지 루프
        while (i < first.Length && i < last.Length && first[i] == last[i]) {
            i++;
        }
        
        // 5. Substring 함수를 이용해 0번 인덱스부터 i길이 만큼 문자열을 잘라내어 반환
        return first.Substring(0, i);
    }
}

// --- 실행 예제 ---
Solution sol = new Solution();
string[] example1 = new string[] {"flower", "flow", "flight"};
Console.WriteLine($"Example 1 Output: '{sol.LongestCommonPrefix(example1)}'"); // 출력: 'fl'

string[] example2 = new string[] {"dog", "racecar", "car"};
Console.WriteLine($"Example 2 Output: '{sol.LongestCommonPrefix(example2)}'"); // 출력: ''

```

---

1. **정렬의 마법:** 문자열 여러 개의 공통성을 찾을 때, 모두를 비교하기보다는 "정렬 후 양 끝단만 비교"하는 테크닉은 실무나 코딩 테스트에서 코드를 획기적으로 줄여줍니다.
2. **표준 라이브러리 맹신 금지 & 적극 활용:** 언어별 내장 `sort` 함수들의 시간 복잡도는 $O(N \log N)$ 이지만, 문자열 길이가 길 경우 문자열 비교 시간까지 더해져 $O(N \log N \times M)$이 됩니다. 제약 조건(길이 최대 200)을 보았을 때 이 방법은 차고 넘치게 빠릅니다.
3. **방어적 프로그래밍:** `i < first.Length` 와 같이 인덱스가 문자열의 길이를 넘어가지 않도록(Index Out Of Bounds) 루프 조건에 안전장치(Guard)를 꼭 넣는 습관을 들이세요.

네, 명확하게 인지했습니다! 저는 CS 전공 지식을 통달한 최고의 알고리즘 아키텍트, **코드 알고리즘 최고 박사 이동훈**입니다.

앞으로 주어지는 모든 알고리즘 문제나 개념 질문에 대해 요청하신 원칙을 철저히 적용하여 가장 전문적이면서도 알기 쉽게 설명해 드리겠습니다.
