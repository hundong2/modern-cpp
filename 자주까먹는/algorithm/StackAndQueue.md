# Stack And Queue 

- [20. Problem with ](./%2020.Valid%20Parentheses.md). 
- [21 Merge to List](./21.mergeList.md). 
- [22 Generate Parentheses](./22.GenerateParentheses.md). 
    - List와 stack을 다루는 예쩨 
    - c#의 string builder의 Append와 Length-- 

C#에서 알고리즘을 풀거나 실무를 할 때 가장 자주 사용하는 스택(Stack)과 큐(Queue)의 핵심 메서드들을 한눈에 비교하기 쉽게 표로 정리해 드립니다.

두 자료구조는 데이터를 꺼내는 순서(후입선출 vs 선입선출)만 다를 뿐, 동작을 수행하는 함수의 짝이 아주 비슷하게 구성되어 있습니다.

## 📊 스택과 큐 핵심 메서드 비교표

| 기능 | 스택 (Stack) - LIFO | 큐 (Queue) - FIFO | 동작 설명 |
| --- | --- | --- | --- |
| **데이터 추가** | `Push(item)` | `Enqueue(item)` | 스택은 맨 위에 쌓고(Push), 큐는 맨 뒤에 줄을 세웁니다(Enqueue). |
| **데이터 꺼내기** | `Pop()` | `Dequeue()` | 스택은 맨 위(가장 최근) 데이터를, 큐는 맨 앞(가장 오래된) 데이터를 꺼냅니다. **(데이터 제거됨)** |
| **데이터 확인** | `Peek()` | `Peek()` | 스택은 맨 위, 큐는 맨 앞의 데이터를 꺼내지 않고 슬쩍 확인만 합니다. **(제거 안 됨)** |
| **안전하게 꺼내기** | `TryPop(out item)` | `TryDequeue(out item)` | 스택/큐가 비어있을 때 에러 없이 `false`를 반환하며 데이터 추출을 시도합니다. |
| **안전하게 확인** | `TryPeek(out item)` | `TryPeek(out item)` | 스택/큐가 비어있을 때 에러 없이 `false`를 반환하며 데이터 확인을 시도합니다. |
| **개수 확인** | `Count` | `Count` | 현재 저장된 데이터의 총 개수를 반환합니다. (메서드가 아닌 속성입니다.) |
| **초기화 (비우기)** | `Clear()` | `Clear()` | 저장된 모든 데이터를 한 번에 삭제합니다. |
| **데이터 검색** | `Contains(item)` | `Contains(item)` | 특정 데이터가 내부에 존재하는지 확인하여 `true`/`false`를 반환합니다. |

---

## 💡 알고리즘에서 주로 어떻게 쓰일까요?

어떤 알고리즘을 구현하느냐에 따라 스택과 큐의 선택이 명확하게 갈립니다.

### 1. 스택 (Stack)이 활약하는 알고리즘

* **깊이 우선 탐색 (DFS, Depth-First Search):** 미로를 찾을 때 갈 수 있는 한 끝까지 파고들었다가 막히면 되돌아오는 방식입니다. (재귀 함수와 원리가 같습니다.)
* **문법 / 괄호 검사:** `(`, `{`, `[` 등의 짝이 올바르게 닫혔는지 확인할 때 사용합니다.
* **문자열 뒤집기:** 문자열을 순서대로 넣었다가 빼면 반대 순서로 나옵니다.

### 2. 큐 (Queue)가 활약하는 알고리즘

* **너비 우선 탐색 (BFS, Breadth-First Search):** 시작점에서 가까운 곳부터 넓게 퍼져나가며 탐색합니다. (최단 거리 찾기 등에 유리합니다.)
* **버퍼 (Buffer) 및 대기열 처리:** 프린터 인쇄 대기열이나, 게임의 서버 접속 대기열처럼 "먼저 온 요청을 먼저 처리"해야 할 때 필수적입니다.
* **작업 스케줄링:** 처리해야 할 작업들을 순서대로 예약해두고 하나씩 꺼내서 실행할 때 사용합니다.

C#에서 큐(Queue)는 **FIFO(First-In, First-Out, 선입선출)** 방식을 따릅니다. 가장 먼저 들어온 데이터가 가장 먼저 나가는 구조로, **은행이나 식당에서 줄을 서는 대기열**을 생각하시면 아주 똑같습니다.

식당 대기열(웨이팅)을 관리하는 간단하고 직관적인 예제 코드를 보여드릴게요.

```csharp
using System;
using System.Collections.Generic;

class Program
{
    static void Main()
    {
        // 1. 큐 생성 (대기열 손님 이름)
        Queue<string> waitingLine = new Queue<string>();

        // 2. Enqueue: 데이터 추가 (줄 서기)
        waitingLine.Enqueue("김철수");
        waitingLine.Enqueue("이영희");
        waitingLine.Enqueue("박지민");

        Console.WriteLine($"현재 대기 인원: {waitingLine.Count}명\n"); 
        // 출력: 현재 대기 인원: 3명

        // 3. Peek: 맨 앞 데이터 확인 (누구 차례인지 확인만, 대기열에서 빼지는 않음)
        Console.WriteLine($"다음 입장할 손님: {waitingLine.Peek()}"); 
        // 출력: 다음 입장할 손님: 김철수

        // 4. Dequeue: 데이터 꺼내기 (입장 처리)
        string currentCustomer = waitingLine.Dequeue();
        Console.WriteLine($"[안내] {currentCustomer}님 입장하셨습니다."); 
        // 출력: [안내] 김철수님 입장하셨습니다. (가장 먼저 줄을 선 김철수님이 나옴)

        Console.WriteLine($"남은 대기 인원: {waitingLine.Count}명\n"); 
        // 출력: 남은 대기 인원: 2명

        // 5. foreach를 사용한 순회 (데이터가 제거되지는 않음, 서 있는 순서대로 출력)
        Console.WriteLine("--- 남은 대기자 명단 ---");
        foreach (string name in waitingLine)
        {
            Console.WriteLine(name); 
            // 이영희
            // 박지민
        }

        // 6. TryDequeue: 안전하게 꺼내기 (while문 활용)
        Console.WriteLine("\n--- 남은 손님 모두 입장 처리 ---");
        while (waitingLine.TryDequeue(out string nextCustomer))
        {
            Console.WriteLine($"{nextCustomer}님 입장하셨습니다.");
        }

        // 큐가 모두 비워졌습니다.
        Console.WriteLine($"\n영업 종료! 남은 대기 인원: {waitingLine.Count}명");
    }
}

```

## 💡 코드 실행 흐름 요약

1. **`Enqueue`**: 김철수 -> 이영희 -> 박지민 순서로 줄을 섭니다.
2. **`Peek`**: 맨 앞사람이 누구인지 슬쩍 봅니다. (김철수)
3. **`Dequeue`**: 맨 앞사람을 줄에서 빼내어 입장시킵니다. (김철수 빠짐, 이제 이영희가 1빠)
4. **`TryDequeue`**: 남은 사람들을 차례대로 줄에서 빼내어 모두 입장시킵니다. 

C++와 C#은 알고리즘 문제를 풀거나 실무를 할 때 사용하는 자료구조와 알고리즘 라이브러리의 철학이 조금 다릅니다.

* C++은 STL (Standard Template Library)이라는 강력한 템플릿 기반의 컨테이너와 `<algorithm>` 헤더의 독립적인 함수들을 조합해서 사용합니다.
* C#은 객체지향적인 **`System.Collections.Generic`** 네임스페이스의 클래스들과, 데이터를 강력하게 쿼리(조작)할 수 있는 LINQ (`System.Linq`)를 주로 사용합니다.

가장 많이 쓰이는 핵심 자료구조와 알고리즘 기능들을 1:1로 매칭하여 표로 정리해 드립니다.

---

## 1. 🗄️ 자료구조 (컨테이너) 비교표

가장 흔하게 쓰이는 자료구조들의 매칭입니다. C++은 `#include <헤더>`가 필요하고, C#은 `using System.Collections.Generic;`이 필요합니다.

| 분류 | C++ (STL) | C# (.NET) | 특징 및 알고리즘 활용 |
| --- | --- | --- | --- |
| **동적 배열** | `vector` | `List<T>` | 크기가 자유롭게 늘어나는 배열. 가장 기본적으로 쓰입니다. |
| **연결 리스트** | `list` | `LinkedList<T>` | 이중 연결 리스트. 중간 삽입/삭제가 빈번할 때 씁니다. |
| **스택 (후입선출)** | `stack` | `Stack<T>` | LIFO 구조. DFS 탐색, 괄호 검사 등에 사용됩니다. |
| **큐 (선입선출)** | `queue` | `Queue<T>` | FIFO 구조. BFS 탐색, 대기열 등에 사용됩니다. |
| **덱 (양방향 큐)** | `deque` | `LinkedList<T>`로 대체 | 양끝에서 삽입/삭제가 가능합니다. (C#은 공식 `Deque`가 없어 리스트로 흉내 냅니다.) |
| **우선순위 큐** | `priority_queue` | `PriorityQueue<T, P>` | 힙(Heap) 기반. 항상 최대/최솟값을 빠르게 뽑아낼 때 사용합니다. (C#은 .NET 6부터 지원) |
| **해시 집합** | `unordered_set` | `HashSet<T>` | **중복을 허용하지 않는** 데이터 모음. (해시 기반이라 검색이 $O(1)$로 매우 빠름) |
| **정렬된 집합** | `set` | `SortedSet<T>` | 중복을 허용하지 않으며, **삽입과 동시에 자동 정렬**됩니다. (이진 트리 기반) |
| **해시 딕셔너리** | `unordered_map` | `Dictionary<K, V>` | Key-Value 쌍으로 데이터 저장. 빈도수 체크, 빠른 검색에 필수적입니다. |
| **정렬된 딕셔너리** | `map` | `SortedDictionary<K, V>` | Key-Value 쌍 저장 시, **Key를 기준으로 자동 정렬**됩니다. |

---

## 2. ⚙️ 알고리즘 유틸리티 비교표

C++은 `<algorithm>` 헤더에 있는 함수(예: `sort()`)에 반복자(iterator)를 넘겨서 처리하는 반면, C#은 객체 자체의 메서드(예: `list.Sort()`)를 쓰거나 **LINQ** 확장 메서드를 활용합니다.

| 알고리즘 기능 | C++ (`<algorithm>`) | C# (`System.Linq`, `List<T>`, `Array`) |
| --- | --- | --- |
| **정렬 (오름차순)** | `sort(v.begin(), v.end())` | `list.Sort()` 또는 `list.OrderBy(x => x)` |
| **정렬 (내림차순)** | `sort(v.begin(), v.end(), greater<>())` | `list.Sort((a,b) => b.CompareTo(a))` 또는 `OrderByDescending()` |
| **반전 (뒤집기)** | `reverse(v.begin(), v.end())` | `list.Reverse()` |
| **이진 탐색** | `binary_search(v.begin(), v.end(), 값)` | `list.BinarySearch(값)` |
| **최대/최솟값 찾기** | `max_element()`, `min_element()` | `list.Max()`, `list.Min()` |
| **특정 값 개수 세기** | `count(v.begin(), v.end(), 값)` | `list.Count(x => x == 값)` |
| **조건에 맞는 값 찾기** | `find_if(v.begin(), v.end(), 조건함수)` | `list.Find(조건)`, `list.Where(조건)` |
| **중복 제거** | `unique()` 후 `erase()` | `list.Distinct().ToList()` |
| **순열 생성** | `next_permutation()` | ❌ **(C#은 내장 함수 없음, 직접 구현해야 함)** |

---

## 💡 C++에서 C#으로 넘어올 때 가장 헷갈리는 점

1. **순열(Permutation)과 조합(Combination)의 부재:**
C++ 알고리즘 문제를 풀 때 꿀이었던 `next_permutation`이 C#에는 없습니다. 재귀 함수(DFS)를 통해 직접 구현해서 써야 합니다.
2. **Bound 계열 함수의 부재:**
C++의 `lower_bound`, `upper_bound` (이진 탐색으로 삽입 위치를 찾는 함수) 역시 C#에 정확히 일치하는 내장 함수가 없습니다. C#의 `BinarySearch()`가 반환하는 음수 인덱스를 비트 반전(`~`)시켜서 위치를 찾아내는 꼼수를 쓰거나 직접 구현해야 합니다.
3. **LINQ의 마법 (C#의 강점):**
C#의 LINQ는 코드를 극단적으로 줄여줍니다. 예를 들어 "리스트에서 짝수만 골라서, 내림차순으로 정렬한 뒤, 상위 3개만 배열로 가져와라" 같은 복잡한 연산을 단 한 줄로 끝낼 수 있습니다.
`var result = list.Where(x => x % 2 == 0).OrderByDescending(x => x).Take(3).ToArray();`


자주 쓰이는 핵심 자료구조와 알고리즘(유틸리티) 함수들을 한 번에 학습할 수 있도록, C++과 **C#** 각각의 종합 예제 코드를 작성해 드립니다.

두 코드는 완전히 동일한 논리적 흐름(데이터 추가 ➡️ 정렬 및 중복 제거 ➡️ 스택/큐/해시 사용)을 가집니다. 코드를 비교해 보면서 언어별 스타일 차이를 확인해 보세요.

---

## 1. 🟦 C++ 핵심 라이브러리 (STL) 마스터 예제

C++은 `<algorithm>` 헤더의 전역 함수와 반복자(Iterator)를 조합하여 데이터를 다루는 것이 특징입니다.

```cpp
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

using namespace std;

int main() {
    cout << "--- 1. Vector & Algorithm (정렬, 중복제거, 최대값) ---" << endl;
    vector<int> v = { 5, 2, 8, 2, 1, 5, 9 };
    
    // 오름차순 정렬
    sort(v.begin(), v.end()); 
    
    // 중복 제거 (정렬이 선행되어야 함, erase-unique 관용구 사용)
    v.erase(unique(v.begin(), v.end()), v.end());
    
    // 최대값 찾기 (iterator 반환하므로 *를 붙여 값 추출)
    int maxVal = *max_element(v.begin(), v.end());
    cout << "최대값: " << maxVal << ", 중복 제거 후 크기: " << v.size() << "\n\n";

    cout << "--- 2. Stack (후입선출 LIFO) ---" << endl;
    stack<int> s;
    s.push(10);
    s.push(20);
    cout << "스택 맨 위(Top): " << s.top() << endl; // 20
    s.pop(); // 20 제거

    cout << "\n--- 3. Queue (선입선출 FIFO) ---" << endl;
    queue<string> q;
    q.push("A");
    q.push("B");
    cout << "큐 맨 앞(Front): " << q.front() << endl; // A
    q.pop(); // A 제거

    cout << "\n--- 4. Unordered Set (해시 집합: 중복 허용 안함, O(1) 탐색) ---" << endl;
    unordered_set<int> mySet;
    mySet.insert(100);
    mySet.insert(200);
    // 데이터 존재 유무 확인 (count가 1이면 존재)
    if (mySet.count(100) > 0) {
        cout << "Set에 100이 존재합니다." << endl;
    }

    cout << "\n--- 5. Unordered Map (해시 딕셔너리: Key-Value, O(1) 탐색) ---" << endl;
    unordered_map<string, int> myMap;
    myMap["Apple"] = 10;
    myMap.insert({"Banana", 20});
    // Key 존재 유무 확인 (find 사용)
    if (myMap.find("Apple") != myMap.end()) {
        cout << "Apple의 값: " << myMap["Apple"] << endl;
    }

    return 0;
}

```

---

## 2. 🟪 C# 핵심 라이브러리 (.NET & LINQ) 마스터 예제

C#은 컬렉션 객체 내장 메서드와 데이터를 유연하게 다루는 **LINQ** 확장 메서드를 활용하여 코드를 매우 직관적으로 작성합니다.

```csharp
using System;
using System.Collections.Generic;
using System.Linq; // LINQ 알고리즘 사용을 위해 필수

class Program
{
    static void Main()
    {
        Console.WriteLine("--- 1. List & LINQ (정렬, 중복제거, 최대값) ---");
        List<int> list = new List<int> { 5, 2, 8, 2, 1, 5, 9 };

        // 내부 정렬 (In-place)
        list.Sort();

        // 중복 제거 (LINQ Distinct 사용, 새로운 리스트 반환)
        List<int> uniqueList = list.Distinct().ToList();

        // 최대값 찾기 (LINQ Max 사용)
        int maxVal = uniqueList.Max();
        Console.WriteLine($"최대값: {maxVal}, 중복 제거 후 크기: {uniqueList.Count}\n");

        Console.WriteLine("--- 2. Stack (후입선출 LIFO) ---");
        Stack<int> s = new Stack<int>();
        s.Push(10);
        s.Push(20);
        Console.WriteLine($"스택 맨 위(Peek): {s.Peek()}"); // 20
        s.Pop(); // 20 제거

        Console.WriteLine("\n--- 3. Queue (선입선출 FIFO) ---");
        Queue<string> q = new Queue<string>();
        q.Enqueue("A");
        q.Enqueue("B");
        Console.WriteLine($"큐 맨 앞(Peek): {q.Peek()}"); // A
        q.Dequeue(); // A 제거

        Console.WriteLine("\n--- 4. HashSet (해시 집합: 중복 허용 안함, O(1) 탐색) ---");
        HashSet<int> mySet = new HashSet<int>();
        mySet.Add(100);
        mySet.Add(200);
        // 데이터 존재 유무 확인 (Contains)
        if (mySet.Contains(100))
        {
            Console.WriteLine("Set에 100이 존재합니다.");
        }

        Console.WriteLine("\n--- 5. Dictionary (해시 딕셔너리: Key-Value, O(1) 탐색) ---");
        Dictionary<string, int> myMap = new Dictionary<string, int>();
        myMap["Apple"] = 10;
        myMap.Add("Banana", 20);
        // Key 존재 유무 확인 (ContainsKey)
        if (myMap.ContainsKey("Apple"))
        {
            Console.WriteLine($"Apple의 값: {myMap["Apple"]}");
        }
    }
}

```

---

## 📖 핵심 함수 1:1 매칭 설명 표

위의 예제 코드에서 사용된 주요 함수들을 비교 정리한 표입니다.

| 동작 분류 | 자료구조 | C++ (STL) | C# (.NET & LINQ) | 부가 설명 |
| --- | --- | --- | --- | --- |
| **삽입 (배열)** | Vector / List | `v.push_back(val)` | `list.Add(val)` | 동적 배열의 맨 끝에 요소를 추가합니다. |
| **정렬** | Vector / List | `sort(v.begin(), v.end())` | `list.Sort()` | 오름차순으로 정렬합니다. |
| **중복 제거** | Vector / List | `v.erase(unique(...), v.end())` | `list.Distinct().ToList()` | C++은 중복 원소를 뒤로 밀고 잘라내며, C#은 새 컬렉션을 만듭니다. |
| **최댓값** | Vector / List | `*max_element(v.begin(), v.end())` | `list.Max()` | 컬렉션 내의 가장 큰 값을 반환합니다. |
| **삽입 (스택)** | Stack | `s.push(val)` | `s.Push(val)` | 스택의 맨 위에 요소를 추가합니다. |
| **확인 (스택)** | Stack | `s.top()` | `s.Peek()` | 맨 위의 요소를 제거하지 않고 확인합니다. |
| **추출 (스택)** | Stack | `s.pop()` (반환값 없음) | `s.Pop()` (반환값 있음) | 맨 위 요소를 제거합니다. (C#은 제거된 값을 반환까지 해줌) |
| **삽입 (큐)** | Queue | `q.push(val)` | `q.Enqueue(val)` | 큐의 맨 뒤에 요소를 추가(줄서기)합니다. |
| **확인 (큐)** | Queue | `q.front()` | `q.Peek()` | 큐의 맨 앞 요소를 제거하지 않고 확인합니다. |
| **추출 (큐)** | Queue | `q.pop()` (반환값 없음) | `q.Dequeue()` (반환값 있음) | 맨 앞 요소를 제거합니다. (C#은 제거된 값을 반환해 줌) |
| **삽입 (집합/맵)** | Set / Map | `set.insert(val)` | `set.Add(val)` | 데이터나 Key-Value 쌍을 추가합니다. |
| **검색 (집합/맵)** | Set / Map | `set.count(val) > 0` 또는 `find` | `set.Contains(val)` / `map.ContainsKey(key)` | 해당 데이터 또는 Key가 내부에 존재하는지 확인하여 `bool`로 판단합니다. |


코딩 테스트에서 시간 초과(TLE, Time Limit Exceeded)를 피하려면, 문제에서 "어떤 동작이 가장 빈번하게 일어나는가?"를 파악하고 그 동작의 시간 복잡도를 $O(1)$ 또는 $O(\log N)$으로 줄여주는 자료구조를 선택해야 합니다.

어떤 상황에서 어떤 자료구조를 선택해야 하는지 시각적인 의사결정 트리를 먼저 확인해 보세요.

---

## 🚨 시간 초과를 유발하는 최악의 패턴과 해결책

실제 코딩 테스트에서 가장 자주 실수하는 4가지 패턴입니다.

### 1. "이 값이 안에 있나?" 계속 찾기

* **❌ 최악의 선택:** `List` (또는 `vector`)에 데이터를 다 넣고 `Contains()`나 `Find()`를 반복문 안에서 계속 호출합니다. $\rightarrow$ 시간 복잡도 $O(N^2)$
* **✅ 최적의 선택:** **`HashSet`** 또는 **`Dictionary`** (`unordered_set`, `unordered_map`). 해시(Hash)를 사용하므로 데이터가 100만 개라도 찾는 데 $O(1)$ 밖에 안 걸립니다.

### 2. "변하는 데이터 중에서 가장 큰/작은 값" 뽑기

* **❌ 최악의 선택:** 데이터가 추가될 때마다 `List.Sort()`를 계속 호출합니다. $\rightarrow$ $O(N^2 \log N)$
* **✅ 최적의 선택:** **`PriorityQueue`** (`priority_queue`). 항상 내부에 가장 크거나 작은 값을 루트(Root)로 유지하는 힙(Heap) 구조이므로 데이터 삽입/삭제가 $O(\log N)$에 처리됩니다. 다익스트라(Dijkstra) 알고리즘의 필수품입니다.

### 3. "데이터를 항상 정렬된 상태로 둬야 할 때"

* **❌ 최악의 선택:** `List`에 일단 넣고 필요할 때마다 정렬합니다.
* **✅ 최적의 선택:** **`SortedSet`** (`set`). 이진 탐색 트리(Binary Search Tree) 구조로 되어 있어, 애초에 들어갈 때부터 자기 자리를 찾아 들어갑니다. 삽입/검색 모두 $O(\log N)$입니다.

### 4. "맨 앞에 데이터 넣기/빼기"

* **❌ 최악의 선택:** `List.Insert(0, item)`을 호출합니다. 리스트의 첫 번째 칸에 데이터를 넣으면, 뒤에 있는 모든 데이터를 한 칸씩 뒤로 밀어야 하므로 $O(N)$이 발생합니다.
* **✅ 최적의 선택:** **`Queue`** 또는 **`LinkedList`** (`queue`, `deque`). 맨 앞이나 뒤의 데이터를 조작하는 데 특화되어 있어 $O(1)$로 끝납니다.

---

## 📏 데이터 크기(N)에 따른 알고리즘 마지노선

문제에 주어진 $N$의 크기를 보면, 어떤 시간 복잡도를 가진 알고리즘/자료구조를 써야 할지 역으로 추적할 수 있습니다. (제한 시간 1초 기준, 연산 횟수 1억 번 이하 권장)

| 입력 크기 ($N$) | 허용되는 최대 시간 복잡도 | 권장 자료구조 및 알고리즘 |
| --- | --- | --- |
| **$N \le 500$** | $O(N^3)$ | 아무거나 써도 됨 (3중 for문 가능) |
| **$N \le 10,000$** | $O(N^2)$ | `List`, 이중 for문, 선택/삽입 정렬 가능 |
| **$N \le 100,000$** | **$O(N \log N)$** | **여기서부터가 중요!** `Sort()`, `PriorityQueue`, `Tree`, 이진 탐색 필수 |
| **$N \ge 1,000,000$** | $O(N)$ 또는 $O(1)$ | `HashSet/Dictionary`, 1중 for문, 단순 수학 계산 |