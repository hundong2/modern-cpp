- [CPP My solution code](#c-my-solution-code). 

# old code 

```csharp
// integer_to_roman.csx
using System;
using System.Text;
using System.Collections.Generic;
using System.Linq; // OrderDescending을 사용하기 위한 필수 네임스페이스

public class Solution {
    public string IntToRoman(int num) {
        var resultValue = new StringBuilder();
        Dictionary<int, string> roman_ = new()
        {
            {1, "I"}, {4, "IV"}, {5, "V"}, {9, "IX"}, {10, "X"},
            {40, "XL"}, {50, "L"}, {90, "XC"}, {100, "C"},
            {400, "CD"}, {500, "D"}, {900, "CM"}, {1000, "M"}
        };
        
        // OrderDescending()은 .NET 6부터 지원되는 LINQ 메서드입니다.
        // 이전 버전에서는 OrderByDescending(x => x) 를 사용해야 합니다.
        foreach( var element in roman_.Keys.OrderDescending())
        {
            if( num / element != 0 )
            {
                for(int i = 0; i < num / element; i++ )
                    resultValue.Append(roman_[element]);
                num %= element; 
            }
        }
        return resultValue.ToString();
    } 
}

// === 실행 및 테스트 코드 ===
var sol = new Solution();
Console.WriteLine(sol.IntToRoman(3749)); // 출력: MMMDCCXLIX
Console.WriteLine(sol.IntToRoman(58));   // 출력: LVIII
Console.WriteLine(sol.IntToRoman(1994)); // 출력: MCMXCIV
```

## 1. 병목 원인 분석 및 최적화 직관적 이해 (Intuition)

이전 코드의 속도를 저하시킨 핵심 원인은 "함수 호출 시마다 발생하는 동적 할당과 정렬"입니다.

### 🐢 기존 방식 (Dictionary + LINQ)의 오버헤드

```text
[ IntToRoman 호출될 때마다 반복되는 무거운 작업 ]
1. Dictionary 객체 생성 (메모리 Heap 할당)
2. 13개의 키-값 쌍 해시 계산 및 삽입
3. roman_.Keys.OrderDescending() 실행 -> 배열 복사 및 동적 정렬 (LINQ 오버헤드)
4. foreach 순회 중 키를 이용해 다시 Dictionary 해시 룩업 ( roman_[element] )

```

### 🐇 최적화 방식 (미리 정렬된 배열 사용)

로마 숫자 기호는 13개로 **고정**되어 있으며 절대 변하지 않습니다. 따라서 무거운 Dictionary나 정렬(LINQ)을 쓸 필요 없이, 미리 내림차순으로 정렬된 고정 배열(Array)을 사용하는 것이 압도적으로 빠릅니다.

```text
[ 미리 정렬된 2개의 병렬 배열 ]
Values  : [1000, 900,  500, 400,  100, ...]
Symbols : ["M",  "CM", "D", "CD", "C", ...]

작업:
1. 값 배열을 0번 인덱스부터 순서대로 읽기만 함 (정렬 X, 해시 룩업 X)
2. 메모리 캐시 적중률(Cache Hit Ratio) 극대화 -> CPU가 가장 좋아하는 연속된 메모리 구조

```

---

## 2. 극강의 속도로 최적화된 C# 코드 정밀 분석

가장 빠른 속도를 내기 위해 `Dictionary`와 `LINQ`를 모두 제거하고, 병렬 배열(Parallel Arrays)과 단순 `while` 루프를 적용했습니다.

### 🔷 C# Script (`.csx`)

```csharp
// integer_to_roman_optimized.csx
using System;
using System.Text;

public class Solution {
    // 1. static readonly 배열 선언 (클래스 로드 시 1번만 메모리에 할당됨)
    // 함수가 수천 번 호출되어도 다시 생성하거나 정렬할 필요가 없습니다.
    private static readonly int[] values = { 
        1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1 
    };
    private static readonly string[] symbols = { 
        "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" 
    };

    public string IntToRoman(int num) {
        var resultValue = new StringBuilder();

        // 2. 고정된 배열을 인덱스(i)를 이용해 순차 탐색합니다.
        for (int i = 0; i < values.Length; i++) {
            // 3. 나누기(/)와 나머지(%) 연산 대신 단순 빼기(-)를 사용합니다.
            // 로마 숫자 기호는 최대 3번까지만 반복되므로, 
            // 무거운 나눗셈 연산보다 while문으로 빼는 것이 CPU 연산 단계를 줄여 더 빠릅니다.
            while (num >= values[i]) {
                num -= values[i];                 // 값 차감
                resultValue.Append(symbols[i]);   // 문자열 추가 (배열 직접 참조로 해시 룩업 비용 0)
            }
            
            // 4. num이 0이 되면 더 이상 탐색할 필요가 없으므로 즉시 종료하여 속도를 높입니다.
            if (num == 0) break;
        }

        return resultValue.ToString();
    }
}

// === 실행 및 테스트 코드 ===
var sol = new Solution();
Console.WriteLine(sol.IntToRoman(3749)); // 출력: MMMDCCXLIX
Console.WriteLine(sol.IntToRoman(1994)); // 출력: MCMXCIV

```

---

## 3. 타 언어로의 최적화 코드 적용 예시

동일한 최적화 기법(고정 배열 + 단순 순회)을 C++와 Python에도 적용하여 최고의 성능을 끌어낸 코드입니다.

### 🔶 C++ (`.cpp`)

C++에서는 `std::pair`의 고정 크기 배열인 `std::array`를 사용하면 컴파일 타임에 모든 것이 결정되어 런타임 오버헤드가 제로(0)에 수렴합니다.

```cpp
// integer_to_roman_optimized.cpp
#include <iostream>
#include <string>
#include <array>

using namespace std;

class Solution {
public:
    string intToRoman(int num) {
        // 1. constexpr과 std::array를 사용하여 컴파일 타임에 상수 배열로 고정합니다.
        // 메모리 할당 시간조차 소모되지 않습니다.
        constexpr std::array<pair<int, const char*>, 13> roman = {{
            {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"},
            {100, "C"}, {90, "XC"}, {50, "L"}, {40, "XL"},
            {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}
        }};
        
        string result = "";
        
        // 2. 배열을 순차적으로 순회합니다.
        for (const auto& [val, sym] : roman) {
            while (num >= val) {
                result += sym;
                num -= val;
            }
            if (num == 0) break;
        }
        
        return result;
    }
};

// === 실행 및 테스트 코드 ===
int main() {
    Solution sol;
    cout << sol.intToRoman(3749) << "\n"; // 출력: MMMDCCXLIX
    return 0;
}

```

### 🐍 Python 3 (`.py`)

Python 역시 매번 함수 안에서 딕셔너리를 만드는 대신, 튜플의 리스트를 함수 외부에 선언하여 캐싱하는 방식이 가장 빠릅니다.

```python
# integer_to_roman_optimized.py

class Solution:
    # 1. 클래스 변수로 선언하여 인스턴스 생성 시마다 할당되는 비용을 없앱니다.
    ROMAN_SYMBOLS = (
        (1000, "M"), (900, "CM"), (500, "D"), (400, "CD"),
        (100, "C"), (90, "XC"), (50, "L"), (40, "XL"),
        (10, "X"), (9, "IX"), (5, "V"), (4, "IV"), (1, "I")
    )

    def intToRoman(self, num: int) -> str:
        res = []
        
        # 2. 순서가 보장된 튜플을 순회하며 내장 함수 divmod를 사용합니다.
        for val, sym in self.ROMAN_SYMBOLS:
            if num == 0:
                break
            
            # Python은 while문보다 내장 C로 구현된 divmod 연산이 더 빠를 때가 많습니다.
            count, num = divmod(num, val)
            if count:
                res.append(sym * count)
                
        return "".join(res)

# === 실행 및 테스트 코드 ===
if __name__ == "__main__":
    sol = Solution()
    print(sol.intToRoman(3749)) # 출력: MMMDCCXLIX

```

## C++ my solution code 

```cpp
class Solution {
public:
    string intToRoman(int num) {
        std::vector<std::string> symbol { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" };
        std::vector<int> values { 1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1 };
        int number{std::move(num)};
        std::string result_value{};
        for(int i{0}; i < values.size(); i++){
            while(number >= values[i]){
                number -= values[i];
                result_value += symbol[i];
            }
        }

        return result_value;
    }
};
```