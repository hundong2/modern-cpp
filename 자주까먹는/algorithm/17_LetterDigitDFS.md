 **알고리즘 패러다임을 재귀(DFS)에서 반복(Iterative/BFS)으로 전환**하거나, **각 언어의 고도로 최적화된 표준 라이브러리를 활용**하면 함수 호출 오버헤드(Call Stack)를 없애고 메모리 할당을 최소화하여 실제 실행 속도(Constant Factor)를 극적으로 끌어올릴 수 있습니다.

---

## 1. 최적화 알고리즘 시각화: 큐(Queue)를 활용한 BFS 기법

재귀 함수 대신 **큐(Queue) 자료구조** 또는 **임시 배열 교체(Swap)** 방식을 사용하면, 함수가 깊어지면서 발생하는 스택 오버헤드를 완전히 제거할 수 있습니다. 이는 데이터를 가로로 넓게 탐색하는 너비 우선 탐색(BFS)의 형태를 띱니다.

```text
[초기 상태] 결과 리스트 = [""]  (빈 문자열 1개)

[숫자 '2' 입력 시] ('a', 'b', 'c')
기존 [""]의 원소들에 'a', 'b', 'c'를 각각 붙입니다.
-> 결과 리스트 갱신 = ["a", "b", "c"]

[숫자 '3' 입력 시] ('d', 'e', 'f')
기존 ["a", "b", "c"]의 각 원소에 'd', 'e', 'f'를 붙입니다.
- 'a' -> "ad", "ae", "af"
- 'b' -> "bd", "be", "bf"
- 'c' -> "cd", "ce", "cf"
-> 결과 리스트 갱신 = ["ad", "ae", "af", "bd", "be", "bf", "cd", "ce", "cf"]

```

이러한 **누적(Accumulation) 및 교체** 방식은 최적화의 핵심입니다. 각 언어별로 가장 최적화된 방식을 살펴보겠습니다.

---

## 2. Python 최적화: C언어로 구현된 `itertools` 활용 (`.py`)

Python에서는 데카르트 곱(Cartesian Product)을 구하는 표준 라이브러리인 `itertools.product`를 사용하는 것이 가장 빠릅니다. 이 라이브러리는 내부적으로 C언어로 최적화되어 있어 Python의 `for`문이나 재귀보다 압도적인 속도를 냅니다.

```python
# letter_combinations_optimized.py
import itertools

def letterCombinations(digits: str) -> list[str]:
    # 1. 예외 처리: 입력값이 비어있으면 조기 종료
    if not digits:
        return []

    # 2. 매핑 딕셔너리 정의
    phone_map = {
        '2': "abc", '3': "def", '4': "ghi", '5': "jkl",
        '6': "mno", '7': "pqrs", '8': "tuv", '9': "wxyz"
    }
    
    # 3. 입력된 숫자에 해당하는 문자열들의 리스트를 생성 (예: ['abc', 'def'])
    pools = [phone_map[d] for d in digits]
    
    # 4. itertools.product(*pools)를 사용하여 모든 조합을 한 번에 생성
    # *pools는 언패킹(Unpacking) 연산자로, 리스트의 원소들을 개별 인자로 전달합니다.
    # product는 튜플을 반환하므로 (예: ('a', 'd')), "".join()으로 문자열로 합쳐 반환합니다.
    return ["".join(combo) for combo in itertools.product(*pools)]

if __name__ == "__main__":
    print(letterCombinations("23")) 

```

---

## 3. C++ 최적화: 반복문과 `std::move` 메모리 최적화 (`.cpp`)

C++에서는 재귀 대신 임시 `vector`를 만들고 덧붙인 다음, `std::move`를 사용해 메모리 복사 비용(Copy Cost)을 없애는 반복(Iterative) 방식이 가장 효율적입니다.

```cpp
// letter_combinations_optimized.cpp
#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Solution {
public:
    vector<string> letterCombinations(string digits) {
        // 1. 예외 처리
        if (digits.empty()) return {};

        // 2. 인덱스 접근이 가장 빠른 배열(Array)을 사용하여 매핑 정의
        // 해시맵(unordered_map)보다 배열 인덱스 접근이 훨씬 빠릅니다.
        const vector<string> phone_map = {
            "", "", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"
        };

        // 3. BFS의 초기 상태 설정: 빈 문자열을 가진 배열
        vector<string> result = {""};

        // 4. 입력된 각 숫자에 대해 반복 (예: '2', '3')
        for (char digit : digits) {
            vector<string> temp; // 다음 단계의 조합을 저장할 임시 배열
            const string& letters = phone_map[digit - '0']; // '2' -> 정수 2로 변환하여 접근
            
            // 기존 결과물(result)의 모든 요소에 새로운 문자를 덧붙임
            for (const string& s : result) {
                for (char c : letters) {
                    temp.push_back(s + c); // 문자열 결합
                }
            }
            // 5. 메모리 최적화: 복사하지 않고 소유권을 넘김 (std::move)
            // C++11 이후의 이동 의미론(Move Semantics)으로 오버헤드를 극적으로 줄입니다.
            result = move(temp); 
        }

        return result;
    }
};

int main() {
    Solution sol;
    vector<string> res = sol.letterCombinations("23");
    for (const string& s : res) cout << s << " ";
    return 0;
}

```

---

## 4. C# 최적화: 함수형 프로그래밍 LINQ 활용 (`.csx`)

C#에서는 강력한 컬렉션 제어 언어인 LINQ(Language Integrated Query)의 `Aggregate`와 `SelectMany`를 사용하여 C++의 반복문 기법을 단 한 줄의 선언적 코드로 완벽하게 최적화할 수 있습니다.

```csharp
// letter_combinations_optimized.csx
using System;
using System.Collections.Generic;
using System.Linq;

public class Solution {
    public IList<string> LetterCombinations(string digits) {
        // 1. 예외 처리
        if (string.IsNullOrEmpty(digits)) return new List<string>();

        // 2. 인덱스 매핑용 문자열 배열 정의
        string[] phoneMap = {"", "", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};

        // 3. LINQ를 활용한 선언적 최적화 탐색
        return digits
            // '2', '3' 문자를 "abc", "def" 문자열로 변환 (Select)
            .Select(d => phoneMap[d - '0']) 
            
            // 누적기(Aggregate)를 사용하여 BFS 방식으로 조합 생성
            // 초기 누적값: 빈 문자열의 배열 (new[] { "" })
            .Aggregate(
                (IEnumerable<string>)new[] { "" }, 
                // acc(기존 누적 배열), letters(현재 붙일 문자열 "def")
                // SelectMany를 통해 데카르트 곱을 수행하여 새로운 배열 생성
                (acc, letters) => acc.SelectMany(
                    a => letters,       // 기존 문자열 각각에 대해 letters 순회
                    (a, l) => a + l     // 기존 문자열 'a'에 새 문자 'l'을 결합
                )
            )
            .ToList(); // 최종 결과를 List로 변환
    }
}

// --- 실행 예제 ---
var sol = new Solution();
var result = sol.LetterCombinations("23");
Console.WriteLine(string.Join(", ", result)); // 출력: ad, ae, af, bd, be, bf, cd, ce, cf

```

---

## My Solution

```cpp
class Solution {
public:
    vector<string> letterCombinations(string digits) {
        std::vector<std::string> result = {""};
        for(const auto& element: digits){
            std::vector<std::string> temp;
            const auto& value = getNumbers(element);
            for(const std::string& s: result){
                for(const auto& c: value){
                    temp.push_back(s + c);
                }
                
            }
            result = std::move(temp);
        }
        return result;
    }
private:
    [[nodiscard]] constexpr std::string_view getNumbers(char value) {
        switch(value) {
            case '2': return "abc"; case '3': return "def"; case '4': return "ghi";
            case '5': return "jkl"; case '6': return "mno"; case '7': return "pqrs";
            case '8': return "tuv"; case '9': return "wxyz";
        };
        return {};
    }
};
```

### C#

```csharp
public class Solution {
    private readonly string[] mpp = { "", "", "abc", "def", "ghi", "jkl", "mno", "pqrs", "tuv", "wxyz"};
    public IList<string> LetterCombinations(string digits) {
        if(string.IsNullOrEmpty(digits)) return default;
        var ans = new List<string>();
        var curr = new StringBuilder();
        Fun(0, digits, curr, ans);
        return ans;
    }
    private void Fun(int i, string digits, StringBuilder curr, IList<string> ans){
        if(i == digits.Length) {
            ans.Add(curr.ToString());
            return;
        }
        string temp = mpp[digits[i] - '0'];
        foreach(var c in temp){
            curr.Append(c);
            Fun(i + 1, digits, curr, ans);
            curr.Length--;
        }
    }
}
```