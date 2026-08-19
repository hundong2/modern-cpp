# 13번 문제, **"Roman to Integer (로마 숫자를 정수로 변환하기)"** 

- 이 문제는 문자열 처리와 해시 맵(Hash Map) 자료구조의 기본기를 다지기에 아주 훌륭한 문제입니다.

### 💡 1. 문제의 핵심 원리 파악하기

로마 숫자는 기본적으로 **왼쪽에서 오른쪽으로 가장 큰 수부터 작은 수 순서**로 씁니다. 이때는 그냥 각 문자에 해당하는 값을 **더해주기만 하면** 됩니다.
(예: `LVIII` = 50 + 5 + 1 + 1 + 1 = 58)

하지만, 예외 규칙(뺄셈 규칙)이 존재합니다.
작은 숫자가 큰 숫자보다 왼쪽(앞)에 나오는 경우에는, 큰 숫자에서 작은 숫자를 **빼야** 합니다.
(예: `IV` = 5 - 1 = 4, `XC` = 100 - 10 = 90)

결론적으로 우리가 알고리즘으로 구현해야 할 핵심 로직은 딱 하나입니다.

> **"현재 문자의 값이 바로 다음(오른쪽) 문자의 값보다 작으면 뺀다. 그렇지 않으면 더한다."**

---

### 🎨 2. 직관적인 그림(그래프)으로 이해하기

가장 복잡한 예제 3번인 `MCMXCIV` (1994)를 시각화해 보겠습니다.

```text
문자열:     M     C     M     X     C     I     V
값:       1000   100   1000   10   100    1     5

비교 및 연산 과정:
[1] M(1000) 과 C(100) 비교  -> 1000 > 100  (크거나 같음) => +1000
[2] C(100)  과 M(1000) 비교 -> 100  < 1000 (작음!)       => -100
[3] M(1000) 과 X(10) 비교   -> 1000 > 10   (크거나 같음) => +1000
[4] X(10)   과 C(100) 비교  -> 10   < 100  (작음!)       => -10
[5] C(100)  과 I(1) 비교    -> 100  > 1    (크거나 같음) => +100
[6] I(1)    과 V(5) 비교    -> 1    < 5    (작음!)       => -1
[7] V(5) (마지막 문자)      -> 비교 대상 없음              => +5

총합계 = 1000 - 100 + 1000 - 10 + 100 - 1 + 5 = 1994 🎉

```

이렇게 현재 문자와 **그다음 문자를 묶어서 비교**하는 슬라이딩 윈도우(Sliding Window) 적인 사고를 적용하면 쉽게 풀립니다.

---

### 💻 3. 언어별 예제 코드 및 라인별 상세 분석

해시 테이블을 사용하여 문자를 숫자로 매핑(Mapping)하는 표준 라이브러리를 적극 활용하겠습니다.

#### 🐍 [Python] 딕셔너리(Dictionary)를 활용한 풀이 (`solution.py`)

파이썬의 내장 자료구조인 `dict`는 해시 테이블로 구현되어 있어 O(1)의 시간 복잡도로 값을 조회할 수 있습니다.

```python
def romanToInt(s: str) -> int:
    # 1. 로마 숫자와 정수를 1:1 매핑하는 딕셔너리를 선언합니다.
    roman_map = {'I': 1, 'V': 5, 'X': 10, 'L': 50, 'C': 100, 'D': 500, 'M': 1000}
    
    # 2. 최종 결과를 저장할 누적 변수를 0으로 초기화합니다.
    total = 0
    
    # 3. 문자열의 첫 번째 문자부터 '마지막 문자 직전'까지 순회합니다.
    for i in range(len(s) - 1):
        # 4. 현재 문자의 값이 다음 문자의 값보다 작다면 (예: I < V)
        if roman_map[s[i]] < roman_map[s[i+1]]:
            # 5. 뺄셈 규칙 적용: 누적값에서 현재 값을 뺍니다.
            total -= roman_map[s[i]]
        else:
            # 6. 덧셈 규칙 적용: 누적값에 현재 값을 더합니다.
            total += roman_map[s[i]]
            
    # 7. 마지막 문자는 비교할 대상이 없으므로 무조건 더해줍니다.
    return total + roman_map[s[-1]]

# 실행 예제
if __name__ == "__main__":
    print(romanToInt("III"))      # 결과: 3
    print(romanToInt("LVIII"))    # 결과: 58
    print(romanToInt("MCMXCIV"))  # 결과: 1994

```

#### ⚡ [C++] `std::unordered_map`을 활용한 풀이 (`solution.cpp`)

C++에서는 STL(Standard Template Library)의 `<unordered_map>`을 사용합니다. 일반 `map`(레드-블랙 트리)과 달리 해시 테이블 기반이므로 O(1) 탐색이 가능합니다.

```cpp
#include <iostream>
#include <string>
#include <unordered_map> // 해시 맵 표준 라이브러리 포함

using namespace std;

int romanToInt(string s) {
    // 1. C++ STL인 unordered_map을 이용해 로마 문자(char)를 정수(int)로 매핑합니다.
    unordered_map<char, int> roman = {
        {'I', 1}, {'V', 5}, {'X', 10}, {'L', 50},
        {'C', 100}, {'D', 500}, {'M', 1000}
    };
    
    // 2. 결과를 누적할 변수 total을 0으로 초기화합니다.
    int total = 0;
    
    // 3. 문자열의 길이만큼 순회하는 for 문을 시작합니다.
    for (int i = 0; i < s.length(); ++i) {
        // 4. (i + 1 < s.length()): 다음 문자가 존재하는지 바운더리 체크를 먼저 합니다. (단락 평가)
        // 5. 현재 문자(roman[s[i]])가 다음 문자(roman[s[i+1]])보다 작은지 비교합니다.
        if (i + 1 < s.length() && roman[s[i]] < roman[s[i+1]]) {
            // 6. 예외(뺄셈) 규칙에 해당하므로 전체 합에서 현재 값을 뺍니다.
            total -= roman[s[i]];
        } else {
            // 7. 일반 규칙에 해당하므로 전체 합에 현재 값을 더합니다.
            total += roman[s[i]];
        }
    }
    
    // 8. 최종 계산된 total 값을 반환합니다.
    return total;
}

// 실행 예제
int main() {
    cout << romanToInt("III") << endl;      // 결과: 3
    cout << romanToInt("LVIII") << endl;    // 결과: 58
    cout << romanToInt("MCMXCIV") << endl;  // 결과: 1994
    return 0;
}

```

#### 💠 [C#] `Dictionary<TKey, TValue>`를 활용한 스크립트 풀이 (`solution.csx`)

C#에서는 `System.Collections.Generic` 네임스페이스의 `Dictionary`를 사용합니다. C#의 안전한 형식(Type-safe) 해시 테이블 구조입니다.

```csharp
using System;
using System.Collections.Generic; // Dictionary를 사용하기 위한 네임스페이스

public int RomanToInt(string s) {
    // 1. 문자(char)를 키로, 숫자(int)를 값으로 갖는 Dictionary 생성 및 초기화
    Dictionary<char, int> roman = new Dictionary<char, int>() {
        {'I', 1}, {'V', 5}, {'X', 10}, {'L', 50},
        {'C', 100}, {'D', 500}, {'M', 1000}
    };
    
    // 2. 결과값을 누적할 변수 선언
    int total = 0;
    
    // 3. 문자열 s의 길이만큼 순회
    for (int i = 0; i < s.Length; i++) {
        // 4. IndexOutOfRangeException을 방지하기 위해 다음 인덱스(i+1)가 배열 길이 이내인지 검사
        // 5. 조건이 참이면, 현재 문자의 값이 다음 문자의 값보다 작은지 검사
        if (i + 1 < s.Length && roman[s[i]] < roman[s[i+1]]) {
            // 6. 뺄셈 규칙: 작은 숫자가 앞서 나왔으므로 누적값에서 차감
            total -= roman[s[i]];
        } else {
            // 7. 덧셈 규칙: 누적값에 더함
            total += roman[s[i]];
        }
    }
    
    // 8. 계산이 완료된 총합 반환
    return total;
}

// C# 스크립트(.csx) 실행을 위한 콘솔 출력
Console.WriteLine(RomanToInt("III"));      // 결과: 3
Console.WriteLine(RomanToInt("LVIII"));    // 결과: 58
Console.WriteLine(RomanToInt("MCMXCIV"));  // 결과: 1994

```

---

* **시간 복잡도 (Time Complexity):** **$O(N)$**
입력된 문자열 `s`의 길이 $N$만큼 단 한 번의 루프를 순회합니다. 해시 맵에서의 값 검색은 $O(1)$의 시간이 소요되므로 전체 연산은 문자열 길이에 정비례합니다. (단, 문제 조건에서 $s$의 길이는 최대 15로 주어졌기 때문에 $O(1)$로 보아도 무방할 정도로 매우 빠릅니다.)
* **공간 복잡도 (Space Complexity):** **$O(1)$**
로마 문자 매핑을 위해 생성한 해시 맵은 단 7개의 키-값 쌍(상수 크기)만을 가집니다. 입력 문자열의 길이가 늘어나도 사용되는 추가 메모리는 변하지 않습니다.

이 문제는 알고리즘의 최적화 기법보다는 **예외 케이스(Edge case)를 얼마나 깔끔한 조건문으로 처리하는지** 묻는 소프트웨어 엔지니어링의 기본을 확인하는 문제입니다.

# 최적화 솔루션 

```cpp
#include <iostream>
#include <string>
using namespace std;

// 1. 컴파일러 최적화를 돕는 inline 함수와 switch-case로 O(1) 초고속 매핑
inline int getVal(char c) {
    switch(c) {
        case 'I': return 1;   case 'V': return 5;   case 'X': return 10;
        case 'L': return 50;  case 'C': return 100; case 'D': return 500;
        case 'M': return 1000; default: return 0;
    }
}

int romanToInt(string s) {
    int total = 0, prev = 0;
    // 2. rbegin()과 rend()를 사용하여 뒤에서 앞으로 순회
    for (auto it = s.rbegin(); it != s.rend(); ++it) {
        int curr = getVal(*it);
        // 3. 로직을 단순화하여 분기 비용을 줄임
        if (curr < prev) total -= curr;
        else total += curr;
        prev = curr;
    }
    return total;
}
```

## csharp Example 

```csharp
public int RomanToInt(string s) {
    int total = 0, prev = 0;
    // 1. Length - 1 부터 0까지 역순 반복 (인덱스 초과 에러 방지)
    for (int i = s.Length - 1; i >= 0; i--) {
        // 2. C# 최신 문법인 switch 식을 활용하여 Dictionary 할당 오버헤드 제거
        int curr = s[i] switch {
            'I' => 1, 'V' => 5, 'X' => 10, 'L' => 50,
            'C' => 100, 'D' => 500, 'M' => 1000, _ => 0
        };
        // 3. 삼항 연산자를 이용해 조건부 덧셈/뺄셈을 한 줄로 처리
        total += (curr < prev) ? -curr : curr;
        prev = curr;
    }
    return total;
}
```